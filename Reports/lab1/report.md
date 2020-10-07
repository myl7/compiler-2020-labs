# lab1 实验报告

***REMOVED*** ***REMOVED***

## 实验要求

基于 Flex 实现支持 cminus-f 的词法解析器。

## 实验难点

给出不同 token 的 regex，并合理安排这些 regex 的顺序。

## 实验设计

由于框架中已实现对于 `lines`, `pos_start`, `pos_end` 处理，这里只需维护这三个变量即可。
在完成 token 匹配后，更新这三个变量。

注意到要求的输出格式，这三个变量应从 1 开始计数。

额外的，对于注释，由于仅此 token 可以同时包含换行符和非换行符，需要特殊处理：历遍其内容，统计上述三个变量的变化。

## 实验结果验证

我自行提供的测试样例位于 `/tests/lab1/testcase/7.cminus`，期望结果位于 `/tests/lab1/TA_token/7.tokens`。

我尽可能地提供了所有的边缘情况，经测试可以通过。

## 实验反馈

See [#62 Suggestion: C global variable sharing and trailing spaces at line end](staff/2020-fall-notice_board#62).
