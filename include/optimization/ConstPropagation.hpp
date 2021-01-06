#ifndef CONSTPROPAGATION_HPP
#define CONSTPROPAGATION_HPP
#include "PassManager.hpp"
#include "Constant.h"
#include "Instruction.h"
#include "Module.h"

#include "Value.h"
#include "IRBuilder.h"
#include <vector>
#include <stack>
#include <map>
#include <set>

// tips: 用来判断value是否为ConstantFP/ConstantInt
ConstantFP *cast_constantfp(Value *value);
ConstantInt *cast_constantint(Value *value);

typedef std::map<std::string, Constant *> ConstMap;

// tips: ConstFloder类

class ConstFolder
{
public:
    ConstFolder(Module *m) : module_(m) {}
    Constant *compute2(Instruction::OpID op, Constant *value1, Constant *value2);
    Constant *compute1(Instruction::OpID op, Constant *a);
    Constant *compute(Instruction *ins, std::vector<Constant *> is_const_args);

private:
    Module *module_;
};

class ConstPropagation : public Pass
{
public:
    ConstPropagation(Module *m) : Pass(m) {}
    void run();

private:
    void pass_bb(BasicBlock *bb, ConstMap const_map);

    std::set<std::string> bb_passed_set;
};

#endif
