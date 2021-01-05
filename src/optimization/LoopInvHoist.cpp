#include <algorithm>
#include "logging.hpp"
#include "LoopSearch.hpp"
#include "LoopInvHoist.hpp"

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
        for (auto &BB : func->get_basic_blocks())
        {
            std::set<Instruction *> left_op_set = {}, right_op_set = {};
            for (auto &instr : BB->get_instructions())
            {
                Instruction::OpID type = instr->get_instr_type();
                // ret br phi gep call alloca
                if (type == 0 || type == 1 || type == 10 || type == 15 || type == 16 || type == 17)
                {
                    continue;
                }
                left_op_set.insert(instr);
                for (auto op : instr->get_operands()) // ? insert instr->get_oprand(0)
                {
                    right_op_set.insert(static_cast<Instruction *>(op));
                }
            }
            std::cout << "right_op_set" << '\n';
            for (auto r : right_op_set)
            {
                std::cout << r->get_name() << '\n';
            }
        }
    }
}