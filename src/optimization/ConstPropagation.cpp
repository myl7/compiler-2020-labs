#include "ConstPropagation.hpp"
#include "logging.hpp"

// 给出了返回整形值的常数折叠实现，大家可以参考，在此基础上拓展
// 当然如果同学们有更好的方式，不强求使用下面这种方式
// Updated by myl to use template
template <typename T>
T *ConstFolder::compute(Instruction::OpID op, T *value1, T *value2)
{

    int c_value1 = value1->get_value();
    int c_value2 = value2->get_value();
    switch (op)
    {
    case Instruction::add:
        return T::get(c_value1 + c_value2, module_);

        break;
    case Instruction::sub:
        return T::get(c_value1 - c_value2, module_);
        break;
    case Instruction::mul:
        return T::get(c_value1 * c_value2, module_);
        break;
    case Instruction::sdiv:
        return T::get((int)(c_value1 / c_value2), module_);
        break;
    default:
        return nullptr;
        break;
    }
}

// 用来判断value是否为ConstantFP，如果不是则会返回nullptr
ConstantFP *cast_constantfp(Value *value)
{
    auto constant_fp_ptr = dynamic_cast<ConstantFP *>(value);
    if (constant_fp_ptr)
    {
        return constant_fp_ptr;
    }
    else
    {
        return nullptr;
    }
}
ConstantInt *cast_constantint(Value *value)
{
    auto constant_int_ptr = dynamic_cast<ConstantInt *>(value);
    if (constant_int_ptr)
    {
        return constant_int_ptr;
    }
    else
    {
        return nullptr;
    }
}

/**
 * Pass through a BB, perform const propagation, and update const variable set, and continue to next bb
 * TODO: remove const br
 */
void ConstPropagation::pass_bb(BasicBlock *bb, ConstMap const_map)
{
    bb_passed_set.insert(bb->get_name());

    ConstMap::iterator k;

    for (auto ins : bb->get_instructions())
    {
        auto res_name = ins->get_operand(0)->get_name();

        if (ins->is_fp2si())
        {
            auto a = ins->get_operand(1);
            auto a_const = cast_constantfp(a);
            if (a_const)
            {
                auto res_v = int(a_const->get_value());
                auto res = ConstantInt::get(res_v, m_);
                const_map.insert({res_name, res});
                bb->delete_instr(ins);
            }
            else if ((k = const_map.find(a->get_name())) != const_map.end())
            {
                auto a_const = dynamic_cast<ConstantFP *>(k->second);
                auto res_v = int(a_const->get_value());
                auto res = ConstantInt::get(res_v, m_);
                const_map.insert({res_name, res});
                bb->delete_instr(ins);
            }
        }
        else if (ins->is_si2fp())
        {
        }
        // TODO:
    }

    std::map<std::string, ConstMap>::iterator i;

    for (auto succ : bb->get_succ_basic_blocks())
    {
        if (bb_passed_set.find(bb->get_name()) != bb_passed_set.end())
        {
            pass_bb(succ, const_map);
        }
    }
}

void ConstPropagation::run()
{
    auto func_list = m_->get_functions();
    for (auto func : func_list)
    {
        if (func->get_basic_blocks().size() == 0)
        {
            continue;
        }

        auto bb = func->get_entry_block();
        pass_bb(bb, ConstMap{});
    }
}
