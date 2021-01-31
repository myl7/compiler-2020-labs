; ModuleID = 'cminus'
source_filename = "/home/haiqwa/2020fall-compiler_cminus/tests/lab5/./testcases/ConstPropagation/testcase-8.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define i32 @main() {
label_entry:
  br label %label2
label2:                                                ; preds = %label_entry, %label32
  %op47 = phi i32 [ 0, %label_entry ], [ %op49, %label32 ]
  %op48 = phi i32 [ 0, %label_entry ], [ %op34, %label32 ]
  %op4 = icmp slt i32 %op48, 100000000
  %op5 = zext i1 %op4 to i32
  %op6 = icmp ne i32 %op5, 0
  br i1 %op6, label %label7, label %label10
label7:                                                ; preds = %label2
  %op9 = icmp ne i32 %op47, 0
  br i1 %op9, label %label12, label %label22
label10:                                                ; preds = %label2
  call void @output(i32 %op47)
  ret i32 0
label12:                                                ; preds = %label7
  br label %label35
label22:                                                ; preds = %label7
  br label %label41
label32:                                                ; preds = %label40, %label46
  %op49 = phi i32 [ 1, %label40 ], [ 5, %label46 ]
  %op34 = add i32 %op48, 1
  br label %label2
label35:                                                ; preds = %label12
  br label %label40
label40:                                                ; preds = %label35
  br label %label32
label41:                                                ; preds = %label22
  br label %label46
label46:                                                ; preds = %label41
  br label %label32
}
