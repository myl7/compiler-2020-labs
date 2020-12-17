# Lab5 实验文档

- [Lab5 实验文档](#lab5-实验文档)
  - [0. 前言](#0-前言)
    - [主要工作](#主要工作)
  - [1. 实验框架](#1-实验框架)
  - [2. 运行与调试](#2-运行与调试)
    - [运行 cminusfc](#运行-cminusfc)
    - [自动测试](#自动测试)
    - [logging](#logging)
    - [建议](#建议)
  - [3. 提交要求](#3-提交要求)
    - [目录结构](#目录结构)
    - [提交要求和评分标准](#提交要求和评分标准)

## 0. 前言

本次实验是组队实验，请仔细阅读[组队要求](http://222.195.68.197/staff/2020-fall-notice_board/-/issues/89)，并合理进行分工合作。

**Pass概念**：接受一个`module`作为参数，经过lab4大家可以知道，`module`是IR最上层的结构，而`Pass`则是遍历`module`内的结构，分析出信息(例如对活跃变量的分析 `Pass`)，或者是对`module`内的指令和bb做一些变换(例如本次实验中的常量传播和循环不变式外提 `Pass`)。

经过 Lab4，相信大家已经掌握了 LightIR 的结构，并且对于 LLVM IR 也有了更深的理解。在本次实验中，我们要在理解SSA（静态单赋值）格式的基础上，实现三个简单的块内优化Pass与分析Pass：常量传播，循环不变式外提，活跃变量分析。

值得一提的是`lightir`中的[lightir.md](../common/lightir.md)中的User类中`operands_`成员也就是操作数列表，以及Value类的`use_list_`成员，这两个链表描述了指令间的数据依赖关系，请注意查看。

### 主要工作

#### 代码与材料阅读
具体要求与说明参考：[CodeReading.md](./CodeReading.md)

1. 阅读`Mem2Reg`与`LoopSearch`两个优化Pass的代码，能够描述优化的基本流程，并且回答思考题。
2. 通过阅读代码，掌握如何开发基于LightIR的优化Pass

#### 开发基本优化Pass

1. **常量传播**
能够实现在编译优化阶段，能够计算出结果的变量，就直接替换为常量；补充以下几点需要注意的地方：
    a. 只需要考虑过程内的常量传播，可以不用考虑数组，这里举个例子来说明常量传播：
    ```cpp
    %a = 1 + 1；
    %b = %a + %c;
    ```
    那么首先我们可以将`%a=1+1`折叠成`%a=2`，然后我们发现`%b=%a+%c`这条指令用到了`%a`，那么我们就可以将`%b=%a+%c`中的`%a`直接替换成常量1，代码转化为：
    ```cpp
    %a = 2;
    %b = 2 + %c;
    ```
    当然本次实验还需要额外做一步：将`%a=2`这条无用语句删掉，因为`%a`是常量，并且已经传播给了使用它的地方，那么这条赋值语句就可以删掉了（由于我们的IR是SSA形式，所以不用担心%a被重新赋值）：
    ```cpp
    %b=2+%c;
    ```
    b. 整形浮点型都需要考虑，
    d. 对于`a=1/0`的情形，可以不考虑，即可以做处理也可以不处理。
    c. 做到删除无用的分支将条件跳转变换为强制性跳转，比如下面的语句中，else部分就可以删除
   
    ```cpp
    int a;
    a=1;
    if(a){...}
    else{...}
    ```
    同时对于分支嵌套的情况都能够删除掉无用的分支，这一步之后若出现冗余的块也需要进行合并，并删去相应的跳转指令。
   
2. **循环不变式外提**
    要能够实现将与循环无关的表达式提取到循环的外面。不用考虑数组，但需要考虑全局变量。举个例子：
    
    ```cpp
    while(i<10){
      while(j<10){
        a=i+1;
        j=j+1;
      }
      i=i+1;
    }
    ```
    那么表达式`a=i+1`与内部循环无关，可以提到j循环之外：
    ```cpp
    while(i<10){
      a=i+1;
      while(j<10){
        j=j+1;
      }
      i=i+1;
    }
    ```
    下面给出一些循环外提的tips：
    a. 思考如何判断语句与循环无关，且外提没有副作用
    b. 循环的条件块（就是在LoopSearch中找到的Base块）最多只有两个前驱，思考下，不变式应该外提到哪一个前驱。
    
3. **活跃变量分析**

   能够实现分析bb块的入口和出口的活跃变量，参考资料见附件(紫书9.2.4节)，在`ActiveVars.hpp`中定义了两个成员`live_in`, `live_out`，你需要将`pair<bb, IN[bb]>`插入`live_in`的map 结构中，将`pair<bb, OUT[bb]>`插入`live_out` 的map 结构中，并调用ActiveVars类中的print()方法输出bb活跃变量情况到json文件，助教会根据你输出的json文件进行批改。(为了保证输出变量名字的一致性，请不要对指令，bb等进行命名操作，cminusfc_builder 推荐使用lab4的答案)


#### Bonus：选做优化Pass
不做强制要求，可以自由发挥，但是需要在报告中说明达到的效果，并在线下验收时展示与实验报告里说明，选做Pass 在期末考试前均可以提交。
1. 标记式死代码删除(支持过程间分析)
3. 函数内联
4. 块间公共子表达式删除
5. 尾递归消除

#### Lab5代码与实验报告提交
1. 基本优化Pass的代码都写在`src/optimization/`目录下面，头文件放入`include/optimization/`当中，最后只会在这两个目录下验收代码文件。
2. 对于选做的优化Pass，需要发邮件给助教，统一线下验收。
3. 需要在 `Reports/lab5/` 目录下撰写实验报告，且由队长说明成员贡献比率。其中，在 `report-phase1.md` 中完成代码阅读部分的报告，在 `report-phase2.md` 中解释你们的基本优化PASS的设计，遇到的困难和解决方案，由**队长**在 `contribution.md` 中解释每位队员的贡献，并说明贡献比例



注意：组队实验意味着合作，但是小组间的交流是受限的，且严格**禁止**代码的共享。除此之外，如果小组和其它组进行了交流，必须在 `report-phase1.md` 和`report-phase2.md`  中记录下来交流的小组和你们之间交流内容。

## 1. 实验框架

本次实验使用了由C++编写的 LightIR 来在IR层面完成优化化简，为了便于大家进行实验，助教对之前的`cminusfc`增加了选项，用来选择是否开启某种优化；另外，若想要另外单独去进行某个优化Pass的调试，可以利用助教给出的PassManager来进行Pass的注册和运行。

在`include/optimization/PassManager.hpp`中，定义了一个用于管理Pass的类`PassManager`。它的作用是注册与运行Pass。它提供了以下接口：
```cpp
PassManager pm(module.get())
pm.add_Pass<Mem2Reg>(emit)	//注册Pass，emit为true时打印优化后的IR
pm.run()	//按照注册的顺序运行Pass的run()函数
```
基本Pass开发：

每一个Pass有一个cpp文件和对应的hpp文件，可以在hpp里定义辅助类或者成员变量使用，在cpp里的run()函数实现你的Pass。

## 2. 运行与调试

### 运行 cminusfc
```sh
mkdir build && cd build
cmake ..
make -j
make install
```
编译后会产生 `cminusfc` 程序，它能将cminus文件输出为LLVM IR，也可以利用clang将IR编译成二进制。程序逻辑写在`cminusfc.cpp`中。

它通过`[-mem2reg] [-const-propagation] [-active-vars] [-loop-invariant]`开关来控制优化Pass的使用，当需要对 `.cminus` 文件测试时，可以这样使用：
```bash
./cminusfc [-mem2reg] [-const-propagation] [-active-vars] [-loop-invariant] <input-file>
```
### 自动测试
助教贴心地为大家准备了自动测试脚本，它在 `tests/lab5` 目录下，使用方法如下：
* 有两个可用的选项：`--ConstPropagation`/`-C`，`--LoopInvHoist`/`-L`，分别表示用来评测常量传播Pass以及循环不变式外提Pass。
* 评测脚本会对样例进行编译和执行，然后对生成的可执行文件首先检查结果的正确性，每个样例的正确结果会放在.out文件中，结果正确的情况下才会去进一步评测运行时间。
* 如果显示执行时间的表格中出现了`None`则表示该样例有错误。
* 每个样例会运行三次取平均时间（时间单位是s）并且保留两位小数输出，当然每个样例的运行次数也可以自行更改脚本中`repeated_time`变量。
```sh
# 在 tests/lab5 目录下运行：
./lab5_test.py -C
```
如果完全正确，它会输出：
```
========== ConstPropagation ==========
Compiling  
100%|██████████████| 1/1 [00:00<00:00, 11.22it/s]
Evalution 
100%|██████████████| 1/1 [00:01<00:00,  1.99s/it]
Compiling  -const-propagation
100%|██████████████| 1/1 [00:00<00:00, 12.83it/s]
Evalution 
100%|██████████████| 1/1 [00:01<00:00,  1.59s/it]
testcase         before optimization     after optimization
testcase-1              0.48                    0.39
```
如果要增加样例，直接在样例目录中添加文件即可，命名参考目录下的其他文件。


### logging

[logging](../common/logging.md) 是帮助大家打印调试信息的工具，如有需求可以阅读文档后进行使用

### 建议

1. 比较你们编写的编译器产生的 IR 和 clang 产生的IR来找出可能的问题或发现新的思路
2. 使用 logging 工具来打印调试信息
2. 使用 GDB 等软件进行单步调试来检查错误的原因
3. 合理分工

## 3. 提交要求

### 目录结构

```
.
├── CMakeLists.txt
├── Documentations
│   ├── ...
│   ├── common
│   |   ├── LightIR.md                  <- LightIR 相关文档
│   |   ├── logging.md                  <- logging 工具相关文档
│   |   └── cminusf.md                  <- cminus-f 的语法和语义文档
│   └── lab5
│       └── README.md                   <- lab5 实验文档说明（你在这里）
├── include                             <- 实验所需的头文件
│   ├── ...
│   ├── optimization/*
│   ├── cminusf_builder.hpp
|   └── ast.hpp
├── Reports
│   ├── ...
│   └── lab5
│       ├── report-phase1.md            <- lab5 所需提交的实验阅读部分报告，请详细说明你们的设计（需要上交）
│       ├── report-phase2.md            <- lab5 所需提交的实验基本PASS实验报告，请详细说明你们的设计（需要上交）
│       └── contribution.md             <- lab5 所需提交的队员贡献信息（需要上交）
├── src
│   ├── ...
│   └── optimization
│       ├── LoopSearch.cpp                <- 循环查找
│       ├── Mem2Reg.cpp                   <- Mem2Reg
│       ├── Dominators.cpp                <- 支配树
│       ├── ActiveVars.cpp                <- 活跃变量
│       ├── ConstPropagation.cpp          <- 常量传播
│       └── LoopInvHoist.cpp              <- 循环不变式外提        
│       
└── tests
    ├── ...
    └── lab5
        ├── testcases                   <- 助教提供的测试样例
        └── lab5_test.py                <- 助教提供的测试脚本

```
### 提交要求和评分标准

- 提交时间

  本次实验分阶段验收：

  **阶段一**：验收代码阅读报告及相关思考题  ` report-phase1.md`

  **阶段二**：验收lab5要求提交的代码及`report-phase2.md`

* 提交要求  
  本实验是组队实验，我们将收取**队长**实验仓库中的内容
  
  * 实验部分:
    * 需要填补 
    
      `./include/optimization/ActiveVars.hpp`，`./include/optimization/ConstPropagation.hpp`，`./include/optimization/LoopInvHoist.hpp` `./src/optimization/ActiveVars.cpp`，`./src/optimization/ConstPropagation.cpp.cpp`，`./src/optimization/LoopInvHoist.cpp`
    
    * 需要在 `./Reports/lab5/` 目录下撰写实验报告，且由队长说明成员贡献比率
    
    * 本次实验收取 
    
      `./include/optimization/ActiveVars.hpp`，`./include/optimization/ConstPropagation.hpp`，`./include/optimization/LoopInvHoist.hpp` 
    
      `./src/optimization/ActiveVars.cpp`，`./src/optimization/ConstPropagation.cpp`，`./src/optimization/LoopInvHoist.cpp` 文件和 `./Reports/lab5` 目录下报告，其中`report-phase1.md` 会在阶段一的ddl的时候进行验收；`report-phase2.md`以及实现代码会在阶段二的ddl的时候进行验收。
    
    * 选做Pass在`2021/01/11`之前都可以联系助教验收，然后由助教统一安排线下验收，该部分由学生现场演示，不需要撰写实验报告。
* 评分标准: 
  * 待定
  * 禁止执行恶意代码，违者本次实验0分处理
* 迟交规定
  * `Soft Deadline` :
  
    **阶段一**：2020/12/21 23:59:59 (北京标准时间，UTC+8)  
  
    **阶段二**：2021/01/04 23:59:59 (北京标准时间，UTC+8) 
  
  * `Hard Deadline`：
  
    **阶段一**：2020/12/28 23:59:59 (北京标准时间，UTC+8)  
  
    **阶段二**：2021/01/11 23:59:59 (北京标准时间，UTC+8) 
  
  * 迟交需要邮件通知TA: 
    * 邮箱: 
    whq1516@mail.ustc.edu.cn
    chen16614@mail.ustc.edu.cn
    * 邮件主题: lab5迟交-队长学号
    * 内容: 包括迟交原因、最后版本commitID、迟交时间等
    
  * 迟交分数
    * x为迟交天数(对于`Soft Deadline`而言)，grade为满分
      ``` bash
      final_grade = grade, x = 0
      final_grade = grade * (0.9)^x, 0 < x <= 7
      final_grade = 0, x > 7 # 这一条严格执行,请对自己负责
      ```
* 关于抄袭和雷同
  经过助教和老师判定属于实验抄袭或雷同情况，所有参与方一律零分，不接受任何解释和反驳。
如有任何问题，欢迎提issue进行批判指正。
