# Lab5 实验报告

myl7 ***REMOVED***

yuanyiwei ***REMOVED***

## 实验要求

学习并实现三个选定的「常量传播」、「循环不变式外提」和「活跃变量分析」基本优化 Pass

## 实验难点

C++ 中各种自定类型的转化（Cast）较为难懂

## 实验设计

- 常量传播

实现思路：

DFS 历遍所有未历遍的 BB，从而历遍 BB 中每一条 ins，如果 ins 的 ops 都是常量，则更新随之历遍的常量表 const_map，并将新常量添加到常量表中

通过对 bool(i1) 的常量传播，发现无效的 BB 关系，更新其中的 cond_br 为 br

相应代码：

这里取 BB 访问的部分并做了一部分简化

```cpp
for (auto ins : bb->get_instructions())
{
    // 处理无效的 phi
    if (ins->is_phi())
    {
        for (auto i = 0; i < ins->get_num_operand() / 2; i++)
        {
            auto l = dynamic_cast<BasicBlock *>(ins->get_operand(i * 2 + 1));
            auto in_pre = false;
            for (auto pre : bb->get_pre_basic_blocks())
            {
                if (pre == l)
                {
                    in_pre = true;
                }
            }
            if (!in_pre)
            {
                ins->remove_operands(i * 2, i * 2 + 1);
            }
        }
    }

    std::vector<Constant *> is_const_args(ins->get_num_operand());

    for (auto i = 0; i < ins->get_num_operand(); i++)
    {
        auto op = ins->get_operand(i);
        auto op_iconst = cast_constantint(op);
        auto op_fconst = cast_constantfp(op);

        // 检查参数的 const 性质
        if (op_iconst)
        {
            is_const_args[i] = op_iconst;
        }
        else if (op_fconst)
        {
            is_const_args[i] = op_fconst;
        }
        else
        {
            is_const_args[i] = nullptr;
        }

        // 暂存获得的操作数常量值
        if ((k = const_map.find(op->get_name())) != const_map.end())
        {
            ins->set_operand(i, k->second);
            is_const_args[i] = k->second;
        }
    }

    auto is_res_const = true;
    ...

    if (is_res_const)
    {
        auto res_new = ConstFolder(m_).compute(ins, is_const_args);
        if (res_new != nullptr)
        {
            // 完成常量的记录等处理
            ins->remove_use_of_ops();
            ins->replace_all_use_with(res_new);
            ins2del.push_back(ins);
            const_map.insert({ins->get_name(), res_new});
        }
    }
}

// 删除传播后的无用赋值
for (auto ins : ins2del)
{
    bb->delete_instr(ins);
}

// 完成无效 BB 关系检测与 cond_br - br 的转换
auto last = dynamic_cast<BranchInst *>(bb->get_terminator());
if (last && last->is_cond_br())
{
    auto known = false;
    auto branch = true;

    auto cond_const = cast_constantint(last->get_operand(0));
    if (cond_const)
    {
        known = true;
        branch = cond_const->get_value();
    }
    else if ((k = const_map.find(last->get_operand(0)->get_name())) != const_map.end())
    {
        known = true;
        branch = dynamic_cast<ConstantInt *>(k->second)->get_value();
    }

    if (known)
    {
        auto a = last->get_operand(branch ? 1 : 2);
        auto b = dynamic_cast<BasicBlock *>(last->get_operand(branch ? 2 : 1));
        last->remove_operands(0, 2);
        last->add_operand(a);
        last->add_use(a);
        bb->remove_succ_basic_block(b);
    }
}

// 继续历遍
for (auto succ : bb->get_succ_basic_blocks())
{
    if (bb_passed_set.find(succ->get_name()) == bb_passed_set.end())
    {
        pass_bb(succ, const_map);
    }
}
```

优化前后的 IR 对比（举一个例子）并辅以简单说明：

这里以 testcase-1 为例

优化前：

