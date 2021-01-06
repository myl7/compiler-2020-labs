#include "ConstPropagation.hpp"
#include "logging.hpp"
#include <cassert>

// 给出了返回整形值的常数折叠实现，大家可以参考，在此基础上拓展
// 当然如果同学们有更好的方式，不强求使用下面这种方式
// Updated by myl
Constant *ConstFolder::compute2(Instruction::OpID op, Constant *a, Constant *b)
{
    auto ai = dynamic_cast<ConstantInt *>(a);
    auto bi = dynamic_cast<ConstantInt *>(b);
    auto af = dynamic_cast<ConstantFP *>(a);
    auto bf = dynamic_cast<ConstantFP *>(b);

    switch (op)
    {
    case Instruction::add:
        return ConstantInt::get(ai->get_value() + bi->get_value(), module_);
    case Instruction::sub:
        return ConstantInt::get(ai->get_value() - bi->get_value(), module_);
    case Instruction::mul:
        return ConstantInt::get(ai->get_value() * bi->get_value(), module_);
    case Instruction::sdiv:
        return ConstantInt::get((int)(ai->get_value() / bi->get_value()), module_);
    case Instruction::fadd:
        return ConstantFP::get(af->get_value() + bf->get_value(), module_);
    case Instruction::fsub:
        return ConstantFP::get(af->get_value() - bf->get_value(), module_);
    case Instruction::fmul:
        return ConstantFP::get(af->get_value() * bf->get_value(), module_);
    case Instruction::fdiv:
        return ConstantFP::get(af->get_value() / bf->get_value(), module_);
    default:
        return nullptr;
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

Constant *ConstFolder::compute(Instruction *ins, std::vector<Constant *> is_const_args)
{
    if (ins->is_add() || ins->is_sub() || ins->is_mul() || ins->is_div() || ins->is_fadd() || ins->is_fsub() || ins->is_fmul() || ins->is_fdiv())
    {
        return compute2(ins->get_instr_type(), is_const_args[0], is_const_args[1]);
    }
    return nullptr;
}

/**
 * Pass through a BB, perform const propagation, and update const variable set, and continue to next bb
 * TODO: remove const br
 */
void ConstPropagation::pass_bb(BasicBlock *bb, ConstMap const_map)
{
    LOG(DEBUG) << "Pass BB: " << bb->get_name();
    bb_passed_set.insert(bb->get_name());

    ConstMap::iterator k;
    std::vector<Instruction *> ins2del;

    for (auto ins : bb->get_instructions())
    {
        if (ins->get_num_operand() <= 0)
        {
            continue;
        }

        std::vector<Constant *> is_const_args(ins->get_num_operand());

        for (auto i = 0; i < ins->get_num_operand(); i++)
        {
            LOG(DEBUG) << "check ops";

            auto op = ins->get_operand(i);
            auto op_iconst = cast_constantint(op);
            auto op_fconst = cast_constantfp(op);

            if (op_iconst)
            {
                is_const_args[i] = op_iconst;
            }
            else if (op_fconst)
            {
                is_const_args[i] = op_fconst;
            }
            else
            {
                is_const_args[i] = nullptr;
            }

            LOG(DEBUG) << "check const map";

            if ((k = const_map.find(op->get_name())) != const_map.end())
            {
                LOG(DEBUG) << "found in const map";
                ins->set_operand(i, k->second);
                is_const_args[i] = k->second;
            }
        }

        LOG(DEBUG) << "check ops ok";

        auto is_res_const = true;
        for (auto arg : is_const_args)
        {
            if (arg == nullptr)
            {
                is_res_const = false;
            }
        }

        auto log = ins->get_name() + " gets ";
        for (auto arg : is_const_args)
        {
            log += (arg == nullptr ? "var " : "const ");
        }
        LOG(DEBUG) << log;

        if (is_res_const)
        {
            LOG(DEBUG) << ins->get_name() << " is const";
            auto res_new = ConstFolder(m_).compute(ins, is_const_args);
            if (res_new != nullptr)
            {
                ins->remove_use_of_ops();
                ins->replace_all_use_with(res_new);
                ins2del.push_back(ins);
                const_map.insert({ins->get_name(), res_new});
            }
        }
    }

    for (auto ins : ins2del)
    {
        bb->delete_instr(ins);
    }

    LOG(DEBUG) << "Succ BB num: " << bb->get_succ_basic_blocks().size();

    for (auto succ : bb->get_succ_basic_blocks())
    {
        if (bb_passed_set.find(succ->get_name()) == bb_passed_set.end())
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
