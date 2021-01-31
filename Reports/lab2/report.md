# lab2 实验报告

***REMOVED*** myl7

## 实验要求

基于 Flex 和 Bison 实现支持 cminus-f 的语法解析器。

## 实验难点

Bison 和 Flex 的协作：Flex 作为 Bison 的词法解析部分，从 Bison 定义中获得 token 声明，再词法分析出相应 node，借助 yylval 传回 Bison，由 Bison 完成语法解析。

## 实验设计

此实现设计已在“实验难点”中进行了说明。

## 实验结果验证

本人提供了一份相对综合的测试源码，位于 `tests/lab2/normal/custom.cminus`，对应期望结果位于 `tests/lab2/syntree_normal_std/custom.syntax_tree`，借助提供的测试脚本进行统一的测试，没有问题出现。

## 实验反馈

此实验的拓展阅读相当有趣，希望能有更多这样的内容。
