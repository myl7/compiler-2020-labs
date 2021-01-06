#include "ConstPropagation.hpp"
#include "logging.hpp"

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

Constant *ConstFolder::compute2cmp(CmpInst::CmpOp op, Constant *a, Constant *b)
{
    auto ai = dynamic_cast<ConstantInt *>(a);
    auto bi = dynamic_cast<ConstantInt *>(b);

    switch (op)
    {
    case CmpInst::CmpOp::EQ:
        return ConstantInt::get(ai->get_value() == bi->get_value(), module_);
    case CmpInst::CmpOp::NE:
        return ConstantInt::get(ai->get_value() != bi->get_value(), module_);
    case CmpInst::CmpOp::LE:
        return ConstantInt::get(ai->get_value() <= bi->get_value(), module_);
    case CmpInst::CmpOp::LT:
        return ConstantInt::get(ai->get_value() < bi->get_value(), module_);
    case CmpInst::CmpOp::GE:
        return ConstantInt::get(ai->get_value() >= bi->get_value(), module_);
    case CmpInst::CmpOp::GT:
        return ConstantInt::get(ai->get_value() > bi->get_value(), module_);
    default:
        return nullptr;
    }
}

Constant *ConstFolder::compute2fcmp(FCmpInst::CmpOp op, Constant *a, Constant *b)
{
    auto ai = dynamic_cast<ConstantFP *>(a);
    auto bi = dynamic_cast<ConstantFP *>(b);

    switch (op)
    {
    case FCmpInst::CmpOp::EQ:
        return ConstantInt::get(ai->get_value() == bi->get_value(), module_);
    case FCmpInst::CmpOp::NE:
        return ConstantInt::get(ai->get_value() != bi->get_value(), module_);
    case FCmpInst::CmpOp::LE:
        return ConstantInt::get(ai->get_value() <= bi->get_value(), module_);
    case FCmpInst::CmpOp::LT:
        return ConstantInt::get(ai->get_value() < bi->get_value(), module_);
    case FCmpInst::CmpOp::GE:
        return ConstantInt::get(ai->get_value() >= bi->get_value(), module_);
    case FCmpInst::CmpOp::GT:
        return ConstantInt::get(ai->get_value() > bi->get_value(), module_);
    default:
        return nullptr;
    }
}

Constant *ConstFolder::compute1(Instruction::OpID op, Constant *a)
{
    auto ai = dynamic_cast<ConstantInt *>(a);
    auto af = dynamic_cast<ConstantFP *>(a);

    switch (op)
    {
    case Instruction::fptosi:
        return ConstantInt::get(int(af->get_value()), module_);
    case Instruction::sitofp:
        return ConstantFP::get(float(ai->get_value()), module_);
    case Instruction::zext:
        return ConstantInt::get(bool(ai->get_value()), module_);
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
    else if (ins->is_fp2si() || ins->is_si2fp() || ins->is_zext())
    {
        return compute1(ins->get_instr_type(), is_const_args[0]);
    }
    else if (ins->is_cmp())
    {
        auto cmp_ins = dynamic_cast<CmpInst *>(ins);
        auto op = cmp_ins->get_cmp_op();
        return compute2cmp(op, is_const_args[0], is_const_args[1]);
    }
    else if (ins->is_fcmp())
    {
        auto fcmp_ins = dynamic_cast<FCmpInst *>(ins);
        auto op = fcmp_ins->get_cmp_op();
        return compute2fcmp(op, is_const_args[0], is_const_args[1]);
    }
    return nullptr;
}

/**
 * Pass through a BB, perform const propagation, and update const variable set, and continue to next bb
 * TODO: remove const br
 */
void ConstPropagation::pass_bb(BasicBlock *bb, ConstMap const_map)
{
    bb_passed_set.insert(bb->get_name());

    ConstMap::iterator k;
    std::vector<Instruction *> ins2del;

    for (auto ins : bb->get_instructions())
    {
        if (ins->get_num_operand() <= 0)
        {
            continue;
        }

        if (ins->is_phi())
        {
            for (auto i = 0; i < ins->get_num_operand() / 2; i++)
            {
                auto l = dynamic_cast<BasicBlock *>(ins->get_operand(i * 2 + 1));
                auto in_pre = false;
                for (auto pre : bb->get_pre_basic_blocks())
                {
                    if (pre == l)
                    {
                        in_pre = true;
                    }
                }
                if (!in_pre)
                {
                    ins->remove_operands(i * 2, i * 2 + 1);
                }
            }
        }

        std::vector<Constant *> is_const_args(ins->get_num_operand());

        for (auto i = 0; i < ins->get_num_operand(); i++)
        {
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

            if ((k = const_map.find(op->get_name())) != const_map.end())
            {
                ins->set_operand(i, k->second);
                is_const_args[i] = k->second;
            }
        }

        auto is_res_const = true;
        for (auto arg : is_const_args)
        {
            if (arg == nullptr)
            {
                is_res_const = false;
            }
        }

        if (is_res_const)
        {
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

    auto last = dynamic_cast<BranchInst *>(bb->get_terminator());
    if (last && last->is_cond_br())
    {
        auto known = false;
        auto branch = true;

        auto cond_const = cast_constantint(last->get_operand(0));
        if (cond_const)
        {
            known = true;
            branch = cond_const->get_value();
        }
        else if ((k = const_map.find(last->get_operand(0)->get_name())) != const_map.end())
        {
            known = true;
            branch = dynamic_cast<ConstantInt *>(k->second)->get_value();
        }

        if (known)
        {
            auto a = last->get_operand(branch ? 1 : 2);
            auto b = dynamic_cast<BasicBlock *>(last->get_operand(branch ? 2 : 1));
            last->remove_operands(0, 2);
            last->add_operand(a);
            last->add_use(a);
            bb->remove_succ_basic_block(b);
        }
    }

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
