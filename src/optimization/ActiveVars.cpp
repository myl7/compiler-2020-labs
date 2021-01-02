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

            for (auto &BB : func_->get_basic_blocks())
            {
                std::set<Value *> InSet = {}, OutSet = {};

                for (auto &succBB : BB->get_succ_basic_blocks())
                {
                    std::set_union(live_in[succBB].begin(), live_in[succBB].end(), OutSet.begin(), OutSet.end(), std::back_inserter(OutSet));
                }
                live_out.insert({BB, OutSet});

                /* remove
                for (auto &itemIn : func_->get_args())
                {
                    InSet.insert(itemIn);
                    // xxx loop
                }
                */
                for (auto &itemUse : func_->get_use_list())
                {
                    InSet.insert(itemUse.val_);
                }

                // get def -> defSet
                std::set<Value *> tmpSet = {};
                for (auto &item : OutSet)
                {
                    /* TODO
                    if (item != def)
                        tmpSet.insert(item);
                    */
                }
                std::set_union(InSet.begin(), InSet.end(), tmpSet.begin(), tmpSet.end(), std::back_inserter(InSet));

                // OutSet_B := all_S_is_B's_successor(InSet_S)
                // InSet_B := use_BB + all(OutSet_B - def_BB)

                live_in.insert({BB, InSet});
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