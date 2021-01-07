#include "ActiveVars.hpp"
#include <algorithm>
#include "logging.hpp"

std::map<BasicBlock *, std::set<Value *>> useSet, defSet;

void ActiveVars::run()
{
    std::ofstream output_active_vars;
    output_active_vars.open("active_vars.json", std::ios::out);
    output_active_vars << "[";

    m_->set_print_name();

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
            for (auto &BB : func_->get_basic_blocks())
            {
                std::set<Value *> lhsSet = {}, rhsSet = {};
                for (auto &instr : BB->get_instructions())
                {
                    Instruction::OpID type = instr->get_instr_type();
                    if (type == Instruction::OpID::ret)
                    {
                        if (instr->get_operands().size() == 1)
                        {
                            if (lhsSet.find(instr->get_operand(0)) == lhsSet.end() && instr->get_operand(0)->get_name() != "")
                            {
                                rhsSet.insert(instr->get_operand(0));
                            }
                        }
                        continue;
                    }
                    if (type == Instruction::OpID::br)
                    {
                        if (instr->get_operands().size() == 3)
                        {
                            if (lhsSet.find(instr->get_operand(0)) == lhsSet.end() && instr->get_operand(0)->get_name() != "")
                            {
                                rhsSet.insert(instr->get_operand(0));
                            }
                        }
                        continue;
                    }
                    if (type == Instruction::OpID::alloca)
                    {
                        if (rhsSet.find(instr) == rhsSet.end())
                        {
                            lhsSet.insert(instr);
                        }
                        continue;
                    }
                    if (type == Instruction::OpID::phi)
                    {
                        if (rhsSet.find(instr) == rhsSet.end() && instr->get_name() != "")
                        {
                            lhsSet.insert(instr);
                        }
                        int size = instr->get_operands().size();
                        for (int i = 0; i < size; i++)
                        {
                            auto op = instr->get_operand(i);
                            if (lhsSet.find(op) == lhsSet.end() && op->get_name() != "")
                            {
                                rhsSet.insert(op);
                            }
                            i++;
                        }
                        continue;
                    }
                    if (type == Instruction::OpID::call)
                    {
                        if (!(instr->is_void()))
                        {
                            if (rhsSet.find(instr) == rhsSet.end())
                            {
                                lhsSet.insert(instr);
                            }
                        }
                        for (int i = 1; i < instr->get_num_operand(); i++)
                        {
                            if (lhsSet.find(instr->get_operand(i)) == lhsSet.end() && instr->get_operand(i)->get_name() != "")
                            {
                                rhsSet.insert(instr->get_operand(i));
                            }
                        }
                        continue;
                    }
                    if (type == Instruction::OpID::getelementptr || type == Instruction::OpID::zext || type == Instruction::OpID::fcmp || type == Instruction::OpID::cmp || type == Instruction::OpID::add || type == Instruction::OpID::sub || type == Instruction::OpID::mul || type == Instruction::OpID::sdiv || type == Instruction::OpID::fadd || type == Instruction::OpID::fsub || type == Instruction::OpID::fmul || type == Instruction::OpID::fdiv)
                    {
                        if (rhsSet.find(instr) == rhsSet.end())
                        {
                            lhsSet.insert(instr);
                        }
                        for (auto &op : instr->get_operands())
                        {
                            if (lhsSet.find(op) == lhsSet.end() && op->get_name() != "")
                            {
                                rhsSet.insert(op);
                            }
                        }
                        continue;
                    }
                    if (type == Instruction::OpID::store)
                    {
                        for (auto &op : instr->get_operands())
                        {
                            if (lhsSet.find(op) == lhsSet.end() && op->get_name() != "")
                            {
                                rhsSet.insert(op);
                            }
                        }
                        continue;
                    }
                    if (type == Instruction::OpID::load)
                    {
                        if (rhsSet.find(instr) == rhsSet.end())
                        {
                            lhsSet.insert(instr);
                        }
                        for (auto &op : instr->get_operands())
                        {
                            if (lhsSet.find(op) == lhsSet.end() && op->get_name() != "")
                            {
                                rhsSet.insert(op);
                            }
                        }
                        continue;
                    }
                    for (auto &op : instr->get_operands())
                    {
                        if (lhsSet.find(op) == lhsSet.end() && op->get_name() != "")
                        {
                            rhsSet.insert(op);
                        }
                    }
                    // std::cout << type << "!" << std::endl;
                }
                /*
                std::cout << BB->get_name() << ":" << std::endl;
                std::cout << "use:" << std::endl;
                for (auto &use : rhsSet)
                {
                    std::cout << use->get_name() << std::endl;
                }
                std::cout << "def:" << std::endl;
                for (auto &def : lhsSet)
                {
                    std::cout << def->get_name() << std::endl;
                }
                */
                defSet.insert({BB, lhsSet});
                useSet.insert({BB, rhsSet});
            }
        }
    }
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
            while (flag)
            {
                flag = 0;
                for (auto &BB : func_->get_basic_blocks())
                {
                    if ((BB->get_succ_basic_blocks().size() == 0) && (BB->get_pre_basic_blocks().size() == 0))
                    {
                        continue;
                    }

                    // Out
                    std::set<Value *> OutSet = {};
                    for (auto &succBB : BB->get_succ_basic_blocks())
                    {
                        std::set_union(live_in[succBB].begin(), live_in[succBB].end(), OutSet.begin(), OutSet.end(), std::inserter(OutSet, OutSet.begin()));
                    }
                    live_out.insert({BB, OutSet});

                    // In
                    std::set<Value *> tmpSet = OutSet;
                    for (auto &defItem : defSet[BB])
                    {
                        if (tmpSet.find(defItem) != tmpSet.end())
                        {
                            tmpSet.erase(defItem);
                        }
                    }
                    std::set<Value *> InSet = {};
                    std::set_union(useSet[BB].begin(), useSet[BB].end(), tmpSet.begin(), tmpSet.end(), std::inserter(InSet, InSet.begin()));
                    // InSet will be larger or equal
                    // if ((tmpSet or InSet) == InSet) do nothing
                    // else flag = 1
                    std::cout << InSet.size();
                    if (InSet.size() != useSet[BB].size())
                    {
                        std::cout << BB->get_name() << ":" << std::endl;
                        std::cout << "use:" << std::endl;
                        for (auto &use : useSet[BB])
                        {
                            std::cout << use->get_name() << std::endl;
                        }
                        std::cout << "def:" << std::endl;
                        for (auto &def : defSet[BB])
                        {
                            std::cout << def->get_name() << std::endl;
                        }
                        flag = 1;
                    }
                    live_in.insert({BB, InSet});
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
