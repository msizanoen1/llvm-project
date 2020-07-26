; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt < %s -instsimplify -S | FileCheck %s

declare i8 @llvm.smax.i8(i8, i8)
declare <2 x i8> @llvm.smax.v2i8(<2 x i8>, <2 x i8>)
declare i8 @llvm.smin.i8(i8, i8)
declare <2 x i8> @llvm.smin.v2i8(<2 x i8>, <2 x i8>)
declare i8 @llvm.umax.i8(i8, i8)
declare <2 x i8> @llvm.umax.v2i8(<2 x i8>, <2 x i8>)
declare i8 @llvm.umin.i8(i8, i8)
declare <2 x i8> @llvm.umin.v2i8(<2 x i8>, <2 x i8>)

define i8 @smax_maxval(i8 %x) {
; CHECK-LABEL: @smax_maxval(
; CHECK-NEXT:    ret i8 127
;
  %r = call i8 @llvm.smax.i8(i8 %x, i8 127)
  ret i8 %r
}

define <2 x i8> @smax_maxval_commute(<2 x i8> %x) {
; CHECK-LABEL: @smax_maxval_commute(
; CHECK-NEXT:    ret <2 x i8> <i8 127, i8 127>
;
  %r = call <2 x i8> @llvm.smax.v2i8(<2 x i8> <i8 127, i8 127>, <2 x i8> %x)
  ret <2 x i8> %r
}

define i8 @smin_minval(i8 %x) {
; CHECK-LABEL: @smin_minval(
; CHECK-NEXT:    ret i8 -128
;
  %r = call i8 @llvm.smin.i8(i8 -128, i8 %x)
  ret i8 %r
}

define <2 x i8> @smin_minval_commute(<2 x i8> %x) {
; CHECK-LABEL: @smin_minval_commute(
; CHECK-NEXT:    ret <2 x i8> <i8 -128, i8 -128>
;
  %r = call <2 x i8> @llvm.smin.v2i8(<2 x i8> %x, <2 x i8> <i8 -128, i8 -128>)
  ret <2 x i8> %r
}

define i8 @umax_maxval(i8 %x) {
; CHECK-LABEL: @umax_maxval(
; CHECK-NEXT:    ret i8 -1
;
  %r = call i8 @llvm.umax.i8(i8 %x, i8 255)
  ret i8 %r
}

define <2 x i8> @umax_maxval_commute(<2 x i8> %x) {
; CHECK-LABEL: @umax_maxval_commute(
; CHECK-NEXT:    ret <2 x i8> <i8 -1, i8 -1>
;
  %r = call <2 x i8> @llvm.umax.v2i8(<2 x i8> <i8 255, i8 255>, <2 x i8> %x)
  ret <2 x i8> %r
}

define i8 @umin_minval(i8 %x) {
; CHECK-LABEL: @umin_minval(
; CHECK-NEXT:    ret i8 0
;
  %r = call i8 @llvm.umin.i8(i8 0, i8 %x)
  ret i8 %r
}

define <2 x i8> @umin_minval_commute(<2 x i8> %x) {
; CHECK-LABEL: @umin_minval_commute(
; CHECK-NEXT:    ret <2 x i8> zeroinitializer
;
  %r = call <2 x i8> @llvm.umin.v2i8(<2 x i8> %x, <2 x i8> zeroinitializer)
  ret <2 x i8> %r
}