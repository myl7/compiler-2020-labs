#include <iostream>
#include <vector>

#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"

int main() {
  std::ios::sync_with_stdio(false);

  auto module = new Module("fun");
  auto builder = new IRBuilder(nullptr, module);
  Type *Int32Type = Type::get_int32_type(module);

  // Constant helpers.
  auto const_int = [&module](auto num) {
    return ConstantInt::get(num, module);
  };

  // int callee(int a)
  auto callee_func = Function::create(FunctionType::get(Int32Type, {Int32Type}), "callee", module);
  std::vector<Value *> callee_args;
  for (auto arg = callee_func->arg_begin(); arg != callee_func->arg_end(); arg++) {
    callee_args.push_back(*arg);
  }
  auto callee_bb = BasicBlock::create(module, "callee_body", callee_func);
  builder->set_insert_point(callee_bb);

  // return 2 * a;
  auto callee_ret = builder->create_imul(const_int(2), callee_args[0]);
  builder->create_ret(callee_ret);

  // int main()
  auto main_func = Function::create(FunctionType::get(Int32Type, {}), "main", module);
  auto main_bb = BasicBlock::create(module, "main_body", main_func);
  builder->set_insert_point(main_bb);

  // return callee(110);
  auto ret = builder->create_call(callee_func, {const_int(110)});
  builder->create_ret(ret);

  std::cout << module->print();
  delete module;
  return 0;
}
