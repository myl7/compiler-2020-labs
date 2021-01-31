; ModuleID = 'cminus'
source_filename = "testcase-8.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define void @main() {
label_entry:
  %op70 = mul i32 2, 2
  %op72 = mul i32 %op70, 2
  %op74 = mul i32 %op72, 2
  %op76 = mul i32 %op74, 2
  %op78 = mul i32 %op76, 2
  %op80 = mul i32 %op78, 2
  %op82 = mul i32 %op80, 2
  %op84 = mul i32 %op82, 2
  %op86 = mul i32 %op84, 2
  %op88 = sdiv i32 %op86, 2
  %op90 = sdiv i32 %op88, 2
  %op92 = sdiv i32 %op90, 2
  %op94 = sdiv i32 %op92, 2
  %op96 = sdiv i32 %op94, 2
  %op98 = sdiv i32 %op96, 2
  %op100 = sdiv i32 %op98, 2
  %op102 = sdiv i32 %op100, 2
  %op104 = sdiv i32 %op102, 2
  %op106 = sdiv i32 %op104, 2
  %op60 = icmp sgt i32 2, 1
  %op61 = zext i1 %op60 to i32
  %op62 = icmp ne i32 %op61, 0
  br label %label9
label9:                                                ; preds = %label_entry, %label23
  %op109 = phi i32 [ %op117, %label23 ], [ undef, %label_entry ]
  %op110 = phi i32 [ %op118, %label23 ], [ undef, %label_entry ]
  %op111 = phi i32 [ %op119, %label23 ], [ undef, %label_entry ]
  %op112 = phi i32 [ %op120, %label23 ], [ undef, %label_entry ]
  %op113 = phi i32 [ %op121, %label23 ], [ undef, %label_entry ]
  %op114 = phi i32 [ %op122, %label23 ], [ undef, %label_entry ]
  %op115 = phi i32 [ 0, %label_entry ], [ %op25, %label23 ]
  %op116 = phi i32 [ %op123, %label23 ], [ undef, %label_entry ]
  %op11 = icmp slt i32 %op115, 1000000
  %op12 = zext i1 %op11 to i32
  %op13 = icmp ne i32 %op12, 0
  br i1 %op13, label %label14, label %label15
label14:                                                ; preds = %label9
  br label %label17
label15:                                                ; preds = %label9
  call void @output(i32 %op110)
  ret void
label17:                                                ; preds = %label14, %label32
  %op117 = phi i32 [ %op109, %label14 ], [ %op124, %label32 ]
  %op118 = phi i32 [ %op110, %label14 ], [ %op125, %label32 ]
  %op119 = phi i32 [ %op111, %label14 ], [ %op126, %label32 ]
  %op120 = phi i32 [ %op112, %label14 ], [ %op127, %label32 ]
  %op121 = phi i32 [ %op113, %label14 ], [ %op128, %label32 ]
  %op122 = phi i32 [ %op114, %label14 ], [ %op129, %label32 ]
  %op123 = phi i32 [ 0, %label14 ], [ %op34, %label32 ]
  %op19 = icmp slt i32 %op123, 2
  %op20 = zext i1 %op19 to i32
  %op21 = icmp ne i32 %op20, 0
  br i1 %op21, label %label22, label %label23
label22:                                                ; preds = %label17
  br label %label26
label23:                                                ; preds = %label17
  %op25 = add i32 %op115, 1
  br label %label9
label26:                                                ; preds = %label22, %label41
  %op124 = phi i32 [ %op117, %label22 ], [ %op130, %label41 ]
  %op125 = phi i32 [ %op118, %label22 ], [ %op131, %label41 ]
  %op126 = phi i32 [ %op119, %label22 ], [ %op132, %label41 ]
  %op127 = phi i32 [ %op120, %label22 ], [ %op133, %label41 ]
  %op128 = phi i32 [ %op121, %label22 ], [ %op134, %label41 ]
  %op129 = phi i32 [ 0, %label22 ], [ %op43, %label41 ]
  %op28 = icmp slt i32 %op129, 2
  %op29 = zext i1 %op28 to i32
  %op30 = icmp ne i32 %op29, 0
  br i1 %op30, label %label31, label %label32
label31:                                                ; preds = %label26
  br label %label35
label32:                                                ; preds = %label26
  %op34 = add i32 %op123, 1
  br label %label17
label35:                                                ; preds = %label31, %label50
  %op130 = phi i32 [ %op124, %label31 ], [ %op135, %label50 ]
  %op131 = phi i32 [ %op125, %label31 ], [ %op136, %label50 ]
  %op132 = phi i32 [ %op126, %label31 ], [ %op137, %label50 ]
  %op133 = phi i32 [ %op127, %label31 ], [ %op138, %label50 ]
  %op134 = phi i32 [ 0, %label31 ], [ %op52, %label50 ]
  %op37 = icmp slt i32 %op134, 2
  %op38 = zext i1 %op37 to i32
  %op39 = icmp ne i32 %op38, 0
  br i1 %op39, label %label40, label %label41
label40:                                                ; preds = %label35
  br label %label44
label41:                                                ; preds = %label35
  %op43 = add i32 %op129, 1
  br label %label26
label44:                                                ; preds = %label40, %label63
  %op135 = phi i32 [ %op130, %label40 ], [ %op139, %label63 ]
  %op136 = phi i32 [ %op131, %label40 ], [ %op140, %label63 ]
  %op137 = phi i32 [ %op132, %label40 ], [ %op141, %label63 ]
  %op138 = phi i32 [ 0, %label40 ], [ %op65, %label63 ]
  %op46 = icmp slt i32 %op138, 2
  %op47 = zext i1 %op46 to i32
  %op48 = icmp ne i32 %op47, 0
  br i1 %op48, label %label49, label %label50
label49:                                                ; preds = %label44
  br label %label53
label50:                                                ; preds = %label44
  %op52 = add i32 %op134, 1
  br label %label35
label53:                                                ; preds = %label49, %label67
  %op139 = phi i32 [ %op135, %label49 ], [ %op142, %label67 ]
  %op140 = phi i32 [ %op136, %label49 ], [ %op106, %label67 ]
  %op141 = phi i32 [ 0, %label49 ], [ %op108, %label67 ]
  %op55 = icmp slt i32 %op141, 2
  %op56 = zext i1 %op55 to i32
  %op57 = icmp ne i32 %op56, 0
  br i1 %op57, label %label58, label %label63
label58:                                                ; preds = %label53
  br i1 %op62, label %label66, label %label67
label63:                                                ; preds = %label53
  %op65 = add i32 %op138, 1
  br label %label44
label66:                                                ; preds = %label58
  br label %label67
label67:                                                ; preds = %label58, %label66
  %op142 = phi i32 [ %op139, %label58 ], [ 1, %label66 ]
  %op108 = add i32 %op141, 1
  br label %label53
}
