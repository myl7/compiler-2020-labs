; ModuleID = 'cminus'
source_filename = "/home/haiqwa/2020fall-compiler_cminus/tests/lab5/./testcases/ConstPropagation/testcase-5.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define void @main() {
label_entry:
  br label %label4
label4:                                                ; preds = %label_entry, %label50
  %op53 = phi i32 [ %op21, %label50 ], [ undef, %label_entry ]
  %op54 = phi i32 [ 0, %label_entry ], [ %op56, %label50 ]
  %op55 = phi i32 [ 0, %label_entry ], [ %op52, %label50 ]
  %op6 = icmp slt i32 %op55, 100000000
  %op7 = zext i1 %op6 to i32
  %op8 = icmp ne i32 %op7, 0
  br i1 %op8, label %label9, label %label29
label9:                                                ; preds = %label4
  %op12 = sdiv i32 %op55, 10000000
  %op19 = sitofp i32 %op12 to float
  %op20 = fadd float %op19, 0x0
  %op21 = fptosi float %op20 to i32
  br label %label31
label29:                                                ; preds = %label4
  call void @output(i32 %op54)
  ret void
label31:                                                ; preds = %label9
  %op38 = mul i32 2628, %op21
  %op40 = mul i32 %op38, %op21
  %op42 = mul i32 %op40, %op21
  %op44 = mul i32 %op42, %op21
  %op46 = mul i32 %op44, %op21
  %op48 = mul i32 %op46, %op21
  %op49 = add i32 %op54, %op48
  br label %label50
label50:                                                ; preds = %label31
  %op56 = phi i32 [ %op54, %label9 ], [ %op49, %label31 ]
  %op52 = add i32 %op55, 1
  br label %label4
}
