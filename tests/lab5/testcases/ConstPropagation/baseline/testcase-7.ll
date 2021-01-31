; ModuleID = 'cminus'
source_filename = "/home/haiqwa/2020fall-compiler_cminus/tests/lab5/./testcases/ConstPropagation/testcase-7.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define i32 @score(i32 %arg0, i32 %arg1) {
label_entry:
  br label %label14
label14:                                                ; preds = %label_entry
  br label %label22
label20:                                                ; preds = %label27
  ret i32 %op50
label22:                                                ; preds = %label14
  br label %label28
label27:                                                ; preds = %label33
  br label %label20
label28:                                                ; preds = %label22
  br label %label34
label33:                                                ; preds = %label39
  br label %label27
label34:                                                ; preds = %label28
  br label %label40
label39:                                                ; preds = %label51
  br label %label33
label40:                                                ; preds = %label34
  %op43 = sub i32 %arg1, %arg0
  %op45 = mul i32 %op43, 16
  %op47 = mul i32 %op45, 11
  %op49 = sub i32 10, %arg1
  %op50 = mul i32 %op47, %op49
  br label %label51
label51:                                                ; preds = %label40
  br label %label39
}
define void @main() {
label_entry:
  br label %label4
label4:                                                ; preds = %label_entry, %label9
  %op24 = phi i32 [ 0, %label_entry ], [ %op19, %label9 ]
  %op25 = phi i32 [ %op14, %label9 ], [ undef, %label_entry ]
  %op26 = phi i32 [ %op11, %label9 ], [ undef, %label_entry ]
  %op27 = phi i32 [ 0, %label_entry ], [ %op21, %label9 ]
  %op6 = icmp slt i32 %op27, 100000000
  %op7 = zext i1 %op6 to i32
  %op8 = icmp ne i32 %op7, 0
  br i1 %op8, label %label9, label %label22
label9:                                                ; preds = %label4
  %op11 = sdiv i32 %op27, 10000000
  %op13 = sdiv i32 %op27, 10000000
  %op14 = sub i32 9, %op13
  %op18 = call i32 @score(i32 %op11, i32 %op14)
  %op19 = add i32 %op24, %op18
  %op21 = add i32 %op27, 1
  br label %label4
label22:                                                ; preds = %label4
  call void @output(i32 %op24)
  ret void
}
