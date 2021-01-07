#include <algorithm>
#include "logging.hpp"
#include "LoopSearch.hpp"
#include "LoopInvHoist.hpp"

void LoopInvHoist::run()
{
    // 先通过LoopSearch获取循环的相关信息
    LoopSearch loop_searcher(m_, true);
    loop_searcher.run();

    // 接下来由你来补充啦！
    m_->set_print_name();
    for (auto func : m_->get_functions())
    {
        for (auto bb_set : loop_searcher.get_loops_in_func(func))
        {
            auto parent = loop_searcher.get_parent_loop(bb_set);
            if (!parent)
            {
                continue;
            }

            auto base = loop_searcher.get_loop_base(bb_set);

            // Get defs
            std::set<std::string> defs;
            for (auto bb : *bb_set)
            {
                for (auto ins : bb->get_instructions())
                {
                    if (ins->get_name() != "")
                    {
                        defs.insert(ins->get_name());
                    }
                }
            }

            std::vector<Instruction *> ins2out;

            // Loop over all ins, if all uses in a ins is not def in the loop, then move it out
            auto moved = true;
            while (moved)
            {
                moved = false;
                for (auto bb : *bb_set)
                {
                    for (auto ins : bb->get_instructions())
                    {
                        if (ins->is_phi() || ins->is_gep() || ins->is_call() || ins->is_alloca() || ins->is_load() || ins->is_store() || ins->is_ret() || ins->is_br())
                        {
                            continue;
                        }

                        auto to_continue = false;
                        for (auto ins_out : ins2out)
                        {
                            if (ins_out == ins)
                            {
                                to_continue = true;
                                break;
                            }
                        }
                        if (to_continue)
                        {
                            continue;
                        }

                        auto in_loop = false;
                        for (auto op : ins->get_operands())
                        {
                            if (defs.find(op->get_name()) != defs.end())
                            {
                                in_loop = true;
                                break;
                            }
                        }
                        if (!in_loop)
                        {
                            ins2out.push_back(ins);
                            defs.erase(ins->get_name());
                            moved = true;
                        }
                    }
                }
            }

            if (ins2out.size() <= 0)
            {
                continue;
            }

            for (auto ins : ins2out)
            {
                ins->get_parent()->delete_instr(ins);
            }

            auto outers = base->get_pre_basic_blocks();
            auto outer = outers.front();
            if (outers.size() != 1)
            {
                auto a = outers.front();
                auto k = outers.begin();
                k++;
                auto b = *k;
                outer = find_common_parent(a, b, bb_set);
            }
            LOG(DEBUG) << outer->get_name();

            auto term = outer->get_terminator();
            outer->delete_instr(term);
            for (auto ins : ins2out)
            {
                outer->add_instruction(ins);
                ins->set_parent(outer);
            }
            outer->add_instruction(term);
        }
    }
}

BasicBlock *find_common_parent(BasicBlock *a, BasicBlock *b, BBset_t *bb_set)
{
    if (bb_set->find(a) == bb_set->end())
    {
        return a;
    }
    else
    {
        return b;
    }
}
