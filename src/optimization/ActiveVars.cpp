#include "ActiveVars.hpp"
#include <algorithm>
#include "logging.hpp"

void ActiveVars::run()
{
    std::ofstream output_active_vars;
    output_active_vars.open("active_vars.json", std::ios::out);
    output_active_vars << "[";
    for (auto &func : this->m_->get_functions())
    {
        if (func->get_basic_blocks().empty())
        {
            // EXIT
            continue;
        }
        else
        {
            func_ = func;
            live_in.clear();
            live_out.clear();
            int flag = 1;

            while (flag == 1)
            {
                flag = 0;
                for (auto &BB : func_->get_basic_blocks())
                {
                    std::set<Value *> InSet = {}, OutSet = {};
                    // Out
                    for (auto &succBB : BB->get_succ_basic_blocks())
                    {
                        std::set_union(live_in[succBB].begin(), live_in[succBB].end(), OutSet.begin(), OutSet.end(), std::inserter(OutSet, OutSet.begin()));
                    }
                    live_out.insert({BB, OutSet});

                    // In
                    for (auto &itemUse : func_->get_use_list())
                    {
                        // std::cout << itemUse.val_->get_name();
                        InSet.insert(itemUse.val_);
                    }
                    std::set<Value *> tmpSet = {}, InSetNew = {};
                    std::set<Value *> defSet = {};
                    for (auto &itemDef : BB->get_instructions())
                    {
                        Instruction::OpID type = itemDef->get_instr_type();
                        // skip ret br phi gep call alloca instr
                        if (type == 0 || type == 1 || type == 10 || type == 15 || type == 16 || type == 17)
                        {
                            continue;
                        }

                        std::set<Instruction *> left_op_set = {}, right_op_set = {};
                        left_op_set.insert(itemDef);
                        for (auto &op : itemDef->get_operands())
                        {
                            right_op_set.insert(static_cast<Instruction *>(op));
                        }
                        if (right_op_set.find(itemDef) != right_op_set.end())
                        {
                            defSet.insert(itemDef);
                        }
                    }
                    if (BB->get_name() == "label_entry")
                    {
                        for (auto &itemDef : func_->get_args())
                        {
                            defSet.insert(static_cast<Value *>(itemDef));
                        }
                    }

                    std::set_difference(OutSet.begin(), OutSet.end(), defSet.begin(), defSet.end(), std::inserter(tmpSet, tmpSet.begin()));
                    std::set_union(InSet.begin(), InSet.end(), tmpSet.begin(), tmpSet.end(), std::inserter(InSetNew, InSetNew.begin()));
                    // InSetNew will be larger or equal
                    // if ((tmpSet or InSet) == InSet) do nothing
                    // else flag = 1
                    if (InSet.size() != InSetNew.size())
                    {
                        std::cout << BB->get_name() << std::endl;
                        flag = 1;
                    }
                    // std::cout << BB->get_name() << std::endl;

                    live_in.insert({BB, InSet});

                    // OutSet_B := all_S_is_B's_successor(InSet_S)
                    // InSet_B := use_BB + all(OutSet_B - def_BB)
                }
            }

            func_->set_instr_name();
            // 在此分析 func_ 的每个bb块的活跃变量，并存储在 live_in live_out 结构内

            output_active_vars << print();
            output_active_vars << ",";
        }
    }
    output_active_vars << "]";
    output_active_vars.close();
    return;
}

std::string ActiveVars::print()
{
    std::string active_vars;
    active_vars += "{\n";
    active_vars += "\"function\": \"";
    active_vars += func_->get_name();
    active_vars += "\",\n";

    active_vars += "\"live_in\": {\n";
    for (auto &p : live_in)
    {
        if (p.second.size() == 0)
        {
            continue;
        }
        else
        {
            active_vars += "  \"";
            active_vars += p.first->get_name();
            active_vars += "\": [";
            for (auto &v : p.second)
            {
                active_vars += "\"%";
                active_vars += v->get_name();
                active_vars += "\",";
            }
            active_vars += "]";
            active_vars += ",\n";
        }
    }
    active_vars += "\n";
    active_vars += "    },\n";

    active_vars += "\"live_out\": {\n";
    for (auto &p : live_out)
    {
        if (p.second.size() == 0)
        {
            continue;
        }
        else
        {
            active_vars += "  \"";
            active_vars += p.first->get_name();
            active_vars += "\": [";
            for (auto &v : p.second)
            {
                active_vars += "\"%";
                active_vars += v->get_name();
                active_vars += "\",";
            }
            active_vars += "]";
            active_vars += ",\n";
        }
    }
    active_vars += "\n";
    active_vars += "    }\n";

    active_vars += "}\n";
    active_vars += "\n";
    return active_vars;
}