```llvm
declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define void @main() {
label_entry:
  br label %label2
label2:                                                ; preds = %label_entry, %label7
  %op78 = phi i32 [ 0, %label_entry ], [ %op73, %label7 ]
  %op79 = phi i32 [ 0, %label_entry ], [ %op40, %label7 ]
  %op4 = icmp slt i32 %op78, 100000000
  %op5 = zext i1 %op4 to i32
  %op6 = icmp ne i32 %op5, 0
  br i1 %op6, label %label7, label %label74
label7:                                                ; preds = %label2
  %op8 = add i32 1, 1
  %op9 = add i32 %op8, 1
  %op10 = add i32 %op9, 1
  %op11 = add i32 %op10, 1
  %op12 = add i32 %op11, 1
  %op13 = add i32 %op12, 1
  %op14 = add i32 %op13, 1
  %op15 = add i32 %op14, 1
  %op16 = add i32 %op15, 1
  %op17 = add i32 %op16, 1
  %op18 = add i32 %op17, 1
  %op19 = add i32 %op18, 1
  %op20 = add i32 %op19, 1
  %op21 = add i32 %op20, 1
  %op22 = add i32 %op21, 1
  %op23 = add i32 %op22, 1
  %op24 = add i32 %op23, 1
  %op25 = add i32 %op24, 1
  %op26 = add i32 %op25, 1
  %op27 = add i32 %op26, 1
  %op28 = add i32 %op27, 1
  %op29 = add i32 %op28, 1
  %op30 = add i32 %op29, 1
  %op31 = add i32 %op30, 1
  %op32 = add i32 %op31, 1
  %op33 = add i32 %op32, 1
  %op34 = add i32 %op33, 1
  %op35 = add i32 %op34, 1
  %op36 = add i32 %op35, 1
  %op37 = add i32 %op36, 1
  %op38 = add i32 %op37, 1
  %op39 = add i32 %op38, 1
  %op40 = add i32 %op39, 1
  %op44 = mul i32 %op40, %op40
  %op46 = mul i32 %op44, %op40
  %op48 = mul i32 %op46, %op40
  %op50 = mul i32 %op48, %op40
  %op52 = mul i32 %op50, %op40
  %op54 = mul i32 %op52, %op40
  %op56 = mul i32 %op54, %op40
  %op59 = mul i32 %op40, %op40
  %op61 = mul i32 %op59, %op40
  %op63 = mul i32 %op61, %op40
  %op65 = mul i32 %op63, %op40
  %op67 = mul i32 %op65, %op40
  %op69 = mul i32 %op67, %op40
  %op71 = mul i32 %op69, %op40
  %op72 = sdiv i32 %op56, %op71
  %op73 = add i32 %op78, %op72
  br label %label2
label74:                                                ; preds = %label2
  %op77 = mul i32 %op79, %op79
  call void @output(i32 %op77)
  ret void
}
```

优化后：

```llvm
declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define void @main() {
label_entry:
  br label %label2
label2:                                                ; preds = %label_entry, %label7
  %op78 = phi i32 [ 0, %label_entry ], [ %op73, %label7 ]
  %op79 = phi i32 [ 0, %label_entry ], [ 34, %label7 ]
  %op4 = icmp slt i32 %op78, 100000000
  %op5 = zext i1 %op4 to i32
  %op6 = icmp ne i32 %op5, 0
  br i1 %op6, label %label7, label %label74
label7:                                                ; preds = %label2
  %op73 = add i32 %op78, 1
  br label %label2
label74:                                                ; preds = %label2
  %op77 = mul i32 %op79, %op79
  call void @output(i32 %op77)
  ret void
}
```

可以看到相当大的一块常量计算已经因为常量传播而消失了

- 循环不变式外提

实现思路：
在循环中，历遍所有 BB 的所有 ins 获取完成了赋值的变量表 defs，则对于其中赋值所用操作数均不在循环内被赋值时，此指令可外提。
特殊指令例如 load/store/phi 进行特殊处理。

对于应外提到的 BB，若 prev 有两个 BB 时，其中一个 BB 因循环返回而成为 prev，则取两个 BB 中不位于此循环内，即不在 bb_set 中者即可

相应代码：

这里提取出了 bb_set 处理的代码，同上有一定简化：

