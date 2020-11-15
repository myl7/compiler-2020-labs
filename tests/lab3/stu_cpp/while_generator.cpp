#include <iostream>

#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"

int main() {
  std::ios::sync_with_stdio(false);

  auto module = new Module("while");
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

  // int a;
  auto a_alloca = builder->create_alloca(Int32Type);

  // int i;
  auto i_alloca = builder->create_alloca(Int32Type);

  // a = 10;
  builder->create_store(const_int(10), a_alloca);

  // i = 0;
  builder->create_store(const_int(0), i_alloca);

  // while (i < 10)
  auto cond_bb = BasicBlock::create(module, "main_cond", main_func);
  builder->create_br(cond_bb);
  builder->set_insert_point(cond_bb);
  auto i_1 = builder->create_load(i_alloca);
  auto cond = builder->create_icmp_lt(i_1, const_int(10));
  auto loop_bb = BasicBlock::create(module, "main_loop", main_func);
  auto exit_bb = BasicBlock::create(module, "main_exit", main_func);
  builder->create_cond_br(cond, loop_bb, exit_bb);

  // i = i + 1;
  builder->set_insert_point(loop_bb);
  auto i_2 = builder->create_load(i_alloca);
  auto i_new = builder->create_iadd(i_2, const_int(1));
  builder->create_store(i_new, i_alloca);

  // a = a + i;
  auto a_1 = builder->create_load(a_alloca);
  auto i_3 = builder->create_load(i_alloca);
  auto a_new = builder->create_iadd(a_1, i_3);
  builder->create_store(a_new, a_alloca);
  builder->create_br(cond_bb);

  // return a;
  builder->set_insert_point(exit_bb);
  auto a_2 = builder->create_load(a_alloca);
  builder->create_ret(a_2);

  std::cout << module->print();
  delete module;
  return 0;
}
