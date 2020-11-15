#include <iostream>

#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"

int main() {
  std::ios::sync_with_stdio(false);

  auto module = new Module("if");
  auto builder = new IRBuilder(nullptr, module);
  Type *Int32Type = Type::get_int32_type(module);
  Type *FloatType = Type::get_float_type(module);

  // Constant helpers.
  auto const_int = [&module](auto num) {
    return ConstantInt::get(num, module);
  };
  auto const_fp = [&module](auto num) {
    return ConstantFP::get(num, module);
  };

  // int main()
  auto main_func = Function::create(FunctionType::get(Int32Type, {}), "main", module);
  auto main_bb = BasicBlock::create(module, "main_body", main_func);
  builder->set_insert_point(main_bb);

  // float a = 5.555;
  auto a_alloca = builder->create_alloca(FloatType);
  builder->create_store(const_fp(5.555), a_alloca);

  // if (a > 1)
  auto a = builder->create_load(a_alloca);
  auto cond = builder->create_fcmp_gt(a, const_fp(1.0));
  auto true_bb = BasicBlock::create(module, "main_true", main_func);
  auto false_bb = BasicBlock::create(module, "main_false", main_func);
  builder->create_cond_br(cond, true_bb, false_bb);

  // return 233;
  builder->set_insert_point(true_bb);
  builder->create_ret(const_int(233));

  // return 0;
  builder->set_insert_point(false_bb);
  builder->create_ret(const_int(0));

  std::cout << module->print();
  delete module;
  return 0;
}
