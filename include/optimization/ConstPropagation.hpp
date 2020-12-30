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
#include <unordered_map>

// tips: 用来判断value是否为ConstantFP/ConstantInt
ConstantFP *cast_constantfp(Value *value);
ConstantInt *cast_constantint(Value *value);

typedef std::unordered_map<std::string, Constant *> ConstMap;

// tips: ConstFloder类

class ConstFolder
{
public:
    ConstFolder(Module *m) : module_(m) {}
    template <typename T>
    T *compute(Instruction::OpID op, T *value1, T *value2);
    // ...
private:
    Module *module_;
};

class ConstPropagation : public Pass
{
public:
    ConstPropagation(Module *m) : Pass(m) {}
    void run();

private:
    ConstMap &&pass_bb(BasicBlock *bb, ConstMap &&const_map);
};

#endif