```cpp
auto base = loop_searcher.get_loop_base(bb_set);

// 获得上述的 defs
std::set<std::string> defs;
for (auto bb : *bb_set)
{
    for (auto ins : bb->get_instructions())
    {
        if (ins->get_name() != "")
        {
            defs.insert(ins->get_name());
        }
    }
}

std::vector<Instruction *> ins2out;

// 找出所有的可外移指令
auto moved = true;
while (moved)
{
    moved = false;
    for (auto bb : *bb_set)
    {
        for (auto ins : bb->get_instructions())
        {
            if (ins->is_phi() || ins->is_gep() || ins->is_call() || ins->is_alloca() || ins->is_load() || ins->is_store() || ins->is_ret() || ins->is_br())
            {
                continue;
            }

            auto to_continue = false;
            for (auto ins_out : ins2out)
            {
                if (ins_out == ins)
                {
                    to_continue = true;
                    break;
                }
            }
            if (to_continue)
            {
                continue;
            }

            auto in_loop = false;
            for (auto op : ins->get_operands())
            {
                if (defs.find(op->get_name()) != defs.end())
                {
                    in_loop = true;
                    break;
                }
            }
            if (!in_loop)
            {
                ins2out.push_back(ins);
                defs.erase(ins->get_name());
                moved = true;
            }
        }
    }
}

// 删除原有指令
for (auto ins : ins2out)
{
    ins->get_parent()->delete_instr(ins);
}

// 获得指令的新位置
auto outers = base->get_pre_basic_blocks();
auto outer = outers.front();
if (outers.size() != 1)
{
    auto a = outers.front();
    auto k = outers.begin();
    k++;
    auto b = *k;
    outer = find_common_parent(a, b, bb_set);
}
LOG(DEBUG) << outer->get_name();

// 将指令添加到新位置
auto term = outer->get_terminator();
outer->delete_instr(term);
for (auto ins : ins2out)
{
    outer->add_instruction(ins);
    ins->set_parent(outer);
}
outer->add_instruction(term);
```

优化前后的 IR 对比（举一个例子）并辅以简单说明：

这里以 testcase-3 为例

优化前：

