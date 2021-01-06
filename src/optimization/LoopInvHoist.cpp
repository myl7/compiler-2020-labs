#include <algorithm>
#include "logging.hpp"
#include "LoopSearch.hpp"
#include "LoopInvHoist.hpp"

/*
std::list<Instruction *> markedAsInvariant;
bool isInvariant(std::list<Instruction *> instr, BBset_t *loop)
{
    bool is_invariant = true;
    for (auto iter = instr.begin(); is_invariant && iter != instr.end(); ++iter)
    {
        Value *val = dynamic_cast<Value *>(iter); // ?
        assert(val != NULL);
        // 如果当前操作数不是一个常量，那可能是被标定为循环不变量或者其他
        // 如果是函数参数则可以视为循环体之外的不变量
        // 注意：这里的函数参数，指的是当前循环所在函数的函数参数，不是循环内部调用函数传入的参数
        if (!isa<Constant>(val) && !isa<Argument>(val)) // ?
        {
            if (Instruction *inst = dynamic_cast<Instruction *>(val))
            {
                // 如果既不是循环不变量指令，也不是循环外的指令
                if (std::find(markedAsInvariant.begin(), markedAsInvariant.end(), inst) == markedAsInvariant.end() && loop->contains(inst->get_parent())) // contains?
                // 那么该操作数就不是循环不变量
                {
                    is_invariant = false;
                }
            }
            else
            {
                // 不是所有遍历到的操作数都是指令、常量和函数参数，可能也有基本块的Label等等
                is_invariant = false;
            }
        }
    }
    return is_invariant;
}

*/

void LoopInvHoist::run()
{
    // 先通过LoopSearch获取循环的相关信息
    // LoopSearch loop_searcher(m_, false);
    LoopSearch loop_searcher(m_, true);
    loop_searcher.run();

    // 接下来由你来补充啦！

    m_->set_print_name();
    for (auto &func : m_->get_functions())
    {
        for (auto &BB : loop_searcher.get_loops_in_func(func))
        {
            std::set<Instruction *> lvalSet = {}, rvalSet = {};
            /*
            for (auto &instr : BB->get_instructions())
            {
                Instruction::OpID type = instr->get_instr_type();
                // skip ret br phi gep call alloca instr
                if (type == 0 || type == 1 || type == 10 || type == 15 || type == 16 || type == 17)
                {
                    continue;
                }

                lvalSet.insert(instr);
                for (auto op : instr->get_operands()) // ? insert instr->get_oprand(0)
                {
                    rvalSet.insert(static_cast<Instruction *>(op));
                }
            }
            /*
            /*

            std::cout << "rvalSet" << '\n';
            for (auto &rvalue : rvalSet)
            {
                std::cout << rvalue->get_name() << '\n';
            }
            */
        }
    }
}