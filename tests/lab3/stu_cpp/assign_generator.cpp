#include <iostream>

#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"

int main() {
  std::ios::sync_with_stdio(false);

  auto module = new Module("assign");
  auto builder = new IRBuilder(nullptr, module);
  Type *Int32Type = Type::get_int32_type(module);

  // Constant helpers.
  auto const_int = [&module](auto num) {
    return ConstantInt::get(num, module);
  };

  // int main()
  auto main_func = Function::create(FunctionType::get(Int32Type, {}), "main", module);
  auto main_bb = BasicBlock::create(module, "main_body", main_func);
  builder->set_insert_point(main_bb);

  // int a[10];
  auto *Int10ArrayType = ArrayType::get(Int32Type, 10);
  auto a = builder->create_alloca(Int10ArrayType);

  // a[0] = 10;
  auto a0_gep_1 = builder->create_gep(a, {const_int(0), const_int(0)});
  builder->create_store(const_int(10), a0_gep_1);

  // a[1] = a[0] * 2;
  auto a0_gep_2 = builder->create_gep(a, {const_int(0), const_int(0)});
  auto a0 = builder->create_load(a0_gep_2);
  auto a1_1 = builder->create_imul(a0, const_int(2));
  auto a1_gep_1 = builder->create_gep(a, {const_int(0), const_int(1)});
  builder->create_store(a1_1, a1_gep_1);

  // return a[1];
  auto a1_gep_2 = builder->create_gep(a, {const_int(0), const_int(1)});
  auto a1_2 = builder->create_load(a1_gep_2);
  builder->create_ret(a1_2);

  std::cout << module->print();
  delete module;
  return 0;
}