```llvm
declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define void @main() {
label_entry:
  br label %label8
label8:                                                ; preds = %label_entry, %label22
  %op102 = phi i32 [ %op109, %label22 ], [ undef, %label_entry ]
  %op103 = phi i32 [ %op110, %label22 ], [ undef, %label_entry ]
  %op104 = phi i32 [ %op111, %label22 ], [ undef, %label_entry ]
  %op105 = phi i32 [ %op112, %label22 ], [ undef, %label_entry ]
  %op106 = phi i32 [ %op113, %label22 ], [ undef, %label_entry ]
  %op107 = phi i32 [ 0, %label_entry ], [ %op24, %label22 ]
  %op108 = phi i32 [ %op114, %label22 ], [ undef, %label_entry ]
  %op10 = icmp slt i32 %op107, 1000000
  %op11 = zext i1 %op10 to i32
  %op12 = icmp ne i32 %op11, 0
  br i1 %op12, label %label13, label %label14
label13:                                                ; preds = %label8
  br label %label16
label14:                                                ; preds = %label8
  call void @output(i32 %op102)
  ret void
label16:                                                ; preds = %label13, %label31
  %op109 = phi i32 [ %op102, %label13 ], [ %op115, %label31 ]
  %op110 = phi i32 [ %op103, %label13 ], [ %op116, %label31 ]
  %op111 = phi i32 [ %op104, %label13 ], [ %op117, %label31 ]
  %op112 = phi i32 [ %op105, %label13 ], [ %op118, %label31 ]
  %op113 = phi i32 [ %op106, %label13 ], [ %op119, %label31 ]
  %op114 = phi i32 [ 0, %label13 ], [ %op33, %label31 ]
  %op18 = icmp slt i32 %op114, 2
  %op19 = zext i1 %op18 to i32
  %op20 = icmp ne i32 %op19, 0
  br i1 %op20, label %label21, label %label22
label21:                                                ; preds = %label16
  br label %label25
label22:                                                ; preds = %label16
  %op24 = add i32 %op107, 1
  br label %label8
label25:                                                ; preds = %label21, %label40
  %op115 = phi i32 [ %op109, %label21 ], [ %op120, %label40 ]
  %op116 = phi i32 [ %op110, %label21 ], [ %op121, %label40 ]
  %op117 = phi i32 [ %op111, %label21 ], [ %op122, %label40 ]
  %op118 = phi i32 [ %op112, %label21 ], [ %op123, %label40 ]
  %op119 = phi i32 [ 0, %label21 ], [ %op42, %label40 ]
  %op27 = icmp slt i32 %op119, 2
  %op28 = zext i1 %op27 to i32
  %op29 = icmp ne i32 %op28, 0
  br i1 %op29, label %label30, label %label31
label30:                                                ; preds = %label25
  br label %label34
label31:                                                ; preds = %label25
  %op33 = add i32 %op114, 1
  br label %label16
label34:                                                ; preds = %label30, %label49
  %op120 = phi i32 [ %op115, %label30 ], [ %op124, %label49 ]
  %op121 = phi i32 [ %op116, %label30 ], [ %op125, %label49 ]
  %op122 = phi i32 [ %op117, %label30 ], [ %op126, %label49 ]
  %op123 = phi i32 [ 0, %label30 ], [ %op51, %label49 ]
  %op36 = icmp slt i32 %op123, 2
  %op37 = zext i1 %op36 to i32
  %op38 = icmp ne i32 %op37, 0
  br i1 %op38, label %label39, label %label40
label39:                                                ; preds = %label34
  br label %label43
label40:                                                ; preds = %label34
  %op42 = add i32 %op119, 1
  br label %label25
label43:                                                ; preds = %label39, %label99
  %op124 = phi i32 [ %op120, %label39 ], [ %op127, %label99 ]
  %op125 = phi i32 [ %op121, %label39 ], [ %op128, %label99 ]
  %op126 = phi i32 [ 0, %label39 ], [ %op101, %label99 ]
  %op45 = icmp slt i32 %op126, 2
  %op46 = zext i1 %op45 to i32
  %op47 = icmp ne i32 %op46, 0
  br i1 %op47, label %label48, label %label49
label48:                                                ; preds = %label43
  br label %label52
label49:                                                ; preds = %label43
  %op51 = add i32 %op123, 1
  br label %label34
label52:                                                ; preds = %label48, %label57
  %op127 = phi i32 [ %op124, %label48 ], [ %op96, %label57 ]
  %op128 = phi i32 [ 0, %label48 ], [ %op98, %label57 ]
  %op54 = icmp slt i32 %op128, 2
  %op55 = zext i1 %op54 to i32
  %op56 = icmp ne i32 %op55, 0
  br i1 %op56, label %label57, label %label99
label57:                                                ; preds = %label52
  %op60 = mul i32 2, 2
  %op62 = mul i32 %op60, 2
  %op64 = mul i32 %op62, 2
  %op66 = mul i32 %op64, 2
  %op68 = mul i32 %op66, 2
  %op70 = mul i32 %op68, 2
  %op72 = mul i32 %op70, 2
  %op74 = mul i32 %op72, 2
  %op76 = mul i32 %op74, 2
  %op78 = sdiv i32 %op76, 2
  %op80 = sdiv i32 %op78, 2
  %op82 = sdiv i32 %op80, 2
  %op84 = sdiv i32 %op82, 2
  %op86 = sdiv i32 %op84, 2
  %op88 = sdiv i32 %op86, 2
  %op90 = sdiv i32 %op88, 2
  %op92 = sdiv i32 %op90, 2
  %op94 = sdiv i32 %op92, 2
  %op96 = sdiv i32 %op94, 2
  %op98 = add i32 %op128, 1
  br label %label52
label99:                                                ; preds = %label52
  %op101 = add i32 %op126, 1
  br label %label43
}
```

优化后：

