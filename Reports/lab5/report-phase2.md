# Lab5 实验报告

***REMOVED*** ***REMOVED***

***REMOVED*** ***REMOVED***

## 实验要求

学习并实现三个选定的「常量传播」、「循环不变式外提」和「活跃变量分析」基本优化 Pass

## 实验难点

C++ 中各种自定类型的转化（Cast）较为难懂

## 实验设计

- 常量传播
  实现思路：
  相应代码：
  优化前后的 IR 对比（举一个例子）并辅以简单说明：

- 循环不变式外提

实现思路：
在每段 BB 中，根据找出的指令组成 lvalSet、rvalSet 集合，再生成 e_gen、e_kill

相应代码：
优化前后的 IR 对比（举一个例子）并辅以简单说明：

- 活跃变量分析

实现思路：
利用书上的不断迭代选出活跃变量的方法，将 InSet 和 OutSet 变量集和相应的 BB 绑定

相应的伪代码：

```
live_in 清空
live_out 清空
flag := 1
while (flag)
{
    flag = 0
    for (EXIT 之外的 BB : BBs)
    {
        OutSet(B) := SUM(InSet(S)) // （对所有 B 的后继块 S）
        InSet(B) := use(BB) + SUM(OutSet(B) - def(BB))
        将 OutSet(B) 和 InSet(B) 插入到 live_in 和 live_out 的 map 中
        if (live_in 变化)
            flag = 1
    }
}
```

### 实验总结

学习了 C++ 的 static_cast 和 dynamic_cast 的使用

学习了 std::set 的使用方法

### 实验反馈 （可选 不会评分）

对本次实验的建议

### 组间交流 （可选）

本次实验和哪些组（记录组长学号）交流了哪一部分信息
