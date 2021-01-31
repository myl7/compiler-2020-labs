; ModuleID = 'cminus'
source_filename = "testcase-5.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define void @main() {
label_entry:
  br label %label4
label4:                                                ; preds = %label_entry, %label18
  %op71 = phi i32 [ %op75, %label18 ], [ undef, %label_entry ]
  %op72 = phi i32 [ %op76, %label18 ], [ undef, %label_entry ]
  %op73 = phi i32 [ 1, %label_entry ], [ %op20, %label18 ]
  %op74 = phi i32 [ %op77, %label18 ], [ undef, %label_entry ]
  %op6 = icmp slt i32 %op73, 100
  %op7 = zext i1 %op6 to i32
  %op8 = icmp ne i32 %op7, 0
  br i1 %op8, label %label9, label %label10
label9:                                                ; preds = %label4
  %op29 = mul i32 %op73, %op73
  %op31 = mul i32 %op29, %op73
  %op33 = mul i32 %op31, %op73
  %op35 = mul i32 %op33, %op73
  %op37 = mul i32 %op35, %op73
  %op39 = mul i32 %op37, %op73
  %op41 = mul i32 %op39, %op73
  %op43 = mul i32 %op41, %op73
  %op45 = mul i32 %op43, %op73
  %op47 = sdiv i32 %op45, %op73
  %op49 = sdiv i32 %op47, %op73
  %op51 = sdiv i32 %op49, %op73
  %op53 = sdiv i32 %op51, %op73
  %op55 = sdiv i32 %op53, %op73
  %op57 = sdiv i32 %op55, %op73
  %op59 = sdiv i32 %op57, %op73
  %op61 = sdiv i32 %op59, %op73
  %op63 = sdiv i32 %op61, %op73
  %op65 = sdiv i32 %op63, %op73
  br label %label12
label10:                                                ; preds = %label4
  call void @output(i32 %op71)
  ret void
label12:                                                ; preds = %label9, %label68
  %op75 = phi i32 [ %op71, %label9 ], [ %op78, %label68 ]
  %op76 = phi i32 [ %op72, %label9 ], [ %op79, %label68 ]
  %op77 = phi i32 [ 0, %label9 ], [ %op70, %label68 ]
  %op14 = icmp slt i32 %op77, 100
  %op15 = zext i1 %op14 to i32
  %op16 = icmp ne i32 %op15, 0
  br i1 %op16, label %label17, label %label18
label17:                                                ; preds = %label12
  br label %label21
label18:                                                ; preds = %label12
  %op20 = add i32 %op73, 1
  br label %label4
label21:                                                ; preds = %label17, %label26
  %op78 = phi i32 [ %op75, %label17 ], [ %op65, %label26 ]
  %op79 = phi i32 [ 0, %label17 ], [ %op67, %label26 ]
  %op23 = icmp slt i32 %op79, 1000
  %op24 = zext i1 %op23 to i32
  %op25 = icmp ne i32 %op24, 0
  br i1 %op25, label %label26, label %label68
label26:                                                ; preds = %label21
  %op67 = add i32 %op79, 1
  br label %label21
label68:                                                ; preds = %label21
  %op70 = add i32 %op77, 1
  br label %label12
}