```llvm
declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define void @main() {
label_entry:
  %op60 = mul i32 2, 2
  %op62 = mul i32 %op60, 2
  %op64 = mul i32 %op62, 2
  %op66 = mul i32 %op64, 2
  %op68 = mul i32 %op66, 2
  %op70 = mul i32 %op68, 2
  %op72 = mul i32 %op70, 2
  %op74 = mul i32 %op72, 2
  %op76 = mul i32 %op74, 2
  %op78 = sdiv i32 %op76, 2
  %op80 = sdiv i32 %op78, 2
  %op82 = sdiv i32 %op80, 2
  %op84 = sdiv i32 %op82, 2
  %op86 = sdiv i32 %op84, 2
  %op88 = sdiv i32 %op86, 2
  %op90 = sdiv i32 %op88, 2
  %op92 = sdiv i32 %op90, 2
  %op94 = sdiv i32 %op92, 2
  %op96 = sdiv i32 %op94, 2
  br label %label8
label8:                                                ; preds = %label_entry, %label22
  %op102 = phi i32 [ %op109, %label22 ], [ undef, %label_entry ]
  %op103 = phi i32 [ %op110, %label22 ], [ undef, %label_entry ]
  %op104 = phi i32 [ %op111, %label22 ], [ undef, %label_entry ]
  %op105 = phi i32 [ %op112, %label22 ], [ undef, %label_entry ]
  %op106 = phi i32 [ %op113, %label22 ], [ undef, %label_entry ]
  %op107 = phi i32 [ 0, %label_entry ], [ %op24, %label22 ]
  %op108 = phi i32 [ %op114, %label22 ], [ undef, %label_entry ]
  %op10 = icmp slt i32 %op107, 1000000
  %op11 = zext i1 %op10 to i32
  %op12 = icmp ne i32 %op11, 0
  br i1 %op12, label %label13, label %label14
label13:                                                ; preds = %label8
  br label %label16
label14:                                                ; preds = %label8
  call void @output(i32 %op102)
  ret void
label16:                                                ; preds = %label13, %label31
  %op109 = phi i32 [ %op102, %label13 ], [ %op115, %label31 ]
  %op110 = phi i32 [ %op103, %label13 ], [ %op116, %label31 ]
  %op111 = phi i32 [ %op104, %label13 ], [ %op117, %label31 ]
  %op112 = phi i32 [ %op105, %label13 ], [ %op118, %label31 ]
  %op113 = phi i32 [ %op106, %label13 ], [ %op119, %label31 ]
  %op114 = phi i32 [ 0, %label13 ], [ %op33, %label31 ]
  %op18 = icmp slt i32 %op114, 2
  %op19 = zext i1 %op18 to i32
  %op20 = icmp ne i32 %op19, 0
  br i1 %op20, label %label21, label %label22
label21:                                                ; preds = %label16
  br label %label25
label22:                                                ; preds = %label16
  %op24 = add i32 %op107, 1
  br label %label8
label25:                                                ; preds = %label21, %label40
  %op115 = phi i32 [ %op109, %label21 ], [ %op120, %label40 ]
  %op116 = phi i32 [ %op110, %label21 ], [ %op121, %label40 ]
  %op117 = phi i32 [ %op111, %label21 ], [ %op122, %label40 ]
  %op118 = phi i32 [ %op112, %label21 ], [ %op123, %label40 ]
  %op119 = phi i32 [ 0, %label21 ], [ %op42, %label40 ]
  %op27 = icmp slt i32 %op119, 2
  %op28 = zext i1 %op27 to i32
  %op29 = icmp ne i32 %op28, 0
  br i1 %op29, label %label30, label %label31
label30:                                                ; preds = %label25
  br label %label34
label31:                                                ; preds = %label25
  %op33 = add i32 %op114, 1
  br label %label16
label34:                                                ; preds = %label30, %label49
  %op120 = phi i32 [ %op115, %label30 ], [ %op124, %label49 ]
  %op121 = phi i32 [ %op116, %label30 ], [ %op125, %label49 ]
  %op122 = phi i32 [ %op117, %label30 ], [ %op126, %label49 ]
  %op123 = phi i32 [ 0, %label30 ], [ %op51, %label49 ]
  %op36 = icmp slt i32 %op123, 2
  %op37 = zext i1 %op36 to i32
  %op38 = icmp ne i32 %op37, 0
  br i1 %op38, label %label39, label %label40
label39:                                                ; preds = %label34
  br label %label43
label40:                                                ; preds = %label34
  %op42 = add i32 %op119, 1
  br label %label25
label43:                                                ; preds = %label39, %label99
  %op124 = phi i32 [ %op120, %label39 ], [ %op127, %label99 ]
  %op125 = phi i32 [ %op121, %label39 ], [ %op128, %label99 ]
  %op126 = phi i32 [ 0, %label39 ], [ %op101, %label99 ]
  %op45 = icmp slt i32 %op126, 2
  %op46 = zext i1 %op45 to i32
  %op47 = icmp ne i32 %op46, 0
  br i1 %op47, label %label48, label %label49
label48:                                                ; preds = %label43
  br label %label52
label49:                                                ; preds = %label43
  %op51 = add i32 %op123, 1
  br label %label34
label52:                                                ; preds = %label48, %label57
  %op127 = phi i32 [ %op124, %label48 ], [ %op96, %label57 ]
  %op128 = phi i32 [ 0, %label48 ], [ %op98, %label57 ]
  %op54 = icmp slt i32 %op128, 2
  %op55 = zext i1 %op54 to i32
  %op56 = icmp ne i32 %op55, 0
  br i1 %op56, label %label57, label %label99
label57:                                                ; preds = %label52
  %op98 = add i32 %op128, 1
  br label %label52
label99:                                                ; preds = %label52
  %op101 = add i32 %op126, 1
  br label %label43
}
```

