# lab3 实验报告

***REMOVED*** myl7

## 实验要求

- 手写 LLVM IR；
- cpp 生成 LLVM IR。

## 问题 1: cpp 与 .ll 的对应

请描述你的 cpp 代码片段和 .ll 的每个 BasicBlock 的对应关系。
描述中请附上两者代码。

### assign.c

```cpp
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
```

仅一个 BB，为 `main` function body，对应：

```llvm
; int a[10];
%1 = alloca [10 x i32]
; a[0] = 10;
%2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 0
store i32 10, i32* %2
; a[1] = a[0] * 2;
%3 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 0
%4 = load i32, i32* %3
%5 = mul i32 %4, 2
%6 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 1
store i32 %5, i32* %6
; return a[1];
%7 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 1
%8 = load i32, i32* %7
ret i32 %8
```

### fun.c

```cpp
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
```

为 `callee` function body 对应：

```llvm
; return 2 * a;
%2 = mul i32 2, %0
ret i32 %2
```

```cpp
// int main()
auto main_func = Function::create(FunctionType::get(Int32Type, {}), "main", module);
auto main_bb = BasicBlock::create(module, "main_body", main_func);
builder->set_insert_point(main_bb);

// return callee(110);
auto ret = builder->create_call(callee_func, {const_int(110)});
builder->create_ret(ret);
```

为 `main` function body 对应：

```llvm
; return callee(110);
%1 = call i32 @callee(i32 110)
ret i32 %1
```

### if.c

```cpp
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
```

为 `main` function body `if` 前内容，对应：

```llvm
; float a = 5.555;
%1 = alloca float
store float 0x40163851e0000000, float* %1
; if (a > 1)
%2 = load float, float* %1
%3 = fcmp ugt float %2, 1.0
br i1 %3, label %4, label %5
```

```cpp
// return 233;
builder->set_insert_point(true_bb);
builder->create_ret(const_int(233));
```

为 `main` function body 中 `if` 条件成立后的流程，对应：

```llvm
4:
; return 233;
ret i32 233
```

```cpp
// return 0;
builder->set_insert_point(false_bb);
builder->create_ret(const_int(0));
```

为 `main` function body 中 `if` 条件不成立后的流程，对应：

```llvm
5:
; return 0;
ret i32 0
```

### while.c

```cpp
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
```

为 `main` function body `while` 前内容，对应：

```llvm
; int a;
%1 = alloca i32
; int i;
%2 = alloca i32
; a = 10;
store i32 10, i32* %1
; i = 0;
store i32 0, i32* %2
; while (i < 10)
br label %3
```

```cpp
// while (i < 10)
auto cond_bb = BasicBlock::create(module, "main_cond", main_func);
builder->create_br(cond_bb);
builder->set_insert_point(cond_bb);
auto i_1 = builder->create_load(i_alloca);
auto cond = builder->create_icmp_lt(i_1, const_int(10));
auto loop_bb = BasicBlock::create(module, "main_loop", main_func);
auto exit_bb = BasicBlock::create(module, "main_exit", main_func);
builder->create_cond_br(cond, loop_bb, exit_bb);
```

为 `main` function body `while` 判断的内容，对应：

```llvm
3:
%4 = load i32, i32* %2
%5 = icmp slt i32 %4, 10
br i1 %5, label %6, label %12
```

```cpp
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
```

为 `main` function body `while` 循环的内容，对应：

```llvm
6:
; i = i + 1;
%7 = load i32, i32* %2
%8 = add i32 %7, 1
store i32 %8, i32* %2
; a = a + i;
%9 = load i32, i32* %1
%10 = load i32, i32* %2
%11 = add i32 %9, %10
store i32 %11, i32* %1
br label %3
```

```cpp
// return a;
builder->set_insert_point(exit_bb);
auto a_2 = builder->create_load(a_alloca);
builder->create_ret(a_2);
```

为 `main` function body `while` 退出后的内容，对应：

```llvm
12:
; return a;
%13 = load i32, i32* %1
ret i32 %13
```

## 问题2: Visitor Pattern

请指出 visitor.cpp 中，`treeVisitor.visit(exprRoot)` 执行时，以下几个 Node 的遍历序列: numberA、numberB、exprC、exprD、exprE、numberF、exprRoot。
序列请按如下格式指明：
exprRoot -> numberF -> exprE -> numberA -> exprD

answer: exprRoot -> numberF -> exprE -> exprD -> numberB -> numberA -> exprC -> numberA -> numberB

## 问题3: getelementptr

请给出 `IR.md` 中提到的两种 getelementptr 用法的区别, 并稍加解释:
  - `%2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 %0`
  - `%2 = getelementptr i32, i32* %1, i32 %0`

前者是从 type `i32[10]` 中取出一个 `i32*`，而后者是从 type `i32*` 中取出一个 `i32*`。

关键在于这两种 type 有本质区别：`i32[10]` 相较于 `i32*` 是带有长度信息的，可以利用此信息完成边界校验等工作，例如此时 `%2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 12` 获得的 `%2` 就是指向一个非法地址，`store` 它就是非法的，而 `%2 = getelementptr i32, i32* %1, i32 12` 就没办法决定了。

在 C 系语言中，像这样对于 array 和 pointer 是有明确区分的，在其他编程语言中，类似的思想也很普遍：Rust 的 `[i32]` 和 `[i32; 10]`，Go 的 `[]int` 和 `[10]int` 等。

## 实验难点

> 描述在实验中遇到的问题、分析和解决方案

其实没什么难点。
就我个人而言，一个问题是 Arch Linux pacman 源中的 LLVM 已经是 11.0.0 了，但我翻了一下 Changlog，应该至少不会影响这次实验。

## 实验反馈

> 吐槽? 建议?

建议文档参考 [中文文案排版指北](https://github.com/sparanoid/chinese-copywriting-guidelines) 处理中英文混合时的问题，以及希望源文件中能尽可能避免 trailing spaces。如果可以的话，也希望能 ensure 文本文件尾有一个 newline，这样能让人看到代码时赏心悦目、心潮澎湃、乐不思蜀。
