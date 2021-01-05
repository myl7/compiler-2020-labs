#include "ActiveVars.hpp"
#include <algorithm>

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

                    for (auto &succBB : BB->get_succ_basic_blocks())
                    {
                        std::set_union(live_in[succBB].begin(), live_in[succBB].end(), OutSet.begin(), OutSet.end(), std::back_inserter(OutSet));
                    }
                    live_out.insert({BB, OutSet});

                    for (auto &itemUse : func_->get_use_list())
                    {
                        InSet.insert(itemUse.val_);
                    }

                    // get def -> defSet(Howto) TODO
                    std::set<Value *> tmpSet = {}, InSetNew = {};
                    std::set<Instruction *> defSet = {};
                    for (auto &itemDef : BB->get_instructions())
                    {
                        Instruction::OpID type = itemDef->get_instr_type();
                        // ret br phi gep call alloca
                        if (type == 0 || type == 1 || type == 10 || type == 15 || type == 16 || type == 17)
                        {
                            continue;
                        }

                        std::set<Instruction *> left_op_set = {}, right_op_set = {};
                        left_op_set.insert(itemDef);
                        for (auto &op : itemDef->get_operands()) // ? insert instr->get_oprand(0)
                        {
                            right_op_set.insert(static_cast<Instruction *>(op));
                        }
                        // if (Value(itemDef))
                        if (right_op_set.find(itemDef) != right_op_set.end())
                            defSet.insert(itemDef);
                    }

                    /*
                    for (auto &item : OutSet)
                    {
                        if (item not in def)
                            tmpSet.insert(item);
                    }
                    */
                    std::set_difference(OutSet.begin(), OutSet.end(), defSet.begin(), defSet.end(), std::back_inserter(tmpSet));
                    std::set_union(InSet.begin(), InSet.end(), tmpSet.begin(), tmpSet.end(), std::back_inserter(InSetNew));

                    if (!(!(InSet < InSetNew) && !(InSet > InSetNew)))
                    {
                        flag = 1;
                    }

                    // if ((tmpSet or InSet) == InSet) do nothing
                    // else flag = 1

                    live_in.insert({BB, InSet});

                    // OutSet_B := all_S_is_B's_successor(InSet_S)
                    // InSet_B := use_BB + all(OutSet_B - def_BB)
                }
            }

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
    func_->set_instr_name();
    active_vars += "{\n";
    active_vars += "\"function\": \"";
    active_vars += func_->get_name();
    active_vars += "\",\n";

    active_vars += "\"live_in\": {\n";
    for (auto &p : live_in)
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
        active_vars.pop_back();
        active_vars += "]";
        active_vars += ",\n";
    }
    active_vars.pop_back();
    active_vars.pop_back();
    active_vars += "\n";
    active_vars += "    },\n";

    active_vars += "\"live_out\": {\n";
    for (auto &p : live_out)
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
        active_vars.pop_back();
        active_vars += "]";
        active_vars += ",\n";
    }
    active_vars.pop_back();
    active_vars.pop_back();
    active_vars += "\n";
    active_vars += "    }\n";

    active_vars += "}\n";
    active_vars += "\n";
    return active_vars;
}