可见繁重的计算已经被提到外部了

- 活跃变量分析

实现思路：
利用书上的不断迭代选出活跃变量的方法，将 InSet 和 OutSet 变量集和相应的 BB 绑定，
在处理 phi 指令时，需要把用者身份和当前身份都输出到一个 map 中，在后来的 InSet 迭代时加以判断

相应的实现代码：

`set_print_name()` 把变量转换成可输出的格式

```cpp
m_->set_print_name();
```

对于 `ret` 指令，需要判断是 void ret 还是 int/float ret

```cpp
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
```

对于 `phi` 指令，需要保存用者身份和当前身份

```cpp
if (type == Instruction::OpID::phi)
{
    if (rhsSet.find(instr) == rhsSet.end() && instr->get_name() != "")
    {
        lhsSet.insert(instr);
    }
    int size = instr->get_operands().size();
    for (int i = 0; i < size; i += 2)
    {
        auto op = instr->get_operand(i);
        if (lhsSet.find(op) == lhsSet.end() && op->get_name() != "")
        {
            rhsSet.insert(op);
        }
        phiOut[dynamic_cast<BasicBlock *>(instr->get_operand(i + 1))].insert(op);
        phiUse[BB].insert(op);
    }
    continue;
}
```

对于大部分指令，只需要把 lhs 和 rhs 存入 use 和 def 集合中

```cpp
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
```

对于主体部分，类似书上的做法，用 flag 变量观察循环前后 Live_in 集合是否改变

在 live_out 的部分，判断是否要插入来着 `phi` 语句的 `Value*`

```cpp
std::set<Value *> OutSet = {};
for (auto &succBB : BB->get_succ_basic_blocks())
{
    for (auto &item : live_in[succBB])
    {
        if (phiUse[succBB].find(item) != phiUse[succBB].end() && phiOut[BB].find(item) == phiOut[BB].end())
        {
            continue;
        }
        OutSet.insert(item);
    }
}
live_out[BB] = OutSet;
```

在 live_in 的部分，要从之前产生的 `OutSet - defSet[BB]` 和 `useSet[BB]` 合并

```cpp
std::set<Value *> tmpSet = OutSet;
for (auto &defItem : defSet[BB])
{
    if (tmpSet.find(defItem) != tmpSet.end())
    {
        tmpSet.erase(defItem);
    }
}
std::set<Value *> InSet = tmpSet;
for (auto &item : useSet[BB])
{
    InSet.insert(item);
}
live_in[BB] = InSet;
```

### 实验总结

学习了 C++ 的 static_cast 和 dynamic_cast 的使用

学习了 std::set 的使用方法

数据流分析的基本方法和处理逻辑

### 实验反馈 （可选 不会评分）

对本次实验的建议

### 组间交流 （可选）

本次实验和哪些组（记录组长学号）交流了哪一部分信息
