; ModuleID = 'LLVM backend module Runtime'
target datalayout = "e-i64:64-f80:128-n8:16:32:64"

%EndLineHelperClass = type {}
%FlushHelperClass = type {}
%ConsoleOutputStream = type {}
%MainParameters = type { %CStrPtr, %CStrPtr }
%CStrPtr = type { %CStr* }
%CStr = type { i8* }
%struct._IO_FILE = type opaque
%StringRef = type { i8*, i8* }
%Location = type { %SourceCode, %LineCol, %LineCol }
%SourceCode = type { %UntypedPtr }
%UntypedPtr = type { i8* }
%LineCol = type { i32, i32 }
%ExternalErrorReporter = type {}
%String = type { %"RawPtr[Char/rtct]", %"RawPtr[Char/rtct]", %"RawPtr[Char/rtct]" }
%"RawPtr[Char/rtct]" = type { i8* }
%ScannerContext = type { %File, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]", %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]" }
%File = type { i8* }
%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]" = type { %Location, %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]", %Token, i1, i1, %ExternalErrorReporter }
%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]" = type { %"RangeWithLookahead[FileRange/rtct]", %Location* }
%"RangeWithLookahead[FileRange/rtct]" = type { %FileRange, %"Vector[Char/rtct]" }
%FileRange = type { %File*, i1, i8 }
%"Vector[Char/rtct]" = type { %"RawPtr[Char/rtct]", %"RawPtr[Char/rtct]", %"RawPtr[Char/rtct]" }
%Token = type { %Location, %TokenType, %String, i64, double }
%TokenType = type { i32 }
%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]" = type { %Location, %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]", %Token, i1, i1, %ExternalErrorReporter }
%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]" = type { %"RangeWithLookahead[StringRef/rtct]", %Location* }
%"RangeWithLookahead[StringRef/rtct]" = type { %StringRef, %"Vector[Char/rtct]" }
%StringOutputStream = type { %String }
%"ContiguousMemoryRange[Char/rtct]" = type { %"RawPtr[Char/rtct]", %"RawPtr[Char/rtct]" }
%"$lambdaEnclosure" = type {}
%"FunctionPtr[Bool/rtct, Char/rtct]" = type { i8* }
%"FunctionPtr[Bool, Char]" = type { i8* }
%"$lambdaEnclosure.0" = type {}
%StreamRefWrapperHelperClass = type { i8* }

@.fmtInt = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@.fmtUInt = private unnamed_addr constant [3 x i8] c"%u\00", align 1
@.fmtLong = private unnamed_addr constant [5 x i8] c"%lld\00", align 1
@.fmtULong = private unnamed_addr constant [5 x i8] c"%llu\00", align 1
@.fmtChar = private unnamed_addr constant [3 x i8] c"%c\00", align 1
@.fmtDouble = private unnamed_addr constant [3 x i8] c"%g\00", align 1
@.fmtPointer = private unnamed_addr constant [3 x i8] c"%p\00", align 1
@endl = global %EndLineHelperClass zeroinitializer
@flush = global %FlushHelperClass zeroinitializer
@cout = global %ConsoleOutputStream zeroinitializer
@programArgs = global %MainParameters zeroinitializer
@llvm.global_ctors = appending global [4 x { i32, void ()* }] [{ i32, void ()* } { i32 0, void ()* @__global_ctor }, { i32, void ()* } { i32 1, void ()* @__global_ctor82 }, { i32, void ()* } { i32 2, void ()* @__global_ctor86 }, { i32, void ()* } { i32 3, void ()* @__global_ctor104 }]
@llvm.global_dtors = appending global [4 x { i32, void ()* }] [{ i32, void ()* } { i32 0, void ()* @__global_dtor106 }, { i32, void ()* } { i32 1, void ()* @__global_dtor88 }, { i32, void ()* } { i32 2, void ()* @__global_dtor84 }, { i32, void ()* } { i32 3, void ()* @__global_dtor }]

; Function Attrs: alwaysinline
define void @_zero_init_1(i1* %"$this") #0 {
  store i1 false, i1* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_zero_init_8(i8* %"$this") #0 {
  store i8 0, i8* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_zero_init_16(i16* %"$this") #0 {
  store i16 0, i16* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_zero_init_32(i32* %"$this") #0 {
  store i32 0, i32* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_zero_init_64(i64* %"$this") #0 {
  store i64 0, i64* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_zero_init_f(float* %"$this") #0 {
  store float 0.000000e+00, float* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_zero_init_d(double* %"$this") #0 {
  store double 0.000000e+00, double* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_1_1(i1* %"$this", i1 %src) #0 {
  store i1 %src, i1* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_8_8(i8* %"$this", i8 %src) #0 {
  store i8 %src, i8* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_16_16(i16* %"$this", i16 %src) #0 {
  store i16 %src, i16* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_32_32(i32* %"$this", i32 %src) #0 {
  store i32 %src, i32* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_64_64(i64* %"$this", i64 %src) #0 {
  store i64 %src, i64* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_f_f(float* %"$this", float %src) #0 {
  store float %src, float* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_d_d(double* %"$this", double %src) #0 {
  store double %src, double* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_8_16(i8* %"$this", i16 %src) #0 {
  %1 = trunc i16 %src to i8
  store i8 %1, i8* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_8_32(i8* %"$this", i32 %src) #0 {
  %1 = trunc i32 %src to i8
  store i8 %1, i8* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_8_64(i8* %"$this", i64 %src) #0 {
  %1 = trunc i64 %src to i8
  store i8 %1, i8* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_16_32(i16* %"$this", i32 %src) #0 {
  %1 = trunc i32 %src to i16
  store i16 %1, i16* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_16_64(i16* %"$this", i64 %src) #0 {
  %1 = trunc i64 %src to i16
  store i16 %1, i16* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_32_64(i32* %"$this", i64 %src) #0 {
  %1 = trunc i64 %src to i32
  store i32 %1, i32* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_16_8s(i16* %"$this", i8 %src) #0 {
  %1 = sext i8 %src to i16
  store i16 %1, i16* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_16_8z(i16* %"$this", i8 %src) #0 {
  %1 = zext i8 %src to i16
  store i16 %1, i16* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_32_8s(i32* %"$this", i8 %src) #0 {
  %1 = sext i8 %src to i32
  store i32 %1, i32* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_32_8z(i32* %"$this", i8 %src) #0 {
  %1 = zext i8 %src to i32
  store i32 %1, i32* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_32_16s(i32* %"$this", i16 %src) #0 {
  %1 = sext i16 %src to i32
  store i32 %1, i32* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_32_16z(i32* %"$this", i16 %src) #0 {
  %1 = zext i16 %src to i32
  store i32 %1, i32* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_64_8s(i64* %"$this", i8 %src) #0 {
  %1 = sext i8 %src to i64
  store i64 %1, i64* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_64_8z(i64* %"$this", i8 %src) #0 {
  %1 = zext i8 %src to i64
  store i64 %1, i64* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_64_16s(i64* %"$this", i16 %src) #0 {
  %1 = sext i16 %src to i64
  store i64 %1, i64* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_64_16z(i64* %"$this", i16 %src) #0 {
  %1 = zext i16 %src to i64
  store i64 %1, i64* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_64_32s(i64* %"$this", i32 %src) #0 {
  %1 = sext i32 %src to i64
  store i64 %1, i64* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_64_32z(i64* %"$this", i32 %src) #0 {
  %1 = zext i32 %src to i64
  store i64 %1, i64* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_i8_f(i8* %"$this", float %src) #0 {
  %1 = fptosi float %src to i8
  store i8 %1, i8* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_i8_d(i8* %"$this", double %src) #0 {
  %1 = fptosi double %src to i8
  store i8 %1, i8* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_u8_f(i8* %"$this", float %src) #0 {
  %1 = fptoui float %src to i8
  store i8 %1, i8* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_u8_d(i8* %"$this", double %src) #0 {
  %1 = fptoui double %src to i8
  store i8 %1, i8* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_i16_f(i16* %"$this", float %src) #0 {
  %1 = fptosi float %src to i16
  store i16 %1, i16* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_i16_d(i16* %"$this", double %src) #0 {
  %1 = fptosi double %src to i16
  store i16 %1, i16* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_u16_f(i16* %"$this", float %src) #0 {
  %1 = fptoui float %src to i16
  store i16 %1, i16* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_u16_d(i16* %"$this", double %src) #0 {
  %1 = fptoui double %src to i16
  store i16 %1, i16* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_i32_f(i32* %"$this", float %src) #0 {
  %1 = fptosi float %src to i32
  store i32 %1, i32* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_i32_d(i32* %"$this", double %src) #0 {
  %1 = fptosi double %src to i32
  store i32 %1, i32* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_u32_f(i32* %"$this", float %src) #0 {
  %1 = fptoui float %src to i32
  store i32 %1, i32* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_u32_d(i32* %"$this", double %src) #0 {
  %1 = fptoui double %src to i32
  store i32 %1, i32* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_i64_f(i64* %"$this", float %src) #0 {
  %1 = fptosi float %src to i64
  store i64 %1, i64* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_i64_d(i64* %"$this", double %src) #0 {
  %1 = fptosi double %src to i64
  store i64 %1, i64* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_u64_f(i64* %"$this", float %src) #0 {
  %1 = fptoui float %src to i64
  store i64 %1, i64* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_u64_d(i64* %"$this", double %src) #0 {
  %1 = fptoui double %src to i64
  store i64 %1, i64* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_f_i8(float* %"$this", i8 %src) #0 {
  %1 = sitofp i8 %src to float
  store float %1, float* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_f_u8(float* %"$this", i8 %src) #0 {
  %1 = uitofp i8 %src to float
  store float %1, float* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_f_i16(float* %"$this", i16 %src) #0 {
  %1 = sitofp i16 %src to float
  store float %1, float* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_f_u16(float* %"$this", i16 %src) #0 {
  %1 = uitofp i16 %src to float
  store float %1, float* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_f_i32(float* %"$this", i32 %src) #0 {
  %1 = sitofp i32 %src to float
  store float %1, float* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_f_u32(float* %"$this", i32 %src) #0 {
  %1 = uitofp i32 %src to float
  store float %1, float* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_f_i64(float* %"$this", i64 %src) #0 {
  %1 = sitofp i64 %src to float
  store float %1, float* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_f_u64(float* %"$this", i64 %src) #0 {
  %1 = uitofp i64 %src to float
  store float %1, float* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_f_d(float* %"$this", double %src) #0 {
  %1 = fptrunc double %src to float
  store float %1, float* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_d_i8(double* %"$this", i8 %src) #0 {
  %1 = sitofp i8 %src to double
  store double %1, double* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_d_u8(double* %"$this", i8 %src) #0 {
  %1 = uitofp i8 %src to double
  store double %1, double* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_d_i16(double* %"$this", i16 %src) #0 {
  %1 = sitofp i16 %src to double
  store double %1, double* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_d_u16(double* %"$this", i16 %src) #0 {
  %1 = uitofp i16 %src to double
  store double %1, double* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_d_i32(double* %"$this", i32 %src) #0 {
  %1 = sitofp i32 %src to double
  store double %1, double* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_d_u32(double* %"$this", i32 %src) #0 {
  %1 = uitofp i32 %src to double
  store double %1, double* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_d_i64(double* %"$this", i64 %src) #0 {
  %1 = sitofp i64 %src to double
  store double %1, double* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_d_u64(double* %"$this", i64 %src) #0 {
  %1 = uitofp i64 %src to double
  store double %1, double* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define void @_ass_d_f(double* %"$this", float %src) #0 {
  %1 = fpext float %src to double
  store double %1, double* %"$this"
  ret void
}

; Function Attrs: alwaysinline
define i1 @_Bool_opAssign(i1* %x, i1 %y) #0 {
  store i1 %y, i1* %x
  ret i1 %y
}

; Function Attrs: alwaysinline
define i1 @_Bool_opOr(i1 %x, i1 %y) #0 {
  %1 = or i1 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Bool_opXor(i1 %x, i1 %y) #0 {
  %1 = xor i1 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Bool_opAnd(i1 %x, i1 %y) #0 {
  %1 = and i1 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Bool_opEQ(i1 %x, i1 %y) #0 {
  %1 = icmp eq i1 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Bool_opNE(i1 %x, i1 %y) #0 {
  %1 = icmp ne i1 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Bool_opNeg(i1 %x) #0 {
  %1 = xor i1 %x, true
  ret i1 %1
}

; Function Attrs: alwaysinline
define i8 @_Byte_opAssign(i8* %x, i8 %y) #0 {
  store i8 %y, i8* %x
  ret i8 %y
}

; Function Attrs: alwaysinline
define i1 @_Byte_opEQ(i8 %x, i8 %y) #0 {
  %1 = icmp eq i8 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Byte_opNE(i8 %x, i8 %y) #0 {
  %1 = icmp ne i8 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Byte_opLT(i8 %x, i8 %y) #0 {
  %1 = icmp slt i8 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Byte_opGT(i8 %x, i8 %y) #0 {
  %1 = icmp sgt i8 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Byte_opLE(i8 %x, i8 %y) #0 {
  %1 = icmp sle i8 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Byte_opGE(i8 %x, i8 %y) #0 {
  %1 = icmp sge i8 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i8 @_Byte_opPlus(i8 %x, i8 %y) #0 {
  %1 = add i8 %x, %y
  ret i8 %1
}

; Function Attrs: alwaysinline
define i8 @_Byte_opMinus(i8 %x, i8 %y) #0 {
  %1 = sub i8 %x, %y
  ret i8 %1
}

; Function Attrs: alwaysinline
define i8 @_Byte_opMul(i8 %x, i8 %y) #0 {
  %1 = mul i8 %x, %y
  ret i8 %1
}

; Function Attrs: alwaysinline
define i8 @_Byte_opDiv(i8 %x, i8 %y) #0 {
  %1 = sdiv i8 %x, %y
  ret i8 %1
}

; Function Attrs: alwaysinline
define i8 @_Byte_opMod(i8 %x, i8 %y) #0 {
  %1 = srem i8 %x, %y
  ret i8 %1
}

; Function Attrs: alwaysinline
define i8 @_Byte_opPlus1(i8 %x) #0 {
  ret i8 %x
}

; Function Attrs: alwaysinline
define i8 @_Byte_opMinus1(i8 %x) #0 {
  %1 = sub i8 0, %x
  ret i8 %1
}

; Function Attrs: alwaysinline
define i8 @_UByte_opAssign(i8* %x, i8 %y) #0 {
  store i8 %y, i8* %x
  ret i8 %y
}

; Function Attrs: alwaysinline
define i1 @_UByte_opEQ(i8 %x, i8 %y) #0 {
  %1 = icmp eq i8 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_UByte_opNE(i8 %x, i8 %y) #0 {
  %1 = icmp ne i8 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_UByte_opLT(i8 %x, i8 %y) #0 {
  %1 = icmp ult i8 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_UByte_opGT(i8 %x, i8 %y) #0 {
  %1 = icmp ugt i8 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_UByte_opLE(i8 %x, i8 %y) #0 {
  %1 = icmp ule i8 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_UByte_opGE(i8 %x, i8 %y) #0 {
  %1 = icmp uge i8 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i8 @_UByte_opPlus(i8 %x, i8 %y) #0 {
  %1 = add i8 %x, %y
  ret i8 %1
}

; Function Attrs: alwaysinline
define i8 @_UByte_opMinus(i8 %x, i8 %y) #0 {
  %1 = sub i8 %x, %y
  ret i8 %1
}

; Function Attrs: alwaysinline
define i8 @_UByte_opMul(i8 %x, i8 %y) #0 {
  %1 = mul i8 %x, %y
  ret i8 %1
}

; Function Attrs: alwaysinline
define i8 @_UByte_opDiv(i8 %x, i8 %y) #0 {
  %1 = udiv i8 %x, %y
  ret i8 %1
}

; Function Attrs: alwaysinline
define i8 @_UByte_opMod(i8 %x, i8 %y) #0 {
  %1 = urem i8 %x, %y
  ret i8 %1
}

; Function Attrs: alwaysinline
define i8 @_UByte_opPlus1(i8 %x) #0 {
  ret i8 %x
}

; Function Attrs: alwaysinline
define i16 @_Short_opAssign(i16* %x, i16 %y) #0 {
  store i16 %y, i16* %x
  ret i16 %y
}

; Function Attrs: alwaysinline
define i1 @_Short_opEQ(i16 %x, i16 %y) #0 {
  %1 = icmp eq i16 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Short_opNE(i16 %x, i16 %y) #0 {
  %1 = icmp ne i16 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Short_opLT(i16 %x, i16 %y) #0 {
  %1 = icmp slt i16 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Short_opGT(i16 %x, i16 %y) #0 {
  %1 = icmp sgt i16 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Short_opLE(i16 %x, i16 %y) #0 {
  %1 = icmp sle i16 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Short_opGE(i16 %x, i16 %y) #0 {
  %1 = icmp sge i16 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i16 @_Short_opPlus(i16 %x, i16 %y) #0 {
  %1 = add i16 %x, %y
  ret i16 %1
}

; Function Attrs: alwaysinline
define i16 @_Short_opMinus(i16 %x, i16 %y) #0 {
  %1 = sub i16 %x, %y
  ret i16 %1
}

; Function Attrs: alwaysinline
define i16 @_Short_opMul(i16 %x, i16 %y) #0 {
  %1 = mul i16 %x, %y
  ret i16 %1
}

; Function Attrs: alwaysinline
define i16 @_Short_opDiv(i16 %x, i16 %y) #0 {
  %1 = sdiv i16 %x, %y
  ret i16 %1
}

; Function Attrs: alwaysinline
define i16 @_Short_opMod(i16 %x, i16 %y) #0 {
  %1 = srem i16 %x, %y
  ret i16 %1
}

; Function Attrs: alwaysinline
define i16 @_Short_opPlus1(i16 %x) #0 {
  ret i16 %x
}

; Function Attrs: alwaysinline
define i16 @_Short_opMinus1(i16 %x) #0 {
  %1 = sub i16 0, %x
  ret i16 %1
}

; Function Attrs: alwaysinline
define i16 @_UShort_opAssign(i16* %x, i16 %y) #0 {
  store i16 %y, i16* %x
  ret i16 %y
}

; Function Attrs: alwaysinline
define i1 @_UShort_opEQ(i16 %x, i16 %y) #0 {
  %1 = icmp eq i16 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_UShort_opNE(i16 %x, i16 %y) #0 {
  %1 = icmp ne i16 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_UShort_opLT(i16 %x, i16 %y) #0 {
  %1 = icmp ult i16 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_UShort_opGT(i16 %x, i16 %y) #0 {
  %1 = icmp ugt i16 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_UShort_opLE(i16 %x, i16 %y) #0 {
  %1 = icmp ule i16 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_UShort_opGE(i16 %x, i16 %y) #0 {
  %1 = icmp uge i16 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i16 @_UShort_opPlus(i16 %x, i16 %y) #0 {
  %1 = add i16 %x, %y
  ret i16 %1
}

; Function Attrs: alwaysinline
define i16 @_UShort_opMinus(i16 %x, i16 %y) #0 {
  %1 = sub i16 %x, %y
  ret i16 %1
}

; Function Attrs: alwaysinline
define i16 @_UShort_opMul(i16 %x, i16 %y) #0 {
  %1 = mul i16 %x, %y
  ret i16 %1
}

; Function Attrs: alwaysinline
define i16 @_UShort_opDiv(i16 %x, i16 %y) #0 {
  %1 = udiv i16 %x, %y
  ret i16 %1
}

; Function Attrs: alwaysinline
define i16 @_UShort_opMod(i16 %x, i16 %y) #0 {
  %1 = urem i16 %x, %y
  ret i16 %1
}

; Function Attrs: alwaysinline
define i16 @_UShort_opPlus1(i16 %x) #0 {
  ret i16 %x
}

; Function Attrs: alwaysinline
define i32 @_Int_opAssign(i32* %x, i32 %y) #0 {
  store i32 %y, i32* %x
  ret i32 %y
}

; Function Attrs: alwaysinline
define i1 @_Int_opEQ(i32 %x, i32 %y) #0 {
  %1 = icmp eq i32 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Int_opNE(i32 %x, i32 %y) #0 {
  %1 = icmp ne i32 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Int_opLT(i32 %x, i32 %y) #0 {
  %1 = icmp slt i32 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Int_opGT(i32 %x, i32 %y) #0 {
  %1 = icmp sgt i32 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Int_opLE(i32 %x, i32 %y) #0 {
  %1 = icmp sle i32 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Int_opGE(i32 %x, i32 %y) #0 {
  %1 = icmp sge i32 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i32 @_Int_opPlus(i32 %x, i32 %y) #0 {
  %1 = add i32 %x, %y
  ret i32 %1
}

; Function Attrs: alwaysinline
define i32 @_Int_opMinus(i32 %x, i32 %y) #0 {
  %1 = sub i32 %x, %y
  ret i32 %1
}

; Function Attrs: alwaysinline
define i32 @_Int_opMul(i32 %x, i32 %y) #0 {
  %1 = mul i32 %x, %y
  ret i32 %1
}

; Function Attrs: alwaysinline
define i32 @_Int_opDiv(i32 %x, i32 %y) #0 {
  %1 = sdiv i32 %x, %y
  ret i32 %1
}

; Function Attrs: alwaysinline
define i32 @_Int_opMod(i32 %x, i32 %y) #0 {
  %1 = srem i32 %x, %y
  ret i32 %1
}

; Function Attrs: alwaysinline
define i32 @_Int_opPlus1(i32 %x) #0 {
  ret i32 %x
}

; Function Attrs: alwaysinline
define i32 @_Int_opMinus1(i32 %x) #0 {
  %1 = sub i32 0, %x
  ret i32 %1
}

; Function Attrs: alwaysinline
define i32 @_UInt_opAssign(i32* %x, i32 %y) #0 {
  store i32 %y, i32* %x
  ret i32 %y
}

; Function Attrs: alwaysinline
define i1 @_UInt_opEQ(i32 %x, i32 %y) #0 {
  %1 = icmp eq i32 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_UInt_opNE(i32 %x, i32 %y) #0 {
  %1 = icmp ne i32 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_UInt_opLT(i32 %x, i32 %y) #0 {
  %1 = icmp ult i32 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_UInt_opGT(i32 %x, i32 %y) #0 {
  %1 = icmp ugt i32 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_UInt_opLE(i32 %x, i32 %y) #0 {
  %1 = icmp ule i32 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_UInt_opGE(i32 %x, i32 %y) #0 {
  %1 = icmp uge i32 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i32 @_UInt_opPlus(i32 %x, i32 %y) #0 {
  %1 = add i32 %x, %y
  ret i32 %1
}

; Function Attrs: alwaysinline
define i32 @_UInt_opMinus(i32 %x, i32 %y) #0 {
  %1 = sub i32 %x, %y
  ret i32 %1
}

; Function Attrs: alwaysinline
define i32 @_UInt_opMul(i32 %x, i32 %y) #0 {
  %1 = mul i32 %x, %y
  ret i32 %1
}

; Function Attrs: alwaysinline
define i32 @_UInt_opDiv(i32 %x, i32 %y) #0 {
  %1 = udiv i32 %x, %y
  ret i32 %1
}

; Function Attrs: alwaysinline
define i32 @_UInt_opMod(i32 %x, i32 %y) #0 {
  %1 = urem i32 %x, %y
  ret i32 %1
}

; Function Attrs: alwaysinline
define i32 @_UInt_opPlus1(i32 %x) #0 {
  ret i32 %x
}

; Function Attrs: alwaysinline
define i64 @_Long_opAssign(i64* %x, i64 %y) #0 {
  store i64 %y, i64* %x
  ret i64 %y
}

; Function Attrs: alwaysinline
define i1 @_Long_opEQ(i64 %x, i64 %y) #0 {
  %1 = icmp eq i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Long_opNE(i64 %x, i64 %y) #0 {
  %1 = icmp ne i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Long_opLT(i64 %x, i64 %y) #0 {
  %1 = icmp slt i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Long_opGT(i64 %x, i64 %y) #0 {
  %1 = icmp sgt i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Long_opLE(i64 %x, i64 %y) #0 {
  %1 = icmp sle i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Long_opGE(i64 %x, i64 %y) #0 {
  %1 = icmp sge i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i64 @_Long_opPlus(i64 %x, i64 %y) #0 {
  %1 = add i64 %x, %y
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_Long_opMinus(i64 %x, i64 %y) #0 {
  %1 = sub i64 %x, %y
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_Long_opMul(i64 %x, i64 %y) #0 {
  %1 = mul i64 %x, %y
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_Long_opDiv(i64 %x, i64 %y) #0 {
  %1 = sdiv i64 %x, %y
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_Long_opMod(i64 %x, i64 %y) #0 {
  %1 = srem i64 %x, %y
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_Long_opPlus1(i64 %x) #0 {
  ret i64 %x
}

; Function Attrs: alwaysinline
define i64 @_Long_opMinus1(i64 %x) #0 {
  %1 = sub i64 0, %x
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_ULong_opAssign(i64* %x, i64 %y) #0 {
  store i64 %y, i64* %x
  ret i64 %y
}

; Function Attrs: alwaysinline
define i1 @_ULong_opEQ(i64 %x, i64 %y) #0 {
  %1 = icmp eq i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_ULong_opNE(i64 %x, i64 %y) #0 {
  %1 = icmp ne i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_ULong_opLT(i64 %x, i64 %y) #0 {
  %1 = icmp ult i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_ULong_opGT(i64 %x, i64 %y) #0 {
  %1 = icmp ugt i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_ULong_opLE(i64 %x, i64 %y) #0 {
  %1 = icmp ule i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_ULong_opGE(i64 %x, i64 %y) #0 {
  %1 = icmp uge i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i64 @_ULong_opPlus(i64 %x, i64 %y) #0 {
  %1 = add i64 %x, %y
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_ULong_opMinus(i64 %x, i64 %y) #0 {
  %1 = sub i64 %x, %y
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_ULong_opMul(i64 %x, i64 %y) #0 {
  %1 = mul i64 %x, %y
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_ULong_opDiv(i64 %x, i64 %y) #0 {
  %1 = udiv i64 %x, %y
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_ULong_opMod(i64 %x, i64 %y) #0 {
  %1 = urem i64 %x, %y
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_ULong_opPlus1(i64 %x) #0 {
  ret i64 %x
}

; Function Attrs: alwaysinline
define float @_Float_opAssign(float* %x, float %y) #0 {
  store float %y, float* %x
  ret float %y
}

; Function Attrs: alwaysinline
define i1 @_Float_opEQ(float %x, float %y) #0 {
  %1 = fcmp ueq float %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Float_opNE(float %x, float %y) #0 {
  %1 = fcmp une float %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Float_opLT(float %x, float %y) #0 {
  %1 = fcmp ult float %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Float_opGT(float %x, float %y) #0 {
  %1 = fcmp ugt float %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Float_opLE(float %x, float %y) #0 {
  %1 = fcmp ule float %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Float_opGE(float %x, float %y) #0 {
  %1 = fcmp uge float %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define float @_Float_opPlus(float %x, float %y) #0 {
  %1 = fadd float %x, %y
  ret float %1
}

; Function Attrs: alwaysinline
define float @_Float_opMinus(float %x, float %y) #0 {
  %1 = fsub float %x, %y
  ret float %1
}

; Function Attrs: alwaysinline
define float @_Float_opMul(float %x, float %y) #0 {
  %1 = fmul float %x, %y
  ret float %1
}

; Function Attrs: alwaysinline
define float @_Float_opDiv(float %x, float %y) #0 {
  %1 = fdiv float %x, %y
  ret float %1
}

; Function Attrs: alwaysinline
define float @_Float_opPlus1(float %x) #0 {
  ret float %x
}

; Function Attrs: alwaysinline
define float @_Float_opMinus1(float %x) #0 {
  %1 = fsub float 0.000000e+00, %x
  ret float %1
}

; Function Attrs: alwaysinline
define double @_Double_opAssign(double* %x, double %y) #0 {
  store double %y, double* %x
  ret double %y
}

; Function Attrs: alwaysinline
define i1 @_Double_opEQ(double %x, double %y) #0 {
  %1 = fcmp ueq double %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Double_opNE(double %x, double %y) #0 {
  %1 = fcmp une double %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Double_opLT(double %x, double %y) #0 {
  %1 = fcmp ult double %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Double_opGT(double %x, double %y) #0 {
  %1 = fcmp ugt double %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Double_opLE(double %x, double %y) #0 {
  %1 = fcmp ule double %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Double_opGE(double %x, double %y) #0 {
  %1 = fcmp uge double %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define double @_Double_opPlus(double %x, double %y) #0 {
  %1 = fadd double %x, %y
  ret double %1
}

; Function Attrs: alwaysinline
define double @_Double_opMinus(double %x, double %y) #0 {
  %1 = fsub double %x, %y
  ret double %1
}

; Function Attrs: alwaysinline
define double @_Double_opMul(double %x, double %y) #0 {
  %1 = fmul double %x, %y
  ret double %1
}

; Function Attrs: alwaysinline
define double @_Double_opDiv(double %x, double %y) #0 {
  %1 = fdiv double %x, %y
  ret double %1
}

; Function Attrs: alwaysinline
define double @_Double_opPlus1(double %x) #0 {
  ret double %x
}

; Function Attrs: alwaysinline
define double @_Double_opMinus1(double %x) #0 {
  %1 = fsub double 0.000000e+00, %x
  ret double %1
}

; Function Attrs: alwaysinline
define i8 @_Char_opAssign(i8* %x, i8 %y) #0 {
  store i8 %y, i8* %x
  ret i8 %y
}

; Function Attrs: alwaysinline
define i1 @_Char_opEQ(i8 %x, i8 %y) #0 {
  %1 = icmp eq i8 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Char_opNE(i8 %x, i8 %y) #0 {
  %1 = icmp ne i8 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Char_opLT(i8 %x, i8 %y) #0 {
  %1 = icmp slt i8 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Char_opGT(i8 %x, i8 %y) #0 {
  %1 = icmp sgt i8 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Char_opLE(i8 %x, i8 %y) #0 {
  %1 = icmp sle i8 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_Char_opGE(i8 %x, i8 %y) #0 {
  %1 = icmp sge i8 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i64 @_SizeType_opAssign(i64* %x, i64 %y) #0 {
  store i64 %y, i64* %x
  ret i64 %y
}

; Function Attrs: alwaysinline
define i1 @_SizeType_opEQ(i64 %x, i64 %y) #0 {
  %1 = icmp eq i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_SizeType_opNE(i64 %x, i64 %y) #0 {
  %1 = icmp ne i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_SizeType_opLT(i64 %x, i64 %y) #0 {
  %1 = icmp ult i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_SizeType_opGT(i64 %x, i64 %y) #0 {
  %1 = icmp ugt i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_SizeType_opLE(i64 %x, i64 %y) #0 {
  %1 = icmp ule i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_SizeType_opGE(i64 %x, i64 %y) #0 {
  %1 = icmp uge i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i64 @_SizeType_opPlus(i64 %x, i64 %y) #0 {
  %1 = add i64 %x, %y
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_SizeType_opPlusInt(i64 %x, i32 %y) #0 {
  %1 = sext i32 %y to i64
  %2 = add i64 %x, %1
  ret i64 %2
}

; Function Attrs: alwaysinline
define i64 @_SizeType_opPlusInt1(i32 %x, i64 %y) #0 {
  %1 = sext i32 %x to i64
  %2 = add i64 %1, %y
  ret i64 %2
}

; Function Attrs: alwaysinline
define i64 @_SizeType_opMinus(i64 %x, i64 %y) #0 {
  %1 = sub i64 %x, %y
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_SizeType_opMinusInt(i64 %x, i32 %y) #0 {
  %1 = sext i32 %y to i64
  %2 = sub i64 %x, %1
  ret i64 %2
}

; Function Attrs: alwaysinline
define i64 @_SizeType_opMinusInt1(i32 %x, i64 %y) #0 {
  %1 = sext i32 %x to i64
  %2 = sub i64 %1, %y
  ret i64 %2
}

; Function Attrs: alwaysinline
define i64 @_SizeType_opMul(i64 %x, i64 %y) #0 {
  %1 = mul i64 %x, %y
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_SizeType_opDiv(i64 %x, i64 %y) #0 {
  %1 = udiv i64 %x, %y
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_SizeType_opMod(i64 %x, i64 %y) #0 {
  %1 = urem i64 %x, %y
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_SizeType_opPlus1(i64 %x) #0 {
  ret i64 %x
}

; Function Attrs: alwaysinline
define i64 @_DiffType_opAssign(i64* %x, i64 %y) #0 {
  store i64 %y, i64* %x
  ret i64 %y
}

; Function Attrs: alwaysinline
define i1 @_DiffType_opEQ(i64 %x, i64 %y) #0 {
  %1 = icmp eq i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_DiffType_opNE(i64 %x, i64 %y) #0 {
  %1 = icmp ne i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_DiffType_opLT(i64 %x, i64 %y) #0 {
  %1 = icmp slt i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_DiffType_opGT(i64 %x, i64 %y) #0 {
  %1 = icmp sgt i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_DiffType_opLE(i64 %x, i64 %y) #0 {
  %1 = icmp sle i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_DiffType_opGE(i64 %x, i64 %y) #0 {
  %1 = icmp sge i64 %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i64 @_DiffType_opPlus(i64 %x, i64 %y) #0 {
  %1 = add i64 %x, %y
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_DiffType_opPlusInt(i64 %x, i32 %y) #0 {
  %1 = sext i32 %y to i64
  %2 = add i64 %x, %1
  ret i64 %2
}

; Function Attrs: alwaysinline
define i64 @_DiffType_opPlusInt1(i32 %x, i64 %y) #0 {
  %1 = sext i32 %x to i64
  %2 = add i64 %1, %y
  ret i64 %2
}

; Function Attrs: alwaysinline
define i64 @_DiffType_opMinus(i64 %x, i64 %y) #0 {
  %1 = sub i64 %x, %y
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_DiffType_opMinusInt(i64 %x, i32 %y) #0 {
  %1 = sext i32 %y to i64
  %2 = sub i64 %x, %1
  ret i64 %2
}

; Function Attrs: alwaysinline
define i64 @_DiffType_opMinusInt1(i32 %x, i64 %y) #0 {
  %1 = sext i32 %x to i64
  %2 = sub i64 %1, %y
  ret i64 %2
}

; Function Attrs: alwaysinline
define i64 @_DiffType_opMul(i64 %x, i64 %y) #0 {
  %1 = mul i64 %x, %y
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_DiffType_opDiv(i64 %x, i64 %y) #0 {
  %1 = sdiv i64 %x, %y
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_DiffType_opMod(i64 %x, i64 %y) #0 {
  %1 = srem i64 %x, %y
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_DiffType_opPlus1(i64 %x) #0 {
  ret i64 %x
}

; Function Attrs: alwaysinline
define i64 @_DiffType_opMinus1(i64 %x) #0 {
  %1 = sub i64 0, %x
  ret i64 %1
}

declare i32 @printf(i8*, ...)

; Function Attrs: nounwind
declare i32 @sprintf(i8*, i8*, ...) #1

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #1

; Function Attrs: nounwind
declare void @llvm.memmove.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #1

; Function Attrs: nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture, i8, i64, i32, i1) #1

; Function Attrs: nounwind
declare i32 @fflush(%struct._IO_FILE*) #1

; Function Attrs: alwaysinline
define i1 @isNullRef(i8* %x) #0 {
  %1 = icmp eq i8* %x, null
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_opRefEQ(i8* %x, i8* %y) #0 {
  %1 = icmp eq i8* %x, %y
  ret i1 %1
}

; Function Attrs: alwaysinline
define i1 @_opRefNE(i8* %x, i8* %y) #0 {
  %1 = icmp ne i8* %x, %y
  ret i1 %1
}

define void @_Type_ctor(i8** %"$this") {
  store i8* null, i8** %"$this"
  ret void
}

define void @_Type_copy_ctor(i8** %"$this", i8* %other) {
  store i8* %other, i8** %"$this"
  ret void
}

; Function Attrs: alwaysinline
define i8* @ptrAdd(i8* %p, i64 %n) #0 {
  %1 = getelementptr i8* %p, i64 %n
  ret i8* %1
}

; Function Attrs: alwaysinline
define i8* @ptrSub(i8* %p, i64 %n) #0 {
  %1 = sub i64 0, %n
  %2 = getelementptr i8* %p, i64 %1
  ret i8* %2
}

; Function Attrs: alwaysinline
define i64 @ptrDiff(i8* %p1, i8* %p2) #0 {
  %1 = ptrtoint i8* %p1 to i64
  %2 = ptrtoint i8* %p2 to i64
  %3 = sub i64 %1, %2
  ret i64 %3
}

; Function Attrs: alwaysinline
define void @_spr_memcpy(i8* %dest, i8* %src, i64 %size) #0 {
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %dest, i8* %src, i64 %size, i32 1, i1 false)
  ret void
}

; Function Attrs: alwaysinline
define void @_spr_memmove(i8* %dest, i8* %src, i64 %size) #0 {
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %dest, i8* %src, i64 %size, i32 1, i1 false)
  ret void
}

; Function Attrs: alwaysinline
define void @_spr_memset(i8* %dest, i8 %value, i64 %size) #0 {
  call void @llvm.memset.p0i8.i64(i8* %dest, i8 %value, i64 %size, i32 1, i1 false)
  ret void
}

; Function Attrs: alwaysinline
define void @flushOutput() #0 {
  %1 = call i32 @fflush(%struct._IO_FILE* null)
  ret void
}

; Function Attrs: alwaysinline
define void @writeByte(i8 %val) #0 {
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([3 x i8]* @.fmtInt, i32 0, i32 0), i8 %val)
  ret void
}

; Function Attrs: alwaysinline
define void @writeShort(i16 %val) #0 {
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([3 x i8]* @.fmtInt, i32 0, i32 0), i16 %val)
  ret void
}

; Function Attrs: alwaysinline
define void @writeInt(i32 %val) #0 {
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([3 x i8]* @.fmtInt, i32 0, i32 0), i32 %val)
  ret void
}

; Function Attrs: alwaysinline
define void @writeUInt(i32 %val) #0 {
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([3 x i8]* @.fmtUInt, i32 0, i32 0), i32 %val)
  ret void
}

; Function Attrs: alwaysinline
define void @writeLong(i64 %val) #0 {
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([5 x i8]* @.fmtLong, i32 0, i32 0), i64 %val)
  ret void
}

; Function Attrs: alwaysinline
define void @writeULong(i64 %val) #0 {
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([5 x i8]* @.fmtULong, i32 0, i32 0), i64 %val)
  ret void
}

; Function Attrs: alwaysinline
define void @writeDouble(double %val) #0 {
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([3 x i8]* @.fmtDouble, i32 0, i32 0), double %val)
  ret void
}

; Function Attrs: alwaysinline
define void @writeChar(i8 %val) #0 {
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([3 x i8]* @.fmtChar, i32 0, i32 0), i8 %val)
  ret void
}

; Function Attrs: alwaysinline
define void @writePointer(i8* %val) #0 {
  %1 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([3 x i8]* @.fmtPointer, i32 0, i32 0), i8* %val)
  ret void
}

; Function Attrs: alwaysinline
define void @_Int_to_CString(i32 %val, i8* %buffer) #0 {
  %1 = call i32 (i8*, i8*, ...)* @sprintf(i8* %buffer, i8* getelementptr inbounds ([3 x i8]* @.fmtInt, i32 0, i32 0), i32 %val) #1
  ret void
}

; Function Attrs: alwaysinline
define void @_ULong_to_CString(i64 %val, i8* %buffer) #0 {
  %1 = call i32 (i8*, i8*, ...)* @sprintf(i8* %buffer, i8* getelementptr inbounds ([5 x i8]* @.fmtULong, i32 0, i32 0), i64 %val) #1
  ret void
}

; Function Attrs: alwaysinline
define i8 @_bit_neg_8(i8 %n) #0 {
  %1 = xor i8 %n, 0
  ret i8 %1
}

; Function Attrs: alwaysinline
define i8 @_bit_shl_8(i8 %n, i8 %k) #0 {
  %1 = shl i8 %n, %k
  ret i8 %1
}

; Function Attrs: alwaysinline
define i8 @_bit_lshr_8(i8 %n, i8 %k) #0 {
  %1 = lshr i8 %n, %k
  ret i8 %1
}

; Function Attrs: alwaysinline
define i8 @_bit_ashr_8(i8 %n, i8 %k) #0 {
  %1 = ashr i8 %n, %k
  ret i8 %1
}

; Function Attrs: alwaysinline
define i8 @_bit_xor_8(i8 %n1, i8 %n2) #0 {
  %1 = xor i8 %n1, %n2
  ret i8 %1
}

; Function Attrs: alwaysinline
define i8 @_bit_or_8(i8 %n1, i8 %n2) #0 {
  %1 = or i8 %n1, %n2
  ret i8 %1
}

; Function Attrs: alwaysinline
define i8 @_bit_and_8(i8 %n1, i8 %n2) #0 {
  %1 = and i8 %n1, %n2
  ret i8 %1
}

; Function Attrs: alwaysinline
define i16 @_bit_neg_16(i16 %n) #0 {
  %1 = xor i16 %n, 0
  ret i16 %1
}

; Function Attrs: alwaysinline
define i16 @_bit_shl_16(i16 %n, i16 %k) #0 {
  %1 = shl i16 %n, %k
  ret i16 %1
}

; Function Attrs: alwaysinline
define i16 @_bit_lshr_16(i16 %n, i16 %k) #0 {
  %1 = lshr i16 %n, %k
  ret i16 %1
}

; Function Attrs: alwaysinline
define i16 @_bit_ashr_16(i16 %n, i16 %k) #0 {
  %1 = ashr i16 %n, %k
  ret i16 %1
}

; Function Attrs: alwaysinline
define i16 @_bit_xor_16(i16 %n1, i16 %n2) #0 {
  %1 = xor i16 %n1, %n2
  ret i16 %1
}

; Function Attrs: alwaysinline
define i16 @_bit_or_16(i16 %n1, i16 %n2) #0 {
  %1 = or i16 %n1, %n2
  ret i16 %1
}

; Function Attrs: alwaysinline
define i16 @_bit_and_16(i16 %n1, i16 %n2) #0 {
  %1 = and i16 %n1, %n2
  ret i16 %1
}

; Function Attrs: alwaysinline
define i32 @_bit_neg_32(i32 %n) #0 {
  %1 = xor i32 %n, 0
  ret i32 %1
}

; Function Attrs: alwaysinline
define i32 @_bit_shl_32(i32 %n, i32 %k) #0 {
  %1 = shl i32 %n, %k
  ret i32 %1
}

; Function Attrs: alwaysinline
define i32 @_bit_lshr_32(i32 %n, i32 %k) #0 {
  %1 = lshr i32 %n, %k
  ret i32 %1
}

; Function Attrs: alwaysinline
define i32 @_bit_ashr_32(i32 %n, i32 %k) #0 {
  %1 = ashr i32 %n, %k
  ret i32 %1
}

; Function Attrs: alwaysinline
define i32 @_bit_xor_32(i32 %n1, i32 %n2) #0 {
  %1 = xor i32 %n1, %n2
  ret i32 %1
}

; Function Attrs: alwaysinline
define i32 @_bit_or_32(i32 %n1, i32 %n2) #0 {
  %1 = or i32 %n1, %n2
  ret i32 %1
}

; Function Attrs: alwaysinline
define i32 @_bit_and_32(i32 %n1, i32 %n2) #0 {
  %1 = and i32 %n1, %n2
  ret i32 %1
}

; Function Attrs: alwaysinline
define i64 @_bit_neg_64(i64 %n) #0 {
  %1 = xor i64 %n, 0
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_bit_shl_64(i64 %n, i64 %k) #0 {
  %1 = shl i64 %n, %k
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_bit_lshr_64(i64 %n, i64 %k) #0 {
  %1 = lshr i64 %n, %k
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_bit_ashr_64(i64 %n, i64 %k) #0 {
  %1 = ashr i64 %n, %k
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_bit_xor_64(i64 %n1, i64 %n2) #0 {
  %1 = xor i64 %n1, %n2
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_bit_or_64(i64 %n1, i64 %n2) #0 {
  %1 = or i64 %n1, %n2
  ret i64 %1
}

; Function Attrs: alwaysinline
define i64 @_bit_and_64(i64 %n1, i64 %n2) #0 {
  %1 = and i64 %n1, %n2
  ret i64 %1
}

declare i1 @"$logicalOr"(i1, i1)

declare i1 @"$logicalAnd"(i1, i1)

; Function Attrs: alwaysinline nounwind
define private i8 @"pre_++"(i8* %n) #2 {
  %n.addr = alloca i8*
  store i8* %n, i8** %n.addr
  %tmp.v = alloca i8
  br label %code

code:                                             ; preds = %0
  %1 = load i8** %n.addr
  %2 = load i8* %1
  store i8 1, i8* %tmp.v
  %3 = load i8* %tmp.v
  %4 = call i8 @"+"(i8 %2, i8 %3)
  %5 = load i8** %n.addr
  store i8 %4, i8* %5
  call void @dtor(i8* %tmp.v)
  %6 = load i8** %n.addr
  %7 = load i8* %6
  ret i8 %7
}

; Function Attrs: alwaysinline nounwind
define private i8 @"+"(i8 %x, i8 %y) #2 {
  %x.addr = alloca i8
  store i8 %x, i8* %x.addr
  %y.addr = alloca i8
  store i8 %y, i8* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %x.addr
  %2 = load i8* %y.addr
  %3 = call i8 @cmn(i8 %1, i8 %2)
  %4 = load i8* %y.addr
  %5 = load i8* %x.addr
  %6 = call i8 @cmn(i8 %4, i8 %5)
  %7 = add i8 %3, %6
  ret i8 %7
}

; Function Attrs: alwaysinline nounwind
define private i8 @cmn(i8 %x, i8 %y) #2 {
  %x.addr = alloca i8
  store i8 %x, i8* %x.addr
  %y.addr = alloca i8
  store i8 %y, i8* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %x.addr
  ret i8 %1
}

; Function Attrs: alwaysinline nounwind
define private void @dtor(i8* %"$this") #2 {
  %"$this.addr" = alloca i8*
  store i8* %"$this", i8** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i8 @"pre_++1"(i8* %n) #2 {
  %n.addr = alloca i8*
  store i8* %n, i8** %n.addr
  %tmp.v = alloca i8
  br label %code

code:                                             ; preds = %0
  %1 = load i8** %n.addr
  %2 = load i8* %1
  store i8 1, i8* %tmp.v
  %3 = load i8* %tmp.v
  %4 = call i8 @"+2"(i8 %2, i8 %3)
  %5 = load i8** %n.addr
  store i8 %4, i8* %5
  call void @dtor4(i8* %tmp.v)
  %6 = load i8** %n.addr
  %7 = load i8* %6
  ret i8 %7
}

; Function Attrs: alwaysinline nounwind
define private i8 @"+2"(i8 %x, i8 %y) #2 {
  %x.addr = alloca i8
  store i8 %x, i8* %x.addr
  %y.addr = alloca i8
  store i8 %y, i8* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %x.addr
  %2 = load i8* %y.addr
  %3 = call i8 @cmn3(i8 %1, i8 %2)
  %4 = load i8* %y.addr
  %5 = load i8* %x.addr
  %6 = call i8 @cmn3(i8 %4, i8 %5)
  %7 = add i8 %3, %6
  ret i8 %7
}

; Function Attrs: alwaysinline nounwind
define private i8 @cmn3(i8 %x, i8 %y) #2 {
  %x.addr = alloca i8
  store i8 %x, i8* %x.addr
  %y.addr = alloca i8
  store i8 %y, i8* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %x.addr
  ret i8 %1
}

; Function Attrs: alwaysinline nounwind
define private void @dtor4(i8* %"$this") #2 {
  %"$this.addr" = alloca i8*
  store i8* %"$this", i8** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i16 @"pre_++5"(i16* %n) #2 {
  %n.addr = alloca i16*
  store i16* %n, i16** %n.addr
  %tmp.v = alloca i16
  br label %code

code:                                             ; preds = %0
  %1 = load i16** %n.addr
  %2 = load i16* %1
  store i16 1, i16* %tmp.v
  %3 = load i16* %tmp.v
  %4 = call i16 @"+6"(i16 %2, i16 %3)
  %5 = load i16** %n.addr
  store i16 %4, i16* %5
  call void @dtor8(i16* %tmp.v)
  %6 = load i16** %n.addr
  %7 = load i16* %6
  ret i16 %7
}

; Function Attrs: alwaysinline nounwind
define private i16 @"+6"(i16 %x, i16 %y) #2 {
  %x.addr = alloca i16
  store i16 %x, i16* %x.addr
  %y.addr = alloca i16
  store i16 %y, i16* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i16* %x.addr
  %2 = load i16* %y.addr
  %3 = call i16 @cmn7(i16 %1, i16 %2)
  %4 = load i16* %y.addr
  %5 = load i16* %x.addr
  %6 = call i16 @cmn7(i16 %4, i16 %5)
  %7 = add i16 %3, %6
  ret i16 %7
}

; Function Attrs: alwaysinline nounwind
define private i16 @cmn7(i16 %x, i16 %y) #2 {
  %x.addr = alloca i16
  store i16 %x, i16* %x.addr
  %y.addr = alloca i16
  store i16 %y, i16* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i16* %x.addr
  ret i16 %1
}

; Function Attrs: alwaysinline nounwind
define private void @dtor8(i16* %"$this") #2 {
  %"$this.addr" = alloca i16*
  store i16* %"$this", i16** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i16 @"pre_++9"(i16* %n) #2 {
  %n.addr = alloca i16*
  store i16* %n, i16** %n.addr
  %tmp.v = alloca i16
  br label %code

code:                                             ; preds = %0
  %1 = load i16** %n.addr
  %2 = load i16* %1
  store i16 1, i16* %tmp.v
  %3 = load i16* %tmp.v
  %4 = call i16 @"+10"(i16 %2, i16 %3)
  %5 = load i16** %n.addr
  store i16 %4, i16* %5
  call void @dtor12(i16* %tmp.v)
  %6 = load i16** %n.addr
  %7 = load i16* %6
  ret i16 %7
}

; Function Attrs: alwaysinline nounwind
define private i16 @"+10"(i16 %x, i16 %y) #2 {
  %x.addr = alloca i16
  store i16 %x, i16* %x.addr
  %y.addr = alloca i16
  store i16 %y, i16* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i16* %x.addr
  %2 = load i16* %y.addr
  %3 = call i16 @cmn11(i16 %1, i16 %2)
  %4 = load i16* %y.addr
  %5 = load i16* %x.addr
  %6 = call i16 @cmn11(i16 %4, i16 %5)
  %7 = add i16 %3, %6
  ret i16 %7
}

; Function Attrs: alwaysinline nounwind
define private i16 @cmn11(i16 %x, i16 %y) #2 {
  %x.addr = alloca i16
  store i16 %x, i16* %x.addr
  %y.addr = alloca i16
  store i16 %y, i16* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i16* %x.addr
  ret i16 %1
}

; Function Attrs: alwaysinline nounwind
define private void @dtor12(i16* %"$this") #2 {
  %"$this.addr" = alloca i16*
  store i16* %"$this", i16** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i32 @"pre_++13"(i32* %n) #2 {
  %n.addr = alloca i32*
  store i32* %n, i32** %n.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i32** %n.addr
  %2 = load i32* %1
  %3 = call i32 @"+14"(i32 %2, i32 1)
  %4 = load i32** %n.addr
  store i32 %3, i32* %4
  %5 = load i32** %n.addr
  %6 = load i32* %5
  ret i32 %6
}

; Function Attrs: alwaysinline nounwind
define private i32 @"+14"(i32 %x, i32 %y) #2 {
  %x.addr = alloca i32
  store i32 %x, i32* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i32* %x.addr
  %2 = load i32* %y.addr
  %3 = call i32 @cmn15(i32 %1, i32 %2)
  %4 = load i32* %y.addr
  %5 = load i32* %x.addr
  %6 = call i32 @cmn15(i32 %4, i32 %5)
  %7 = add i32 %3, %6
  ret i32 %7
}

; Function Attrs: alwaysinline nounwind
define private i32 @cmn15(i32 %x, i32 %y) #2 {
  %x.addr = alloca i32
  store i32 %x, i32* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i32* %x.addr
  ret i32 %1
}

; Function Attrs: alwaysinline nounwind
define private i32 @"pre_++16"(i32* %n) #2 {
  %n.addr = alloca i32*
  store i32* %n, i32** %n.addr
  %tmp.v = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i32** %n.addr
  %2 = load i32* %1
  store i32 1, i32* %tmp.v
  %3 = load i32* %tmp.v
  %4 = call i32 @"+17"(i32 %2, i32 %3)
  %5 = load i32** %n.addr
  store i32 %4, i32* %5
  call void @dtor19(i32* %tmp.v)
  %6 = load i32** %n.addr
  %7 = load i32* %6
  ret i32 %7
}

; Function Attrs: alwaysinline nounwind
define private i32 @"+17"(i32 %x, i32 %y) #2 {
  %x.addr = alloca i32
  store i32 %x, i32* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i32* %x.addr
  %2 = load i32* %y.addr
  %3 = call i32 @cmn18(i32 %1, i32 %2)
  %4 = load i32* %y.addr
  %5 = load i32* %x.addr
  %6 = call i32 @cmn18(i32 %4, i32 %5)
  %7 = add i32 %3, %6
  ret i32 %7
}

; Function Attrs: alwaysinline nounwind
define private i32 @cmn18(i32 %x, i32 %y) #2 {
  %x.addr = alloca i32
  store i32 %x, i32* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i32* %x.addr
  ret i32 %1
}

; Function Attrs: alwaysinline nounwind
define private void @dtor19(i32* %"$this") #2 {
  %"$this.addr" = alloca i32*
  store i32* %"$this", i32** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i64 @"pre_++20"(i64* %n) #2 {
  %n.addr = alloca i64*
  store i64* %n, i64** %n.addr
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i64** %n.addr
  %2 = load i64* %1
  store i64 1, i64* %tmp.v
  %3 = load i64* %tmp.v
  %4 = call i64 @"+21"(i64 %2, i64 %3)
  %5 = load i64** %n.addr
  store i64 %4, i64* %5
  call void @dtor23(i64* %tmp.v)
  %6 = load i64** %n.addr
  %7 = load i64* %6
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @"+21"(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn22(i64 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn22(i64 %4, i64 %5)
  %7 = add i64 %3, %6
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @cmn22(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  ret i64 %1
}

; Function Attrs: alwaysinline nounwind
define private void @dtor23(i64* %"$this") #2 {
  %"$this.addr" = alloca i64*
  store i64* %"$this", i64** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i64 @"pre_++24"(i64* %n) #2 {
  %n.addr = alloca i64*
  store i64* %n, i64** %n.addr
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i64** %n.addr
  %2 = load i64* %1
  store i64 1, i64* %tmp.v
  %3 = load i64* %tmp.v
  %4 = call i64 @"+25"(i64 %2, i64 %3)
  %5 = load i64** %n.addr
  store i64 %4, i64* %5
  call void @dtor27(i64* %tmp.v)
  %6 = load i64** %n.addr
  %7 = load i64* %6
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @"+25"(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn26(i64 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn26(i64 %4, i64 %5)
  %7 = add i64 %3, %6
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @cmn26(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  ret i64 %1
}

; Function Attrs: alwaysinline nounwind
define private void @dtor27(i64* %"$this") #2 {
  %"$this.addr" = alloca i64*
  store i64* %"$this", i64** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i64 @"pre_++28"(i64* %n) #2 {
  %n.addr = alloca i64*
  store i64* %n, i64** %n.addr
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i64** %n.addr
  %2 = load i64* %1
  store i64 1, i64* %tmp.v
  %3 = load i64* %tmp.v
  %4 = call i64 @"+29"(i64 %2, i64 %3)
  %5 = load i64** %n.addr
  store i64 %4, i64* %5
  call void @dtor31(i64* %tmp.v)
  %6 = load i64** %n.addr
  %7 = load i64* %6
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @"+29"(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn30(i64 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn30(i64 %4, i64 %5)
  %7 = add i64 %3, %6
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @cmn30(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  ret i64 %1
}

; Function Attrs: alwaysinline nounwind
define private void @dtor31(i64* %"$this") #2 {
  %"$this.addr" = alloca i64*
  store i64* %"$this", i64** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i64 @"pre_++32"(i64* %n) #2 {
  %n.addr = alloca i64*
  store i64* %n, i64** %n.addr
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i64** %n.addr
  %2 = load i64* %1
  store i64 1, i64* %tmp.v
  %3 = load i64* %tmp.v
  %4 = call i64 @"+33"(i64 %2, i64 %3)
  %5 = load i64** %n.addr
  store i64 %4, i64* %5
  call void @dtor35(i64* %tmp.v)
  %6 = load i64** %n.addr
  %7 = load i64* %6
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @"+33"(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn34(i64 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn34(i64 %4, i64 %5)
  %7 = add i64 %3, %6
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @cmn34(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  ret i64 %1
}

; Function Attrs: alwaysinline nounwind
define private void @dtor35(i64* %"$this") #2 {
  %"$this.addr" = alloca i64*
  store i64* %"$this", i64** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i8 @pre_--(i8* %n) #2 {
  %n.addr = alloca i8*
  store i8* %n, i8** %n.addr
  %tmp.v = alloca i8
  br label %code

code:                                             ; preds = %0
  %1 = load i8** %n.addr
  %2 = load i8* %1
  store i8 1, i8* %tmp.v
  %3 = load i8* %tmp.v
  %4 = call i8 @-(i8 %2, i8 %3)
  %5 = load i8** %n.addr
  store i8 %4, i8* %5
  call void @dtor(i8* %tmp.v)
  %6 = load i8** %n.addr
  %7 = load i8* %6
  ret i8 %7
}

; Function Attrs: alwaysinline nounwind
define private i8 @-(i8 %x, i8 %y) #2 {
  %x.addr = alloca i8
  store i8 %x, i8* %x.addr
  %y.addr = alloca i8
  store i8 %y, i8* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %x.addr
  %2 = load i8* %y.addr
  %3 = call i8 @cmn(i8 %1, i8 %2)
  %4 = load i8* %y.addr
  %5 = load i8* %x.addr
  %6 = call i8 @cmn(i8 %4, i8 %5)
  %7 = sub i8 %3, %6
  ret i8 %7
}

; Function Attrs: alwaysinline nounwind
define private i8 @pre_--36(i8* %n) #2 {
  %n.addr = alloca i8*
  store i8* %n, i8** %n.addr
  %tmp.v = alloca i8
  br label %code

code:                                             ; preds = %0
  %1 = load i8** %n.addr
  %2 = load i8* %1
  store i8 1, i8* %tmp.v
  %3 = load i8* %tmp.v
  %4 = call i8 @-37(i8 %2, i8 %3)
  %5 = load i8** %n.addr
  store i8 %4, i8* %5
  call void @dtor4(i8* %tmp.v)
  %6 = load i8** %n.addr
  %7 = load i8* %6
  ret i8 %7
}

; Function Attrs: alwaysinline nounwind
define private i8 @-37(i8 %x, i8 %y) #2 {
  %x.addr = alloca i8
  store i8 %x, i8* %x.addr
  %y.addr = alloca i8
  store i8 %y, i8* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %x.addr
  %2 = load i8* %y.addr
  %3 = call i8 @cmn3(i8 %1, i8 %2)
  %4 = load i8* %y.addr
  %5 = load i8* %x.addr
  %6 = call i8 @cmn3(i8 %4, i8 %5)
  %7 = sub i8 %3, %6
  ret i8 %7
}

; Function Attrs: alwaysinline nounwind
define private i16 @pre_--38(i16* %n) #2 {
  %n.addr = alloca i16*
  store i16* %n, i16** %n.addr
  %tmp.v = alloca i16
  br label %code

code:                                             ; preds = %0
  %1 = load i16** %n.addr
  %2 = load i16* %1
  store i16 1, i16* %tmp.v
  %3 = load i16* %tmp.v
  %4 = call i16 @-39(i16 %2, i16 %3)
  %5 = load i16** %n.addr
  store i16 %4, i16* %5
  call void @dtor8(i16* %tmp.v)
  %6 = load i16** %n.addr
  %7 = load i16* %6
  ret i16 %7
}

; Function Attrs: alwaysinline nounwind
define private i16 @-39(i16 %x, i16 %y) #2 {
  %x.addr = alloca i16
  store i16 %x, i16* %x.addr
  %y.addr = alloca i16
  store i16 %y, i16* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i16* %x.addr
  %2 = load i16* %y.addr
  %3 = call i16 @cmn7(i16 %1, i16 %2)
  %4 = load i16* %y.addr
  %5 = load i16* %x.addr
  %6 = call i16 @cmn7(i16 %4, i16 %5)
  %7 = sub i16 %3, %6
  ret i16 %7
}

; Function Attrs: alwaysinline nounwind
define private i16 @pre_--40(i16* %n) #2 {
  %n.addr = alloca i16*
  store i16* %n, i16** %n.addr
  %tmp.v = alloca i16
  br label %code

code:                                             ; preds = %0
  %1 = load i16** %n.addr
  %2 = load i16* %1
  store i16 1, i16* %tmp.v
  %3 = load i16* %tmp.v
  %4 = call i16 @-41(i16 %2, i16 %3)
  %5 = load i16** %n.addr
  store i16 %4, i16* %5
  call void @dtor12(i16* %tmp.v)
  %6 = load i16** %n.addr
  %7 = load i16* %6
  ret i16 %7
}

; Function Attrs: alwaysinline nounwind
define private i16 @-41(i16 %x, i16 %y) #2 {
  %x.addr = alloca i16
  store i16 %x, i16* %x.addr
  %y.addr = alloca i16
  store i16 %y, i16* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i16* %x.addr
  %2 = load i16* %y.addr
  %3 = call i16 @cmn11(i16 %1, i16 %2)
  %4 = load i16* %y.addr
  %5 = load i16* %x.addr
  %6 = call i16 @cmn11(i16 %4, i16 %5)
  %7 = sub i16 %3, %6
  ret i16 %7
}

; Function Attrs: alwaysinline nounwind
define private i32 @pre_--42(i32* %n) #2 {
  %n.addr = alloca i32*
  store i32* %n, i32** %n.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i32** %n.addr
  %2 = load i32* %1
  %3 = call i32 @-43(i32 %2, i32 1)
  %4 = load i32** %n.addr
  store i32 %3, i32* %4
  %5 = load i32** %n.addr
  %6 = load i32* %5
  ret i32 %6
}

; Function Attrs: alwaysinline nounwind
define private i32 @-43(i32 %x, i32 %y) #2 {
  %x.addr = alloca i32
  store i32 %x, i32* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i32* %x.addr
  %2 = load i32* %y.addr
  %3 = call i32 @cmn15(i32 %1, i32 %2)
  %4 = load i32* %y.addr
  %5 = load i32* %x.addr
  %6 = call i32 @cmn15(i32 %4, i32 %5)
  %7 = sub i32 %3, %6
  ret i32 %7
}

; Function Attrs: alwaysinline nounwind
define private i32 @pre_--44(i32* %n) #2 {
  %n.addr = alloca i32*
  store i32* %n, i32** %n.addr
  %tmp.v = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i32** %n.addr
  %2 = load i32* %1
  store i32 1, i32* %tmp.v
  %3 = load i32* %tmp.v
  %4 = call i32 @-45(i32 %2, i32 %3)
  %5 = load i32** %n.addr
  store i32 %4, i32* %5
  call void @dtor19(i32* %tmp.v)
  %6 = load i32** %n.addr
  %7 = load i32* %6
  ret i32 %7
}

; Function Attrs: alwaysinline nounwind
define private i32 @-45(i32 %x, i32 %y) #2 {
  %x.addr = alloca i32
  store i32 %x, i32* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i32* %x.addr
  %2 = load i32* %y.addr
  %3 = call i32 @cmn18(i32 %1, i32 %2)
  %4 = load i32* %y.addr
  %5 = load i32* %x.addr
  %6 = call i32 @cmn18(i32 %4, i32 %5)
  %7 = sub i32 %3, %6
  ret i32 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @pre_--46(i64* %n) #2 {
  %n.addr = alloca i64*
  store i64* %n, i64** %n.addr
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i64** %n.addr
  %2 = load i64* %1
  store i64 1, i64* %tmp.v
  %3 = load i64* %tmp.v
  %4 = call i64 @-47(i64 %2, i64 %3)
  %5 = load i64** %n.addr
  store i64 %4, i64* %5
  call void @dtor23(i64* %tmp.v)
  %6 = load i64** %n.addr
  %7 = load i64* %6
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @-47(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn22(i64 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn22(i64 %4, i64 %5)
  %7 = sub i64 %3, %6
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @pre_--48(i64* %n) #2 {
  %n.addr = alloca i64*
  store i64* %n, i64** %n.addr
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i64** %n.addr
  %2 = load i64* %1
  store i64 1, i64* %tmp.v
  %3 = load i64* %tmp.v
  %4 = call i64 @-49(i64 %2, i64 %3)
  %5 = load i64** %n.addr
  store i64 %4, i64* %5
  call void @dtor27(i64* %tmp.v)
  %6 = load i64** %n.addr
  %7 = load i64* %6
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @-49(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn26(i64 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn26(i64 %4, i64 %5)
  %7 = sub i64 %3, %6
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @pre_--50(i64* %n) #2 {
  %n.addr = alloca i64*
  store i64* %n, i64** %n.addr
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i64** %n.addr
  %2 = load i64* %1
  store i64 1, i64* %tmp.v
  %3 = load i64* %tmp.v
  %4 = call i64 @-51(i64 %2, i64 %3)
  %5 = load i64** %n.addr
  store i64 %4, i64* %5
  call void @dtor31(i64* %tmp.v)
  %6 = load i64** %n.addr
  %7 = load i64* %6
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @-51(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn30(i64 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn30(i64 %4, i64 %5)
  %7 = call i64 @_SizeType_opMinus(i64 %3, i64 %6)
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @pre_--52(i64* %n) #2 {
  %n.addr = alloca i64*
  store i64* %n, i64** %n.addr
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i64** %n.addr
  %2 = load i64* %1
  store i64 1, i64* %tmp.v
  %3 = load i64* %tmp.v
  %4 = call i64 @-53(i64 %2, i64 %3)
  %5 = load i64** %n.addr
  store i64 %4, i64* %5
  call void @dtor35(i64* %tmp.v)
  %6 = load i64** %n.addr
  %7 = load i64* %6
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @-53(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn34(i64 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn34(i64 %4, i64 %5)
  %7 = call i64 @_DiffType_opMinus(i64 %3, i64 %6)
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i8 @"post_++"(i8* %n) #2 {
  %n.addr = alloca i8*
  store i8* %n, i8** %n.addr
  %old = alloca i8
  %tmp.v = alloca i8
  br label %code

code:                                             ; preds = %0
  %1 = load i8** %n.addr
  %2 = load i8* %1
  store i8 %2, i8* %old
  %3 = load i8** %n.addr
  %4 = load i8* %3
  store i8 1, i8* %tmp.v
  %5 = load i8* %tmp.v
  %6 = call i8 @"+"(i8 %4, i8 %5)
  %7 = load i8** %n.addr
  store i8 %6, i8* %7
  call void @dtor(i8* %tmp.v)
  %8 = load i8* %old
  call void @dtor(i8* %old)
  ret i8 %8

dumy_block:                                       ; No predecessors!
  call void @dtor(i8* %old)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i8 @"post_++54"(i8* %n) #2 {
  %n.addr = alloca i8*
  store i8* %n, i8** %n.addr
  %old = alloca i8
  %tmp.v = alloca i8
  br label %code

code:                                             ; preds = %0
  %1 = load i8** %n.addr
  %2 = load i8* %1
  store i8 %2, i8* %old
  %3 = load i8** %n.addr
  %4 = load i8* %3
  store i8 1, i8* %tmp.v
  %5 = load i8* %tmp.v
  %6 = call i8 @"+2"(i8 %4, i8 %5)
  %7 = load i8** %n.addr
  store i8 %6, i8* %7
  call void @dtor4(i8* %tmp.v)
  %8 = load i8* %old
  call void @dtor4(i8* %old)
  ret i8 %8

dumy_block:                                       ; No predecessors!
  call void @dtor4(i8* %old)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i16 @"post_++55"(i16* %n) #2 {
  %n.addr = alloca i16*
  store i16* %n, i16** %n.addr
  %old = alloca i16
  %tmp.v = alloca i16
  br label %code

code:                                             ; preds = %0
  %1 = load i16** %n.addr
  %2 = load i16* %1
  store i16 %2, i16* %old
  %3 = load i16** %n.addr
  %4 = load i16* %3
  store i16 1, i16* %tmp.v
  %5 = load i16* %tmp.v
  %6 = call i16 @"+6"(i16 %4, i16 %5)
  %7 = load i16** %n.addr
  store i16 %6, i16* %7
  call void @dtor8(i16* %tmp.v)
  %8 = load i16* %old
  call void @dtor8(i16* %old)
  ret i16 %8

dumy_block:                                       ; No predecessors!
  call void @dtor8(i16* %old)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i16 @"post_++56"(i16* %n) #2 {
  %n.addr = alloca i16*
  store i16* %n, i16** %n.addr
  %old = alloca i16
  %tmp.v = alloca i16
  br label %code

code:                                             ; preds = %0
  %1 = load i16** %n.addr
  %2 = load i16* %1
  store i16 %2, i16* %old
  %3 = load i16** %n.addr
  %4 = load i16* %3
  store i16 1, i16* %tmp.v
  %5 = load i16* %tmp.v
  %6 = call i16 @"+10"(i16 %4, i16 %5)
  %7 = load i16** %n.addr
  store i16 %6, i16* %7
  call void @dtor12(i16* %tmp.v)
  %8 = load i16* %old
  call void @dtor12(i16* %old)
  ret i16 %8

dumy_block:                                       ; No predecessors!
  call void @dtor12(i16* %old)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i32 @"post_++57"(i32* %n) #2 {
  %n.addr = alloca i32*
  store i32* %n, i32** %n.addr
  %old = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i32** %n.addr
  %2 = load i32* %1
  store i32 %2, i32* %old
  %3 = load i32** %n.addr
  %4 = load i32* %3
  %5 = call i32 @"+14"(i32 %4, i32 1)
  %6 = load i32** %n.addr
  store i32 %5, i32* %6
  %7 = load i32* %old
  call void @dtor58(i32* %old)
  ret i32 %7

dumy_block:                                       ; No predecessors!
  call void @dtor58(i32* %old)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private void @dtor58(i32* %"$this") #2 {
  %"$this.addr" = alloca i32*
  store i32* %"$this", i32** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i32 @"post_++59"(i32* %n) #2 {
  %n.addr = alloca i32*
  store i32* %n, i32** %n.addr
  %old = alloca i32
  %tmp.v = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i32** %n.addr
  %2 = load i32* %1
  store i32 %2, i32* %old
  %3 = load i32** %n.addr
  %4 = load i32* %3
  store i32 1, i32* %tmp.v
  %5 = load i32* %tmp.v
  %6 = call i32 @"+17"(i32 %4, i32 %5)
  %7 = load i32** %n.addr
  store i32 %6, i32* %7
  call void @dtor19(i32* %tmp.v)
  %8 = load i32* %old
  call void @dtor19(i32* %old)
  ret i32 %8

dumy_block:                                       ; No predecessors!
  call void @dtor19(i32* %old)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i64 @"post_++60"(i64* %n) #2 {
  %n.addr = alloca i64*
  store i64* %n, i64** %n.addr
  %old = alloca i64
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i64** %n.addr
  %2 = load i64* %1
  store i64 %2, i64* %old
  %3 = load i64** %n.addr
  %4 = load i64* %3
  store i64 1, i64* %tmp.v
  %5 = load i64* %tmp.v
  %6 = call i64 @"+21"(i64 %4, i64 %5)
  %7 = load i64** %n.addr
  store i64 %6, i64* %7
  call void @dtor23(i64* %tmp.v)
  %8 = load i64* %old
  call void @dtor23(i64* %old)
  ret i64 %8

dumy_block:                                       ; No predecessors!
  call void @dtor23(i64* %old)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i64 @"post_++61"(i64* %n) #2 {
  %n.addr = alloca i64*
  store i64* %n, i64** %n.addr
  %old = alloca i64
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i64** %n.addr
  %2 = load i64* %1
  store i64 %2, i64* %old
  %3 = load i64** %n.addr
  %4 = load i64* %3
  store i64 1, i64* %tmp.v
  %5 = load i64* %tmp.v
  %6 = call i64 @"+25"(i64 %4, i64 %5)
  %7 = load i64** %n.addr
  store i64 %6, i64* %7
  call void @dtor27(i64* %tmp.v)
  %8 = load i64* %old
  call void @dtor27(i64* %old)
  ret i64 %8

dumy_block:                                       ; No predecessors!
  call void @dtor27(i64* %old)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i64 @"post_++62"(i64* %n) #2 {
  %n.addr = alloca i64*
  store i64* %n, i64** %n.addr
  %old = alloca i64
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i64** %n.addr
  %2 = load i64* %1
  store i64 %2, i64* %old
  %3 = load i64** %n.addr
  %4 = load i64* %3
  store i64 1, i64* %tmp.v
  %5 = load i64* %tmp.v
  %6 = call i64 @"+29"(i64 %4, i64 %5)
  %7 = load i64** %n.addr
  store i64 %6, i64* %7
  call void @dtor31(i64* %tmp.v)
  %8 = load i64* %old
  call void @dtor31(i64* %old)
  ret i64 %8

dumy_block:                                       ; No predecessors!
  call void @dtor31(i64* %old)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i64 @"post_++63"(i64* %n) #2 {
  %n.addr = alloca i64*
  store i64* %n, i64** %n.addr
  %old = alloca i64
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i64** %n.addr
  %2 = load i64* %1
  store i64 %2, i64* %old
  %3 = load i64** %n.addr
  %4 = load i64* %3
  store i64 1, i64* %tmp.v
  %5 = load i64* %tmp.v
  %6 = call i64 @"+33"(i64 %4, i64 %5)
  %7 = load i64** %n.addr
  store i64 %6, i64* %7
  call void @dtor35(i64* %tmp.v)
  %8 = load i64* %old
  call void @dtor35(i64* %old)
  ret i64 %8

dumy_block:                                       ; No predecessors!
  call void @dtor35(i64* %old)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i8 @post_--(i8* %n) #2 {
  %n.addr = alloca i8*
  store i8* %n, i8** %n.addr
  %old = alloca i8
  %tmp.v = alloca i8
  br label %code

code:                                             ; preds = %0
  %1 = load i8** %n.addr
  %2 = load i8* %1
  store i8 %2, i8* %old
  %3 = load i8** %n.addr
  %4 = load i8* %3
  store i8 1, i8* %tmp.v
  %5 = load i8* %tmp.v
  %6 = call i8 @-(i8 %4, i8 %5)
  %7 = load i8** %n.addr
  store i8 %6, i8* %7
  call void @dtor(i8* %tmp.v)
  %8 = load i8* %old
  call void @dtor(i8* %old)
  ret i8 %8

dumy_block:                                       ; No predecessors!
  call void @dtor(i8* %old)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i8 @post_--64(i8* %n) #2 {
  %n.addr = alloca i8*
  store i8* %n, i8** %n.addr
  %old = alloca i8
  %tmp.v = alloca i8
  br label %code

code:                                             ; preds = %0
  %1 = load i8** %n.addr
  %2 = load i8* %1
  store i8 %2, i8* %old
  %3 = load i8** %n.addr
  %4 = load i8* %3
  store i8 1, i8* %tmp.v
  %5 = load i8* %tmp.v
  %6 = call i8 @-37(i8 %4, i8 %5)
  %7 = load i8** %n.addr
  store i8 %6, i8* %7
  call void @dtor4(i8* %tmp.v)
  %8 = load i8* %old
  call void @dtor4(i8* %old)
  ret i8 %8

dumy_block:                                       ; No predecessors!
  call void @dtor4(i8* %old)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i16 @post_--65(i16* %n) #2 {
  %n.addr = alloca i16*
  store i16* %n, i16** %n.addr
  %old = alloca i16
  %tmp.v = alloca i16
  br label %code

code:                                             ; preds = %0
  %1 = load i16** %n.addr
  %2 = load i16* %1
  store i16 %2, i16* %old
  %3 = load i16** %n.addr
  %4 = load i16* %3
  store i16 1, i16* %tmp.v
  %5 = load i16* %tmp.v
  %6 = call i16 @-39(i16 %4, i16 %5)
  %7 = load i16** %n.addr
  store i16 %6, i16* %7
  call void @dtor8(i16* %tmp.v)
  %8 = load i16* %old
  call void @dtor8(i16* %old)
  ret i16 %8

dumy_block:                                       ; No predecessors!
  call void @dtor8(i16* %old)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i16 @post_--66(i16* %n) #2 {
  %n.addr = alloca i16*
  store i16* %n, i16** %n.addr
  %old = alloca i16
  %tmp.v = alloca i16
  br label %code

code:                                             ; preds = %0
  %1 = load i16** %n.addr
  %2 = load i16* %1
  store i16 %2, i16* %old
  %3 = load i16** %n.addr
  %4 = load i16* %3
  store i16 1, i16* %tmp.v
  %5 = load i16* %tmp.v
  %6 = call i16 @-41(i16 %4, i16 %5)
  %7 = load i16** %n.addr
  store i16 %6, i16* %7
  call void @dtor12(i16* %tmp.v)
  %8 = load i16* %old
  call void @dtor12(i16* %old)
  ret i16 %8

dumy_block:                                       ; No predecessors!
  call void @dtor12(i16* %old)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i32 @post_--67(i32* %n) #2 {
  %n.addr = alloca i32*
  store i32* %n, i32** %n.addr
  %old = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i32** %n.addr
  %2 = load i32* %1
  store i32 %2, i32* %old
  %3 = load i32** %n.addr
  %4 = load i32* %3
  %5 = call i32 @-43(i32 %4, i32 1)
  %6 = load i32** %n.addr
  store i32 %5, i32* %6
  %7 = load i32* %old
  call void @dtor58(i32* %old)
  ret i32 %7

dumy_block:                                       ; No predecessors!
  call void @dtor58(i32* %old)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i32 @post_--68(i32* %n) #2 {
  %n.addr = alloca i32*
  store i32* %n, i32** %n.addr
  %old = alloca i32
  %tmp.v = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i32** %n.addr
  %2 = load i32* %1
  store i32 %2, i32* %old
  %3 = load i32** %n.addr
  %4 = load i32* %3
  store i32 1, i32* %tmp.v
  %5 = load i32* %tmp.v
  %6 = call i32 @-45(i32 %4, i32 %5)
  %7 = load i32** %n.addr
  store i32 %6, i32* %7
  call void @dtor19(i32* %tmp.v)
  %8 = load i32* %old
  call void @dtor19(i32* %old)
  ret i32 %8

dumy_block:                                       ; No predecessors!
  call void @dtor19(i32* %old)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i64 @post_--69(i64* %n) #2 {
  %n.addr = alloca i64*
  store i64* %n, i64** %n.addr
  %old = alloca i64
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i64** %n.addr
  %2 = load i64* %1
  store i64 %2, i64* %old
  %3 = load i64** %n.addr
  %4 = load i64* %3
  store i64 1, i64* %tmp.v
  %5 = load i64* %tmp.v
  %6 = call i64 @-47(i64 %4, i64 %5)
  %7 = load i64** %n.addr
  store i64 %6, i64* %7
  call void @dtor23(i64* %tmp.v)
  %8 = load i64* %old
  call void @dtor23(i64* %old)
  ret i64 %8

dumy_block:                                       ; No predecessors!
  call void @dtor23(i64* %old)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i64 @post_--70(i64* %n) #2 {
  %n.addr = alloca i64*
  store i64* %n, i64** %n.addr
  %old = alloca i64
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i64** %n.addr
  %2 = load i64* %1
  store i64 %2, i64* %old
  %3 = load i64** %n.addr
  %4 = load i64* %3
  store i64 1, i64* %tmp.v
  %5 = load i64* %tmp.v
  %6 = call i64 @-49(i64 %4, i64 %5)
  %7 = load i64** %n.addr
  store i64 %6, i64* %7
  call void @dtor27(i64* %tmp.v)
  %8 = load i64* %old
  call void @dtor27(i64* %old)
  ret i64 %8

dumy_block:                                       ; No predecessors!
  call void @dtor27(i64* %old)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i64 @post_--71(i64* %n) #2 {
  %n.addr = alloca i64*
  store i64* %n, i64** %n.addr
  %old = alloca i64
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i64** %n.addr
  %2 = load i64* %1
  store i64 %2, i64* %old
  %3 = load i64** %n.addr
  %4 = load i64* %3
  store i64 1, i64* %tmp.v
  %5 = load i64* %tmp.v
  %6 = call i64 @-51(i64 %4, i64 %5)
  %7 = load i64** %n.addr
  store i64 %6, i64* %7
  call void @dtor31(i64* %tmp.v)
  %8 = load i64* %old
  call void @dtor31(i64* %old)
  ret i64 %8

dumy_block:                                       ; No predecessors!
  call void @dtor31(i64* %old)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i64 @post_--72(i64* %n) #2 {
  %n.addr = alloca i64*
  store i64* %n, i64** %n.addr
  %old = alloca i64
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i64** %n.addr
  %2 = load i64* %1
  store i64 %2, i64* %old
  %3 = load i64** %n.addr
  %4 = load i64* %3
  store i64 1, i64* %tmp.v
  %5 = load i64* %tmp.v
  %6 = call i64 @-53(i64 %4, i64 %5)
  %7 = load i64** %n.addr
  store i64 %6, i64* %7
  call void @dtor35(i64* %tmp.v)
  %8 = load i64* %old
  call void @dtor35(i64* %old)
  ret i64 %8

dumy_block:                                       ; No predecessors!
  call void @dtor35(i64* %old)
  unreachable
}

; Function Attrs: inlinehint nounwind
define private void @"+73"(%StringRef* sret %_result, %StringRef %x, %StringRef %y) #3 {
  %_result.addr = alloca %StringRef*
  store %StringRef* %_result, %StringRef** %_result.addr
  %x.addr = alloca %StringRef
  store %StringRef %x, %StringRef* %x.addr
  %y.addr = alloca %StringRef
  store %StringRef %y, %StringRef* %y.addr
  %sz1 = alloca i64
  %sz2 = alloca i64
  %res = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = call i64 @size(%StringRef* %x.addr)
  store i64 %1, i64* %sz1
  %2 = call i64 @size(%StringRef* %y.addr)
  store i64 %2, i64* %sz2
  %3 = load i64* %sz1
  %4 = load i64* %sz2
  %5 = call i64 @"+29"(i64 %3, i64 %4)
  call void @ctor(%StringRef* %res, i64 %5)
  %6 = getelementptr inbounds %StringRef* %res, i32 0, i32 0
  %7 = load i8** %6
  %8 = getelementptr inbounds %StringRef* %x.addr, i32 0, i32 0
  %9 = load i8** %8
  %10 = load i64* %sz1
  call void @_spr_memcpy(i8* %7, i8* %9, i64 %10)
  %11 = getelementptr inbounds %StringRef* %res, i32 0, i32 0
  %12 = load i8** %11
  %13 = load i64* %sz1
  %14 = call i8* @ptrAdd(i8* %12, i64 %13)
  %15 = getelementptr inbounds %StringRef* %y.addr, i32 0, i32 0
  %16 = load i8** %15
  %17 = load i64* %sz2
  call void @_spr_memcpy(i8* %14, i8* %16, i64 %17)
  %18 = load %StringRef** %_result.addr
  %19 = load %StringRef* %res
  call void @ctor77(%StringRef* %18, %StringRef %19)
  call void @dtor78(%StringRef* %res)
  call void @dtor31(i64* %sz2)
  call void @dtor31(i64* %sz1)
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor78(%StringRef* %res)
  call void @dtor31(i64* %sz2)
  call void @dtor31(i64* %sz1)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i64 @size(%StringRef* %"$this") #2 {
  %"$this.addr" = alloca %StringRef*
  store %StringRef* %"$this", %StringRef** %"$this.addr"
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef** %"$this.addr"
  %2 = getelementptr inbounds %StringRef* %1, i32 0, i32 1
  %3 = load i8** %2
  %4 = load %StringRef** %"$this.addr"
  %5 = getelementptr inbounds %StringRef* %4, i32 0, i32 0
  %6 = load i8** %5
  %7 = call i64 @ptrDiff(i8* %3, i8* %6)
  store i64 %7, i64* %tmp.v
  %8 = load i64* %tmp.v
  call void @dtor31(i64* %tmp.v)
  ret i64 %8

dumy_block:                                       ; No predecessors!
  call void @dtor31(i64* %tmp.v)
  unreachable
}

; Function Attrs: inlinehint nounwind
define private void @ctor(%StringRef* %"$this", i64 %size) #3 {
  %"$this.addr" = alloca %StringRef*
  store %StringRef* %"$this", %StringRef** %"$this.addr"
  %size.addr = alloca i64
  store i64 %size, i64* %size.addr
  %tmp.v = alloca i8
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef** %"$this.addr"
  %2 = getelementptr inbounds %StringRef* %1, i32 0, i32 0
  store i8* null, i8** %2
  %3 = load %StringRef** %"$this.addr"
  %4 = getelementptr inbounds %StringRef* %3, i32 0, i32 1
  store i8* null, i8** %4
  %5 = load i64* %size.addr
  %6 = call i64 @"+74"(i64 %5, i32 1)
  %7 = call i8* @malloc(i64 %6)
  %8 = load %StringRef** %"$this.addr"
  %9 = getelementptr inbounds %StringRef* %8, i32 0, i32 0
  store i8* %7, i8** %9
  %10 = load %StringRef** %"$this.addr"
  %11 = getelementptr inbounds %StringRef* %10, i32 0, i32 0
  %12 = load i8** %11
  %13 = load i64* %size.addr
  %14 = call i8* @ptrAdd(i8* %12, i64 %13)
  %15 = load %StringRef** %"$this.addr"
  %16 = getelementptr inbounds %StringRef* %15, i32 0, i32 1
  store i8* %14, i8** %16
  store i8 0, i8* %tmp.v
  %17 = load i8* %tmp.v
  %18 = load %StringRef** %"$this.addr"
  %19 = getelementptr inbounds %StringRef* %18, i32 0, i32 1
  %20 = load i8** %19
  store i8 %17, i8* %20
  call void @dtor(i8* %tmp.v)
  ret void
}

declare i8* @malloc(i64)

; Function Attrs: alwaysinline nounwind
define private i64 @"+74"(i64 %x, i32 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i32* %y.addr
  %3 = call i64 @cmn75(i64 %1, i32 %2)
  %4 = load i32* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn76(i32 %4, i64 %5)
  %7 = add i64 %3, %6
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @cmn75(i64 %x, i32 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  ret i64 %1
}

; Function Attrs: alwaysinline nounwind
define private i64 @cmn76(i32 %x, i64 %y) #2 {
  %x.addr = alloca i32
  store i32 %x, i32* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i32* %x.addr
  %2 = zext i32 %1 to i64
  store i64 %2, i64* %tmp.v
  %3 = load i64* %tmp.v
  call void @dtor31(i64* %tmp.v)
  ret i64 %3

dumy_block:                                       ; No predecessors!
  call void @dtor31(i64* %tmp.v)
  unreachable
}

; Function Attrs: inlinehint nounwind
define private void @ctor77(%StringRef* %"$this", %StringRef %other) #3 {
  %"$this.addr" = alloca %StringRef*
  store %StringRef* %"$this", %StringRef** %"$this.addr"
  %other.addr = alloca %StringRef
  store %StringRef %other, %StringRef* %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef** %"$this.addr"
  %2 = getelementptr inbounds %StringRef* %1, i32 0, i32 0
  store i8* null, i8** %2
  %3 = load %StringRef** %"$this.addr"
  %4 = getelementptr inbounds %StringRef* %3, i32 0, i32 1
  store i8* null, i8** %4
  %5 = getelementptr inbounds %StringRef* %other.addr, i32 0, i32 0
  %6 = load i8** %5
  %7 = load %StringRef** %"$this.addr"
  %8 = getelementptr inbounds %StringRef* %7, i32 0, i32 0
  store i8* %6, i8** %8
  %9 = getelementptr inbounds %StringRef* %other.addr, i32 0, i32 1
  %10 = load i8** %9
  %11 = load %StringRef** %"$this.addr"
  %12 = getelementptr inbounds %StringRef* %11, i32 0, i32 1
  store i8* %10, i8** %12
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor78(%StringRef* %"$this") #2 {
  %"$this.addr" = alloca %StringRef*
  store %StringRef* %"$this", %StringRef** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @"+79"(%StringRef* sret %_result, %StringRef %x, i8 %y) #3 {
  %_result.addr = alloca %StringRef*
  store %StringRef* %_result, %StringRef** %_result.addr
  %x.addr = alloca %StringRef
  store %StringRef %x, %StringRef* %x.addr
  %y.addr = alloca i8
  store i8 %y, i8* %y.addr
  %sz1 = alloca i64
  %sz2 = alloca i32
  %res = alloca %StringRef
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = call i64 @size(%StringRef* %x.addr)
  store i64 %1, i64* %sz1
  store i32 1, i32* %sz2
  %2 = load i64* %sz1
  %3 = load i32* %sz2
  %4 = call i64 @"+74"(i64 %2, i32 %3)
  call void @ctor(%StringRef* %res, i64 %4)
  %5 = getelementptr inbounds %StringRef* %res, i32 0, i32 0
  %6 = load i8** %5
  %7 = getelementptr inbounds %StringRef* %x.addr, i32 0, i32 0
  %8 = load i8** %7
  %9 = load i64* %sz1
  call void @_spr_memcpy(i8* %6, i8* %8, i64 %9)
  %10 = getelementptr inbounds %StringRef* %res, i32 0, i32 0
  %11 = load i8** %10
  %12 = load i64* %sz1
  %13 = call i8* @ptrAdd(i8* %11, i64 %12)
  %14 = load i32* %sz2
  %15 = zext i32 %14 to i64
  store i64 %15, i64* %tmp.v
  %16 = load i64* %tmp.v
  call void @_spr_memcpy(i8* %13, i8* %y.addr, i64 %16)
  call void @dtor31(i64* %tmp.v)
  %17 = load %StringRef** %_result.addr
  %18 = load %StringRef* %res
  call void @ctor77(%StringRef* %17, %StringRef %18)
  call void @dtor78(%StringRef* %res)
  call void @dtor58(i32* %sz2)
  call void @dtor31(i64* %sz1)
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor78(%StringRef* %res)
  call void @dtor58(i32* %sz2)
  call void @dtor31(i64* %sz1)
  ret void
}

; Function Attrs: nounwind
declare void @cast() #1

; Function Attrs: nounwind
declare void @reinterpretCast() #1

; Function Attrs: nounwind
declare void @construct() #1

declare i8* @calloc(i64, i64)

declare i8* @realloc(i8*, i64)

declare void @free(i8*)

declare i32 @strcmp(i8*, i8*)

declare void @system(i8*)

declare void @sleep(i32)

define private void @__global_ctor() {
  br label %code

code:                                             ; preds = %0
  call void @ctor80(%EndLineHelperClass* @endl)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor80(%EndLineHelperClass* %"$this") #2 {
  %"$this.addr" = alloca %EndLineHelperClass*
  store %EndLineHelperClass* %"$this", %EndLineHelperClass** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

define private void @__global_dtor() {
  br label %code

code:                                             ; preds = %0
  call void @dtor81(%EndLineHelperClass* @endl)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor81(%EndLineHelperClass* %"$this") #2 {
  %"$this.addr" = alloca %EndLineHelperClass*
  store %EndLineHelperClass* %"$this", %EndLineHelperClass** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

define private void @__global_ctor82() {
  br label %code

code:                                             ; preds = %0
  call void @ctor83(%FlushHelperClass* @flush)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor83(%FlushHelperClass* %"$this") #2 {
  %"$this.addr" = alloca %FlushHelperClass*
  store %FlushHelperClass* %"$this", %FlushHelperClass** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

define private void @__global_dtor84() {
  br label %code

code:                                             ; preds = %0
  call void @dtor85(%FlushHelperClass* @flush)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor85(%FlushHelperClass* %"$this") #2 {
  %"$this.addr" = alloca %FlushHelperClass*
  store %FlushHelperClass* %"$this", %FlushHelperClass** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

define private void @__global_ctor86() {
  br label %code

code:                                             ; preds = %0
  call void @ctor87(%ConsoleOutputStream* @cout)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor87(%ConsoleOutputStream* %"$this") #2 {
  %"$this.addr" = alloca %ConsoleOutputStream*
  store %ConsoleOutputStream* %"$this", %ConsoleOutputStream** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

define private void @__global_dtor88() {
  br label %code

code:                                             ; preds = %0
  call void @dtor89(%ConsoleOutputStream* @cout)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor89(%ConsoleOutputStream* %"$this") #2 {
  %"$this.addr" = alloca %ConsoleOutputStream*
  store %ConsoleOutputStream* %"$this", %ConsoleOutputStream** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i1 @isEmpty(%MainParameters* %"$this") #2 {
  %"$this.addr" = alloca %MainParameters*
  store %MainParameters* %"$this", %MainParameters** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %MainParameters** %"$this.addr"
  %2 = call i64 @size90(%MainParameters* %1)
  %3 = call i1 @"=="(i64 %2, i32 0)
  ret i1 %3
}

; Function Attrs: alwaysinline nounwind
define private i1 @"=="(i64 %x, i32 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i32* %y.addr
  %3 = call i64 @cmn75(i64 %1, i32 %2)
  %4 = load i32* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn76(i32 %4, i64 %5)
  %7 = icmp eq i64 %3, %6
  ret i1 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @size90(%MainParameters* %"$this") #2 {
  %"$this.addr" = alloca %MainParameters*
  store %MainParameters* %"$this", %MainParameters** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %MainParameters** %"$this.addr"
  %2 = getelementptr inbounds %MainParameters* %1, i32 0, i32 1
  %3 = load %CStrPtr* %2
  %4 = load %MainParameters** %"$this.addr"
  %5 = getelementptr inbounds %MainParameters* %4, i32 0, i32 0
  %6 = load %CStrPtr* %5
  %7 = call i64 @-91(%CStrPtr %3, %CStrPtr %6)
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @-91(%CStrPtr %p, %CStrPtr %q) #2 {
  %p.addr = alloca %CStrPtr
  store %CStrPtr %p, %CStrPtr* %p.addr
  %q.addr = alloca %CStrPtr
  store %CStrPtr %q, %CStrPtr* %q.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %CStrPtr* %p.addr
  %2 = call i8* @toBytePtr(%CStrPtr %1)
  %3 = load %CStrPtr* %q.addr
  %4 = call i8* @toBytePtr(%CStrPtr %3)
  %5 = call i64 @ptrDiff(i8* %2, i8* %4)
  %6 = call i64 @"/"(i64 %5, i64 8)
  ret i64 %6
}

; Function Attrs: alwaysinline nounwind
define private i64 @"/"(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn92(i64 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn93(i64 %4, i64 %5)
  %7 = call i64 @_SizeType_opDiv(i64 %3, i64 %6)
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @cmn92(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  store i64 %1, i64* %tmp.v
  %2 = load i64* %tmp.v
  call void @dtor31(i64* %tmp.v)
  ret i64 %2

dumy_block:                                       ; No predecessors!
  call void @dtor31(i64* %tmp.v)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i64 @cmn93(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  ret i64 %1
}

; Function Attrs: alwaysinline nounwind
define private i8* @toBytePtr(%CStrPtr %p) #2 {
  %p.addr = alloca %CStrPtr
  store %CStrPtr %p, %CStrPtr* %p.addr
  br label %code

code:                                             ; preds = %0
  %1 = getelementptr inbounds %CStrPtr* %p.addr, i32 0, i32 0
  %2 = load %CStr** %1
  %3 = bitcast %CStr* %2 to i8*
  ret i8* %3
}

; Function Attrs: alwaysinline nounwind
define private void @front(%StringRef* sret %_result, %MainParameters* %"$this") #2 {
  %_result.addr = alloca %StringRef*
  store %StringRef* %_result, %StringRef** %_result.addr
  %"$this.addr" = alloca %MainParameters*
  store %MainParameters* %"$this", %MainParameters** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef** %_result.addr
  %2 = load %MainParameters** %"$this.addr"
  %3 = getelementptr inbounds %MainParameters* %2, i32 0, i32 0
  %4 = load %CStrPtr* %3
  call void @toStringRef(%StringRef* %1, %CStrPtr %4)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @toStringRef(%StringRef* sret %_result, %CStrPtr %p) #2 {
  %_result.addr = alloca %StringRef*
  store %StringRef* %_result, %StringRef** %_result.addr
  %p.addr = alloca %CStrPtr
  store %CStrPtr %p, %CStrPtr* %p.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef** %_result.addr
  %2 = getelementptr inbounds %CStrPtr* %p.addr, i32 0, i32 0
  %3 = load %CStr** %2
  %4 = getelementptr inbounds %CStr* %3, i32 0, i32 0
  %5 = load i8** %4
  call void @_String_fromCString(%StringRef* %1, i8* %5)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @_String_fromCString(%StringRef* sret %_result, i8* %s) #3 {
  %_result.addr = alloca %StringRef*
  store %StringRef* %_result, %StringRef** %_result.addr
  %s.addr = alloca i8*
  store i8* %s, i8** %s.addr
  %len = alloca i64
  %res = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load i8** %s.addr
  %2 = call i64 @cStringLen(i8* %1)
  store i64 %2, i64* %len
  call void @ctor94(%StringRef* %res)
  %3 = load i8** %s.addr
  %4 = getelementptr inbounds %StringRef* %res, i32 0, i32 0
  store i8* %3, i8** %4
  %5 = getelementptr inbounds %StringRef* %res, i32 0, i32 0
  %6 = load i8** %5
  %7 = load i64* %len
  %8 = call i8* @ptrAdd(i8* %6, i64 %7)
  %9 = getelementptr inbounds %StringRef* %res, i32 0, i32 1
  store i8* %8, i8** %9
  %10 = load %StringRef** %_result.addr
  %11 = load %StringRef* %res
  call void @ctor77(%StringRef* %10, %StringRef %11)
  call void @dtor78(%StringRef* %res)
  call void @dtor31(i64* %len)
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor78(%StringRef* %res)
  call void @dtor31(i64* %len)
  ret void
}

; Function Attrs: inlinehint nounwind
define private i64 @cStringLen(i8* %s) #3 {
  %s.addr = alloca i8*
  store i8* %s, i8** %s.addr
  %p = alloca i8*
  %len = alloca i64
  %tmp.v = alloca i8
  br label %code

code:                                             ; preds = %0
  %1 = load i8** %s.addr
  store i8* %1, i8** %p
  store i64 0, i64* %len
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %2 = load i8** %p
  %3 = load i8* %2
  store i8 0, i8* %tmp.v
  %4 = load i8* %tmp.v
  %5 = call i1 @"!="(i8 %3, i8 %4)
  br i1 %5, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  %6 = load i64* %len
  %7 = call i64 @"+74"(i64 %6, i32 1)
  store i64 %7, i64* %len
  br label %while_step

while_step:                                       ; preds = %while_body
  %8 = load i8** %p
  %9 = call i8* @ptrAdd(i8* %8, i64 1)
  store i8* %9, i8** %p
  br label %while_block

while_end:                                        ; preds = %while_block
  call void @dtor(i8* %tmp.v)
  %10 = load i64* %len
  call void @dtor31(i64* %len)
  ret i64 %10

dumy_block:                                       ; No predecessors!
  call void @dtor31(i64* %len)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i1 @"!="(i8 %x, i8 %y) #2 {
  %x.addr = alloca i8
  store i8 %x, i8* %x.addr
  %y.addr = alloca i8
  store i8 %y, i8* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %x.addr
  %2 = load i8* %y.addr
  %3 = call i8 @cmn(i8 %1, i8 %2)
  %4 = load i8* %y.addr
  %5 = load i8* %x.addr
  %6 = call i8 @cmn(i8 %4, i8 %5)
  %7 = icmp ne i8 %3, %6
  ret i1 %7
}

; Function Attrs: alwaysinline nounwind
define private void @ctor94(%StringRef* %"$this") #2 {
  %"$this.addr" = alloca %StringRef*
  store %StringRef* %"$this", %StringRef** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef** %"$this.addr"
  %2 = getelementptr inbounds %StringRef* %1, i32 0, i32 0
  store i8* null, i8** %2
  %3 = load %StringRef** %"$this.addr"
  %4 = getelementptr inbounds %StringRef* %3, i32 0, i32 1
  store i8* null, i8** %4
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @back(%StringRef* sret %_result, %MainParameters* %"$this") #2 {
  %_result.addr = alloca %StringRef*
  store %StringRef* %_result, %StringRef** %_result.addr
  %"$this.addr" = alloca %MainParameters*
  store %MainParameters* %"$this", %MainParameters** %"$this.addr"
  %"$tmpC" = alloca %CStrPtr
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef** %_result.addr
  %2 = load %MainParameters** %"$this.addr"
  %3 = getelementptr inbounds %MainParameters* %2, i32 0, i32 1
  %4 = load %CStrPtr* %3
  call void @-95(%CStrPtr* %"$tmpC", %CStrPtr %4, i64 1)
  %5 = load %CStrPtr* %"$tmpC"
  call void @toStringRef(%StringRef* %1, %CStrPtr %5)
  call void @dtor97(%CStrPtr* %"$tmpC")
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor97(%CStrPtr* %"$tmpC")
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @-95(%CStrPtr* sret %_result, %CStrPtr %p, i64 %n) #2 {
  %_result.addr = alloca %CStrPtr*
  store %CStrPtr* %_result, %CStrPtr** %_result.addr
  %p.addr = alloca %CStrPtr
  store %CStrPtr %p, %CStrPtr* %p.addr
  %n.addr = alloca i64
  store i64 %n, i64* %n.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %CStrPtr** %_result.addr
  %2 = load %CStrPtr* %p.addr
  %3 = call i8* @toBytePtr(%CStrPtr %2)
  %4 = load i64* %n.addr
  %5 = call i64 @"*"(i64 %4, i64 8)
  %6 = call i8* @ptrSub(i8* %3, i64 %5)
  call void @fromBytePtr(%CStrPtr* %1, i8* %6)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @fromBytePtr(%CStrPtr* sret %_result, i8* %p) #2 {
  %_result.addr = alloca %CStrPtr*
  store %CStrPtr* %_result, %CStrPtr** %_result.addr
  %p.addr = alloca i8*
  store i8* %p, i8** %p.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %CStrPtr** %_result.addr
  %2 = load i8** %p.addr
  %3 = bitcast i8* %2 to %CStr*
  call void @ctor96(%CStrPtr* %1, %CStr* %3)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor96(%CStrPtr* %"$this", %CStr* %fptr) #2 {
  %"$this.addr" = alloca %CStrPtr*
  store %CStrPtr* %"$this", %CStrPtr** %"$this.addr"
  %fptr.addr = alloca %CStr*
  store %CStr* %fptr, %CStr** %fptr.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %CStr** %fptr.addr
  %2 = load %CStrPtr** %"$this.addr"
  %3 = getelementptr inbounds %CStrPtr* %2, i32 0, i32 0
  store %CStr* %1, %CStr** %3
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i64 @"*"(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn30(i64 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn30(i64 %4, i64 %5)
  %7 = mul i64 %3, %6
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private void @dtor97(%CStrPtr* %"$this") #2 {
  %"$this.addr" = alloca %CStrPtr*
  store %CStrPtr* %"$this", %CStrPtr** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @"()"(%StringRef* sret %_result, %MainParameters* %"$this") #2 {
  %_result.addr = alloca %StringRef*
  store %StringRef* %_result, %StringRef** %_result.addr
  %"$this.addr" = alloca %MainParameters*
  store %MainParameters* %"$this", %MainParameters** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef** %_result.addr
  %2 = load %MainParameters** %"$this.addr"
  %3 = getelementptr inbounds %MainParameters* %2, i32 0, i32 0
  %4 = load %CStrPtr* %3
  call void @toStringRef(%StringRef* %1, %CStrPtr %4)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @"()98"(%StringRef* sret %_result, %MainParameters* %"$this", i64 %n) #2 {
  %_result.addr = alloca %StringRef*
  store %StringRef* %_result, %StringRef** %_result.addr
  %"$this.addr" = alloca %MainParameters*
  store %MainParameters* %"$this", %MainParameters** %"$this.addr"
  %n.addr = alloca i64
  store i64 %n, i64* %n.addr
  %"$tmpC" = alloca %CStrPtr
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef** %_result.addr
  %2 = load %MainParameters** %"$this.addr"
  %3 = getelementptr inbounds %MainParameters* %2, i32 0, i32 0
  %4 = load %CStrPtr* %3
  %5 = load i64* %n.addr
  call void @"+99"(%CStrPtr* %"$tmpC", %CStrPtr %4, i64 %5)
  %6 = load %CStrPtr* %"$tmpC"
  call void @toStringRef(%StringRef* %1, %CStrPtr %6)
  call void @dtor97(%CStrPtr* %"$tmpC")
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor97(%CStrPtr* %"$tmpC")
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @"+99"(%CStrPtr* sret %_result, %CStrPtr %p, i64 %n) #2 {
  %_result.addr = alloca %CStrPtr*
  store %CStrPtr* %_result, %CStrPtr** %_result.addr
  %p.addr = alloca %CStrPtr
  store %CStrPtr %p, %CStrPtr* %p.addr
  %n.addr = alloca i64
  store i64 %n, i64* %n.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %CStrPtr** %_result.addr
  %2 = load %CStrPtr* %p.addr
  %3 = call i8* @toBytePtr(%CStrPtr %2)
  %4 = load i64* %n.addr
  %5 = call i64 @"*"(i64 %4, i64 8)
  %6 = call i8* @ptrAdd(i8* %3, i64 %5)
  call void @fromBytePtr(%CStrPtr* %1, i8* %6)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @popFront(%MainParameters* %"$this") #2 {
  %"$this.addr" = alloca %MainParameters*
  store %MainParameters* %"$this", %MainParameters** %"$this.addr"
  %"$tmpC" = alloca %CStrPtr
  br label %code

code:                                             ; preds = %0
  %1 = load %MainParameters** %"$this.addr"
  %2 = getelementptr inbounds %MainParameters* %1, i32 0, i32 0
  %3 = load %MainParameters** %"$this.addr"
  %4 = getelementptr inbounds %MainParameters* %3, i32 0, i32 0
  %5 = load %CStrPtr* %4
  call void @"+99"(%CStrPtr* %"$tmpC", %CStrPtr %5, i64 1)
  call void @"="(%CStrPtr* %2, %CStrPtr* %"$tmpC")
  call void @dtor97(%CStrPtr* %"$tmpC")
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @"="(%CStrPtr* %"$this", %CStrPtr* %other) #2 {
  %"$this.addr" = alloca %CStrPtr*
  store %CStrPtr* %"$this", %CStrPtr** %"$this.addr"
  %other.addr = alloca %CStrPtr*
  store %CStrPtr* %other, %CStrPtr** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %CStrPtr** %other.addr
  %2 = getelementptr inbounds %CStrPtr* %1, i32 0, i32 0
  %3 = load %CStr** %2
  %4 = load %CStrPtr** %"$this.addr"
  %5 = getelementptr inbounds %CStrPtr* %4, i32 0, i32 0
  store %CStr* %3, %CStr** %5
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @popBack(%MainParameters* %"$this") #2 {
  %"$this.addr" = alloca %MainParameters*
  store %MainParameters* %"$this", %MainParameters** %"$this.addr"
  %"$tmpC" = alloca %CStrPtr
  br label %code

code:                                             ; preds = %0
  %1 = load %MainParameters** %"$this.addr"
  %2 = getelementptr inbounds %MainParameters* %1, i32 0, i32 1
  %3 = load %MainParameters** %"$this.addr"
  %4 = getelementptr inbounds %MainParameters* %3, i32 0, i32 1
  %5 = load %CStrPtr* %4
  call void @-95(%CStrPtr* %"$tmpC", %CStrPtr %5, i64 1)
  call void @"="(%CStrPtr* %2, %CStrPtr* %"$tmpC")
  call void @dtor97(%CStrPtr* %"$tmpC")
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @popFront100(%MainParameters* %"$this", i64 %n) #2 {
  %"$this.addr" = alloca %MainParameters*
  store %MainParameters* %"$this", %MainParameters** %"$this.addr"
  %n.addr = alloca i64
  store i64 %n, i64* %n.addr
  %"$tmpC" = alloca %CStrPtr
  br label %code

code:                                             ; preds = %0
  %1 = load %MainParameters** %"$this.addr"
  %2 = getelementptr inbounds %MainParameters* %1, i32 0, i32 0
  %3 = load %MainParameters** %"$this.addr"
  %4 = getelementptr inbounds %MainParameters* %3, i32 0, i32 0
  %5 = load %CStrPtr* %4
  %6 = load i64* %n.addr
  call void @"+99"(%CStrPtr* %"$tmpC", %CStrPtr %5, i64 %6)
  call void @"="(%CStrPtr* %2, %CStrPtr* %"$tmpC")
  call void @dtor97(%CStrPtr* %"$tmpC")
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @popBack101(%MainParameters* %"$this", i64 %n) #2 {
  %"$this.addr" = alloca %MainParameters*
  store %MainParameters* %"$this", %MainParameters** %"$this.addr"
  %n.addr = alloca i64
  store i64 %n, i64* %n.addr
  %"$tmpC" = alloca %CStrPtr
  br label %code

code:                                             ; preds = %0
  %1 = load %MainParameters** %"$this.addr"
  %2 = getelementptr inbounds %MainParameters* %1, i32 0, i32 1
  %3 = load %MainParameters** %"$this.addr"
  %4 = getelementptr inbounds %MainParameters* %3, i32 0, i32 1
  %5 = load %CStrPtr* %4
  %6 = load i64* %n.addr
  call void @-95(%CStrPtr* %"$tmpC", %CStrPtr %5, i64 %6)
  call void @"="(%CStrPtr* %2, %CStrPtr* %"$tmpC")
  call void @dtor97(%CStrPtr* %"$tmpC")
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctor102(%MainParameters* %"$this", i32 %argc, i8** %argv) #3 {
  %"$this.addr" = alloca %MainParameters*
  store %MainParameters* %"$this", %MainParameters** %"$this.addr"
  %argc.addr = alloca i32
  store i32 %argc, i32* %argc.addr
  %argv.addr = alloca i8**
  store i8** %argv, i8*** %argv.addr
  %"$tmpC" = alloca %CStrPtr
  %"$tmpC1" = alloca %CStrPtr
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load %MainParameters** %"$this.addr"
  %2 = getelementptr inbounds %MainParameters* %1, i32 0, i32 0
  call void @ctor103(%CStrPtr* %2)
  %3 = load %MainParameters** %"$this.addr"
  %4 = getelementptr inbounds %MainParameters* %3, i32 0, i32 1
  call void @ctor103(%CStrPtr* %4)
  %5 = load %MainParameters** %"$this.addr"
  %6 = getelementptr inbounds %MainParameters* %5, i32 0, i32 0
  %7 = load i8*** %argv.addr
  call void @fromArgvPtr(%CStrPtr* %"$tmpC", i8** %7)
  call void @"="(%CStrPtr* %6, %CStrPtr* %"$tmpC")
  call void @dtor97(%CStrPtr* %"$tmpC")
  %8 = load %MainParameters** %"$this.addr"
  %9 = getelementptr inbounds %MainParameters* %8, i32 0, i32 1
  %10 = load %MainParameters** %"$this.addr"
  %11 = getelementptr inbounds %MainParameters* %10, i32 0, i32 0
  %12 = load %CStrPtr* %11
  %13 = load i32* %argc.addr
  %14 = zext i32 %13 to i64
  store i64 %14, i64* %tmp.v
  %15 = load i64* %tmp.v
  call void @"+99"(%CStrPtr* %"$tmpC1", %CStrPtr %12, i64 %15)
  call void @"="(%CStrPtr* %9, %CStrPtr* %"$tmpC1")
  call void @dtor97(%CStrPtr* %"$tmpC1")
  call void @dtor31(i64* %tmp.v)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor103(%CStrPtr* %"$this") #2 {
  %"$this.addr" = alloca %CStrPtr*
  store %CStrPtr* %"$this", %CStrPtr** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %CStrPtr** %"$this.addr"
  %2 = getelementptr inbounds %CStrPtr* %1, i32 0, i32 0
  store %CStr* null, %CStr** %2
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @fromArgvPtr(%CStrPtr* sret %_result, i8** %argv) #2 {
  %_result.addr = alloca %CStrPtr*
  store %CStrPtr* %_result, %CStrPtr** %_result.addr
  %argv.addr = alloca i8**
  store i8** %argv, i8*** %argv.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %CStrPtr** %_result.addr
  %2 = load i8*** %argv.addr
  %3 = bitcast i8** %2 to %CStr*
  call void @ctor96(%CStrPtr* %1, %CStr* %3)
  ret void
}

define private void @__global_ctor104() {
  br label %code

code:                                             ; preds = %0
  call void @ctor105(%MainParameters* @programArgs)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor105(%MainParameters* %"$this") #2 {
  %"$this.addr" = alloca %MainParameters*
  store %MainParameters* %"$this", %MainParameters** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %MainParameters** %"$this.addr"
  %2 = getelementptr inbounds %MainParameters* %1, i32 0, i32 0
  call void @ctor103(%CStrPtr* %2)
  %3 = load %MainParameters** %"$this.addr"
  %4 = getelementptr inbounds %MainParameters* %3, i32 0, i32 1
  call void @ctor103(%CStrPtr* %4)
  ret void
}

define private void @__global_dtor106() {
  br label %code

code:                                             ; preds = %0
  call void @dtor107(%MainParameters* @programArgs)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor107(%MainParameters* %"$this") #2 {
  %"$this.addr" = alloca %MainParameters*
  store %MainParameters* %"$this", %MainParameters** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %MainParameters** %"$this.addr"
  %2 = getelementptr inbounds %MainParameters* %1, i32 0, i32 1
  call void @dtor97(%CStrPtr* %2)
  %3 = load %MainParameters** %"$this.addr"
  %4 = getelementptr inbounds %MainParameters* %3, i32 0, i32 0
  call void @dtor97(%CStrPtr* %4)
  ret void
}

; Function Attrs: nounwind
declare void @sprMain() #1

; Function Attrs: alwaysinline nounwind
define private i32 @asInt(%StringRef %src) #2 {
  %src.addr = alloca %StringRef
  store %StringRef %src, %StringRef* %src.addr
  br label %code

code:                                             ; preds = %0
  %1 = call i8* @cStr(%StringRef* %src.addr)
  %2 = call i32 @atoi(i8* %1)
  ret i32 %2
}

declare i32 @atoi(i8*)

; Function Attrs: alwaysinline nounwind
define private i8* @cStr(%StringRef* %"$this") #2 {
  %"$this.addr" = alloca %StringRef*
  store %StringRef* %"$this", %StringRef** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef** %"$this.addr"
  %2 = getelementptr inbounds %StringRef* %1, i32 0, i32 0
  %3 = load i8** %2
  ret i8* %3
}

; Function Attrs: alwaysinline nounwind
define private i64 @asLong(%StringRef %src) #2 {
  %src.addr = alloca %StringRef
  store %StringRef %src, %StringRef* %src.addr
  br label %code

code:                                             ; preds = %0
  %1 = call i8* @cStr(%StringRef* %src.addr)
  %2 = call i64 @atoll(i8* %1)
  ret i64 %2
}

declare i64 @atoll(i8*)

; Function Attrs: alwaysinline nounwind
define private double @asDouble(%StringRef %src) #2 {
  %src.addr = alloca %StringRef
  store %StringRef %src, %StringRef* %src.addr
  br label %code

code:                                             ; preds = %0
  %1 = call i8* @cStr(%StringRef* %src.addr)
  %2 = call double @atof(i8* %1)
  ret double %2
}

declare double @atof(i8*)

declare void @comp_scannerIf_reportError(%Location, %StringRef)

; Function Attrs: inlinehint nounwind
define private void @reportError(%ExternalErrorReporter* %"$this", %Location %loc, %String* %msg) #3 {
  %"$this.addr" = alloca %ExternalErrorReporter*
  store %ExternalErrorReporter* %"$this", %ExternalErrorReporter** %"$this.addr"
  %loc.addr = alloca %Location
  store %Location %loc, %Location* %loc.addr
  %msg.addr = alloca %String*
  store %String* %msg, %String** %msg.addr
  %"$tmpC" = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %Location* %loc.addr
  %2 = load %String** %msg.addr
  call void @asStringRef(%StringRef* %"$tmpC", %String* %2)
  %3 = load %StringRef* %"$tmpC"
  call void @comp_scannerIf_reportError(%Location %1, %StringRef %3)
  call void @dtor78(%StringRef* %"$tmpC")
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @asStringRef(%StringRef* sret %_result, %String* %"$this") #2 {
  %_result.addr = alloca %StringRef*
  store %StringRef* %_result, %StringRef** %_result.addr
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef** %_result.addr
  %2 = load %String** %"$this.addr"
  %3 = getelementptr inbounds %String* %2, i32 0, i32 0
  %4 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %3)
  %5 = load %String** %"$this.addr"
  %6 = getelementptr inbounds %String* %5, i32 0, i32 1
  %7 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %6)
  call void @ctor108(%StringRef* %1, i8* %4, i8* %7)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctor108(%StringRef* %"$this", i8* %begin, i8* %end) #3 {
  %"$this.addr" = alloca %StringRef*
  store %StringRef* %"$this", %StringRef** %"$this.addr"
  %begin.addr = alloca i8*
  store i8* %begin, i8** %begin.addr
  %end.addr = alloca i8*
  store i8* %end, i8** %end.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef** %"$this.addr"
  %2 = getelementptr inbounds %StringRef* %1, i32 0, i32 0
  store i8* null, i8** %2
  %3 = load %StringRef** %"$this.addr"
  %4 = getelementptr inbounds %StringRef* %3, i32 0, i32 1
  store i8* null, i8** %4
  %5 = load i8** %begin.addr
  %6 = load %StringRef** %"$this.addr"
  %7 = getelementptr inbounds %StringRef* %6, i32 0, i32 0
  store i8* %5, i8** %7
  %8 = load i8** %end.addr
  %9 = load %StringRef** %"$this.addr"
  %10 = getelementptr inbounds %StringRef* %9, i32 0, i32 1
  store i8* %8, i8** %10
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i8* @bytePtr(%"RawPtr[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"RawPtr[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"RawPtr[Char/rtct]"* %1, i32 0, i32 0
  %3 = load i8** %2
  ret i8* %3
}

; Function Attrs: inlinehint nounwind
define private void @ctor109(%ScannerContext* %"$this", %StringRef %filename, %StringRef %code, %Location %loc) #3 {
  %"$this.addr" = alloca %ScannerContext*
  store %ScannerContext* %"$this", %ScannerContext** %"$this.addr"
  %filename.addr = alloca %StringRef
  store %StringRef %filename, %StringRef* %filename.addr
  %code.addr = alloca %StringRef
  store %StringRef %code, %StringRef* %code.addr
  %loc.addr = alloca %Location
  store %Location %loc, %Location* %loc.addr
  %reporter = alloca %ExternalErrorReporter
  %const.bytes = alloca [2 x i8]
  %const.struct = alloca %StringRef
  %"$tmpC" = alloca %FileRange
  %"$tmpC5" = alloca %String
  %const.bytes6 = alloca [19 x i8]
  %const.struct7 = alloca %StringRef
  br label %code1

code1:                                            ; preds = %0
  %1 = load %ScannerContext** %"$this.addr"
  %2 = getelementptr inbounds %ScannerContext* %1, i32 0, i32 0
  call void @ctor110(%File* %2)
  %3 = load %ScannerContext** %"$this.addr"
  %4 = getelementptr inbounds %ScannerContext* %3, i32 0, i32 1
  call void @ctor111(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %4)
  %5 = load %ScannerContext** %"$this.addr"
  %6 = getelementptr inbounds %ScannerContext* %5, i32 0, i32 2
  call void @ctor125(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %6)
  call void @ctor124(%ExternalErrorReporter* %reporter)
  br label %if_block

if_block:                                         ; preds = %code1
  %7 = call i1 @isEmpty128(%StringRef* %code.addr)
  br i1 %7, label %if_then, label %if_else

if_then:                                          ; preds = %if_block
  %8 = load %ScannerContext** %"$this.addr"
  %9 = getelementptr inbounds %ScannerContext* %8, i32 0, i32 0
  %10 = load %StringRef* %filename.addr
  store [2 x i8] c"r\00", [2 x i8]* %const.bytes
  %11 = getelementptr inbounds [2 x i8]* %const.bytes, i32 0, i32 0
  %12 = getelementptr inbounds [2 x i8]* %const.bytes, i32 0, i32 1
  %13 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %14 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %11, i8** %13
  store i8* %12, i8** %14
  %15 = load %StringRef* %const.struct
  call void @ctor132(%File* %9, %StringRef %10, %StringRef %15)
  %16 = load %ScannerContext** %"$this.addr"
  %17 = getelementptr inbounds %ScannerContext* %16, i32 0, i32 1
  %18 = load %ScannerContext** %"$this.addr"
  %19 = getelementptr inbounds %ScannerContext* %18, i32 0, i32 0
  call void @all(%FileRange* %"$tmpC", %File* %19)
  %20 = load %FileRange* %"$tmpC"
  %21 = load %ExternalErrorReporter* %reporter
  call void @ctor133(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %17, %FileRange %20, %ExternalErrorReporter %21, %Location* %loc.addr)
  call void @dtor166(%FileRange* %"$tmpC")
  %22 = load %ScannerContext** %"$this.addr"
  %23 = getelementptr inbounds %ScannerContext* %22, i32 0, i32 2
  call void @ctor125(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %23)
  br label %if_block2

if_else:                                          ; preds = %if_block
  %24 = load %ScannerContext** %"$this.addr"
  %25 = getelementptr inbounds %ScannerContext* %24, i32 0, i32 0
  %26 = load %ScannerContext** %"$this.addr"
  %27 = getelementptr inbounds %ScannerContext* %26, i32 0, i32 1
  call void @ctor111(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %27)
  %28 = load %ScannerContext** %"$this.addr"
  %29 = getelementptr inbounds %ScannerContext* %28, i32 0, i32 2
  %30 = load %StringRef* %code.addr
  %31 = load %ExternalErrorReporter* %reporter
  call void @ctor187(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %29, %StringRef %30, %ExternalErrorReporter %31, %Location* %loc.addr)
  br label %if_end

if_end:                                           ; preds = %if_else, %if_end4
  call void @dtor197(%ExternalErrorReporter* %reporter)
  ret void

if_block2:                                        ; preds = %if_then
  %32 = load %ScannerContext** %"$this.addr"
  %33 = getelementptr inbounds %ScannerContext* %32, i32 0, i32 0
  %34 = call i1 @isOpen(%File* %33)
  %35 = xor i1 true, %34
  br i1 %35, label %if_then3, label %if_end4

if_then3:                                         ; preds = %if_block2
  %36 = load %Location* %loc.addr
  store [19 x i8] c"Cannot open file: \00", [19 x i8]* %const.bytes6
  %37 = getelementptr inbounds [19 x i8]* %const.bytes6, i32 0, i32 0
  %38 = getelementptr inbounds [19 x i8]* %const.bytes6, i32 0, i32 18
  %39 = getelementptr inbounds %StringRef* %const.struct7, i32 0, i32 0
  %40 = getelementptr inbounds %StringRef* %const.struct7, i32 0, i32 1
  store i8* %37, i8** %39
  store i8* %38, i8** %40
  %41 = load %StringRef* %const.struct7
  %42 = load %StringRef* %filename.addr
  call void @toString(%String* %"$tmpC5", %StringRef %41, %StringRef %42)
  call void @reportError(%ExternalErrorReporter* %reporter, %Location %36, %String* %"$tmpC5")
  call void @dtor186(%String* %"$tmpC5")
  br label %if_end4

if_end4:                                          ; preds = %if_then3, %if_block2
  br label %if_end
}

; Function Attrs: inlinehint nounwind
define private void @ctor110(%File* %"$this") #3 {
  %"$this.addr" = alloca %File*
  store %File* %"$this", %File** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %File** %"$this.addr"
  %2 = getelementptr inbounds %File* %1, i32 0, i32 0
  store i8* null, i8** %2
  %3 = load %File** %"$this.addr"
  %4 = getelementptr inbounds %File* %3, i32 0, i32 0
  store i8* null, i8** %4
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor111(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %"$this") #2 {
  %"$this.addr" = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %"$this", %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  %2 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1, i32 0, i32 0
  call void @ctor112(%Location* %2)
  %3 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  %4 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %3, i32 0, i32 1
  call void @ctor116(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %4)
  %5 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  %6 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %5, i32 0, i32 2
  call void @ctor121(%Token* %6)
  %7 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  %8 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %7, i32 0, i32 3
  store i1 false, i1* %8
  %9 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  %10 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %9, i32 0, i32 4
  store i1 false, i1* %10
  %11 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  %12 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %11, i32 0, i32 5
  call void @ctor124(%ExternalErrorReporter* %12)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor112(%Location* %"$this") #2 {
  %"$this.addr" = alloca %Location*
  store %Location* %"$this", %Location** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %Location** %"$this.addr"
  %2 = getelementptr inbounds %Location* %1, i32 0, i32 0
  call void @ctor113(%SourceCode* %2)
  %3 = load %Location** %"$this.addr"
  %4 = getelementptr inbounds %Location* %3, i32 0, i32 1
  call void @ctor115(%LineCol* %4)
  %5 = load %Location** %"$this.addr"
  %6 = getelementptr inbounds %Location* %5, i32 0, i32 2
  call void @ctor115(%LineCol* %6)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor113(%SourceCode* %"$this") #2 {
  %"$this.addr" = alloca %SourceCode*
  store %SourceCode* %"$this", %SourceCode** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %SourceCode** %"$this.addr"
  %2 = getelementptr inbounds %SourceCode* %1, i32 0, i32 0
  call void @ctor114(%UntypedPtr* %2)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor114(%UntypedPtr* %"$this") #2 {
  %"$this.addr" = alloca %UntypedPtr*
  store %UntypedPtr* %"$this", %UntypedPtr** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %UntypedPtr** %"$this.addr"
  %2 = getelementptr inbounds %UntypedPtr* %1, i32 0, i32 0
  store i8* null, i8** %2
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor115(%LineCol* %"$this") #2 {
  %"$this.addr" = alloca %LineCol*
  store %LineCol* %"$this", %LineCol** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %LineCol** %"$this.addr"
  %2 = getelementptr inbounds %LineCol* %1, i32 0, i32 0
  store i32 0, i32* %2
  %3 = load %LineCol** %"$this.addr"
  %4 = getelementptr inbounds %LineCol* %3, i32 0, i32 1
  store i32 0, i32* %4
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor116(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %"$this") #2 {
  %"$this.addr" = alloca %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %"$this", %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %"$this.addr"
  %2 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %1, i32 0, i32 0
  call void @ctor117(%"RangeWithLookahead[FileRange/rtct]"* %2)
  %3 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %"$this.addr"
  %4 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %3, i32 0, i32 1
  store %Location* null, %Location** %4
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor117(%"RangeWithLookahead[FileRange/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"RangeWithLookahead[FileRange/rtct]"*
  store %"RangeWithLookahead[FileRange/rtct]"* %"$this", %"RangeWithLookahead[FileRange/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"RangeWithLookahead[FileRange/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %1, i32 0, i32 0
  call void @ctor118(%FileRange* %2)
  %3 = load %"RangeWithLookahead[FileRange/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %3, i32 0, i32 1
  call void @ctor119(%"Vector[Char/rtct]"* %4)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor118(%FileRange* %"$this") #2 {
  %"$this.addr" = alloca %FileRange*
  store %FileRange* %"$this", %FileRange** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %FileRange** %"$this.addr"
  %2 = getelementptr inbounds %FileRange* %1, i32 0, i32 0
  store %File* null, %File** %2
  %3 = load %FileRange** %"$this.addr"
  %4 = getelementptr inbounds %FileRange* %3, i32 0, i32 1
  store i1 false, i1* %4
  %5 = load %FileRange** %"$this.addr"
  %6 = getelementptr inbounds %FileRange* %5, i32 0, i32 2
  store i8 0, i8* %6
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor119(%"Vector[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"Vector[Char/rtct]"* %1, i32 0, i32 0
  call void @ctor120(%"RawPtr[Char/rtct]"* %2)
  %3 = load %"Vector[Char/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"Vector[Char/rtct]"* %3, i32 0, i32 1
  call void @ctor120(%"RawPtr[Char/rtct]"* %4)
  %5 = load %"Vector[Char/rtct]"** %"$this.addr"
  %6 = getelementptr inbounds %"Vector[Char/rtct]"* %5, i32 0, i32 2
  call void @ctor120(%"RawPtr[Char/rtct]"* %6)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor120(%"RawPtr[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"RawPtr[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"RawPtr[Char/rtct]"* %1, i32 0, i32 0
  store i8* null, i8** %2
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor121(%Token* %"$this") #2 {
  %"$this.addr" = alloca %Token*
  store %Token* %"$this", %Token** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %Token** %"$this.addr"
  %2 = getelementptr inbounds %Token* %1, i32 0, i32 0
  call void @ctor112(%Location* %2)
  %3 = load %Token** %"$this.addr"
  %4 = getelementptr inbounds %Token* %3, i32 0, i32 1
  call void @ctor122(%TokenType* %4)
  %5 = load %Token** %"$this.addr"
  %6 = getelementptr inbounds %Token* %5, i32 0, i32 2
  call void @ctor123(%String* %6)
  %7 = load %Token** %"$this.addr"
  %8 = getelementptr inbounds %Token* %7, i32 0, i32 3
  store i64 0, i64* %8
  %9 = load %Token** %"$this.addr"
  %10 = getelementptr inbounds %Token* %9, i32 0, i32 4
  store double 0.000000e+00, double* %10
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor122(%TokenType* %"$this") #2 {
  %"$this.addr" = alloca %TokenType*
  store %TokenType* %"$this", %TokenType** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %TokenType** %"$this.addr"
  %2 = getelementptr inbounds %TokenType* %1, i32 0, i32 0
  store i32 0, i32* %2
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctor123(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  %2 = getelementptr inbounds %String* %1, i32 0, i32 0
  call void @ctor120(%"RawPtr[Char/rtct]"* %2)
  %3 = load %String** %"$this.addr"
  %4 = getelementptr inbounds %String* %3, i32 0, i32 1
  call void @ctor120(%"RawPtr[Char/rtct]"* %4)
  %5 = load %String** %"$this.addr"
  %6 = getelementptr inbounds %String* %5, i32 0, i32 2
  call void @ctor120(%"RawPtr[Char/rtct]"* %6)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor124(%ExternalErrorReporter* %"$this") #2 {
  %"$this.addr" = alloca %ExternalErrorReporter*
  store %ExternalErrorReporter* %"$this", %ExternalErrorReporter** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor125(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %"$this") #2 {
  %"$this.addr" = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %"$this", %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  %2 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1, i32 0, i32 0
  call void @ctor112(%Location* %2)
  %3 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  %4 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %3, i32 0, i32 1
  call void @ctor126(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %4)
  %5 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  %6 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %5, i32 0, i32 2
  call void @ctor121(%Token* %6)
  %7 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  %8 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %7, i32 0, i32 3
  store i1 false, i1* %8
  %9 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  %10 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %9, i32 0, i32 4
  store i1 false, i1* %10
  %11 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  %12 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %11, i32 0, i32 5
  call void @ctor124(%ExternalErrorReporter* %12)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor126(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %"$this") #2 {
  %"$this.addr" = alloca %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %"$this", %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %"$this.addr"
  %2 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %1, i32 0, i32 0
  call void @ctor127(%"RangeWithLookahead[StringRef/rtct]"* %2)
  %3 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %"$this.addr"
  %4 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %3, i32 0, i32 1
  store %Location* null, %Location** %4
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor127(%"RangeWithLookahead[StringRef/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"RangeWithLookahead[StringRef/rtct]"*
  store %"RangeWithLookahead[StringRef/rtct]"* %"$this", %"RangeWithLookahead[StringRef/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"RangeWithLookahead[StringRef/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %1, i32 0, i32 0
  call void @ctor94(%StringRef* %2)
  %3 = load %"RangeWithLookahead[StringRef/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %3, i32 0, i32 1
  call void @ctor119(%"Vector[Char/rtct]"* %4)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i1 @isEmpty128(%StringRef* %"$this") #2 {
  %"$this.addr" = alloca %StringRef*
  store %StringRef* %"$this", %StringRef** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef** %"$this.addr"
  %2 = getelementptr inbounds %StringRef* %1, i32 0, i32 1
  %3 = load i8** %2
  %4 = load %StringRef** %"$this.addr"
  %5 = getelementptr inbounds %StringRef* %4, i32 0, i32 0
  %6 = load i8** %5
  %7 = call i64 @ptrDiff(i8* %3, i8* %6)
  %8 = call i1 @"==129"(i64 %7, i32 0)
  ret i1 %8
}

; Function Attrs: alwaysinline nounwind
define private i1 @"==129"(i64 %x, i32 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i32* %y.addr
  %3 = call i64 @cmn130(i64 %1, i32 %2)
  %4 = load i32* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn131(i32 %4, i64 %5)
  %7 = icmp eq i64 %3, %6
  ret i1 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @cmn130(i64 %x, i32 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  ret i64 %1
}

; Function Attrs: alwaysinline nounwind
define private i64 @cmn131(i32 %x, i64 %y) #2 {
  %x.addr = alloca i32
  store i32 %x, i32* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i32* %x.addr
  %2 = sext i32 %1 to i64
  store i64 %2, i64* %tmp.v
  %3 = load i64* %tmp.v
  call void @dtor35(i64* %tmp.v)
  ret i64 %3

dumy_block:                                       ; No predecessors!
  call void @dtor35(i64* %tmp.v)
  unreachable
}

; Function Attrs: inlinehint nounwind
define private void @ctor132(%File* %"$this", %StringRef %filename, %StringRef %mode) #3 {
  %"$this.addr" = alloca %File*
  store %File* %"$this", %File** %"$this.addr"
  %filename.addr = alloca %StringRef
  store %StringRef %filename, %StringRef* %filename.addr
  %mode.addr = alloca %StringRef
  store %StringRef %mode, %StringRef* %mode.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %File** %"$this.addr"
  %2 = getelementptr inbounds %File* %1, i32 0, i32 0
  store i8* null, i8** %2
  %3 = call i8* @cStr(%StringRef* %filename.addr)
  %4 = call i8* @cStr(%StringRef* %mode.addr)
  %5 = call i8* @fopen(i8* %3, i8* %4)
  %6 = load %File** %"$this.addr"
  %7 = getelementptr inbounds %File* %6, i32 0, i32 0
  store i8* %5, i8** %7
  ret void
}

declare i8* @fopen(i8*, i8*)

; Function Attrs: inlinehint nounwind
define private void @ctor133(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %"$this", %FileRange %source, %ExternalErrorReporter %errorReporter, %Location* %iniLocation) #3 {
  %"$this.addr" = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %"$this", %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  %source.addr = alloca %FileRange
  store %FileRange %source, %FileRange* %source.addr
  %errorReporter.addr = alloca %ExternalErrorReporter
  store %ExternalErrorReporter %errorReporter, %ExternalErrorReporter* %errorReporter.addr
  %iniLocation.addr = alloca %Location*
  store %Location* %iniLocation, %Location** %iniLocation.addr
  %tmp.v = alloca %"RangeWithLookahead[FileRange/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  %2 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1, i32 0, i32 5
  call void @ctor124(%ExternalErrorReporter* %2)
  %3 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  %4 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %3, i32 0, i32 0
  %5 = load %Location** %iniLocation.addr
  call void @ctor134(%Location* %4, %Location* %5)
  %6 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  %7 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %6, i32 0, i32 1
  %8 = load %FileRange* %source.addr
  call void @ctor150(%"RangeWithLookahead[FileRange/rtct]"* %tmp.v, %FileRange %8)
  %9 = load %"RangeWithLookahead[FileRange/rtct]"* %tmp.v
  %10 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  %11 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %10, i32 0, i32 0
  call void @ctor138(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %7, %"RangeWithLookahead[FileRange/rtct]" %9, %Location* %11)
  call void @dtor164(%"RangeWithLookahead[FileRange/rtct]"* %tmp.v)
  %12 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  %13 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %12, i32 0, i32 2
  call void @ctor121(%Token* %13)
  %14 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  %15 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %14, i32 0, i32 3
  store i1 false, i1* %15
  %16 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  %17 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %16, i32 0, i32 4
  store i1 false, i1* %17
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor134(%Location* %"$this", %Location* %other) #2 {
  %"$this.addr" = alloca %Location*
  store %Location* %"$this", %Location** %"$this.addr"
  %other.addr = alloca %Location*
  store %Location* %other, %Location** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %Location** %"$this.addr"
  %2 = getelementptr inbounds %Location* %1, i32 0, i32 0
  %3 = load %Location** %other.addr
  %4 = getelementptr inbounds %Location* %3, i32 0, i32 0
  call void @ctor135(%SourceCode* %2, %SourceCode* %4)
  %5 = load %Location** %"$this.addr"
  %6 = getelementptr inbounds %Location* %5, i32 0, i32 1
  %7 = load %Location** %other.addr
  %8 = getelementptr inbounds %Location* %7, i32 0, i32 1
  call void @ctor137(%LineCol* %6, %LineCol* %8)
  %9 = load %Location** %"$this.addr"
  %10 = getelementptr inbounds %Location* %9, i32 0, i32 2
  %11 = load %Location** %other.addr
  %12 = getelementptr inbounds %Location* %11, i32 0, i32 2
  call void @ctor137(%LineCol* %10, %LineCol* %12)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor135(%SourceCode* %"$this", %SourceCode* %other) #2 {
  %"$this.addr" = alloca %SourceCode*
  store %SourceCode* %"$this", %SourceCode** %"$this.addr"
  %other.addr = alloca %SourceCode*
  store %SourceCode* %other, %SourceCode** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %SourceCode** %"$this.addr"
  %2 = getelementptr inbounds %SourceCode* %1, i32 0, i32 0
  %3 = load %SourceCode** %other.addr
  %4 = getelementptr inbounds %SourceCode* %3, i32 0, i32 0
  call void @ctor136(%UntypedPtr* %2, %UntypedPtr* %4)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor136(%UntypedPtr* %"$this", %UntypedPtr* %other) #2 {
  %"$this.addr" = alloca %UntypedPtr*
  store %UntypedPtr* %"$this", %UntypedPtr** %"$this.addr"
  %other.addr = alloca %UntypedPtr*
  store %UntypedPtr* %other, %UntypedPtr** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %UntypedPtr** %other.addr
  %2 = getelementptr inbounds %UntypedPtr* %1, i32 0, i32 0
  %3 = load i8** %2
  %4 = load %UntypedPtr** %"$this.addr"
  %5 = getelementptr inbounds %UntypedPtr* %4, i32 0, i32 0
  store i8* %3, i8** %5
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor137(%LineCol* %"$this", %LineCol* %other) #2 {
  %"$this.addr" = alloca %LineCol*
  store %LineCol* %"$this", %LineCol** %"$this.addr"
  %other.addr = alloca %LineCol*
  store %LineCol* %other, %LineCol** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %LineCol** %other.addr
  %2 = getelementptr inbounds %LineCol* %1, i32 0, i32 0
  %3 = load i32* %2
  %4 = load %LineCol** %"$this.addr"
  %5 = getelementptr inbounds %LineCol* %4, i32 0, i32 0
  store i32 %3, i32* %5
  %6 = load %LineCol** %other.addr
  %7 = getelementptr inbounds %LineCol* %6, i32 0, i32 1
  %8 = load i32* %7
  %9 = load %LineCol** %"$this.addr"
  %10 = getelementptr inbounds %LineCol* %9, i32 0, i32 1
  store i32 %8, i32* %10
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor138(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %"$this", %"RangeWithLookahead[FileRange/rtct]" %fsource, %Location* %flocation) #2 {
  %"$this.addr" = alloca %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %"$this", %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %"$this.addr"
  %fsource.addr = alloca %"RangeWithLookahead[FileRange/rtct]"
  store %"RangeWithLookahead[FileRange/rtct]" %fsource, %"RangeWithLookahead[FileRange/rtct]"* %fsource.addr
  %flocation.addr = alloca %Location*
  store %Location* %flocation, %Location** %flocation.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %"$this.addr"
  %2 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %1, i32 0, i32 0
  call void @ctor139(%"RangeWithLookahead[FileRange/rtct]"* %2, %"RangeWithLookahead[FileRange/rtct]"* %fsource.addr)
  %3 = load %Location** %flocation.addr
  %4 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %"$this.addr"
  %5 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %4, i32 0, i32 1
  store %Location* %3, %Location** %5
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor139(%"RangeWithLookahead[FileRange/rtct]"* %"$this", %"RangeWithLookahead[FileRange/rtct]"* %other) #2 {
  %"$this.addr" = alloca %"RangeWithLookahead[FileRange/rtct]"*
  store %"RangeWithLookahead[FileRange/rtct]"* %"$this", %"RangeWithLookahead[FileRange/rtct]"** %"$this.addr"
  %other.addr = alloca %"RangeWithLookahead[FileRange/rtct]"*
  store %"RangeWithLookahead[FileRange/rtct]"* %other, %"RangeWithLookahead[FileRange/rtct]"** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"RangeWithLookahead[FileRange/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %1, i32 0, i32 0
  %3 = load %"RangeWithLookahead[FileRange/rtct]"** %other.addr
  %4 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %3, i32 0, i32 0
  call void @ctor140(%FileRange* %2, %FileRange* %4)
  %5 = load %"RangeWithLookahead[FileRange/rtct]"** %"$this.addr"
  %6 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %5, i32 0, i32 1
  %7 = load %"RangeWithLookahead[FileRange/rtct]"** %other.addr
  %8 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %7, i32 0, i32 1
  call void @ctor141(%"Vector[Char/rtct]"* %6, %"Vector[Char/rtct]"* %8)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor140(%FileRange* %"$this", %FileRange* %other) #2 {
  %"$this.addr" = alloca %FileRange*
  store %FileRange* %"$this", %FileRange** %"$this.addr"
  %other.addr = alloca %FileRange*
  store %FileRange* %other, %FileRange** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %FileRange** %other.addr
  %2 = getelementptr inbounds %FileRange* %1, i32 0, i32 0
  %3 = load %File** %2
  %4 = load %FileRange** %"$this.addr"
  %5 = getelementptr inbounds %FileRange* %4, i32 0, i32 0
  store %File* %3, %File** %5
  %6 = load %FileRange** %other.addr
  %7 = getelementptr inbounds %FileRange* %6, i32 0, i32 1
  %8 = load i1* %7
  %9 = load %FileRange** %"$this.addr"
  %10 = getelementptr inbounds %FileRange* %9, i32 0, i32 1
  store i1 %8, i1* %10
  %11 = load %FileRange** %other.addr
  %12 = getelementptr inbounds %FileRange* %11, i32 0, i32 2
  %13 = load i8* %12
  %14 = load %FileRange** %"$this.addr"
  %15 = getelementptr inbounds %FileRange* %14, i32 0, i32 2
  store i8 %13, i8* %15
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctor141(%"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"* %other) #3 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %other.addr = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %other, %"Vector[Char/rtct]"** %other.addr
  %size = alloca i64
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  %"$tmpC1" = alloca %"RawPtr[Char/rtct]"
  %dst = alloca %"RawPtr[Char/rtct]"
  %src = alloca %"RawPtr[Char/rtct]"
  %"$tmpC2" = alloca %"RawPtr[Char/rtct]"
  %"$tmpC3" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"Vector[Char/rtct]"* %1, i32 0, i32 0
  call void @ctor120(%"RawPtr[Char/rtct]"* %2)
  %3 = load %"Vector[Char/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"Vector[Char/rtct]"* %3, i32 0, i32 1
  call void @ctor120(%"RawPtr[Char/rtct]"* %4)
  %5 = load %"Vector[Char/rtct]"** %"$this.addr"
  %6 = getelementptr inbounds %"Vector[Char/rtct]"* %5, i32 0, i32 2
  call void @ctor120(%"RawPtr[Char/rtct]"* %6)
  %7 = load %"Vector[Char/rtct]"** %other.addr
  %8 = call i64 @size142(%"Vector[Char/rtct]"* %7)
  store i64 %8, i64* %size
  %9 = load %"Vector[Char/rtct]"** %"$this.addr"
  %10 = getelementptr inbounds %"Vector[Char/rtct]"* %9, i32 0, i32 0
  %11 = load i64* %size
  call void @alloc(%"RawPtr[Char/rtct]"* %"$tmpC", i64 %11)
  %12 = load %"RawPtr[Char/rtct]"* %"$tmpC"
  call void @"=144"(%"RawPtr[Char/rtct]"* %10, %"RawPtr[Char/rtct]" %12)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  %13 = load %"Vector[Char/rtct]"** %"$this.addr"
  %14 = getelementptr inbounds %"Vector[Char/rtct]"* %13, i32 0, i32 1
  %15 = load %"Vector[Char/rtct]"** %"$this.addr"
  %16 = getelementptr inbounds %"Vector[Char/rtct]"* %15, i32 0, i32 0
  %17 = load i64* %size
  call void @advance(%"RawPtr[Char/rtct]"* %"$tmpC1", %"RawPtr[Char/rtct]"* %16, i64 %17)
  %18 = load %"RawPtr[Char/rtct]"* %"$tmpC1"
  call void @"=144"(%"RawPtr[Char/rtct]"* %14, %"RawPtr[Char/rtct]" %18)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC1")
  %19 = load %"Vector[Char/rtct]"** %"$this.addr"
  %20 = getelementptr inbounds %"Vector[Char/rtct]"* %19, i32 0, i32 2
  %21 = load %"Vector[Char/rtct]"** %"$this.addr"
  %22 = getelementptr inbounds %"Vector[Char/rtct]"* %21, i32 0, i32 1
  %23 = load %"RawPtr[Char/rtct]"* %22
  call void @"=144"(%"RawPtr[Char/rtct]"* %20, %"RawPtr[Char/rtct]" %23)
  %24 = load %"Vector[Char/rtct]"** %"$this.addr"
  %25 = getelementptr inbounds %"Vector[Char/rtct]"* %24, i32 0, i32 0
  call void @ctor145(%"RawPtr[Char/rtct]"* %dst, %"RawPtr[Char/rtct]"* %25)
  %26 = load %"Vector[Char/rtct]"** %other.addr
  %27 = getelementptr inbounds %"Vector[Char/rtct]"* %26, i32 0, i32 0
  call void @ctor145(%"RawPtr[Char/rtct]"* %src, %"RawPtr[Char/rtct]"* %27)
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %28 = load %"Vector[Char/rtct]"** %"$this.addr"
  %29 = getelementptr inbounds %"Vector[Char/rtct]"* %28, i32 0, i32 1
  %30 = load %"RawPtr[Char/rtct]"* %29
  %31 = call i1 @"==148"(%"RawPtr[Char/rtct]"* %dst, %"RawPtr[Char/rtct]" %30)
  %32 = xor i1 true, %31
  br i1 %32, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  %33 = call i8* @value(%"RawPtr[Char/rtct]"* %src)
  %34 = load i8* %33
  %35 = call i8* @value(%"RawPtr[Char/rtct]"* %dst)
  store i8 %34, i8* %35
  call void @advance149(%"RawPtr[Char/rtct]"* %"$tmpC2", %"RawPtr[Char/rtct]"* %dst)
  %36 = load %"RawPtr[Char/rtct]"* %"$tmpC2"
  call void @"=144"(%"RawPtr[Char/rtct]"* %dst, %"RawPtr[Char/rtct]" %36)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC2")
  call void @advance149(%"RawPtr[Char/rtct]"* %"$tmpC3", %"RawPtr[Char/rtct]"* %src)
  %37 = load %"RawPtr[Char/rtct]"* %"$tmpC3"
  call void @"=144"(%"RawPtr[Char/rtct]"* %src, %"RawPtr[Char/rtct]" %37)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC3")
  br label %while_step

while_step:                                       ; preds = %while_body
  br label %while_block

while_end:                                        ; preds = %while_block
  call void @dtor146(%"RawPtr[Char/rtct]"* %src)
  call void @dtor146(%"RawPtr[Char/rtct]"* %dst)
  call void @dtor31(i64* %size)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i64 @size142(%"Vector[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"Vector[Char/rtct]"* %1, i32 0, i32 1
  %3 = load %"Vector[Char/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"Vector[Char/rtct]"* %3, i32 0, i32 0
  %5 = load %"RawPtr[Char/rtct]"* %4
  %6 = call i64 @diff(%"RawPtr[Char/rtct]"* %2, %"RawPtr[Char/rtct]" %5)
  store i64 %6, i64* %tmp.v
  %7 = load i64* %tmp.v
  call void @dtor31(i64* %tmp.v)
  ret i64 %7

dumy_block:                                       ; No predecessors!
  call void @dtor31(i64* %tmp.v)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i64 @diff(%"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]" %other) #2 {
  %"$this.addr" = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]"** %"$this.addr"
  %other.addr = alloca %"RawPtr[Char/rtct]"
  store %"RawPtr[Char/rtct]" %other, %"RawPtr[Char/rtct]"* %other.addr
  %tmp.v = alloca i64
  %tmp.v1 = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load %"RawPtr[Char/rtct]"** %"$this.addr"
  %2 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %1)
  %3 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %other.addr)
  %4 = call i64 @ptrDiff(i8* %2, i8* %3)
  store i64 1, i64* %tmp.v1
  %5 = load i64* %tmp.v1
  %6 = call i64 @"/143"(i64 %4, i64 %5)
  store i64 %6, i64* %tmp.v
  %7 = load i64* %tmp.v
  call void @dtor35(i64* %tmp.v)
  call void @dtor35(i64* %tmp.v1)
  ret i64 %7

dumy_block:                                       ; No predecessors!
  call void @dtor35(i64* %tmp.v)
  call void @dtor35(i64* %tmp.v1)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i64 @"/143"(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn34(i64 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn34(i64 %4, i64 %5)
  %7 = sdiv i64 %3, %6
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private void @"=144"(%"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]" %other) #2 {
  %"$this.addr" = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]"** %"$this.addr"
  %other.addr = alloca %"RawPtr[Char/rtct]"
  store %"RawPtr[Char/rtct]" %other, %"RawPtr[Char/rtct]"* %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = getelementptr inbounds %"RawPtr[Char/rtct]"* %other.addr, i32 0, i32 0
  %2 = load i8** %1
  %3 = load %"RawPtr[Char/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"RawPtr[Char/rtct]"* %3, i32 0, i32 0
  store i8* %2, i8** %4
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @alloc(%"RawPtr[Char/rtct]"* sret %_result, i64 %num) #3 {
  %_result.addr = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %_result, %"RawPtr[Char/rtct]"** %_result.addr
  %num.addr = alloca i64
  store i64 %num, i64* %num.addr
  %res = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  call void @ctor120(%"RawPtr[Char/rtct]"* %res)
  %1 = load i64* %num.addr
  %2 = call i64 @"*"(i64 %1, i64 1)
  %3 = call i8* @malloc(i64 %2)
  %4 = getelementptr inbounds %"RawPtr[Char/rtct]"* %res, i32 0, i32 0
  store i8* %3, i8** %4
  %5 = load %"RawPtr[Char/rtct]"** %_result.addr
  call void @ctor145(%"RawPtr[Char/rtct]"* %5, %"RawPtr[Char/rtct]"* %res)
  call void @dtor146(%"RawPtr[Char/rtct]"* %res)
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor146(%"RawPtr[Char/rtct]"* %res)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor145(%"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]"* %other) #2 {
  %"$this.addr" = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]"** %"$this.addr"
  %other.addr = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %other, %"RawPtr[Char/rtct]"** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"RawPtr[Char/rtct]"** %other.addr
  %2 = getelementptr inbounds %"RawPtr[Char/rtct]"* %1, i32 0, i32 0
  %3 = load i8** %2
  %4 = load %"RawPtr[Char/rtct]"** %"$this.addr"
  %5 = getelementptr inbounds %"RawPtr[Char/rtct]"* %4, i32 0, i32 0
  store i8* %3, i8** %5
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor146(%"RawPtr[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @advance(%"RawPtr[Char/rtct]"* sret %_result, %"RawPtr[Char/rtct]"* %"$this", i64 %n) #2 {
  %_result.addr = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %_result, %"RawPtr[Char/rtct]"** %_result.addr
  %"$this.addr" = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]"** %"$this.addr"
  %n.addr = alloca i64
  store i64 %n, i64* %n.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"RawPtr[Char/rtct]"** %_result.addr
  %2 = load %"RawPtr[Char/rtct]"** %"$this.addr"
  %3 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %2)
  %4 = load i64* %n.addr
  %5 = call i64 @"*"(i64 %4, i64 1)
  %6 = call i8* @ptrAdd(i8* %3, i64 %5)
  call void @fromBytePtr147(%"RawPtr[Char/rtct]"* %1, i8* %6)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @fromBytePtr147(%"RawPtr[Char/rtct]"* sret %_result, i8* %ptr) #3 {
  %_result.addr = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %_result, %"RawPtr[Char/rtct]"** %_result.addr
  %ptr.addr = alloca i8*
  store i8* %ptr, i8** %ptr.addr
  %res = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  call void @ctor120(%"RawPtr[Char/rtct]"* %res)
  %1 = load i8** %ptr.addr
  %2 = getelementptr inbounds %"RawPtr[Char/rtct]"* %res, i32 0, i32 0
  store i8* %1, i8** %2
  %3 = load %"RawPtr[Char/rtct]"** %_result.addr
  call void @ctor145(%"RawPtr[Char/rtct]"* %3, %"RawPtr[Char/rtct]"* %res)
  call void @dtor146(%"RawPtr[Char/rtct]"* %res)
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor146(%"RawPtr[Char/rtct]"* %res)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i1 @"==148"(%"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]" %other) #2 {
  %"$this.addr" = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]"** %"$this.addr"
  %other.addr = alloca %"RawPtr[Char/rtct]"
  store %"RawPtr[Char/rtct]" %other, %"RawPtr[Char/rtct]"* %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"RawPtr[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"RawPtr[Char/rtct]"* %1, i32 0, i32 0
  %3 = load i8** %2
  %4 = getelementptr inbounds %"RawPtr[Char/rtct]"* %other.addr, i32 0, i32 0
  %5 = load i8** %4
  %6 = call i1 @_opRefEQ(i8* %3, i8* %5)
  ret i1 %6
}

; Function Attrs: alwaysinline nounwind
define private i8* @value(%"RawPtr[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"RawPtr[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"RawPtr[Char/rtct]"* %1, i32 0, i32 0
  %3 = load i8** %2
  ret i8* %3
}

; Function Attrs: alwaysinline nounwind
define private void @advance149(%"RawPtr[Char/rtct]"* sret %_result, %"RawPtr[Char/rtct]"* %"$this") #2 {
  %_result.addr = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %_result, %"RawPtr[Char/rtct]"** %_result.addr
  %"$this.addr" = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"RawPtr[Char/rtct]"** %_result.addr
  %2 = load %"RawPtr[Char/rtct]"** %"$this.addr"
  %3 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %2)
  %4 = call i8* @ptrAdd(i8* %3, i64 1)
  call void @fromBytePtr147(%"RawPtr[Char/rtct]"* %1, i8* %4)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctor150(%"RangeWithLookahead[FileRange/rtct]"* %"$this", %FileRange %src) #3 {
  %"$this.addr" = alloca %"RangeWithLookahead[FileRange/rtct]"*
  store %"RangeWithLookahead[FileRange/rtct]"* %"$this", %"RangeWithLookahead[FileRange/rtct]"** %"$this.addr"
  %src.addr = alloca %FileRange
  store %FileRange %src, %FileRange* %src.addr
  %"$tmpForRef" = alloca i8
  br label %code

code:                                             ; preds = %0
  %1 = load %"RangeWithLookahead[FileRange/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %1, i32 0, i32 0
  call void @ctor118(%FileRange* %2)
  %3 = load %"RangeWithLookahead[FileRange/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %3, i32 0, i32 0
  call void @"=151"(%FileRange* %4, %FileRange* %src.addr)
  %5 = load %"RangeWithLookahead[FileRange/rtct]"** %"$this.addr"
  %6 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %5, i32 0, i32 1
  call void @ctor119(%"Vector[Char/rtct]"* %6)
  %7 = load %"RangeWithLookahead[FileRange/rtct]"** %"$this.addr"
  %8 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %7, i32 0, i32 1
  call void @reserve(%"Vector[Char/rtct]"* %8, i64 10)
  br label %if_block

if_block:                                         ; preds = %code
  %9 = load %"RangeWithLookahead[FileRange/rtct]"** %"$this.addr"
  %10 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %9, i32 0, i32 0
  %11 = call i1 @"pre_!!"(%FileRange* %10)
  br i1 %11, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %12 = load %"RangeWithLookahead[FileRange/rtct]"** %"$this.addr"
  %13 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %12, i32 0, i32 1
  %14 = load %"RangeWithLookahead[FileRange/rtct]"** %"$this.addr"
  %15 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %14, i32 0, i32 0
  %16 = call i8 @"post_++159"(%FileRange* %15)
  store i8 %16, i8* %"$tmpForRef"
  call void @"+="(%"Vector[Char/rtct]"* %13, i8* %"$tmpForRef")
  br label %if_end

if_end:                                           ; preds = %if_then, %if_block
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @"=151"(%FileRange* %"$this", %FileRange* %other) #2 {
  %"$this.addr" = alloca %FileRange*
  store %FileRange* %"$this", %FileRange** %"$this.addr"
  %other.addr = alloca %FileRange*
  store %FileRange* %other, %FileRange** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %FileRange** %other.addr
  %2 = getelementptr inbounds %FileRange* %1, i32 0, i32 0
  %3 = load %File** %2
  %4 = load %FileRange** %"$this.addr"
  %5 = getelementptr inbounds %FileRange* %4, i32 0, i32 0
  store %File* %3, %File** %5
  %6 = load %FileRange** %other.addr
  %7 = getelementptr inbounds %FileRange* %6, i32 0, i32 1
  %8 = load i1* %7
  %9 = load %FileRange** %"$this.addr"
  %10 = getelementptr inbounds %FileRange* %9, i32 0, i32 1
  store i1 %8, i1* %10
  %11 = load %FileRange** %other.addr
  %12 = getelementptr inbounds %FileRange* %11, i32 0, i32 2
  %13 = load i8* %12
  %14 = load %FileRange** %"$this.addr"
  %15 = getelementptr inbounds %FileRange* %14, i32 0, i32 2
  store i8 %13, i8* %15
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @reserve(%"Vector[Char/rtct]"* %"$this", i64 %n) #3 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %n.addr = alloca i64
  store i64 %n, i64* %n.addr
  %curCapacity = alloca i64
  %tmp.v = alloca i64
  %curSize = alloca i64
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  %"$tmpC7" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = call i64 @capacity(%"Vector[Char/rtct]"* %1)
  store i64 %2, i64* %curCapacity
  br label %if_block

if_block:                                         ; preds = %code
  %3 = load i64* %n.addr
  %4 = load i64* %curCapacity
  %5 = call i1 @"<="(i64 %3, i64 %4)
  br i1 %5, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  call void @dtor31(i64* %curCapacity)
  ret void

if_end:                                           ; preds = %dumy_block, %if_block
  br label %if_block1

dumy_block:                                       ; No predecessors!
  br label %if_end

if_block1:                                        ; preds = %if_end
  %6 = load i64* %n.addr
  %7 = call i1 @"<"(i64 %6, i32 8)
  br i1 %7, label %if_then2, label %if_end3

if_then2:                                         ; preds = %if_block1
  store i64 8, i64* %n.addr
  br label %if_end3

if_end3:                                          ; preds = %if_then2, %if_block1
  br label %if_block4

if_block4:                                        ; preds = %if_end3
  %8 = load i64* %n.addr
  %9 = load i64* %curCapacity
  %10 = call double @"*156"(double 2.000000e+00, i64 %9)
  %11 = call i1 @"<152"(i64 %8, double %10)
  br i1 %11, label %if_then5, label %if_end6

if_then5:                                         ; preds = %if_block4
  %12 = load i64* %curCapacity
  %13 = call double @"*156"(double 2.000000e+00, i64 %12)
  %14 = fptoui double %13 to i64
  store i64 %14, i64* %tmp.v
  %15 = load i64* %tmp.v
  store i64 %15, i64* %n.addr
  call void @dtor31(i64* %tmp.v)
  br label %if_end6

if_end6:                                          ; preds = %if_then5, %if_block4
  %16 = load %"Vector[Char/rtct]"** %"$this.addr"
  %17 = call i64 @size142(%"Vector[Char/rtct]"* %16)
  store i64 %17, i64* %curSize
  %18 = load %"Vector[Char/rtct]"** %"$this.addr"
  %19 = getelementptr inbounds %"Vector[Char/rtct]"* %18, i32 0, i32 0
  %20 = load i64* %n.addr
  call void @reallocPtr(%"RawPtr[Char/rtct]"* %19, i64 %20)
  %21 = load %"Vector[Char/rtct]"** %"$this.addr"
  %22 = getelementptr inbounds %"Vector[Char/rtct]"* %21, i32 0, i32 1
  %23 = load %"Vector[Char/rtct]"** %"$this.addr"
  %24 = getelementptr inbounds %"Vector[Char/rtct]"* %23, i32 0, i32 0
  %25 = load i64* %curSize
  call void @advance(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %24, i64 %25)
  %26 = load %"RawPtr[Char/rtct]"* %"$tmpC"
  call void @"=144"(%"RawPtr[Char/rtct]"* %22, %"RawPtr[Char/rtct]" %26)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  %27 = load %"Vector[Char/rtct]"** %"$this.addr"
  %28 = getelementptr inbounds %"Vector[Char/rtct]"* %27, i32 0, i32 2
  %29 = load %"Vector[Char/rtct]"** %"$this.addr"
  %30 = getelementptr inbounds %"Vector[Char/rtct]"* %29, i32 0, i32 0
  %31 = load i64* %n.addr
  call void @advance(%"RawPtr[Char/rtct]"* %"$tmpC7", %"RawPtr[Char/rtct]"* %30, i64 %31)
  %32 = load %"RawPtr[Char/rtct]"* %"$tmpC7"
  call void @"=144"(%"RawPtr[Char/rtct]"* %28, %"RawPtr[Char/rtct]" %32)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC7")
  call void @dtor31(i64* %curSize)
  call void @dtor31(i64* %curCapacity)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i64 @capacity(%"Vector[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"Vector[Char/rtct]"* %1, i32 0, i32 2
  %3 = load %"Vector[Char/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"Vector[Char/rtct]"* %3, i32 0, i32 0
  %5 = load %"RawPtr[Char/rtct]"* %4
  %6 = call i64 @diff(%"RawPtr[Char/rtct]"* %2, %"RawPtr[Char/rtct]" %5)
  store i64 %6, i64* %tmp.v
  %7 = load i64* %tmp.v
  call void @dtor31(i64* %tmp.v)
  ret i64 %7

dumy_block:                                       ; No predecessors!
  call void @dtor31(i64* %tmp.v)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i1 @"<="(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn30(i64 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn30(i64 %4, i64 %5)
  %7 = icmp sle i64 %3, %6
  ret i1 %7
}

; Function Attrs: alwaysinline nounwind
define private i1 @"<"(i64 %x, i32 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i32* %y.addr
  %3 = call i64 @cmn75(i64 %1, i32 %2)
  %4 = load i32* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn76(i32 %4, i64 %5)
  %7 = icmp slt i64 %3, %6
  ret i1 %7
}

; Function Attrs: alwaysinline nounwind
define private i1 @"<152"(i64 %x, double %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca double
  store double %y, double* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load double* %y.addr
  %3 = call double @cmn153(i64 %1, double %2)
  %4 = load double* %y.addr
  %5 = load i64* %x.addr
  %6 = call double @cmn155(double %4, i64 %5)
  %7 = call i1 @_Double_opLT(double %3, double %6)
  ret i1 %7
}

; Function Attrs: alwaysinline nounwind
define private double @cmn153(i64 %x, double %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca double
  store double %y, double* %y.addr
  %tmp.v = alloca double
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = sitofp i64 %1 to double
  store double %2, double* %tmp.v
  %3 = load double* %tmp.v
  call void @dtor154(double* %tmp.v)
  ret double %3

dumy_block:                                       ; No predecessors!
  call void @dtor154(double* %tmp.v)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private void @dtor154(double* %"$this") #2 {
  %"$this.addr" = alloca double*
  store double* %"$this", double** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private double @cmn155(double %x, i64 %y) #2 {
  %x.addr = alloca double
  store double %x, double* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load double* %x.addr
  ret double %1
}

; Function Attrs: alwaysinline nounwind
define private double @"*156"(double %x, i64 %y) #2 {
  %x.addr = alloca double
  store double %x, double* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load double* %x.addr
  %2 = load i64* %y.addr
  %3 = call double @cmn155(double %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load double* %x.addr
  %6 = call double @cmn153(i64 %4, double %5)
  %7 = call double @_Double_opMul(double %3, double %6)
  ret double %7
}

; Function Attrs: inlinehint nounwind
define private void @reallocPtr(%"RawPtr[Char/rtct]"* %"$this", i64 %n) #3 {
  %"$this.addr" = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]"** %"$this.addr"
  %n.addr = alloca i64
  store i64 %n, i64* %n.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"RawPtr[Char/rtct]"** %"$this.addr"
  %2 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %1)
  %3 = load i64* %n.addr
  %4 = call i64 @"*"(i64 %3, i64 1)
  %5 = call i8* @realloc(i8* %2, i64 %4)
  %6 = load %"RawPtr[Char/rtct]"** %"$this.addr"
  %7 = getelementptr inbounds %"RawPtr[Char/rtct]"* %6, i32 0, i32 0
  store i8* %5, i8** %7
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i1 @"pre_!!"(%FileRange* %r) #2 {
  %r.addr = alloca %FileRange*
  store %FileRange* %r, %FileRange** %r.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %FileRange** %r.addr
  %2 = call i1 @isEmpty157(%FileRange* %1)
  %3 = xor i1 true, %2
  ret i1 %3
}

; Function Attrs: alwaysinline nounwind
define private i1 @isEmpty157(%FileRange* %s) #2 {
  %s.addr = alloca %FileRange*
  store %FileRange* %s, %FileRange** %s.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %FileRange** %s.addr
  %2 = getelementptr inbounds %FileRange* %1, i32 0, i32 1
  %3 = load i1* %2
  ret i1 %3
}

; Function Attrs: alwaysinline nounwind
define private void @"+="(%"Vector[Char/rtct]"* %"$this", i8* %value) #2 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %value.addr = alloca i8*
  store i8* %value, i8** %value.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = load i8** %value.addr
  call void @pushBack(%"Vector[Char/rtct]"* %1, i8* %2)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @pushBack(%"Vector[Char/rtct]"* %"$this", i8* %value) #3 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %value.addr = alloca i8*
  store i8* %value, i8** %value.addr
  %t = alloca i64
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  br label %if_block

if_block:                                         ; preds = %code
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"Vector[Char/rtct]"* %1, i32 0, i32 1
  %3 = load %"Vector[Char/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"Vector[Char/rtct]"* %3, i32 0, i32 2
  %5 = load %"RawPtr[Char/rtct]"* %4
  %6 = call i1 @"==148"(%"RawPtr[Char/rtct]"* %2, %"RawPtr[Char/rtct]" %5)
  br i1 %6, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %7 = load %"Vector[Char/rtct]"** %"$this.addr"
  %8 = call i64 @capacity(%"Vector[Char/rtct]"* %7)
  %9 = call i64 @"*158"(i32 2, i64 %8)
  store i64 %9, i64* %t
  br label %if_block1

if_end:                                           ; preds = %if_end3, %if_block
  %10 = load i8** %value.addr
  %11 = load i8* %10
  %12 = load %"Vector[Char/rtct]"** %"$this.addr"
  %13 = getelementptr inbounds %"Vector[Char/rtct]"* %12, i32 0, i32 1
  %14 = call i8* @value(%"RawPtr[Char/rtct]"* %13)
  store i8 %11, i8* %14
  %15 = load %"Vector[Char/rtct]"** %"$this.addr"
  %16 = getelementptr inbounds %"Vector[Char/rtct]"* %15, i32 0, i32 1
  %17 = load %"Vector[Char/rtct]"** %"$this.addr"
  %18 = getelementptr inbounds %"Vector[Char/rtct]"* %17, i32 0, i32 1
  call void @advance149(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %18)
  %19 = load %"RawPtr[Char/rtct]"* %"$tmpC"
  call void @"=144"(%"RawPtr[Char/rtct]"* %16, %"RawPtr[Char/rtct]" %19)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  ret void

if_block1:                                        ; preds = %if_then
  %20 = load i64* %t
  %21 = call i1 @"<"(i64 %20, i32 2)
  br i1 %21, label %if_then2, label %if_end3

if_then2:                                         ; preds = %if_block1
  store i64 2, i64* %t
  br label %if_end3

if_end3:                                          ; preds = %if_then2, %if_block1
  %22 = load %"Vector[Char/rtct]"** %"$this.addr"
  %23 = load i64* %t
  call void @reserve(%"Vector[Char/rtct]"* %22, i64 %23)
  call void @dtor31(i64* %t)
  br label %if_end
}

; Function Attrs: alwaysinline nounwind
define private i64 @"*158"(i32 %x, i64 %y) #2 {
  %x.addr = alloca i32
  store i32 %x, i32* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i32* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn76(i32 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i32* %x.addr
  %6 = call i64 @cmn75(i64 %4, i32 %5)
  %7 = mul i64 %3, %6
  ret i64 %7
}

; Function Attrs: inlinehint nounwind
define private i8 @"post_++159"(%FileRange* %r) #3 {
  %r.addr = alloca %FileRange*
  store %FileRange* %r, %FileRange** %r.addr
  %res = alloca i8
  br label %code

code:                                             ; preds = %0
  %1 = load %FileRange** %r.addr
  %2 = call i8 @front160(%FileRange* %1)
  store i8 %2, i8* %res
  %3 = load %FileRange** %r.addr
  call void @popFront161(%FileRange* %3)
  %4 = load i8* %res
  call void @dtor162(i8* %res)
  ret i8 %4

dumy_block:                                       ; No predecessors!
  call void @dtor162(i8* %res)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i8 @front160(%FileRange* %s) #2 {
  %s.addr = alloca %FileRange*
  store %FileRange* %s, %FileRange** %s.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %FileRange** %s.addr
  %2 = getelementptr inbounds %FileRange* %1, i32 0, i32 2
  %3 = load i8* %2
  ret i8 %3
}

; Function Attrs: inlinehint nounwind
define private void @popFront161(%FileRange* %s) #3 {
  %s.addr = alloca %FileRange*
  store %FileRange* %s, %FileRange** %s.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %FileRange** %s.addr
  %2 = getelementptr inbounds %FileRange* %1, i32 0, i32 0
  %3 = load %File** %2
  %4 = call i8 @readChar(%File* %3)
  %5 = load %FileRange** %s.addr
  %6 = getelementptr inbounds %FileRange* %5, i32 0, i32 2
  store i8 %4, i8* %6
  br label %if_block

if_block:                                         ; preds = %code
  %7 = load %FileRange** %s.addr
  %8 = getelementptr inbounds %FileRange* %7, i32 0, i32 0
  %9 = load %File** %8
  %10 = call i1 @isEof(%File* %9)
  br i1 %10, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %11 = load %FileRange** %s.addr
  %12 = getelementptr inbounds %FileRange* %11, i32 0, i32 1
  store i1 true, i1* %12
  br label %if_end

if_end:                                           ; preds = %if_then, %if_block
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i8 @readChar(%File* %"$this") #2 {
  %"$this.addr" = alloca %File*
  store %File* %"$this", %File** %"$this.addr"
  %tmp.v = alloca i8
  br label %code

code:                                             ; preds = %0
  %1 = load %File** %"$this.addr"
  %2 = getelementptr inbounds %File* %1, i32 0, i32 0
  %3 = load i8** %2
  %4 = call i32 @fgetc(i8* %3)
  %5 = trunc i32 %4 to i8
  store i8 %5, i8* %tmp.v
  %6 = load i8* %tmp.v
  call void @dtor162(i8* %tmp.v)
  ret i8 %6

dumy_block:                                       ; No predecessors!
  call void @dtor162(i8* %tmp.v)
  unreachable
}

declare i32 @fgetc(i8*)

; Function Attrs: alwaysinline nounwind
define private void @dtor162(i8* %"$this") #2 {
  %"$this.addr" = alloca i8*
  store i8* %"$this", i8** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i1 @isEof(%File* %"$this") #2 {
  %"$this.addr" = alloca %File*
  store %File* %"$this", %File** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %File** %"$this.addr"
  %2 = getelementptr inbounds %File* %1, i32 0, i32 0
  %3 = load i8** %2
  %4 = call i32 @feof(i8* %3)
  %5 = call i1 @"!=163"(i32 0, i32 %4)
  ret i1 %5
}

; Function Attrs: alwaysinline nounwind
define private i1 @"!=163"(i32 %x, i32 %y) #2 {
  %x.addr = alloca i32
  store i32 %x, i32* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i32* %x.addr
  %2 = load i32* %y.addr
  %3 = call i32 @cmn15(i32 %1, i32 %2)
  %4 = load i32* %y.addr
  %5 = load i32* %x.addr
  %6 = call i32 @cmn15(i32 %4, i32 %5)
  %7 = icmp ne i32 %3, %6
  ret i1 %7
}

declare i32 @feof(i8*)

; Function Attrs: alwaysinline nounwind
define private void @dtor164(%"RangeWithLookahead[FileRange/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"RangeWithLookahead[FileRange/rtct]"*
  store %"RangeWithLookahead[FileRange/rtct]"* %"$this", %"RangeWithLookahead[FileRange/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"RangeWithLookahead[FileRange/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %1, i32 0, i32 1
  call void @dtor165(%"Vector[Char/rtct]"* %2)
  %3 = load %"RangeWithLookahead[FileRange/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %3, i32 0, i32 0
  call void @dtor166(%FileRange* %4)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @dtor165(%"Vector[Char/rtct]"* %"$this") #3 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %p = alloca %"RawPtr[Char/rtct]"
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"Vector[Char/rtct]"* %1, i32 0, i32 0
  call void @ctor145(%"RawPtr[Char/rtct]"* %p, %"RawPtr[Char/rtct]"* %2)
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %3 = load %"Vector[Char/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"Vector[Char/rtct]"* %3, i32 0, i32 1
  %5 = load %"RawPtr[Char/rtct]"* %4
  %6 = call i1 @"==148"(%"RawPtr[Char/rtct]"* %p, %"RawPtr[Char/rtct]" %5)
  %7 = xor i1 true, %6
  br i1 %7, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  %8 = call i8* @value(%"RawPtr[Char/rtct]"* %p)
  call void @dtor162(i8* %8)
  br label %while_step

while_step:                                       ; preds = %while_body
  call void @advance149(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %p)
  %9 = load %"RawPtr[Char/rtct]"* %"$tmpC"
  call void @"=144"(%"RawPtr[Char/rtct]"* %p, %"RawPtr[Char/rtct]" %9)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  br label %while_block

while_end:                                        ; preds = %while_block
  %10 = load %"Vector[Char/rtct]"** %"$this.addr"
  %11 = getelementptr inbounds %"Vector[Char/rtct]"* %10, i32 0, i32 0
  call void @freePtr(%"RawPtr[Char/rtct]"* %11)
  %12 = load %"Vector[Char/rtct]"** %"$this.addr"
  %13 = getelementptr inbounds %"Vector[Char/rtct]"* %12, i32 0, i32 2
  call void @dtor146(%"RawPtr[Char/rtct]"* %13)
  %14 = load %"Vector[Char/rtct]"** %"$this.addr"
  %15 = getelementptr inbounds %"Vector[Char/rtct]"* %14, i32 0, i32 1
  call void @dtor146(%"RawPtr[Char/rtct]"* %15)
  %16 = load %"Vector[Char/rtct]"** %"$this.addr"
  %17 = getelementptr inbounds %"Vector[Char/rtct]"* %16, i32 0, i32 0
  call void @dtor146(%"RawPtr[Char/rtct]"* %17)
  call void @dtor146(%"RawPtr[Char/rtct]"* %p)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @freePtr(%"RawPtr[Char/rtct]"* %"$this") #3 {
  %"$this.addr" = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  br label %if_block

if_block:                                         ; preds = %code
  %1 = load %"RawPtr[Char/rtct]"** %"$this.addr"
  %2 = call i1 @isSet(%"RawPtr[Char/rtct]"* %1)
  br i1 %2, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %3 = load %"RawPtr[Char/rtct]"** %"$this.addr"
  %4 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %3)
  call void @free(i8* %4)
  br label %if_end

if_end:                                           ; preds = %if_then, %if_block
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i1 @isSet(%"RawPtr[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"RawPtr[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"RawPtr[Char/rtct]"* %1, i32 0, i32 0
  %3 = load i8** %2
  %4 = call i1 @_opRefNE(i8* %3, i8* null)
  ret i1 %4
}

; Function Attrs: alwaysinline nounwind
define private void @dtor166(%FileRange* %"$this") #2 {
  %"$this.addr" = alloca %FileRange*
  store %FileRange* %"$this", %FileRange** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %FileRange** %"$this.addr"
  %2 = getelementptr inbounds %FileRange* %1, i32 0, i32 2
  call void @dtor162(i8* %2)
  %3 = load %FileRange** %"$this.addr"
  %4 = getelementptr inbounds %FileRange* %3, i32 0, i32 1
  call void @dtor167(i1* %4)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor167(i1* %"$this") #2 {
  %"$this.addr" = alloca i1*
  store i1* %"$this", i1** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @all(%FileRange* sret %_result, %File* %"$this") #2 {
  %_result.addr = alloca %FileRange*
  store %FileRange* %_result, %FileRange** %_result.addr
  %"$this.addr" = alloca %File*
  store %File* %"$this", %File** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %FileRange** %_result.addr
  %2 = load %File** %"$this.addr"
  call void @ctor168(%FileRange* %1, %File* %2)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctor168(%FileRange* %"$this", %File* %file) #3 {
  %"$this.addr" = alloca %FileRange*
  store %FileRange* %"$this", %FileRange** %"$this.addr"
  %file.addr = alloca %File*
  store %File* %file, %File** %file.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %FileRange** %"$this.addr"
  %2 = getelementptr inbounds %FileRange* %1, i32 0, i32 0
  store %File* null, %File** %2
  %3 = load %FileRange** %"$this.addr"
  %4 = getelementptr inbounds %FileRange* %3, i32 0, i32 1
  store i1 false, i1* %4
  %5 = load %File** %file.addr
  %6 = load %FileRange** %"$this.addr"
  %7 = getelementptr inbounds %FileRange* %6, i32 0, i32 0
  store %File* %5, %File** %7
  %8 = load %FileRange** %"$this.addr"
  %9 = getelementptr inbounds %FileRange* %8, i32 0, i32 1
  store i1 true, i1* %9
  %10 = load %FileRange** %"$this.addr"
  %11 = getelementptr inbounds %FileRange* %10, i32 0, i32 2
  store i8 0, i8* %11
  br label %if_block

if_block:                                         ; preds = %code
  %12 = load %File** %file.addr
  %13 = call i1 @isEof(%File* %12)
  %14 = xor i1 true, %13
  br i1 %14, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %15 = load %File** %file.addr
  %16 = call i8 @readChar(%File* %15)
  %17 = load %FileRange** %"$this.addr"
  %18 = getelementptr inbounds %FileRange* %17, i32 0, i32 2
  store i8 %16, i8* %18
  %19 = load %FileRange** %"$this.addr"
  %20 = getelementptr inbounds %FileRange* %19, i32 0, i32 1
  store i1 false, i1* %20
  br label %if_end

if_end:                                           ; preds = %if_then, %if_block
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i1 @isOpen(%File* %"$this") #2 {
  %"$this.addr" = alloca %File*
  store %File* %"$this", %File** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %File** %"$this.addr"
  %2 = getelementptr inbounds %File* %1, i32 0, i32 0
  %3 = load i8** %2
  %4 = call i1 @_opRefNE(i8* %3, i8* null)
  ret i1 %4
}

; Function Attrs: inlinehint nounwind
define private void @toString(%String* sret %_result, %StringRef %a1, %StringRef %a2) #3 {
  %_result.addr = alloca %String*
  store %String* %_result, %String** %_result.addr
  %a1.addr = alloca %StringRef
  store %StringRef %a1, %StringRef* %a1.addr
  %a2.addr = alloca %StringRef
  store %StringRef %a2, %StringRef* %a2.addr
  %s = alloca %StringOutputStream
  br label %code

code:                                             ; preds = %0
  call void @ctor169(%StringOutputStream* %s)
  %1 = call %StringOutputStream* @"<<"(%StringOutputStream* %s, %StringRef* %a1.addr)
  %2 = call %StringOutputStream* @"<<"(%StringOutputStream* %1, %StringRef* %a2.addr)
  %3 = load %String** %_result.addr
  %4 = getelementptr inbounds %StringOutputStream* %s, i32 0, i32 0
  call void @ctor183(%String* %3, %String* %4)
  call void @dtor185(%StringOutputStream* %s)
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor185(%StringOutputStream* %s)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor169(%StringOutputStream* %"$this") #2 {
  %"$this.addr" = alloca %StringOutputStream*
  store %StringOutputStream* %"$this", %StringOutputStream** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %StringOutputStream** %"$this.addr"
  %2 = getelementptr inbounds %StringOutputStream* %1, i32 0, i32 0
  call void @ctor123(%String* %2)
  ret void
}

; Function Attrs: inlinehint nounwind
define private %StringOutputStream* @"<<"(%StringOutputStream* %s, %StringRef* %x) #3 {
  %s.addr = alloca %StringOutputStream*
  store %StringOutputStream* %s, %StringOutputStream** %s.addr
  %x.addr = alloca %StringRef*
  store %StringRef* %x, %StringRef** %x.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %StringOutputStream** %s.addr
  %2 = load %StringRef** %x.addr
  call void @"<<<"(%StringOutputStream* %1, %StringRef* %2)
  %3 = load %StringOutputStream** %s.addr
  ret %StringOutputStream* %3
}

; Function Attrs: alwaysinline nounwind
define private void @"<<<"(%StringOutputStream* %"$this", %StringRef* %s) #2 {
  %"$this.addr" = alloca %StringOutputStream*
  store %StringOutputStream* %"$this", %StringOutputStream** %"$this.addr"
  %s.addr = alloca %StringRef*
  store %StringRef* %s, %StringRef** %s.addr
  %tmp.v = alloca %"ContiguousMemoryRange[Char/rtct]"
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  %"$tmpC1" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %StringOutputStream** %"$this.addr"
  %2 = getelementptr inbounds %StringOutputStream* %1, i32 0, i32 0
  %3 = load %StringRef** %s.addr
  %4 = getelementptr inbounds %StringRef* %3, i32 0, i32 0
  %5 = load i8** %4
  call void @fromBytePtr147(%"RawPtr[Char/rtct]"* %"$tmpC", i8* %5)
  %6 = load %"RawPtr[Char/rtct]"* %"$tmpC"
  %7 = load %StringRef** %s.addr
  %8 = getelementptr inbounds %StringRef* %7, i32 0, i32 1
  %9 = load i8** %8
  call void @fromBytePtr147(%"RawPtr[Char/rtct]"* %"$tmpC1", i8* %9)
  %10 = load %"RawPtr[Char/rtct]"* %"$tmpC1"
  call void @ctor181(%"ContiguousMemoryRange[Char/rtct]"* %tmp.v, %"RawPtr[Char/rtct]" %6, %"RawPtr[Char/rtct]" %10)
  %11 = load %"ContiguousMemoryRange[Char/rtct]"* %tmp.v
  call void @append(%String* %2, %"ContiguousMemoryRange[Char/rtct]" %11)
  call void @dtor182(%"ContiguousMemoryRange[Char/rtct]"* %tmp.v)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC1")
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @append(%String* %"$this", %"ContiguousMemoryRange[Char/rtct]" %range) #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %range.addr = alloca %"ContiguousMemoryRange[Char/rtct]"
  store %"ContiguousMemoryRange[Char/rtct]" %range, %"ContiguousMemoryRange[Char/rtct]"* %range.addr
  %tmp.v = alloca %"ContiguousMemoryRange[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  %2 = load %"ContiguousMemoryRange[Char/rtct]"* %range.addr
  %3 = load %String** %"$this.addr"
  %4 = getelementptr inbounds %String* %3, i32 0, i32 1
  %5 = load %"RawPtr[Char/rtct]"* %4
  %6 = load %String** %"$this.addr"
  %7 = getelementptr inbounds %String* %6, i32 0, i32 1
  %8 = load %"RawPtr[Char/rtct]"* %7
  call void @ctor181(%"ContiguousMemoryRange[Char/rtct]"* %tmp.v, %"RawPtr[Char/rtct]" %5, %"RawPtr[Char/rtct]" %8)
  %9 = load %"ContiguousMemoryRange[Char/rtct]"* %tmp.v
  call void @insertBefore(%String* %1, %"ContiguousMemoryRange[Char/rtct]" %2, %"ContiguousMemoryRange[Char/rtct]" %9)
  call void @dtor182(%"ContiguousMemoryRange[Char/rtct]"* %tmp.v)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @insertBefore(%String* %"$this", %"ContiguousMemoryRange[Char/rtct]" %range, %"ContiguousMemoryRange[Char/rtct]" %pos) #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %range.addr = alloca %"ContiguousMemoryRange[Char/rtct]"
  store %"ContiguousMemoryRange[Char/rtct]" %range, %"ContiguousMemoryRange[Char/rtct]"* %range.addr
  %pos.addr = alloca %"ContiguousMemoryRange[Char/rtct]"
  store %"ContiguousMemoryRange[Char/rtct]" %pos, %"ContiguousMemoryRange[Char/rtct]"* %pos.addr
  %n = alloca i64
  %index = alloca i64
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  %p = alloca %"RawPtr[Char/rtct]"
  %q = alloca %"RawPtr[Char/rtct]"
  %tmp.v = alloca i64
  %"$tmpC1" = alloca %"RawPtr[Char/rtct]"
  %tmp.v2 = alloca i64
  %"$tmpC3" = alloca %"RawPtr[Char/rtct]"
  %"$tmpC4" = alloca %"RawPtr[Char/rtct]"
  %"$tmpC9" = alloca %"RawPtr[Char/rtct]"
  %"$tmpC10" = alloca %"RawPtr[Char/rtct]"
  %tmp.v11 = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = call i64 @size170(%"ContiguousMemoryRange[Char/rtct]"* %range.addr)
  store i64 %1, i64* %n
  call void @frontPtr(%"RawPtr[Char/rtct]"* %"$tmpC", %"ContiguousMemoryRange[Char/rtct]"* %pos.addr)
  %2 = load %String** %"$this.addr"
  %3 = getelementptr inbounds %String* %2, i32 0, i32 0
  %4 = load %"RawPtr[Char/rtct]"* %3
  %5 = call i64 @diff(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]" %4)
  store i64 %5, i64* %index
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  %6 = load %String** %"$this.addr"
  %7 = load %String** %"$this.addr"
  %8 = call i64 @size174(%String* %7)
  %9 = load i64* %n
  %10 = call i64 @"+29"(i64 %8, i64 %9)
  call void @reserve171(%String* %6, i64 %10)
  %11 = load %String** %"$this.addr"
  %12 = getelementptr inbounds %String* %11, i32 0, i32 1
  call void @advance175(%"RawPtr[Char/rtct]"* %p, %"RawPtr[Char/rtct]"* %12, i64 -1)
  %13 = load %String** %"$this.addr"
  %14 = getelementptr inbounds %String* %13, i32 0, i32 0
  %15 = load i64* %index
  store i64 1, i64* %tmp.v
  %16 = load i64* %tmp.v
  %17 = call i64 @-53(i64 %15, i64 %16)
  call void @advance175(%"RawPtr[Char/rtct]"* %q, %"RawPtr[Char/rtct]"* %14, i64 %17)
  call void @dtor35(i64* %tmp.v)
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %18 = load %"RawPtr[Char/rtct]"* %q
  %19 = call i1 @"==148"(%"RawPtr[Char/rtct]"* %p, %"RawPtr[Char/rtct]" %18)
  %20 = xor i1 true, %19
  br i1 %20, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  %21 = call i8* @value(%"RawPtr[Char/rtct]"* %p)
  %22 = load i8* %21
  %23 = load i64* %n
  store i64 %23, i64* %tmp.v2
  %24 = load i64* %tmp.v2
  call void @advance175(%"RawPtr[Char/rtct]"* %"$tmpC1", %"RawPtr[Char/rtct]"* %p, i64 %24)
  %25 = call i8* @value(%"RawPtr[Char/rtct]"* %"$tmpC1")
  store i8 %22, i8* %25
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC1")
  call void @dtor35(i64* %tmp.v2)
  %26 = call i8* @value(%"RawPtr[Char/rtct]"* %p)
  call void @dtor162(i8* %26)
  br label %while_step

while_step:                                       ; preds = %while_body
  call void @advance175(%"RawPtr[Char/rtct]"* %"$tmpC3", %"RawPtr[Char/rtct]"* %p, i64 -1)
  %27 = load %"RawPtr[Char/rtct]"* %"$tmpC3"
  call void @"=144"(%"RawPtr[Char/rtct]"* %p, %"RawPtr[Char/rtct]" %27)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC3")
  br label %while_block

while_end:                                        ; preds = %while_block
  call void @advance149(%"RawPtr[Char/rtct]"* %"$tmpC4", %"RawPtr[Char/rtct]"* %p)
  %28 = load %"RawPtr[Char/rtct]"* %"$tmpC4"
  call void @"=144"(%"RawPtr[Char/rtct]"* %p, %"RawPtr[Char/rtct]" %28)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC4")
  br label %while_block5

while_block5:                                     ; preds = %while_step7, %while_end
  %29 = call i1 @isEmpty177(%"ContiguousMemoryRange[Char/rtct]"* %range.addr)
  %30 = xor i1 true, %29
  br i1 %30, label %while_body6, label %while_end8

while_body6:                                      ; preds = %while_block5
  %31 = call i8* @front179(%"ContiguousMemoryRange[Char/rtct]"* %range.addr)
  %32 = load i8* %31
  %33 = call i8* @value(%"RawPtr[Char/rtct]"* %p)
  store i8 %32, i8* %33
  call void @advance149(%"RawPtr[Char/rtct]"* %"$tmpC9", %"RawPtr[Char/rtct]"* %p)
  %34 = load %"RawPtr[Char/rtct]"* %"$tmpC9"
  call void @"=144"(%"RawPtr[Char/rtct]"* %p, %"RawPtr[Char/rtct]" %34)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC9")
  call void @popFront180(%"ContiguousMemoryRange[Char/rtct]"* %range.addr)
  br label %while_step7

while_step7:                                      ; preds = %while_body6
  br label %while_block5

while_end8:                                       ; preds = %while_block5
  %35 = load %String** %"$this.addr"
  %36 = getelementptr inbounds %String* %35, i32 0, i32 1
  %37 = load %String** %"$this.addr"
  %38 = getelementptr inbounds %String* %37, i32 0, i32 1
  %39 = load i64* %n
  store i64 %39, i64* %tmp.v11
  %40 = load i64* %tmp.v11
  call void @advance175(%"RawPtr[Char/rtct]"* %"$tmpC10", %"RawPtr[Char/rtct]"* %38, i64 %40)
  %41 = load %"RawPtr[Char/rtct]"* %"$tmpC10"
  call void @"=144"(%"RawPtr[Char/rtct]"* %36, %"RawPtr[Char/rtct]" %41)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC10")
  call void @dtor35(i64* %tmp.v11)
  call void @dtor146(%"RawPtr[Char/rtct]"* %q)
  call void @dtor146(%"RawPtr[Char/rtct]"* %p)
  call void @dtor35(i64* %index)
  call void @dtor31(i64* %n)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i64 @size170(%"ContiguousMemoryRange[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %"$this", %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %1, i32 0, i32 1
  %3 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %3, i32 0, i32 0
  %5 = load %"RawPtr[Char/rtct]"* %4
  %6 = call i64 @diff(%"RawPtr[Char/rtct]"* %2, %"RawPtr[Char/rtct]" %5)
  store i64 %6, i64* %tmp.v
  %7 = load i64* %tmp.v
  call void @dtor31(i64* %tmp.v)
  ret i64 %7

dumy_block:                                       ; No predecessors!
  call void @dtor31(i64* %tmp.v)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private void @frontPtr(%"RawPtr[Char/rtct]"* sret %_result, %"ContiguousMemoryRange[Char/rtct]"* %"$this") #2 {
  %_result.addr = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %_result, %"RawPtr[Char/rtct]"** %_result.addr
  %"$this.addr" = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %"$this", %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"RawPtr[Char/rtct]"** %_result.addr
  %2 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %3 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %2, i32 0, i32 0
  call void @ctor145(%"RawPtr[Char/rtct]"* %1, %"RawPtr[Char/rtct]"* %3)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @reserve171(%String* %"$this", i64 %n) #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %n.addr = alloca i64
  store i64 %n, i64* %n.addr
  %curCapacity = alloca i64
  %tmp.v = alloca i64
  %curSize = alloca i64
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  %tmp.v7 = alloca i64
  %"$tmpC8" = alloca %"RawPtr[Char/rtct]"
  %tmp.v9 = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  %2 = call i64 @capacity172(%String* %1)
  store i64 %2, i64* %curCapacity
  br label %if_block

if_block:                                         ; preds = %code
  %3 = load i64* %n.addr
  %4 = load i64* %curCapacity
  %5 = call i1 @"<="(i64 %3, i64 %4)
  br i1 %5, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  call void @dtor31(i64* %curCapacity)
  ret void

if_end:                                           ; preds = %dumy_block, %if_block
  br label %if_block1

dumy_block:                                       ; No predecessors!
  br label %if_end

if_block1:                                        ; preds = %if_end
  %6 = load i64* %n.addr
  %7 = call i1 @"<173"(i64 %6, i64 16)
  br i1 %7, label %if_then2, label %if_end3

if_then2:                                         ; preds = %if_block1
  store i64 16, i64* %n.addr
  br label %if_end3

if_end3:                                          ; preds = %if_then2, %if_block1
  br label %if_block4

if_block4:                                        ; preds = %if_end3
  %8 = load i64* %n.addr
  %9 = load i64* %curCapacity
  %10 = call double @"*156"(double 1.500000e+00, i64 %9)
  %11 = call i1 @"<152"(i64 %8, double %10)
  br i1 %11, label %if_then5, label %if_end6

if_then5:                                         ; preds = %if_block4
  %12 = load i64* %curCapacity
  %13 = call double @"*156"(double 1.500000e+00, i64 %12)
  %14 = fptoui double %13 to i64
  store i64 %14, i64* %tmp.v
  %15 = load i64* %tmp.v
  store i64 %15, i64* %n.addr
  call void @dtor31(i64* %tmp.v)
  br label %if_end6

if_end6:                                          ; preds = %if_then5, %if_block4
  %16 = load %String** %"$this.addr"
  %17 = call i64 @size174(%String* %16)
  store i64 %17, i64* %curSize
  %18 = load %String** %"$this.addr"
  %19 = getelementptr inbounds %String* %18, i32 0, i32 0
  %20 = load i64* %n.addr
  call void @reallocPtr(%"RawPtr[Char/rtct]"* %19, i64 %20)
  %21 = load %String** %"$this.addr"
  %22 = getelementptr inbounds %String* %21, i32 0, i32 1
  %23 = load %String** %"$this.addr"
  %24 = getelementptr inbounds %String* %23, i32 0, i32 0
  %25 = load i64* %curSize
  store i64 %25, i64* %tmp.v7
  %26 = load i64* %tmp.v7
  call void @advance175(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %24, i64 %26)
  %27 = load %"RawPtr[Char/rtct]"* %"$tmpC"
  call void @"=144"(%"RawPtr[Char/rtct]"* %22, %"RawPtr[Char/rtct]" %27)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  call void @dtor35(i64* %tmp.v7)
  %28 = load %String** %"$this.addr"
  %29 = getelementptr inbounds %String* %28, i32 0, i32 2
  %30 = load %String** %"$this.addr"
  %31 = getelementptr inbounds %String* %30, i32 0, i32 0
  %32 = load i64* %n.addr
  store i64 %32, i64* %tmp.v9
  %33 = load i64* %tmp.v9
  call void @advance175(%"RawPtr[Char/rtct]"* %"$tmpC8", %"RawPtr[Char/rtct]"* %31, i64 %33)
  %34 = load %"RawPtr[Char/rtct]"* %"$tmpC8"
  call void @"=144"(%"RawPtr[Char/rtct]"* %29, %"RawPtr[Char/rtct]" %34)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC8")
  call void @dtor35(i64* %tmp.v9)
  call void @dtor31(i64* %curSize)
  call void @dtor31(i64* %curCapacity)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i64 @capacity172(%String* %"$this") #2 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  %2 = getelementptr inbounds %String* %1, i32 0, i32 2
  %3 = load %String** %"$this.addr"
  %4 = getelementptr inbounds %String* %3, i32 0, i32 0
  %5 = load %"RawPtr[Char/rtct]"* %4
  %6 = call i64 @diff(%"RawPtr[Char/rtct]"* %2, %"RawPtr[Char/rtct]" %5)
  store i64 %6, i64* %tmp.v
  %7 = load i64* %tmp.v
  call void @dtor31(i64* %tmp.v)
  ret i64 %7

dumy_block:                                       ; No predecessors!
  call void @dtor31(i64* %tmp.v)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i1 @"<173"(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn30(i64 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn30(i64 %4, i64 %5)
  %7 = icmp slt i64 %3, %6
  ret i1 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @size174(%String* %"$this") #2 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  %2 = getelementptr inbounds %String* %1, i32 0, i32 1
  %3 = load %String** %"$this.addr"
  %4 = getelementptr inbounds %String* %3, i32 0, i32 0
  %5 = load %"RawPtr[Char/rtct]"* %4
  %6 = call i64 @diff(%"RawPtr[Char/rtct]"* %2, %"RawPtr[Char/rtct]" %5)
  store i64 %6, i64* %tmp.v
  %7 = load i64* %tmp.v
  call void @dtor31(i64* %tmp.v)
  ret i64 %7

dumy_block:                                       ; No predecessors!
  call void @dtor31(i64* %tmp.v)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private void @advance175(%"RawPtr[Char/rtct]"* sret %_result, %"RawPtr[Char/rtct]"* %"$this", i64 %n) #2 {
  %_result.addr = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %_result, %"RawPtr[Char/rtct]"** %_result.addr
  %"$this.addr" = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]"** %"$this.addr"
  %n.addr = alloca i64
  store i64 %n, i64* %n.addr
  %tmp.v = alloca i64
  %tmp.v1 = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load %"RawPtr[Char/rtct]"** %_result.addr
  %2 = load %"RawPtr[Char/rtct]"** %"$this.addr"
  %3 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %2)
  %4 = load i64* %n.addr
  store i64 1, i64* %tmp.v1
  %5 = load i64* %tmp.v1
  %6 = call i64 @"*176"(i64 %4, i64 %5)
  store i64 %6, i64* %tmp.v
  %7 = load i64* %tmp.v
  %8 = call i8* @ptrAdd(i8* %3, i64 %7)
  call void @fromBytePtr147(%"RawPtr[Char/rtct]"* %1, i8* %8)
  call void @dtor31(i64* %tmp.v)
  call void @dtor35(i64* %tmp.v1)
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor31(i64* %tmp.v)
  call void @dtor35(i64* %tmp.v1)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i64 @"*176"(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn34(i64 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn34(i64 %4, i64 %5)
  %7 = mul i64 %3, %6
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i1 @isEmpty177(%"ContiguousMemoryRange[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %"$this", %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %1, i32 0, i32 1
  %3 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %3, i32 0, i32 0
  %5 = load %"RawPtr[Char/rtct]"* %4
  %6 = call i64 @diff(%"RawPtr[Char/rtct]"* %2, %"RawPtr[Char/rtct]" %5)
  %7 = call i1 @"<=178"(i64 %6, i32 0)
  ret i1 %7
}

; Function Attrs: alwaysinline nounwind
define private i1 @"<=178"(i64 %x, i32 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i32* %y.addr
  %3 = call i64 @cmn130(i64 %1, i32 %2)
  %4 = load i32* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn131(i32 %4, i64 %5)
  %7 = icmp sle i64 %3, %6
  ret i1 %7
}

; Function Attrs: alwaysinline nounwind
define private i8* @front179(%"ContiguousMemoryRange[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %"$this", %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %1, i32 0, i32 0
  %3 = call i8* @value(%"RawPtr[Char/rtct]"* %2)
  ret i8* %3
}

; Function Attrs: alwaysinline nounwind
define private void @popFront180(%"ContiguousMemoryRange[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %"$this", %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %1, i32 0, i32 0
  %3 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %3, i32 0, i32 0
  call void @advance149(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %4)
  %5 = load %"RawPtr[Char/rtct]"* %"$tmpC"
  call void @"=144"(%"RawPtr[Char/rtct]"* %2, %"RawPtr[Char/rtct]" %5)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctor181(%"ContiguousMemoryRange[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]" %begin, %"RawPtr[Char/rtct]" %end) #3 {
  %"$this.addr" = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %"$this", %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %begin.addr = alloca %"RawPtr[Char/rtct]"
  store %"RawPtr[Char/rtct]" %begin, %"RawPtr[Char/rtct]"* %begin.addr
  %end.addr = alloca %"RawPtr[Char/rtct]"
  store %"RawPtr[Char/rtct]" %end, %"RawPtr[Char/rtct]"* %end.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %1, i32 0, i32 0
  call void @ctor145(%"RawPtr[Char/rtct]"* %2, %"RawPtr[Char/rtct]"* %begin.addr)
  %3 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %3, i32 0, i32 1
  call void @ctor145(%"RawPtr[Char/rtct]"* %4, %"RawPtr[Char/rtct]"* %end.addr)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor182(%"ContiguousMemoryRange[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %"$this", %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %1, i32 0, i32 1
  call void @dtor146(%"RawPtr[Char/rtct]"* %2)
  %3 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %3, i32 0, i32 0
  call void @dtor146(%"RawPtr[Char/rtct]"* %4)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctor183(%String* %"$this", %String* %other) #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %other.addr = alloca %String*
  store %String* %other, %String** %other.addr
  %size = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %other.addr
  %2 = call i64 @size174(%String* %1)
  store i64 %2, i64* %size
  %3 = load %String** %"$this.addr"
  %4 = load i64* %size
  call void @ctor184(%String* %3, i64 %4)
  %5 = load %String** %"$this.addr"
  %6 = getelementptr inbounds %String* %5, i32 0, i32 0
  %7 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %6)
  %8 = load %String** %other.addr
  %9 = getelementptr inbounds %String* %8, i32 0, i32 0
  %10 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %9)
  %11 = load i64* %size
  call void @_spr_memcpy(i8* %7, i8* %10, i64 %11)
  call void @dtor31(i64* %size)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctor184(%String* %"$this", i64 %size) #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %size.addr = alloca i64
  store i64 %size, i64* %size.addr
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  %"$tmpC1" = alloca %"RawPtr[Char/rtct]"
  %tmp.v = alloca i64
  %tmp.v2 = alloca i8
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  %2 = getelementptr inbounds %String* %1, i32 0, i32 0
  %3 = load i64* %size.addr
  %4 = call i64 @"+74"(i64 %3, i32 1)
  call void @alloc(%"RawPtr[Char/rtct]"* %"$tmpC", i64 %4)
  call void @ctor145(%"RawPtr[Char/rtct]"* %2, %"RawPtr[Char/rtct]"* %"$tmpC")
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  %5 = load %String** %"$this.addr"
  %6 = getelementptr inbounds %String* %5, i32 0, i32 1
  %7 = load %String** %"$this.addr"
  %8 = getelementptr inbounds %String* %7, i32 0, i32 0
  %9 = load i64* %size.addr
  store i64 %9, i64* %tmp.v
  %10 = load i64* %tmp.v
  call void @advance175(%"RawPtr[Char/rtct]"* %"$tmpC1", %"RawPtr[Char/rtct]"* %8, i64 %10)
  call void @ctor145(%"RawPtr[Char/rtct]"* %6, %"RawPtr[Char/rtct]"* %"$tmpC1")
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC1")
  call void @dtor35(i64* %tmp.v)
  %11 = load %String** %"$this.addr"
  %12 = getelementptr inbounds %String* %11, i32 0, i32 2
  %13 = load %String** %"$this.addr"
  %14 = getelementptr inbounds %String* %13, i32 0, i32 1
  call void @ctor145(%"RawPtr[Char/rtct]"* %12, %"RawPtr[Char/rtct]"* %14)
  store i8 0, i8* %tmp.v2
  %15 = load i8* %tmp.v2
  %16 = load %String** %"$this.addr"
  %17 = getelementptr inbounds %String* %16, i32 0, i32 1
  %18 = call i8* @value(%"RawPtr[Char/rtct]"* %17)
  store i8 %15, i8* %18
  call void @dtor162(i8* %tmp.v2)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor185(%StringOutputStream* %"$this") #2 {
  %"$this.addr" = alloca %StringOutputStream*
  store %StringOutputStream* %"$this", %StringOutputStream** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %StringOutputStream** %"$this.addr"
  %2 = getelementptr inbounds %StringOutputStream* %1, i32 0, i32 0
  call void @dtor186(%String* %2)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @dtor186(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  %2 = getelementptr inbounds %String* %1, i32 0, i32 0
  call void @freePtr(%"RawPtr[Char/rtct]"* %2)
  %3 = load %String** %"$this.addr"
  %4 = getelementptr inbounds %String* %3, i32 0, i32 2
  call void @dtor146(%"RawPtr[Char/rtct]"* %4)
  %5 = load %String** %"$this.addr"
  %6 = getelementptr inbounds %String* %5, i32 0, i32 1
  call void @dtor146(%"RawPtr[Char/rtct]"* %6)
  %7 = load %String** %"$this.addr"
  %8 = getelementptr inbounds %String* %7, i32 0, i32 0
  call void @dtor146(%"RawPtr[Char/rtct]"* %8)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctor187(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %"$this", %StringRef %source, %ExternalErrorReporter %errorReporter, %Location* %iniLocation) #3 {
  %"$this.addr" = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %"$this", %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  %source.addr = alloca %StringRef
  store %StringRef %source, %StringRef* %source.addr
  %errorReporter.addr = alloca %ExternalErrorReporter
  store %ExternalErrorReporter %errorReporter, %ExternalErrorReporter* %errorReporter.addr
  %iniLocation.addr = alloca %Location*
  store %Location* %iniLocation, %Location** %iniLocation.addr
  %tmp.v = alloca %"RangeWithLookahead[StringRef/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  %2 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1, i32 0, i32 5
  call void @ctor124(%ExternalErrorReporter* %2)
  %3 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  %4 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %3, i32 0, i32 0
  %5 = load %Location** %iniLocation.addr
  call void @ctor134(%Location* %4, %Location* %5)
  %6 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  %7 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %6, i32 0, i32 1
  %8 = load %StringRef* %source.addr
  call void @ctor190(%"RangeWithLookahead[StringRef/rtct]"* %tmp.v, %StringRef %8)
  %9 = load %"RangeWithLookahead[StringRef/rtct]"* %tmp.v
  %10 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  %11 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %10, i32 0, i32 0
  call void @ctor188(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %7, %"RangeWithLookahead[StringRef/rtct]" %9, %Location* %11)
  call void @dtor196(%"RangeWithLookahead[StringRef/rtct]"* %tmp.v)
  %12 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  %13 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %12, i32 0, i32 2
  call void @ctor121(%Token* %13)
  %14 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  %15 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %14, i32 0, i32 3
  store i1 false, i1* %15
  %16 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  %17 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %16, i32 0, i32 4
  store i1 false, i1* %17
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor188(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %"$this", %"RangeWithLookahead[StringRef/rtct]" %fsource, %Location* %flocation) #2 {
  %"$this.addr" = alloca %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %"$this", %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %"$this.addr"
  %fsource.addr = alloca %"RangeWithLookahead[StringRef/rtct]"
  store %"RangeWithLookahead[StringRef/rtct]" %fsource, %"RangeWithLookahead[StringRef/rtct]"* %fsource.addr
  %flocation.addr = alloca %Location*
  store %Location* %flocation, %Location** %flocation.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %"$this.addr"
  %2 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %1, i32 0, i32 0
  call void @ctor189(%"RangeWithLookahead[StringRef/rtct]"* %2, %"RangeWithLookahead[StringRef/rtct]"* %fsource.addr)
  %3 = load %Location** %flocation.addr
  %4 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %"$this.addr"
  %5 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %4, i32 0, i32 1
  store %Location* %3, %Location** %5
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor189(%"RangeWithLookahead[StringRef/rtct]"* %"$this", %"RangeWithLookahead[StringRef/rtct]"* %other) #2 {
  %"$this.addr" = alloca %"RangeWithLookahead[StringRef/rtct]"*
  store %"RangeWithLookahead[StringRef/rtct]"* %"$this", %"RangeWithLookahead[StringRef/rtct]"** %"$this.addr"
  %other.addr = alloca %"RangeWithLookahead[StringRef/rtct]"*
  store %"RangeWithLookahead[StringRef/rtct]"* %other, %"RangeWithLookahead[StringRef/rtct]"** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"RangeWithLookahead[StringRef/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %1, i32 0, i32 0
  %3 = load %"RangeWithLookahead[StringRef/rtct]"** %other.addr
  %4 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %3, i32 0, i32 0
  %5 = load %StringRef* %4
  call void @ctor77(%StringRef* %2, %StringRef %5)
  %6 = load %"RangeWithLookahead[StringRef/rtct]"** %"$this.addr"
  %7 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %6, i32 0, i32 1
  %8 = load %"RangeWithLookahead[StringRef/rtct]"** %other.addr
  %9 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %8, i32 0, i32 1
  call void @ctor141(%"Vector[Char/rtct]"* %7, %"Vector[Char/rtct]"* %9)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctor190(%"RangeWithLookahead[StringRef/rtct]"* %"$this", %StringRef %src) #3 {
  %"$this.addr" = alloca %"RangeWithLookahead[StringRef/rtct]"*
  store %"RangeWithLookahead[StringRef/rtct]"* %"$this", %"RangeWithLookahead[StringRef/rtct]"** %"$this.addr"
  %src.addr = alloca %StringRef
  store %StringRef %src, %StringRef* %src.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"RangeWithLookahead[StringRef/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %1, i32 0, i32 0
  call void @ctor94(%StringRef* %2)
  %3 = load %"RangeWithLookahead[StringRef/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %3, i32 0, i32 0
  %5 = load %StringRef* %src.addr
  %6 = call %StringRef* @"=191"(%StringRef* %4, %StringRef %5)
  %7 = load %"RangeWithLookahead[StringRef/rtct]"** %"$this.addr"
  %8 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %7, i32 0, i32 1
  call void @ctor119(%"Vector[Char/rtct]"* %8)
  %9 = load %"RangeWithLookahead[StringRef/rtct]"** %"$this.addr"
  %10 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %9, i32 0, i32 1
  call void @reserve(%"Vector[Char/rtct]"* %10, i64 10)
  br label %if_block

if_block:                                         ; preds = %code
  %11 = load %"RangeWithLookahead[StringRef/rtct]"** %"$this.addr"
  %12 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %11, i32 0, i32 0
  %13 = call i1 @"pre_!!192"(%StringRef* %12)
  br i1 %13, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %14 = load %"RangeWithLookahead[StringRef/rtct]"** %"$this.addr"
  %15 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %14, i32 0, i32 1
  %16 = load %"RangeWithLookahead[StringRef/rtct]"** %"$this.addr"
  %17 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %16, i32 0, i32 0
  %18 = call i8* @"post_++193"(%StringRef* %17)
  call void @"+="(%"Vector[Char/rtct]"* %15, i8* %18)
  br label %if_end

if_end:                                           ; preds = %if_then, %if_block
  ret void
}

; Function Attrs: inlinehint nounwind
define private %StringRef* @"=191"(%StringRef* %"$this", %StringRef %src) #3 {
  %"$this.addr" = alloca %StringRef*
  store %StringRef* %"$this", %StringRef** %"$this.addr"
  %src.addr = alloca %StringRef
  store %StringRef %src, %StringRef* %src.addr
  br label %code

code:                                             ; preds = %0
  %1 = getelementptr inbounds %StringRef* %src.addr, i32 0, i32 0
  %2 = load i8** %1
  %3 = load %StringRef** %"$this.addr"
  %4 = getelementptr inbounds %StringRef* %3, i32 0, i32 0
  store i8* %2, i8** %4
  %5 = getelementptr inbounds %StringRef* %src.addr, i32 0, i32 1
  %6 = load i8** %5
  %7 = load %StringRef** %"$this.addr"
  %8 = getelementptr inbounds %StringRef* %7, i32 0, i32 1
  store i8* %6, i8** %8
  %9 = load %StringRef** %"$this.addr"
  ret %StringRef* %9
}

; Function Attrs: alwaysinline nounwind
define private i1 @"pre_!!192"(%StringRef* %r) #2 {
  %r.addr = alloca %StringRef*
  store %StringRef* %r, %StringRef** %r.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef** %r.addr
  %2 = call i1 @isEmpty128(%StringRef* %1)
  %3 = xor i1 true, %2
  ret i1 %3
}

; Function Attrs: inlinehint nounwind
define private i8* @"post_++193"(%StringRef* %r) #3 {
  %r.addr = alloca %StringRef*
  store %StringRef* %r, %StringRef** %r.addr
  %res = alloca i8
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef** %r.addr
  %2 = call i8* @front194(%StringRef* %1)
  %3 = load i8* %2
  store i8 %3, i8* %res
  %4 = load %StringRef** %r.addr
  call void @popFront195(%StringRef* %4)
  call void @dtor162(i8* %res)
  ret i8* %res

dumy_block:                                       ; No predecessors!
  call void @dtor162(i8* %res)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i8* @front194(%StringRef* %"$this") #2 {
  %"$this.addr" = alloca %StringRef*
  store %StringRef* %"$this", %StringRef** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef** %"$this.addr"
  %2 = getelementptr inbounds %StringRef* %1, i32 0, i32 0
  %3 = load i8** %2
  ret i8* %3
}

; Function Attrs: alwaysinline nounwind
define private void @popFront195(%StringRef* %"$this") #2 {
  %"$this.addr" = alloca %StringRef*
  store %StringRef* %"$this", %StringRef** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef** %"$this.addr"
  %2 = getelementptr inbounds %StringRef* %1, i32 0, i32 0
  %3 = load i8** %2
  %4 = call i8* @ptrAdd(i8* %3, i64 1)
  %5 = load %StringRef** %"$this.addr"
  %6 = getelementptr inbounds %StringRef* %5, i32 0, i32 0
  store i8* %4, i8** %6
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor196(%"RangeWithLookahead[StringRef/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"RangeWithLookahead[StringRef/rtct]"*
  store %"RangeWithLookahead[StringRef/rtct]"* %"$this", %"RangeWithLookahead[StringRef/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"RangeWithLookahead[StringRef/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %1, i32 0, i32 1
  call void @dtor165(%"Vector[Char/rtct]"* %2)
  %3 = load %"RangeWithLookahead[StringRef/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %3, i32 0, i32 0
  call void @dtor78(%StringRef* %4)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor197(%ExternalErrorReporter* %"$this") #2 {
  %"$this.addr" = alloca %ExternalErrorReporter*
  store %ExternalErrorReporter* %"$this", %ExternalErrorReporter** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: noinline nounwind
define %ScannerContext* @spr_scannerIf_createScanner(%StringRef %filename, %StringRef %code, %Location* %loc) #4 {
  %filename.addr = alloca %StringRef
  store %StringRef %filename, %StringRef* %filename.addr
  %code.addr = alloca %StringRef
  store %StringRef %code, %StringRef* %code.addr
  %loc.addr = alloca %Location*
  store %Location* %loc, %Location** %loc.addr
  br label %code1

code1:                                            ; preds = %0
  %1 = load %Location** %loc.addr
  %2 = call %ScannerContext* @new(%StringRef* %filename.addr, %StringRef* %code.addr, %Location* %1)
  ret %ScannerContext* %2
}

; Function Attrs: inlinehint nounwind
define private %ScannerContext* @new(%StringRef* %arg1, %StringRef* %arg2, %Location* %arg3) #3 {
  %arg1.addr = alloca %StringRef*
  store %StringRef* %arg1, %StringRef** %arg1.addr
  %arg2.addr = alloca %StringRef*
  store %StringRef* %arg2, %StringRef** %arg2.addr
  %arg3.addr = alloca %Location*
  store %Location* %arg3, %Location** %arg3.addr
  %res = alloca %ScannerContext*
  br label %code

code:                                             ; preds = %0
  %1 = call i8* @malloc(i64 312)
  %2 = bitcast i8* %1 to %ScannerContext*
  store %ScannerContext* %2, %ScannerContext** %res
  %3 = load %ScannerContext** %res
  %4 = load %StringRef** %arg1.addr
  %5 = load %StringRef* %4
  %6 = load %StringRef** %arg2.addr
  %7 = load %StringRef* %6
  %8 = load %Location** %arg3.addr
  %9 = load %Location* %8
  call void @ctor109(%ScannerContext* %3, %StringRef %5, %StringRef %7, %Location %9)
  %10 = load %ScannerContext** %res
  ret %ScannerContext* %10
}

; Function Attrs: noinline nounwind
define void @spr_scannerIf_destroyScanner(%ScannerContext* %ctx) #4 {
  %ctx.addr = alloca %ScannerContext*
  store %ScannerContext* %ctx, %ScannerContext** %ctx.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %ScannerContext** %ctx.addr
  call void @delete(%ScannerContext* %1)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @delete(%ScannerContext* %obj) #3 {
  %obj.addr = alloca %ScannerContext*
  store %ScannerContext* %obj, %ScannerContext** %obj.addr
  br label %code

code:                                             ; preds = %0
  br label %if_block

if_block:                                         ; preds = %code
  %1 = load %ScannerContext** %obj.addr
  %2 = bitcast %ScannerContext* %1 to i8*
  %3 = call i1 @_opRefNE(i8* %2, i8* null)
  br i1 %3, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %4 = load %ScannerContext** %obj.addr
  call void @dtor198(%ScannerContext* %4)
  %5 = load %ScannerContext** %obj.addr
  %6 = bitcast %ScannerContext* %5 to i8*
  call void @free(i8* %6)
  br label %if_end

if_end:                                           ; preds = %if_then, %if_block
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor198(%ScannerContext* %"$this") #2 {
  %"$this.addr" = alloca %ScannerContext*
  store %ScannerContext* %"$this", %ScannerContext** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %ScannerContext** %"$this.addr"
  %2 = getelementptr inbounds %ScannerContext* %1, i32 0, i32 2
  call void @dtor199(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %2)
  %3 = load %ScannerContext** %"$this.addr"
  %4 = getelementptr inbounds %ScannerContext* %3, i32 0, i32 1
  call void @dtor207(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %4)
  %5 = load %ScannerContext** %"$this.addr"
  %6 = getelementptr inbounds %ScannerContext* %5, i32 0, i32 0
  call void @dtor209(%File* %6)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor199(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %"$this") #2 {
  %"$this.addr" = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %"$this", %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  %2 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1, i32 0, i32 5
  call void @dtor197(%ExternalErrorReporter* %2)
  %3 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  %4 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %3, i32 0, i32 4
  call void @dtor167(i1* %4)
  %5 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  %6 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %5, i32 0, i32 3
  call void @dtor167(i1* %6)
  %7 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  %8 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %7, i32 0, i32 2
  call void @dtor200(%Token* %8)
  %9 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  %10 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %9, i32 0, i32 1
  call void @dtor206(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %10)
  %11 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %"$this.addr"
  %12 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %11, i32 0, i32 0
  call void @dtor202(%Location* %12)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor200(%Token* %"$this") #2 {
  %"$this.addr" = alloca %Token*
  store %Token* %"$this", %Token** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %Token** %"$this.addr"
  %2 = getelementptr inbounds %Token* %1, i32 0, i32 4
  call void @dtor154(double* %2)
  %3 = load %Token** %"$this.addr"
  %4 = getelementptr inbounds %Token* %3, i32 0, i32 3
  call void @dtor27(i64* %4)
  %5 = load %Token** %"$this.addr"
  %6 = getelementptr inbounds %Token* %5, i32 0, i32 2
  call void @dtor186(%String* %6)
  %7 = load %Token** %"$this.addr"
  %8 = getelementptr inbounds %Token* %7, i32 0, i32 1
  call void @dtor201(%TokenType* %8)
  %9 = load %Token** %"$this.addr"
  %10 = getelementptr inbounds %Token* %9, i32 0, i32 0
  call void @dtor202(%Location* %10)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor201(%TokenType* %"$this") #2 {
  %"$this.addr" = alloca %TokenType*
  store %TokenType* %"$this", %TokenType** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %TokenType** %"$this.addr"
  %2 = getelementptr inbounds %TokenType* %1, i32 0, i32 0
  call void @dtor58(i32* %2)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor202(%Location* %"$this") #2 {
  %"$this.addr" = alloca %Location*
  store %Location* %"$this", %Location** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %Location** %"$this.addr"
  %2 = getelementptr inbounds %Location* %1, i32 0, i32 2
  call void @dtor203(%LineCol* %2)
  %3 = load %Location** %"$this.addr"
  %4 = getelementptr inbounds %Location* %3, i32 0, i32 1
  call void @dtor203(%LineCol* %4)
  %5 = load %Location** %"$this.addr"
  %6 = getelementptr inbounds %Location* %5, i32 0, i32 0
  call void @dtor204(%SourceCode* %6)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor203(%LineCol* %"$this") #2 {
  %"$this.addr" = alloca %LineCol*
  store %LineCol* %"$this", %LineCol** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %LineCol** %"$this.addr"
  %2 = getelementptr inbounds %LineCol* %1, i32 0, i32 1
  call void @dtor19(i32* %2)
  %3 = load %LineCol** %"$this.addr"
  %4 = getelementptr inbounds %LineCol* %3, i32 0, i32 0
  call void @dtor19(i32* %4)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor204(%SourceCode* %"$this") #2 {
  %"$this.addr" = alloca %SourceCode*
  store %SourceCode* %"$this", %SourceCode** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %SourceCode** %"$this.addr"
  %2 = getelementptr inbounds %SourceCode* %1, i32 0, i32 0
  call void @dtor205(%UntypedPtr* %2)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor205(%UntypedPtr* %"$this") #2 {
  %"$this.addr" = alloca %UntypedPtr*
  store %UntypedPtr* %"$this", %UntypedPtr** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor206(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %"$this") #2 {
  %"$this.addr" = alloca %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %"$this", %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %"$this.addr"
  %2 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %1, i32 0, i32 0
  call void @dtor196(%"RangeWithLookahead[StringRef/rtct]"* %2)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor207(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %"$this") #2 {
  %"$this.addr" = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %"$this", %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  %2 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1, i32 0, i32 5
  call void @dtor197(%ExternalErrorReporter* %2)
  %3 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  %4 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %3, i32 0, i32 4
  call void @dtor167(i1* %4)
  %5 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  %6 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %5, i32 0, i32 3
  call void @dtor167(i1* %6)
  %7 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  %8 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %7, i32 0, i32 2
  call void @dtor200(%Token* %8)
  %9 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  %10 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %9, i32 0, i32 1
  call void @dtor208(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %10)
  %11 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %"$this.addr"
  %12 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %11, i32 0, i32 0
  call void @dtor202(%Location* %12)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor208(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %"$this") #2 {
  %"$this.addr" = alloca %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %"$this", %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %"$this.addr"
  %2 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %1, i32 0, i32 0
  call void @dtor164(%"RangeWithLookahead[FileRange/rtct]"* %2)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @dtor209(%File* %"$this") #3 {
  %"$this.addr" = alloca %File*
  store %File* %"$this", %File** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %File** %"$this.addr"
  call void @close(%File* %1)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @close(%File* %"$this") #3 {
  %"$this.addr" = alloca %File*
  store %File* %"$this", %File** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  br label %if_block

if_block:                                         ; preds = %code
  %1 = load %File** %"$this.addr"
  %2 = call i1 @isOpen(%File* %1)
  br i1 %2, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %3 = load %File** %"$this.addr"
  %4 = getelementptr inbounds %File* %3, i32 0, i32 0
  %5 = load i8** %4
  call void @fclose(i8* %5)
  br label %if_end

if_end:                                           ; preds = %if_then, %if_block
  %6 = load %File** %"$this.addr"
  %7 = getelementptr inbounds %File* %6, i32 0, i32 0
  store i8* null, i8** %7
  ret void
}

declare void @fclose(i8*)

; Function Attrs: noinline nounwind
define void @spr_scannerIf_nextToken(%ScannerContext* %ctx, %Token* %outToken) #4 {
  %ctx.addr = alloca %ScannerContext*
  store %ScannerContext* %ctx, %ScannerContext** %ctx.addr
  %outToken.addr = alloca %Token*
  store %Token* %outToken, %Token** %outToken.addr
  %"$tmpC" = alloca %Token
  %"$tmpC1" = alloca %Token
  br label %code

code:                                             ; preds = %0
  br label %if_block

if_block:                                         ; preds = %code
  %1 = load %ScannerContext** %ctx.addr
  %2 = getelementptr inbounds %ScannerContext* %1, i32 0, i32 0
  %3 = call i1 @isOpen(%File* %2)
  br i1 %3, label %if_then, label %if_else

if_then:                                          ; preds = %if_block
  %4 = load %Token** %outToken.addr
  %5 = load %ScannerContext** %ctx.addr
  %6 = getelementptr inbounds %ScannerContext* %5, i32 0, i32 1
  call void @"post_++218"(%Token* %"$tmpC", %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %6)
  call void @"=210"(%Token* %4, %Token* %"$tmpC")
  call void @dtor200(%Token* %"$tmpC")
  br label %if_end

if_else:                                          ; preds = %if_block
  %7 = load %Token** %outToken.addr
  %8 = load %ScannerContext** %ctx.addr
  %9 = getelementptr inbounds %ScannerContext* %8, i32 0, i32 2
  call void @"post_++287"(%Token* %"$tmpC1", %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %9)
  call void @"=210"(%Token* %7, %Token* %"$tmpC1")
  call void @dtor200(%Token* %"$tmpC1")
  br label %if_end

if_end:                                           ; preds = %if_else, %if_then
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @"=210"(%Token* %"$this", %Token* %other) #2 {
  %"$this.addr" = alloca %Token*
  store %Token* %"$this", %Token** %"$this.addr"
  %other.addr = alloca %Token*
  store %Token* %other, %Token** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %Token** %"$this.addr"
  %2 = getelementptr inbounds %Token* %1, i32 0, i32 0
  %3 = load %Token** %other.addr
  %4 = getelementptr inbounds %Token* %3, i32 0, i32 0
  call void @"=211"(%Location* %2, %Location* %4)
  %5 = load %Token** %"$this.addr"
  %6 = getelementptr inbounds %Token* %5, i32 0, i32 1
  %7 = load %Token** %other.addr
  %8 = getelementptr inbounds %Token* %7, i32 0, i32 1
  call void @"=215"(%TokenType* %6, %TokenType* %8)
  %9 = load %Token** %"$this.addr"
  %10 = getelementptr inbounds %Token* %9, i32 0, i32 2
  %11 = load %Token** %other.addr
  %12 = getelementptr inbounds %Token* %11, i32 0, i32 2
  %13 = call %String* @"=216"(%String* %10, %String* %12)
  %14 = load %Token** %other.addr
  %15 = getelementptr inbounds %Token* %14, i32 0, i32 3
  %16 = load i64* %15
  %17 = load %Token** %"$this.addr"
  %18 = getelementptr inbounds %Token* %17, i32 0, i32 3
  store i64 %16, i64* %18
  %19 = load %Token** %other.addr
  %20 = getelementptr inbounds %Token* %19, i32 0, i32 4
  %21 = load double* %20
  %22 = load %Token** %"$this.addr"
  %23 = getelementptr inbounds %Token* %22, i32 0, i32 4
  store double %21, double* %23
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @"=211"(%Location* %"$this", %Location* %other) #2 {
  %"$this.addr" = alloca %Location*
  store %Location* %"$this", %Location** %"$this.addr"
  %other.addr = alloca %Location*
  store %Location* %other, %Location** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %Location** %"$this.addr"
  %2 = getelementptr inbounds %Location* %1, i32 0, i32 0
  %3 = load %Location** %other.addr
  %4 = getelementptr inbounds %Location* %3, i32 0, i32 0
  call void @"=212"(%SourceCode* %2, %SourceCode* %4)
  %5 = load %Location** %"$this.addr"
  %6 = getelementptr inbounds %Location* %5, i32 0, i32 1
  %7 = load %Location** %other.addr
  %8 = getelementptr inbounds %Location* %7, i32 0, i32 1
  call void @"=214"(%LineCol* %6, %LineCol* %8)
  %9 = load %Location** %"$this.addr"
  %10 = getelementptr inbounds %Location* %9, i32 0, i32 2
  %11 = load %Location** %other.addr
  %12 = getelementptr inbounds %Location* %11, i32 0, i32 2
  call void @"=214"(%LineCol* %10, %LineCol* %12)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @"=212"(%SourceCode* %"$this", %SourceCode* %other) #2 {
  %"$this.addr" = alloca %SourceCode*
  store %SourceCode* %"$this", %SourceCode** %"$this.addr"
  %other.addr = alloca %SourceCode*
  store %SourceCode* %other, %SourceCode** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %SourceCode** %"$this.addr"
  %2 = getelementptr inbounds %SourceCode* %1, i32 0, i32 0
  %3 = load %SourceCode** %other.addr
  %4 = getelementptr inbounds %SourceCode* %3, i32 0, i32 0
  call void @"=213"(%UntypedPtr* %2, %UntypedPtr* %4)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @"=213"(%UntypedPtr* %"$this", %UntypedPtr* %other) #2 {
  %"$this.addr" = alloca %UntypedPtr*
  store %UntypedPtr* %"$this", %UntypedPtr** %"$this.addr"
  %other.addr = alloca %UntypedPtr*
  store %UntypedPtr* %other, %UntypedPtr** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %UntypedPtr** %other.addr
  %2 = getelementptr inbounds %UntypedPtr* %1, i32 0, i32 0
  %3 = load i8** %2
  %4 = load %UntypedPtr** %"$this.addr"
  %5 = getelementptr inbounds %UntypedPtr* %4, i32 0, i32 0
  store i8* %3, i8** %5
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @"=214"(%LineCol* %"$this", %LineCol* %other) #2 {
  %"$this.addr" = alloca %LineCol*
  store %LineCol* %"$this", %LineCol** %"$this.addr"
  %other.addr = alloca %LineCol*
  store %LineCol* %other, %LineCol** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %LineCol** %other.addr
  %2 = getelementptr inbounds %LineCol* %1, i32 0, i32 0
  %3 = load i32* %2
  %4 = load %LineCol** %"$this.addr"
  %5 = getelementptr inbounds %LineCol* %4, i32 0, i32 0
  store i32 %3, i32* %5
  %6 = load %LineCol** %other.addr
  %7 = getelementptr inbounds %LineCol* %6, i32 0, i32 1
  %8 = load i32* %7
  %9 = load %LineCol** %"$this.addr"
  %10 = getelementptr inbounds %LineCol* %9, i32 0, i32 1
  store i32 %8, i32* %10
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @"=215"(%TokenType* %"$this", %TokenType* %other) #2 {
  %"$this.addr" = alloca %TokenType*
  store %TokenType* %"$this", %TokenType** %"$this.addr"
  %other.addr = alloca %TokenType*
  store %TokenType* %other, %TokenType** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %TokenType** %other.addr
  %2 = getelementptr inbounds %TokenType* %1, i32 0, i32 0
  %3 = load i32* %2
  %4 = load %TokenType** %"$this.addr"
  %5 = getelementptr inbounds %TokenType* %4, i32 0, i32 0
  store i32 %3, i32* %5
  ret void
}

; Function Attrs: inlinehint nounwind
define private %String* @"=216"(%String* %"$this", %String* %other) #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %other.addr = alloca %String*
  store %String* %other, %String** %other.addr
  %tmp = alloca %String
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %other.addr
  call void @ctor183(%String* %tmp, %String* %1)
  %2 = load %String** %"$this.addr"
  call void @swap(%String* %tmp, %String* %2)
  %3 = load %String** %"$this.addr"
  call void @dtor186(%String* %tmp)
  ret %String* %3

dumy_block:                                       ; No predecessors!
  call void @dtor186(%String* %tmp)
  unreachable
}

; Function Attrs: inlinehint nounwind
define private void @swap(%String* %"$this", %String* %other) #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %other.addr = alloca %String*
  store %String* %other, %String** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  %2 = getelementptr inbounds %String* %1, i32 0, i32 0
  %3 = load %String** %other.addr
  %4 = getelementptr inbounds %String* %3, i32 0, i32 0
  call void @swap217(%"RawPtr[Char/rtct]"* %2, %"RawPtr[Char/rtct]"* %4)
  %5 = load %String** %"$this.addr"
  %6 = getelementptr inbounds %String* %5, i32 0, i32 1
  %7 = load %String** %other.addr
  %8 = getelementptr inbounds %String* %7, i32 0, i32 1
  call void @swap217(%"RawPtr[Char/rtct]"* %6, %"RawPtr[Char/rtct]"* %8)
  %9 = load %String** %"$this.addr"
  %10 = getelementptr inbounds %String* %9, i32 0, i32 2
  %11 = load %String** %other.addr
  %12 = getelementptr inbounds %String* %11, i32 0, i32 2
  call void @swap217(%"RawPtr[Char/rtct]"* %10, %"RawPtr[Char/rtct]"* %12)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @swap217(%"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]"* %other) #3 {
  %"$this.addr" = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]"** %"$this.addr"
  %other.addr = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %other, %"RawPtr[Char/rtct]"** %other.addr
  %t = alloca i8*
  br label %code

code:                                             ; preds = %0
  %1 = load %"RawPtr[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"RawPtr[Char/rtct]"* %1, i32 0, i32 0
  %3 = load i8** %2
  store i8* %3, i8** %t
  %4 = load %"RawPtr[Char/rtct]"** %other.addr
  %5 = getelementptr inbounds %"RawPtr[Char/rtct]"* %4, i32 0, i32 0
  %6 = load i8** %5
  %7 = load %"RawPtr[Char/rtct]"** %"$this.addr"
  %8 = getelementptr inbounds %"RawPtr[Char/rtct]"* %7, i32 0, i32 0
  store i8* %6, i8** %8
  %9 = load i8** %t
  %10 = load %"RawPtr[Char/rtct]"** %other.addr
  %11 = getelementptr inbounds %"RawPtr[Char/rtct]"* %10, i32 0, i32 0
  store i8* %9, i8** %11
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @"post_++218"(%Token* sret %_result, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %r) #3 {
  %_result.addr = alloca %Token*
  store %Token* %_result, %Token** %_result.addr
  %r.addr = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %r, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %r.addr
  %res = alloca %Token
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %r.addr
  call void @front219(%Token* %res, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1)
  %2 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %r.addr
  call void @popFront220(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %2)
  %3 = load %Token** %_result.addr
  call void @ctor286(%Token* %3, %Token* %res)
  call void @dtor200(%Token* %res)
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor200(%Token* %res)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @front219(%Token* sret %_result, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s) #3 {
  %_result.addr = alloca %Token*
  store %Token* %_result, %Token** %_result.addr
  %s.addr = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  br label %code

code:                                             ; preds = %0
  br label %if_block

if_block:                                         ; preds = %code
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1, i32 0, i32 3
  %3 = load i1* %2
  %4 = xor i1 true, %3
  br i1 %4, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %5 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %6 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %5, i32 0, i32 3
  store i1 true, i1* %6
  %7 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @popFront220(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %7)
  br label %if_end

if_end:                                           ; preds = %if_then, %if_block
  %8 = load %Token** %_result.addr
  %9 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %10 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %9, i32 0, i32 2
  call void @ctor286(%Token* %8, %Token* %10)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @popFront220(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s) #3 {
  %s.addr = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %"$tmpC" = alloca %TokenType
  %tmp.v = alloca %TokenType
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1, i32 0, i32 2
  %3 = getelementptr inbounds %Token* %2, i32 0, i32 1
  %4 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @nextToken(%TokenType* %"$tmpC", %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %4)
  call void @"=215"(%TokenType* %3, %TokenType* %"$tmpC")
  call void @dtor201(%TokenType* %"$tmpC")
  %5 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %6 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %5, i32 0, i32 2
  %7 = getelementptr inbounds %Token* %6, i32 0, i32 0
  %8 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %9 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %8, i32 0, i32 1
  %10 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %9, i32 0, i32 1
  %11 = load %Location** %10
  call void @"=211"(%Location* %7, %Location* %11)
  %12 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %13 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %12, i32 0, i32 2
  %14 = getelementptr inbounds %Token* %13, i32 0, i32 1
  call void @ctor224(%TokenType* %tmp.v, i32 0)
  %15 = call i1 @"==284"(%TokenType* %14, %TokenType* %tmp.v)
  %16 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %17 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %16, i32 0, i32 4
  store i1 %15, i1* %17
  call void @dtor201(%TokenType* %tmp.v)
  ret void
}

; Function Attrs: noinline nounwind
define void @nextToken(%TokenType* sret %_result, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s) #4 {
  %_result.addr = alloca %TokenType*
  store %TokenType* %_result, %TokenType** %_result.addr
  %s.addr = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %loc = alloca %Location*
  %ch = alloca i8
  %ch2 = alloca i8
  %funptr = alloca i1 (i8)*
  %tmp.v = alloca %"$lambdaEnclosure"
  %c1 = alloca i8
  %c2 = alloca i8
  %"$tmpForRef" = alloca %String
  %tmp.v34 = alloca %String
  %"$tmpC" = alloca %String
  %const.bytes = alloca [27 x i8]
  %const.struct = alloca %StringRef
  %const.bytes140 = alloca [2 x i8]
  %const.struct141 = alloca %StringRef
  %tmp.v142 = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  %3 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %2, i32 0, i32 1
  %4 = load %Location** %3
  store %Location* %4, %Location** %loc
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  br i1 true, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  %5 = load %Location** %loc
  call void @stepOver(%Location* %5)
  %6 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %7 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %6, i32 0, i32 2
  %8 = getelementptr inbounds %Token* %7, i32 0, i32 2
  call void @clear(%String* %8)
  br label %if_block

while_step:                                       ; preds = %dumy_block143, %if_end33, %if_end10, %if_then2
  br label %while_block

while_end:                                        ; preds = %while_block
  ret void

if_block:                                         ; preds = %while_body
  %9 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %10 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %9, i32 0, i32 1
  %11 = call i1 @isEmpty221(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %10)
  br i1 %11, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %12 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %12, i32 0)
  ret void

if_end:                                           ; preds = %dumy_block, %if_block
  %13 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %14 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %13, i32 0, i32 1
  %15 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %14)
  store i8 %15, i8* %ch
  %16 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %17 = call i8 @peekChar(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %16)
  store i8 %17, i8* %ch2
  br label %if_block1

dumy_block:                                       ; No predecessors!
  br label %if_end

if_block1:                                        ; preds = %if_end
  %18 = load i8* %ch
  %19 = call i1 @isSpace(i8 %18)
  br i1 %19, label %if_then2, label %if_end3

if_then2:                                         ; preds = %if_block1
  %20 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %21 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %20, i32 0, i32 1
  store i1 (i8)* @isSpace, i1 (i8)** %funptr
  %22 = bitcast i1 (i8)** %funptr to %"FunctionPtr[Bool/rtct, Char/rtct]"*
  %23 = load %"FunctionPtr[Bool/rtct, Char/rtct]"* %22
  call void @advanceIf(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %21, %"FunctionPtr[Bool/rtct, Char/rtct]" %23)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  br label %while_step

if_end3:                                          ; preds = %dumy_block4, %if_block1
  br label %if_block5

dumy_block4:                                      ; No predecessors!
  br label %if_end3

if_block5:                                        ; preds = %if_end3
  %24 = load i8* %ch
  %25 = icmp eq i8 %24, 47
  br i1 %25, label %cond.true, label %cond.false

if_then6:                                         ; preds = %cond.end
  %26 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %27 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %26, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %27)
  %28 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %29 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %28, i32 0, i32 1
  call void @ctor245(%"$lambdaEnclosure"* %tmp.v)
  %30 = load %"$lambdaEnclosure"* %tmp.v
  call void @advanceIf243(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %29, %"$lambdaEnclosure" %30)
  call void @dtor246(%"$lambdaEnclosure"* %tmp.v)
  br label %if_block8

if_end7:                                          ; preds = %dumy_block11, %cond.end
  br label %if_block12

cond.true:                                        ; preds = %if_block5
  %31 = load i8* %ch2
  %32 = icmp eq i8 %31, 47
  br label %cond.end

cond.false:                                       ; preds = %if_block5
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ %32, %cond.true ], [ false, %cond.false ]
  br i1 %cond.res, label %if_then6, label %if_end7

if_block8:                                        ; preds = %if_then6
  %33 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %34 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %33, i32 0, i32 1
  %35 = call i1 @"pre_!!247"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %34)
  br i1 %35, label %if_then9, label %if_end10

if_then9:                                         ; preds = %if_block8
  %36 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %37 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %36, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %37)
  br label %if_end10

if_end10:                                         ; preds = %if_then9, %if_block8
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  br label %while_step

dumy_block11:                                     ; No predecessors!
  br label %if_end7

if_block12:                                       ; preds = %if_end7
  %38 = load i8* %ch
  %39 = icmp eq i8 %38, 47
  br i1 %39, label %cond.true15, label %cond.false16

if_then13:                                        ; preds = %cond.end17
  %40 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %41 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %40, i32 0, i32 1
  call void @advance248(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %41, i64 2)
  store i8 97, i8* %c1
  store i8 97, i8* %c2
  br label %while_block19

if_end14:                                         ; preds = %dumy_block36, %cond.end17
  br label %if_block37

cond.true15:                                      ; preds = %if_block12
  %42 = load i8* %ch2
  %43 = icmp eq i8 %42, 42
  br label %cond.end17

cond.false16:                                     ; preds = %if_block12
  br label %cond.end17

cond.end17:                                       ; preds = %cond.false16, %cond.true15
  %cond.res18 = phi i1 [ %43, %cond.true15 ], [ false, %cond.false16 ]
  br i1 %cond.res18, label %if_then13, label %if_end14

while_block19:                                    ; preds = %while_step21, %if_then13
  %44 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %45 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %44, i32 0, i32 1
  %46 = call i1 @"pre_!!247"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %45)
  br i1 %46, label %cond.true23, label %cond.false24

while_body20:                                     ; preds = %cond.end25
  %47 = load i8* %c2
  store i8 %47, i8* %c1
  %48 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %49 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %48, i32 0, i32 1
  %50 = call i8 @"pre_++249"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %49)
  store i8 %50, i8* %c2
  br label %while_step21

while_step21:                                     ; preds = %while_body20
  br label %while_block19

while_end22:                                      ; preds = %cond.end25
  br label %if_block31

cond.true23:                                      ; preds = %while_block19
  %51 = load i8* %c1
  %52 = icmp ne i8 %51, 42
  br i1 %52, label %cond.true26, label %cond.false27

cond.false24:                                     ; preds = %while_block19
  br label %cond.end25

cond.end25:                                       ; preds = %cond.false24, %cond.end28
  %cond.res30 = phi i1 [ %cond.res29, %cond.end28 ], [ false, %cond.false24 ]
  br i1 %cond.res30, label %while_body20, label %while_end22

cond.true26:                                      ; preds = %cond.true23
  br label %cond.end28

cond.false27:                                     ; preds = %cond.true23
  %53 = load i8* %c2
  %54 = icmp ne i8 %53, 47
  br label %cond.end28

cond.end28:                                       ; preds = %cond.false27, %cond.true26
  %cond.res29 = phi i1 [ true, %cond.true26 ], [ %54, %cond.false27 ]
  br label %cond.end25

if_block31:                                       ; preds = %while_end22
  %55 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %56 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %55, i32 0, i32 1
  %57 = call i1 @"pre_!"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %56)
  br i1 %57, label %if_then32, label %if_end33

if_then32:                                        ; preds = %if_block31
  %58 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @ctorFromCt(%String* %tmp.v34)
  %59 = load %String* %tmp.v34
  store %String %59, %String* %"$tmpForRef"
  call void @reportError250(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %58, %String* %"$tmpForRef")
  call void @dtor186(%String* %tmp.v34)
  %60 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %60, i32 0)
  call void @dtor162(i8* %c2)
  call void @dtor162(i8* %c1)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end33:                                         ; preds = %dumy_block35, %if_block31
  %61 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %62 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %61, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %62)
  call void @dtor162(i8* %c2)
  call void @dtor162(i8* %c1)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  br label %while_step

dumy_block35:                                     ; No predecessors!
  br label %if_end33

dumy_block36:                                     ; No predecessors!
  call void @dtor162(i8* %c2)
  call void @dtor162(i8* %c1)
  br label %if_end14

if_block37:                                       ; preds = %if_end14
  %63 = load i8* %ch
  %64 = icmp eq i8 %63, 39
  br i1 %64, label %if_then38, label %if_end39

if_then38:                                        ; preds = %if_block37
  %65 = load %TokenType** %_result.addr
  %66 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @parseCharacter(%TokenType* %65, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %66)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end39:                                         ; preds = %dumy_block40, %if_block37
  br label %if_block41

dumy_block40:                                     ; No predecessors!
  br label %if_end39

if_block41:                                       ; preds = %if_end39
  %67 = load i8* %ch
  %68 = icmp eq i8 %67, 34
  br i1 %68, label %if_then42, label %if_end43

if_then42:                                        ; preds = %if_block41
  %69 = load %TokenType** %_result.addr
  %70 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @parseString(%TokenType* %69, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %70)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end43:                                         ; preds = %dumy_block44, %if_block41
  br label %if_block45

dumy_block44:                                     ; No predecessors!
  br label %if_end43

if_block45:                                       ; preds = %if_end43
  %71 = load i8* %ch
  %72 = icmp eq i8 %71, 60
  br i1 %72, label %cond.true48, label %cond.false49

if_then46:                                        ; preds = %cond.end50
  %73 = load %TokenType** %_result.addr
  %74 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @parseStringNE(%TokenType* %73, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %74)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end47:                                         ; preds = %dumy_block52, %cond.end50
  br label %if_block53

cond.true48:                                      ; preds = %if_block45
  %75 = load i8* %ch2
  %76 = icmp eq i8 %75, 123
  br label %cond.end50

cond.false49:                                     ; preds = %if_block45
  br label %cond.end50

cond.end50:                                       ; preds = %cond.false49, %cond.true48
  %cond.res51 = phi i1 [ %76, %cond.true48 ], [ false, %cond.false49 ]
  br i1 %cond.res51, label %if_then46, label %if_end47

dumy_block52:                                     ; No predecessors!
  br label %if_end47

if_block53:                                       ; preds = %if_end47
  %77 = load i8* %ch
  %78 = icmp eq i8 %77, 123
  br i1 %78, label %if_then54, label %if_end55

if_then54:                                        ; preds = %if_block53
  %79 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %80 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %79, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %80)
  %81 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %81, i32 287)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end55:                                         ; preds = %dumy_block56, %if_block53
  br label %if_block57

dumy_block56:                                     ; No predecessors!
  br label %if_end55

if_block57:                                       ; preds = %if_end55
  %82 = load i8* %ch
  %83 = icmp eq i8 %82, 123
  br i1 %83, label %if_then58, label %if_end59

if_then58:                                        ; preds = %if_block57
  %84 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %85 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %84, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %85)
  %86 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %86, i32 287)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end59:                                         ; preds = %dumy_block60, %if_block57
  br label %if_block61

dumy_block60:                                     ; No predecessors!
  br label %if_end59

if_block61:                                       ; preds = %if_end59
  %87 = load i8* %ch
  %88 = icmp eq i8 %87, 125
  br i1 %88, label %if_then62, label %if_end63

if_then62:                                        ; preds = %if_block61
  %89 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %90 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %89, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %90)
  %91 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %91, i32 288)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end63:                                         ; preds = %dumy_block64, %if_block61
  br label %if_block65

dumy_block64:                                     ; No predecessors!
  br label %if_end63

if_block65:                                       ; preds = %if_end63
  %92 = load i8* %ch
  %93 = icmp eq i8 %92, 91
  br i1 %93, label %if_then66, label %if_end67

if_then66:                                        ; preds = %if_block65
  %94 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %95 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %94, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %95)
  %96 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %96, i32 289)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end67:                                         ; preds = %dumy_block68, %if_block65
  br label %if_block69

dumy_block68:                                     ; No predecessors!
  br label %if_end67

if_block69:                                       ; preds = %if_end67
  %97 = load i8* %ch
  %98 = icmp eq i8 %97, 93
  br i1 %98, label %if_then70, label %if_end71

if_then70:                                        ; preds = %if_block69
  %99 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %100 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %99, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %100)
  %101 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %101, i32 290)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end71:                                         ; preds = %dumy_block72, %if_block69
  br label %if_block73

dumy_block72:                                     ; No predecessors!
  br label %if_end71

if_block73:                                       ; preds = %if_end71
  %102 = load i8* %ch
  %103 = icmp eq i8 %102, 40
  br i1 %103, label %if_then74, label %if_end75

if_then74:                                        ; preds = %if_block73
  %104 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %105 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %104, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %105)
  %106 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %106, i32 291)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end75:                                         ; preds = %dumy_block76, %if_block73
  br label %if_block77

dumy_block76:                                     ; No predecessors!
  br label %if_end75

if_block77:                                       ; preds = %if_end75
  %107 = load i8* %ch
  %108 = icmp eq i8 %107, 41
  br i1 %108, label %if_then78, label %if_end79

if_then78:                                        ; preds = %if_block77
  %109 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %110 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %109, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %110)
  %111 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %111, i32 292)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end79:                                         ; preds = %dumy_block80, %if_block77
  br label %if_block81

dumy_block80:                                     ; No predecessors!
  br label %if_end79

if_block81:                                       ; preds = %if_end79
  %112 = load i8* %ch
  %113 = icmp eq i8 %112, 59
  br i1 %113, label %if_then82, label %if_end83

if_then82:                                        ; preds = %if_block81
  %114 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %115 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %114, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %115)
  %116 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %116, i32 294)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end83:                                         ; preds = %dumy_block84, %if_block81
  br label %if_block85

dumy_block84:                                     ; No predecessors!
  br label %if_end83

if_block85:                                       ; preds = %if_end83
  %117 = load i8* %ch
  %118 = icmp eq i8 %117, 44
  br i1 %118, label %if_then86, label %if_end87

if_then86:                                        ; preds = %if_block85
  %119 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %120 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %119, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %120)
  %121 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %121, i32 295)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end87:                                         ; preds = %dumy_block88, %if_block85
  br label %if_block89

dumy_block88:                                     ; No predecessors!
  br label %if_end87

if_block89:                                       ; preds = %if_end87
  %122 = load i8* %ch
  %123 = icmp eq i8 %122, 96
  br i1 %123, label %if_then90, label %if_end91

if_then90:                                        ; preds = %if_block89
  %124 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %125 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %124, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %125)
  %126 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %126, i32 297)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end91:                                         ; preds = %dumy_block92, %if_block89
  br label %if_block93

dumy_block92:                                     ; No predecessors!
  br label %if_end91

if_block93:                                       ; preds = %if_end91
  %127 = load i8* %ch
  %128 = icmp eq i8 %127, 58
  br i1 %128, label %cond.true96, label %cond.false97

if_then94:                                        ; preds = %cond.end98
  %129 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %130 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %129, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %130)
  %131 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %131, i32 293)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end95:                                         ; preds = %dumy_block100, %cond.end98
  br label %if_block101

cond.true96:                                      ; preds = %if_block93
  %132 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %133 = call i8 @peekChar(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %132)
  %134 = call i1 @isOpChar(i8 %133)
  %135 = xor i1 true, %134
  br label %cond.end98

cond.false97:                                     ; preds = %if_block93
  br label %cond.end98

cond.end98:                                       ; preds = %cond.false97, %cond.true96
  %cond.res99 = phi i1 [ %135, %cond.true96 ], [ false, %cond.false97 ]
  br i1 %cond.res99, label %if_then94, label %if_end95

dumy_block100:                                    ; No predecessors!
  br label %if_end95

if_block101:                                      ; preds = %if_end95
  %136 = load i8* %ch
  %137 = icmp eq i8 %136, 61
  br i1 %137, label %cond.true104, label %cond.false105

if_then102:                                       ; preds = %cond.end106
  %138 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %139 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %138, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %139)
  %140 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %140, i32 298)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end103:                                        ; preds = %dumy_block108, %cond.end106
  br label %if_block109

cond.true104:                                     ; preds = %if_block101
  %141 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %142 = call i8 @peekChar(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %141)
  %143 = call i1 @isOpChar(i8 %142)
  %144 = xor i1 true, %143
  br label %cond.end106

cond.false105:                                    ; preds = %if_block101
  br label %cond.end106

cond.end106:                                      ; preds = %cond.false105, %cond.true104
  %cond.res107 = phi i1 [ %144, %cond.true104 ], [ false, %cond.false105 ]
  br i1 %cond.res107, label %if_then102, label %if_end103

dumy_block108:                                    ; No predecessors!
  br label %if_end103

if_block109:                                      ; preds = %if_end103
  %145 = load i8* %ch
  %146 = call i1 @isOpCharDot(i8 %145)
  br i1 %146, label %if_then110, label %if_end111

if_then110:                                       ; preds = %if_block109
  br label %if_block112

if_end111:                                        ; preds = %if_end114, %if_block109
  br label %if_block116

if_block112:                                      ; preds = %if_then110
  %147 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %148 = call i1 @parseOperator(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %147)
  br i1 %148, label %if_then113, label %if_end114

if_then113:                                       ; preds = %if_block112
  %149 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %149, i32 300)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end114:                                        ; preds = %dumy_block115, %if_block112
  br label %if_end111

dumy_block115:                                    ; No predecessors!
  br label %if_end114

if_block116:                                      ; preds = %if_end111
  %150 = load i8* %ch
  %151 = icmp eq i8 %150, 46
  br i1 %151, label %cond.true119, label %cond.false120

if_then117:                                       ; preds = %cond.end121
  %152 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %153 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %152, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %153)
  %154 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %154, i32 296)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end118:                                        ; preds = %dumy_block123, %cond.end121
  br label %if_block124

cond.true119:                                     ; preds = %if_block116
  %155 = load i8* %ch2
  %156 = call i1 @isDigit(i8 %155)
  %157 = xor i1 true, %156
  br label %cond.end121

cond.false120:                                    ; preds = %if_block116
  br label %cond.end121

cond.end121:                                      ; preds = %cond.false120, %cond.true119
  %cond.res122 = phi i1 [ %157, %cond.true119 ], [ false, %cond.false120 ]
  br i1 %cond.res122, label %if_then117, label %if_end118

dumy_block123:                                    ; No predecessors!
  br label %if_end118

if_block124:                                      ; preds = %if_end118
  %158 = load i8* %ch
  %159 = call i1 @isAlpha(i8 %158)
  br i1 %159, label %cond.true127, label %cond.false128

if_then125:                                       ; preds = %cond.end129
  %160 = load %TokenType** %_result.addr
  %161 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @parseIdentifer(%TokenType* %160, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %161)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end126:                                        ; preds = %dumy_block131, %cond.end129
  br label %if_block132

cond.true127:                                     ; preds = %if_block124
  br label %cond.end129

cond.false128:                                    ; preds = %if_block124
  %162 = load i8* %ch
  %163 = icmp eq i8 %162, 95
  br label %cond.end129

cond.end129:                                      ; preds = %cond.false128, %cond.true127
  %cond.res130 = phi i1 [ true, %cond.true127 ], [ %163, %cond.false128 ]
  br i1 %cond.res130, label %if_then125, label %if_end126

dumy_block131:                                    ; No predecessors!
  br label %if_end126

if_block132:                                      ; preds = %if_end126
  %164 = load i8* %ch
  %165 = call i1 @isDigit(i8 %164)
  br i1 %165, label %cond.true135, label %cond.false136

if_then133:                                       ; preds = %cond.end137
  %166 = load %TokenType** %_result.addr
  %167 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @parseNumeric(%TokenType* %166, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %167)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end134:                                        ; preds = %dumy_block139, %cond.end137
  %168 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  store [27 x i8] c"Invalid character found: '\00", [27 x i8]* %const.bytes
  %169 = getelementptr inbounds [27 x i8]* %const.bytes, i32 0, i32 0
  %170 = getelementptr inbounds [27 x i8]* %const.bytes, i32 0, i32 26
  %171 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %172 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %169, i8** %171
  store i8* %170, i8** %172
  %173 = load %StringRef* %const.struct
  %174 = load i8* %ch
  store [2 x i8] c"'\00", [2 x i8]* %const.bytes140
  %175 = getelementptr inbounds [2 x i8]* %const.bytes140, i32 0, i32 0
  %176 = getelementptr inbounds [2 x i8]* %const.bytes140, i32 0, i32 1
  %177 = getelementptr inbounds %StringRef* %const.struct141, i32 0, i32 0
  %178 = getelementptr inbounds %StringRef* %const.struct141, i32 0, i32 1
  store i8* %175, i8** %177
  store i8* %176, i8** %178
  %179 = load %StringRef* %const.struct141
  %180 = load i8* %ch
  call void @_ass_32_8z(i32* %tmp.v142, i8 %180)
  %181 = load i32* %tmp.v142
  call void @toString276(%String* %"$tmpC", %StringRef %173, i8 %174, %StringRef %179, i32 %181)
  call void @reportError250(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %168, %String* %"$tmpC")
  call void @dtor186(%String* %"$tmpC")
  call void @dtor58(i32* %tmp.v142)
  %182 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %183 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %182, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %183)
  %184 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %184, i32 0)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

cond.true135:                                     ; preds = %if_block132
  br label %cond.end137

cond.false136:                                    ; preds = %if_block132
  %185 = load i8* %ch
  %186 = icmp eq i8 %185, 46
  br label %cond.end137

cond.end137:                                      ; preds = %cond.false136, %cond.true135
  %cond.res138 = phi i1 [ true, %cond.true135 ], [ %186, %cond.false136 ]
  br i1 %cond.res138, label %if_then133, label %if_end134

dumy_block139:                                    ; No predecessors!
  br label %if_end134

dumy_block143:                                    ; No predecessors!
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  br label %while_step
}

; Function Attrs: inlinehint nounwind
define private void @stepOver(%Location* %l) #3 {
  %l.addr = alloca %Location*
  store %Location* %l, %Location** %l.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %Location** %l.addr
  %2 = getelementptr inbounds %Location* %1, i32 0, i32 1
  %3 = load %Location** %l.addr
  %4 = getelementptr inbounds %Location* %3, i32 0, i32 2
  call void @"=214"(%LineCol* %2, %LineCol* %4)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @clear(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %tmp.v = alloca %"ContiguousMemoryRange[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  %2 = load %String** %"$this.addr"
  %3 = getelementptr inbounds %String* %2, i32 0, i32 0
  %4 = load %"RawPtr[Char/rtct]"* %3
  %5 = load %String** %"$this.addr"
  %6 = getelementptr inbounds %String* %5, i32 0, i32 1
  %7 = load %"RawPtr[Char/rtct]"* %6
  call void @ctor181(%"ContiguousMemoryRange[Char/rtct]"* %tmp.v, %"RawPtr[Char/rtct]" %4, %"RawPtr[Char/rtct]" %7)
  %8 = load %"ContiguousMemoryRange[Char/rtct]"* %tmp.v
  call void @remove(%String* %1, %"ContiguousMemoryRange[Char/rtct]" %8)
  call void @dtor182(%"ContiguousMemoryRange[Char/rtct]"* %tmp.v)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @remove(%String* %"$this", %"ContiguousMemoryRange[Char/rtct]" %range) #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %range.addr = alloca %"ContiguousMemoryRange[Char/rtct]"
  store %"ContiguousMemoryRange[Char/rtct]" %range, %"ContiguousMemoryRange[Char/rtct]"* %range.addr
  %rSize = alloca i64
  %rBegin = alloca %"RawPtr[Char/rtct]"
  %rEnd = alloca %"RawPtr[Char/rtct]"
  %tmp.v = alloca i64
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  %"$tmpC1" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = call i64 @size170(%"ContiguousMemoryRange[Char/rtct]"* %range.addr)
  store i64 %1, i64* %rSize
  call void @frontPtr(%"RawPtr[Char/rtct]"* %rBegin, %"ContiguousMemoryRange[Char/rtct]"* %range.addr)
  %2 = load i64* %rSize
  store i64 %2, i64* %tmp.v
  %3 = load i64* %tmp.v
  call void @advance175(%"RawPtr[Char/rtct]"* %rEnd, %"RawPtr[Char/rtct]"* %rBegin, i64 %3)
  call void @dtor35(i64* %tmp.v)
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %4 = load %String** %"$this.addr"
  %5 = getelementptr inbounds %String* %4, i32 0, i32 1
  %6 = load %"RawPtr[Char/rtct]"* %5
  %7 = call i1 @"==148"(%"RawPtr[Char/rtct]"* %rEnd, %"RawPtr[Char/rtct]" %6)
  %8 = xor i1 true, %7
  br i1 %8, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  %9 = call i8* @value(%"RawPtr[Char/rtct]"* %rBegin)
  call void @dtor162(i8* %9)
  %10 = call i8* @value(%"RawPtr[Char/rtct]"* %rEnd)
  %11 = load i8* %10
  %12 = call i8* @value(%"RawPtr[Char/rtct]"* %rBegin)
  store i8 %11, i8* %12
  call void @advance149(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %rBegin)
  %13 = load %"RawPtr[Char/rtct]"* %"$tmpC"
  call void @"=144"(%"RawPtr[Char/rtct]"* %rBegin, %"RawPtr[Char/rtct]" %13)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  call void @advance149(%"RawPtr[Char/rtct]"* %"$tmpC1", %"RawPtr[Char/rtct]"* %rEnd)
  %14 = load %"RawPtr[Char/rtct]"* %"$tmpC1"
  call void @"=144"(%"RawPtr[Char/rtct]"* %rEnd, %"RawPtr[Char/rtct]" %14)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC1")
  br label %while_step

while_step:                                       ; preds = %while_body
  br label %while_block

while_end:                                        ; preds = %while_block
  %15 = load %String** %"$this.addr"
  %16 = getelementptr inbounds %String* %15, i32 0, i32 1
  %17 = load %"RawPtr[Char/rtct]"* %rBegin
  call void @"=144"(%"RawPtr[Char/rtct]"* %16, %"RawPtr[Char/rtct]" %17)
  call void @dtor146(%"RawPtr[Char/rtct]"* %rEnd)
  call void @dtor146(%"RawPtr[Char/rtct]"* %rBegin)
  call void @dtor31(i64* %rSize)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i1 @isEmpty221(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %s) #2 {
  %s.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %s, %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %s.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %s.addr
  %2 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %1, i32 0, i32 0
  %3 = call i1 @isEmpty222(%"RangeWithLookahead[FileRange/rtct]"* %2)
  ret i1 %3
}

; Function Attrs: alwaysinline nounwind
define private i1 @isEmpty222(%"RangeWithLookahead[FileRange/rtct]"* %s) #2 {
  %s.addr = alloca %"RangeWithLookahead[FileRange/rtct]"*
  store %"RangeWithLookahead[FileRange/rtct]"* %s, %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %2 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %1, i32 0, i32 1
  %3 = call i1 @isEmpty223(%"Vector[Char/rtct]"* %2)
  ret i1 %3
}

; Function Attrs: alwaysinline nounwind
define private i1 @isEmpty223(%"Vector[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"Vector[Char/rtct]"* %1, i32 0, i32 0
  %3 = load %"Vector[Char/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"Vector[Char/rtct]"* %3, i32 0, i32 1
  %5 = load %"RawPtr[Char/rtct]"* %4
  %6 = call i1 @"==148"(%"RawPtr[Char/rtct]"* %2, %"RawPtr[Char/rtct]" %5)
  ret i1 %6
}

; Function Attrs: alwaysinline nounwind
define private void @ctor224(%TokenType* %"$this", i32 %f_data) #2 {
  %"$this.addr" = alloca %TokenType*
  store %TokenType* %"$this", %TokenType** %"$this.addr"
  %f_data.addr = alloca i32
  store i32 %f_data, i32* %f_data.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i32* %f_data.addr
  %2 = load %TokenType** %"$this.addr"
  %3 = getelementptr inbounds %TokenType* %2, i32 0, i32 0
  store i32 %1, i32* %3
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %r) #2 {
  %r.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %r, %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  %2 = call i8 @front225(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %1)
  ret i8 %2
}

; Function Attrs: alwaysinline nounwind
define private i8 @front225(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %s) #2 {
  %s.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %s, %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %s.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %s.addr
  %2 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %1, i32 0, i32 0
  %3 = call i8 @front226(%"RangeWithLookahead[FileRange/rtct]"* %2)
  ret i8 %3
}

; Function Attrs: alwaysinline nounwind
define private i8 @front226(%"RangeWithLookahead[FileRange/rtct]"* %s) #2 {
  %s.addr = alloca %"RangeWithLookahead[FileRange/rtct]"*
  store %"RangeWithLookahead[FileRange/rtct]"* %s, %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %2 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %1, i32 0, i32 1
  %3 = call i8* @front227(%"Vector[Char/rtct]"* %2)
  %4 = load i8* %3
  ret i8 %4
}

; Function Attrs: alwaysinline nounwind
define private i8* @front227(%"Vector[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"Vector[Char/rtct]"* %1, i32 0, i32 0
  %3 = call i8* @value(%"RawPtr[Char/rtct]"* %2)
  ret i8* %3
}

; Function Attrs: inlinehint nounwind
define private i8 @peekChar(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s) #3 {
  %s.addr = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  %3 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %2, i32 0, i32 0
  %4 = call i8 @peek(%"RangeWithLookahead[FileRange/rtct]"* %3, i32 1)
  ret i8 %4
}

; Function Attrs: inlinehint nounwind
define private i8 @peek(%"RangeWithLookahead[FileRange/rtct]"* %s, i32 %n) #3 {
  %s.addr = alloca %"RangeWithLookahead[FileRange/rtct]"*
  store %"RangeWithLookahead[FileRange/rtct]"* %s, %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %n.addr = alloca i32
  store i32 %n, i32* %n.addr
  %"$tmpForRef" = alloca i8
  %tmp.v = alloca i64
  %tmp.v1 = alloca i8
  br label %code

code:                                             ; preds = %0
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %1 = load i32* %n.addr
  %2 = load %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %3 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %2, i32 0, i32 1
  %4 = call i64 @size142(%"Vector[Char/rtct]"* %3)
  %5 = call i1 @">="(i32 %1, i64 %4)
  br i1 %5, label %cond.true, label %cond.false

while_body:                                       ; preds = %cond.end
  %6 = load %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %7 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %6, i32 0, i32 1
  %8 = load %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %9 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %8, i32 0, i32 0
  %10 = call i8 @"post_++159"(%FileRange* %9)
  store i8 %10, i8* %"$tmpForRef"
  call void @"+="(%"Vector[Char/rtct]"* %7, i8* %"$tmpForRef")
  br label %while_step

while_step:                                       ; preds = %while_body
  br label %while_block

while_end:                                        ; preds = %cond.end
  br label %if_block

cond.true:                                        ; preds = %while_block
  %11 = load %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %12 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %11, i32 0, i32 0
  %13 = call i1 @"pre_!!"(%FileRange* %12)
  br label %cond.end

cond.false:                                       ; preds = %while_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ %13, %cond.true ], [ false, %cond.false ]
  br i1 %cond.res, label %while_body, label %while_end

if_block:                                         ; preds = %while_end
  %14 = load i32* %n.addr
  %15 = load %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %16 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %15, i32 0, i32 1
  %17 = call i64 @size142(%"Vector[Char/rtct]"* %16)
  %18 = call i1 @"<230"(i32 %14, i64 %17)
  br i1 %18, label %if_then, label %if_else

if_then:                                          ; preds = %if_block
  %19 = load %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %20 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %19, i32 0, i32 1
  %21 = load i32* %n.addr
  %22 = zext i32 %21 to i64
  store i64 %22, i64* %tmp.v
  %23 = load i64* %tmp.v
  %24 = call i8* @"()231"(%"Vector[Char/rtct]"* %20, i64 %23)
  %25 = load i8* %24
  call void @dtor31(i64* %tmp.v)
  ret i8 %25

if_else:                                          ; preds = %if_block
  store i8 0, i8* %tmp.v1
  %26 = load i8* %tmp.v1
  call void @dtor162(i8* %tmp.v1)
  ret i8 %26

if_end:                                           ; preds = %dumy_block2, %dumy_block
  unreachable

dumy_block:                                       ; No predecessors!
  call void @dtor31(i64* %tmp.v)
  br label %if_end

dumy_block2:                                      ; No predecessors!
  call void @dtor162(i8* %tmp.v1)
  br label %if_end
}

; Function Attrs: alwaysinline nounwind
define private i1 @">="(i32 %x, i64 %y) #2 {
  %x.addr = alloca i32
  store i32 %x, i32* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i32* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn228(i32 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i32* %x.addr
  %6 = call i64 @cmn229(i64 %4, i32 %5)
  %7 = icmp sge i64 %3, %6
  ret i1 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @cmn228(i32 %x, i64 %y) #2 {
  %x.addr = alloca i32
  store i32 %x, i32* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i32* %x.addr
  %2 = zext i32 %1 to i64
  store i64 %2, i64* %tmp.v
  %3 = load i64* %tmp.v
  call void @dtor31(i64* %tmp.v)
  ret i64 %3

dumy_block:                                       ; No predecessors!
  call void @dtor31(i64* %tmp.v)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i64 @cmn229(i64 %x, i32 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  ret i64 %1
}

; Function Attrs: alwaysinline nounwind
define private i1 @"<230"(i32 %x, i64 %y) #2 {
  %x.addr = alloca i32
  store i32 %x, i32* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i32* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn228(i32 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i32* %x.addr
  %6 = call i64 @cmn229(i64 %4, i32 %5)
  %7 = icmp slt i64 %3, %6
  ret i1 %7
}

; Function Attrs: alwaysinline nounwind
define private i8* @"()231"(%"Vector[Char/rtct]"* %"$this", i64 %index) #2 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %index.addr = alloca i64
  store i64 %index, i64* %index.addr
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"Vector[Char/rtct]"* %1, i32 0, i32 0
  %3 = load i64* %index.addr
  call void @advance(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %2, i64 %3)
  %4 = call i8* @value(%"RawPtr[Char/rtct]"* %"$tmpC")
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  ret i8* %4

dumy_block:                                       ; No predecessors!
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i1 @isSpace(i8 %c) #2 {
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  %tmp.v = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  call void @_ass_32_8z(i32* %tmp.v, i8 %1)
  %2 = load i32* %tmp.v
  %3 = call i32 @isspace(i32 %2)
  %4 = call i1 @"!=163"(i32 0, i32 %3)
  call void @dtor58(i32* %tmp.v)
  ret i1 %4

dumy_block:                                       ; No predecessors!
  call void @dtor58(i32* %tmp.v)
  unreachable
}

declare i32 @isspace(i32)

; Function Attrs: inlinehint nounwind
define private void @advanceIf(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %range, %"FunctionPtr[Bool/rtct, Char/rtct]" %pred) #3 {
  %range.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %range, %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %range.addr
  %pred.addr = alloca %"FunctionPtr[Bool/rtct, Char/rtct]"
  store %"FunctionPtr[Bool/rtct, Char/rtct]" %pred, %"FunctionPtr[Bool/rtct, Char/rtct]"* %pred.addr
  br label %code

code:                                             ; preds = %0
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %range.addr
  %2 = call i1 @isEmpty221(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %1)
  %3 = xor i1 true, %2
  br i1 %3, label %cond.true, label %cond.false

while_body:                                       ; preds = %cond.end
  %4 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %range.addr
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %4)
  br label %while_step

while_step:                                       ; preds = %while_body
  br label %while_block

while_end:                                        ; preds = %cond.end
  ret void

cond.true:                                        ; preds = %while_block
  %5 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %range.addr
  %6 = call i8 @front225(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %5)
  %7 = call i1 @"()232"(%"FunctionPtr[Bool/rtct, Char/rtct]"* %pred.addr, i8 %6)
  br label %cond.end

cond.false:                                       ; preds = %while_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ %7, %cond.true ], [ false, %cond.false ]
  br i1 %cond.res, label %while_body, label %while_end
}

; Function Attrs: alwaysinline nounwind
define private i1 @"()232"(%"FunctionPtr[Bool/rtct, Char/rtct]"* %"$this", i8 %p1) #2 {
  %"$this.addr" = alloca %"FunctionPtr[Bool/rtct, Char/rtct]"*
  store %"FunctionPtr[Bool/rtct, Char/rtct]"* %"$this", %"FunctionPtr[Bool/rtct, Char/rtct]"** %"$this.addr"
  %p1.addr = alloca i8
  store i8 %p1, i8* %p1.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"FunctionPtr[Bool/rtct, Char/rtct]"** %"$this.addr"
  %2 = load i8* %p1.addr
  %3 = bitcast %"FunctionPtr[Bool/rtct, Char/rtct]"* %1 to i1 (i8)**
  %4 = load i1 (i8)** %3
  %5 = call i1 %4(i8 %2)
  ret i1 %5
}

; Function Attrs: inlinehint nounwind
define private void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %s) #3 {
  %s.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %s, %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %s.addr
  br label %code

code:                                             ; preds = %0
  br label %if_block

if_block:                                         ; preds = %code
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %s.addr
  %2 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %1, i32 0, i32 0
  %3 = call i8 @"pre_*234"(%"RangeWithLookahead[FileRange/rtct]"* %2)
  %4 = icmp eq i8 %3, 10
  br i1 %4, label %if_then, label %if_else

if_then:                                          ; preds = %if_block
  %5 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %s.addr
  %6 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %5, i32 0, i32 1
  %7 = load %Location** %6
  call void @addLines(%Location* %7, i32 1)
  br label %if_end

if_else:                                          ; preds = %if_block
  %8 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %s.addr
  %9 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %8, i32 0, i32 1
  %10 = load %Location** %9
  call void @addColumns(%Location* %10, i32 1)
  br label %if_end

if_end:                                           ; preds = %if_else, %if_then
  %11 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %s.addr
  %12 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %11, i32 0, i32 0
  call void @popFront235(%"RangeWithLookahead[FileRange/rtct]"* %12)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i8 @"pre_*234"(%"RangeWithLookahead[FileRange/rtct]"* %r) #2 {
  %r.addr = alloca %"RangeWithLookahead[FileRange/rtct]"*
  store %"RangeWithLookahead[FileRange/rtct]"* %r, %"RangeWithLookahead[FileRange/rtct]"** %r.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"RangeWithLookahead[FileRange/rtct]"** %r.addr
  %2 = call i8 @front226(%"RangeWithLookahead[FileRange/rtct]"* %1)
  ret i8 %2
}

; Function Attrs: inlinehint nounwind
define private void @addLines(%Location* %l, i32 %count) #3 {
  %l.addr = alloca %Location*
  store %Location* %l, %Location** %l.addr
  %count.addr = alloca i32
  store i32 %count, i32* %count.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %Location** %l.addr
  %2 = getelementptr inbounds %Location* %1, i32 0, i32 2
  %3 = getelementptr inbounds %LineCol* %2, i32 0, i32 0
  %4 = load i32* %3
  %5 = load i32* %count.addr
  %6 = call i32 @"+17"(i32 %4, i32 %5)
  %7 = load %Location** %l.addr
  %8 = getelementptr inbounds %Location* %7, i32 0, i32 2
  %9 = getelementptr inbounds %LineCol* %8, i32 0, i32 0
  store i32 %6, i32* %9
  %10 = load %Location** %l.addr
  %11 = getelementptr inbounds %Location* %10, i32 0, i32 2
  %12 = getelementptr inbounds %LineCol* %11, i32 0, i32 1
  store i32 1, i32* %12
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @addColumns(%Location* %l, i32 %count) #3 {
  %l.addr = alloca %Location*
  store %Location* %l, %Location** %l.addr
  %count.addr = alloca i32
  store i32 %count, i32* %count.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %Location** %l.addr
  %2 = getelementptr inbounds %Location* %1, i32 0, i32 2
  %3 = getelementptr inbounds %LineCol* %2, i32 0, i32 1
  %4 = load i32* %3
  %5 = load i32* %count.addr
  %6 = call i32 @"+17"(i32 %4, i32 %5)
  %7 = load %Location** %l.addr
  %8 = getelementptr inbounds %Location* %7, i32 0, i32 2
  %9 = getelementptr inbounds %LineCol* %8, i32 0, i32 1
  store i32 %6, i32* %9
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @popFront235(%"RangeWithLookahead[FileRange/rtct]"* %s) #3 {
  %s.addr = alloca %"RangeWithLookahead[FileRange/rtct]"*
  store %"RangeWithLookahead[FileRange/rtct]"* %s, %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %"$tmpForRef" = alloca i8
  br label %code

code:                                             ; preds = %0
  %1 = load %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %2 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %1, i32 0, i32 1
  call void @remove236(%"Vector[Char/rtct]"* %2, i64 0)
  br label %if_block

if_block:                                         ; preds = %code
  %3 = load %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %4 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %3, i32 0, i32 1
  %5 = call i1 @isEmpty223(%"Vector[Char/rtct]"* %4)
  br i1 %5, label %cond.true, label %cond.false

if_then:                                          ; preds = %cond.end
  %6 = load %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %7 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %6, i32 0, i32 1
  %8 = load %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %9 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %8, i32 0, i32 0
  %10 = call i8 @"post_++159"(%FileRange* %9)
  store i8 %10, i8* %"$tmpForRef"
  call void @"+="(%"Vector[Char/rtct]"* %7, i8* %"$tmpForRef")
  br label %if_end

if_end:                                           ; preds = %if_then, %cond.end
  ret void

cond.true:                                        ; preds = %if_block
  %11 = load %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %12 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %11, i32 0, i32 0
  %13 = call i1 @"pre_!!"(%FileRange* %12)
  br label %cond.end

cond.false:                                       ; preds = %if_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ %13, %cond.true ], [ false, %cond.false ]
  br i1 %cond.res, label %if_then, label %if_end
}

; Function Attrs: inlinehint nounwind
define private void @remove236(%"Vector[Char/rtct]"* %"$this", i64 %index) #3 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %index.addr = alloca i64
  store i64 %index, i64* %index.addr
  %r = alloca %"ContiguousMemoryRange[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  call void @all237(%"ContiguousMemoryRange[Char/rtct]"* %r, %"Vector[Char/rtct]"* %1)
  %2 = load i64* %index.addr
  call void @popFront238(%"ContiguousMemoryRange[Char/rtct]"* %r, i64 %2)
  %3 = load %"Vector[Char/rtct]"** %"$this.addr"
  %4 = call i64 @size142(%"Vector[Char/rtct]"* %3)
  %5 = load i64* %index.addr
  %6 = call i64 @-51(i64 %4, i64 %5)
  %7 = call i64 @-240(i64 %6, i32 1)
  call void @popBack239(%"ContiguousMemoryRange[Char/rtct]"* %r, i64 %7)
  %8 = load %"Vector[Char/rtct]"** %"$this.addr"
  %9 = load %"ContiguousMemoryRange[Char/rtct]"* %r
  call void @remove241(%"Vector[Char/rtct]"* %8, %"ContiguousMemoryRange[Char/rtct]" %9)
  call void @dtor182(%"ContiguousMemoryRange[Char/rtct]"* %r)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @all237(%"ContiguousMemoryRange[Char/rtct]"* sret %_result, %"Vector[Char/rtct]"* %"$this") #2 {
  %_result.addr = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %_result, %"ContiguousMemoryRange[Char/rtct]"** %_result.addr
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"ContiguousMemoryRange[Char/rtct]"** %_result.addr
  %2 = load %"Vector[Char/rtct]"** %"$this.addr"
  %3 = getelementptr inbounds %"Vector[Char/rtct]"* %2, i32 0, i32 0
  %4 = load %"RawPtr[Char/rtct]"* %3
  %5 = load %"Vector[Char/rtct]"** %"$this.addr"
  %6 = getelementptr inbounds %"Vector[Char/rtct]"* %5, i32 0, i32 1
  %7 = load %"RawPtr[Char/rtct]"* %6
  call void @ctor181(%"ContiguousMemoryRange[Char/rtct]"* %1, %"RawPtr[Char/rtct]" %4, %"RawPtr[Char/rtct]" %7)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @popFront238(%"ContiguousMemoryRange[Char/rtct]"* %"$this", i64 %n) #2 {
  %"$this.addr" = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %"$this", %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %n.addr = alloca i64
  store i64 %n, i64* %n.addr
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %1, i32 0, i32 0
  %3 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %3, i32 0, i32 0
  %5 = load i64* %n.addr
  store i64 %5, i64* %tmp.v
  %6 = load i64* %tmp.v
  call void @advance175(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %4, i64 %6)
  %7 = load %"RawPtr[Char/rtct]"* %"$tmpC"
  call void @"=144"(%"RawPtr[Char/rtct]"* %2, %"RawPtr[Char/rtct]" %7)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  call void @dtor35(i64* %tmp.v)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @popBack239(%"ContiguousMemoryRange[Char/rtct]"* %"$this", i64 %n) #2 {
  %"$this.addr" = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %"$this", %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %n.addr = alloca i64
  store i64 %n, i64* %n.addr
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %1, i32 0, i32 1
  %3 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %3, i32 0, i32 1
  %5 = load i64* %n.addr
  store i64 %5, i64* %tmp.v
  %6 = load i64* %tmp.v
  %7 = sub i64 0, %6
  call void @advance175(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %4, i64 %7)
  %8 = load %"RawPtr[Char/rtct]"* %"$tmpC"
  call void @"=144"(%"RawPtr[Char/rtct]"* %2, %"RawPtr[Char/rtct]" %8)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  call void @dtor35(i64* %tmp.v)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i64 @-240(i64 %x, i32 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i32* %y.addr
  %3 = call i64 @cmn75(i64 %1, i32 %2)
  %4 = load i32* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn76(i32 %4, i64 %5)
  %7 = call i64 @_SizeType_opMinus(i64 %3, i64 %6)
  ret i64 %7
}

; Function Attrs: inlinehint nounwind
define private void @remove241(%"Vector[Char/rtct]"* %"$this", %"ContiguousMemoryRange[Char/rtct]" %range) #3 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %range.addr = alloca %"ContiguousMemoryRange[Char/rtct]"
  store %"ContiguousMemoryRange[Char/rtct]" %range, %"ContiguousMemoryRange[Char/rtct]"* %range.addr
  %rBegin = alloca %"RawPtr[Char/rtct]"
  %rEnd = alloca %"RawPtr[Char/rtct]"
  %"$rangeVar" = alloca %"ContiguousMemoryRange[Char/rtct]"
  %el = alloca i8*
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  %"$tmpC5" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  call void @frontPtr(%"RawPtr[Char/rtct]"* %rBegin, %"ContiguousMemoryRange[Char/rtct]"* %range.addr)
  call void @backPtr(%"RawPtr[Char/rtct]"* %rEnd, %"ContiguousMemoryRange[Char/rtct]"* %range.addr)
  call void @ctor242(%"ContiguousMemoryRange[Char/rtct]"* %"$rangeVar", %"ContiguousMemoryRange[Char/rtct]"* %range.addr)
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %1 = call i1 @isEmpty177(%"ContiguousMemoryRange[Char/rtct]"* %"$rangeVar")
  %2 = xor i1 true, %1
  br i1 %2, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  %3 = call i8* @front179(%"ContiguousMemoryRange[Char/rtct]"* %"$rangeVar")
  store i8* %3, i8** %el
  %4 = load i8** %el
  call void @dtor162(i8* %4)
  br label %while_step

while_step:                                       ; preds = %while_body
  call void @popFront180(%"ContiguousMemoryRange[Char/rtct]"* %"$rangeVar")
  br label %while_block

while_end:                                        ; preds = %while_block
  call void @dtor182(%"ContiguousMemoryRange[Char/rtct]"* %"$rangeVar")
  br label %while_block1

while_block1:                                     ; preds = %while_step3, %while_end
  %5 = load %"Vector[Char/rtct]"** %"$this.addr"
  %6 = getelementptr inbounds %"Vector[Char/rtct]"* %5, i32 0, i32 1
  %7 = load %"RawPtr[Char/rtct]"* %6
  %8 = call i1 @"==148"(%"RawPtr[Char/rtct]"* %rEnd, %"RawPtr[Char/rtct]" %7)
  %9 = xor i1 true, %8
  br i1 %9, label %while_body2, label %while_end4

while_body2:                                      ; preds = %while_block1
  %10 = call i8* @value(%"RawPtr[Char/rtct]"* %rEnd)
  %11 = load i8* %10
  %12 = call i8* @value(%"RawPtr[Char/rtct]"* %rBegin)
  store i8 %11, i8* %12
  %13 = call i8* @value(%"RawPtr[Char/rtct]"* %rEnd)
  call void @dtor162(i8* %13)
  call void @advance149(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %rBegin)
  %14 = load %"RawPtr[Char/rtct]"* %"$tmpC"
  call void @"=144"(%"RawPtr[Char/rtct]"* %rBegin, %"RawPtr[Char/rtct]" %14)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  call void @advance149(%"RawPtr[Char/rtct]"* %"$tmpC5", %"RawPtr[Char/rtct]"* %rEnd)
  %15 = load %"RawPtr[Char/rtct]"* %"$tmpC5"
  call void @"=144"(%"RawPtr[Char/rtct]"* %rEnd, %"RawPtr[Char/rtct]" %15)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC5")
  br label %while_step3

while_step3:                                      ; preds = %while_body2
  br label %while_block1

while_end4:                                       ; preds = %while_block1
  %16 = load %"Vector[Char/rtct]"** %"$this.addr"
  %17 = getelementptr inbounds %"Vector[Char/rtct]"* %16, i32 0, i32 1
  %18 = load %"RawPtr[Char/rtct]"* %rBegin
  call void @"=144"(%"RawPtr[Char/rtct]"* %17, %"RawPtr[Char/rtct]" %18)
  call void @dtor146(%"RawPtr[Char/rtct]"* %rEnd)
  call void @dtor146(%"RawPtr[Char/rtct]"* %rBegin)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @backPtr(%"RawPtr[Char/rtct]"* sret %_result, %"ContiguousMemoryRange[Char/rtct]"* %"$this") #2 {
  %_result.addr = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %_result, %"RawPtr[Char/rtct]"** %_result.addr
  %"$this.addr" = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %"$this", %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"RawPtr[Char/rtct]"** %_result.addr
  %2 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %3 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %2, i32 0, i32 1
  call void @ctor145(%"RawPtr[Char/rtct]"* %1, %"RawPtr[Char/rtct]"* %3)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor242(%"ContiguousMemoryRange[Char/rtct]"* %"$this", %"ContiguousMemoryRange[Char/rtct]"* %other) #2 {
  %"$this.addr" = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %"$this", %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %other.addr = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %other, %"ContiguousMemoryRange[Char/rtct]"** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %1, i32 0, i32 0
  %3 = load %"ContiguousMemoryRange[Char/rtct]"** %other.addr
  %4 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %3, i32 0, i32 0
  call void @ctor145(%"RawPtr[Char/rtct]"* %2, %"RawPtr[Char/rtct]"* %4)
  %5 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %6 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %5, i32 0, i32 1
  %7 = load %"ContiguousMemoryRange[Char/rtct]"** %other.addr
  %8 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %7, i32 0, i32 1
  call void @ctor145(%"RawPtr[Char/rtct]"* %6, %"RawPtr[Char/rtct]"* %8)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @advanceIf243(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %range, %"$lambdaEnclosure" %pred) #3 {
  %range.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %range, %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %range.addr
  %pred.addr = alloca %"$lambdaEnclosure"
  store %"$lambdaEnclosure" %pred, %"$lambdaEnclosure"* %pred.addr
  br label %code

code:                                             ; preds = %0
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %range.addr
  %2 = call i1 @isEmpty221(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %1)
  %3 = xor i1 true, %2
  br i1 %3, label %cond.true, label %cond.false

while_body:                                       ; preds = %cond.end
  %4 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %range.addr
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %4)
  br label %while_step

while_step:                                       ; preds = %while_body
  br label %while_block

while_end:                                        ; preds = %cond.end
  ret void

cond.true:                                        ; preds = %while_block
  %5 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %range.addr
  %6 = call i8 @front225(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %5)
  %7 = call i1 @"()244"(%"$lambdaEnclosure"* %pred.addr, i8 %6)
  br label %cond.end

cond.false:                                       ; preds = %while_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ %7, %cond.true ], [ false, %cond.false ]
  br i1 %cond.res, label %while_body, label %while_end
}

; Function Attrs: alwaysinline nounwind
define private i1 @"()244"(%"$lambdaEnclosure"* %"$this", i8 %c) #2 {
  %"$this.addr" = alloca %"$lambdaEnclosure"*
  store %"$lambdaEnclosure"* %"$this", %"$lambdaEnclosure"** %"$this.addr"
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  %2 = icmp ne i8 %1, 10
  ret i1 %2
}

; Function Attrs: alwaysinline nounwind
define private void @ctor245(%"$lambdaEnclosure"* %"$this") #2 {
  %"$this.addr" = alloca %"$lambdaEnclosure"*
  store %"$lambdaEnclosure"* %"$this", %"$lambdaEnclosure"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor246(%"$lambdaEnclosure"* %"$this") #2 {
  %"$this.addr" = alloca %"$lambdaEnclosure"*
  store %"$lambdaEnclosure"* %"$this", %"$lambdaEnclosure"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i1 @"pre_!!247"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %r) #2 {
  %r.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %r, %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  %2 = call i1 @isEmpty221(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %1)
  %3 = xor i1 true, %2
  ret i1 %3
}

; Function Attrs: inlinehint nounwind
define private void @advance248(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %range, i64 %n) #3 {
  %range.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %range, %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %range.addr
  %n.addr = alloca i64
  store i64 %n, i64* %n.addr
  br label %code

code:                                             ; preds = %0
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %1 = load i64* %n.addr
  %2 = call i1 @">"(i64 %1, i32 0)
  br i1 %2, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  %3 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %range.addr
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %3)
  br label %while_step

while_step:                                       ; preds = %while_body
  %4 = load i64* %n.addr
  %5 = call i64 @-240(i64 %4, i32 1)
  store i64 %5, i64* %n.addr
  br label %while_block

while_end:                                        ; preds = %while_block
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i1 @">"(i64 %x, i32 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i32* %y.addr
  %3 = call i64 @cmn75(i64 %1, i32 %2)
  %4 = load i32* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn76(i32 %4, i64 %5)
  %7 = icmp sgt i64 %3, %6
  ret i1 %7
}

; Function Attrs: inlinehint nounwind
define private i8 @"pre_++249"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %r) #3 {
  %r.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %r, %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %1)
  %2 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  %3 = call i8 @front225(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %2)
  ret i8 %3
}

; Function Attrs: alwaysinline nounwind
define private i1 @"pre_!"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %r) #2 {
  %r.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %r, %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  %2 = call i1 @isEmpty221(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %1)
  ret i1 %2
}

; Function Attrs: inlinehint nounwind
define private void @reportError250(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, %String* %msg) #3 {
  %s.addr = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %msg.addr = alloca %String*
  store %String* %msg, %String** %msg.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1, i32 0, i32 5
  %3 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %4 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %3, i32 0, i32 1
  %5 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %4, i32 0, i32 1
  %6 = load %Location** %5
  %7 = load %Location* %6
  %8 = load %String** %msg.addr
  call void @reportError(%ExternalErrorReporter* %2, %Location %7, %String* %8)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctorFromCt(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [33 x i8]
  %const.struct = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  store [33 x i8] c"End of file found inside comment\00", [33 x i8]* %const.bytes
  %2 = getelementptr inbounds [33 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [33 x i8]* %const.bytes, i32 0, i32 32
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  call void @ctor251(%String* %1, %StringRef* %"$tmpForRef")
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctor251(%String* %"$this", %StringRef* %other) #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %other.addr = alloca %StringRef*
  store %StringRef* %other, %StringRef** %other.addr
  %size = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef** %other.addr
  %2 = call i64 @size(%StringRef* %1)
  store i64 %2, i64* %size
  %3 = load %String** %"$this.addr"
  %4 = load i64* %size
  call void @ctor184(%String* %3, i64 %4)
  %5 = load %String** %"$this.addr"
  %6 = getelementptr inbounds %String* %5, i32 0, i32 0
  %7 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %6)
  %8 = load %StringRef** %other.addr
  %9 = getelementptr inbounds %StringRef* %8, i32 0, i32 0
  %10 = load i8** %9
  %11 = load i64* %size
  call void @_spr_memcpy(i8* %7, i8* %10, i64 %11)
  call void @dtor31(i64* %size)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @parseCharacter(%TokenType* sret %_result, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s) #3 {
  %_result.addr = alloca %TokenType*
  store %TokenType* %_result, %TokenType** %_result.addr
  %s.addr = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %"$tmpForRef" = alloca %String
  %tmp.v = alloca %String
  %ch = alloca i8
  %"$tmpForRef4" = alloca %String
  %tmp.v5 = alloca %String
  %"$tmpForRef13" = alloca %String
  %tmp.v14 = alloca %String
  %"$tmpForRef19" = alloca %String
  %tmp.v20 = alloca %String
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %2)
  br label %if_block

if_block:                                         ; preds = %code
  %3 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %4 = call i1 @hasLessThan(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %3, i32 2)
  br i1 %4, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %5 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @ctorFromCt253(%String* %tmp.v)
  %6 = load %String* %tmp.v
  store %String %6, %String* %"$tmpForRef"
  call void @reportError250(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %5, %String* %"$tmpForRef")
  call void @dtor186(%String* %tmp.v)
  %7 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %7, i32 0)
  ret void

if_end:                                           ; preds = %dumy_block, %if_block
  %8 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %9 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %8, i32 0, i32 1
  %10 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %9)
  store i8 %10, i8* %ch
  br label %if_block1

dumy_block:                                       ; No predecessors!
  br label %if_end

if_block1:                                        ; preds = %if_end
  %11 = load i8* %ch
  %12 = icmp eq i8 %11, 39
  br i1 %12, label %if_then2, label %if_else

if_then2:                                         ; preds = %if_block1
  %13 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @ctorFromCt254(%String* %tmp.v5)
  %14 = load %String* %tmp.v5
  store %String %14, %String* %"$tmpForRef4"
  call void @reportError250(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %13, %String* %"$tmpForRef4")
  call void @dtor186(%String* %tmp.v5)
  %15 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %16 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %15, i32 0, i32 2
  %17 = getelementptr inbounds %Token* %16, i32 0, i32 2
  call void @"+=255"(%String* %17, i8 63)
  %18 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %18, i32 301)
  call void @dtor162(i8* %ch)
  ret void

if_else:                                          ; preds = %if_block1
  br label %if_block7

if_end3:                                          ; preds = %if_end9, %dumy_block6
  br label %if_block10

dumy_block6:                                      ; No predecessors!
  br label %if_end3

if_block7:                                        ; preds = %if_else
  %19 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %20 = call i1 @checkEscapeChar(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %19)
  %21 = xor i1 true, %20
  br i1 %21, label %if_then8, label %if_end9

if_then8:                                         ; preds = %if_block7
  %22 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @advanceAndCapture1(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %22)
  br label %if_end9

if_end9:                                          ; preds = %if_then8, %if_block7
  br label %if_end3

if_block10:                                       ; preds = %if_end3
  %23 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %24 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %23, i32 0, i32 1
  %25 = call i1 @"pre_!"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %24)
  br i1 %25, label %if_then11, label %if_end12

if_then11:                                        ; preds = %if_block10
  %26 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @ctorFromCt259(%String* %tmp.v14)
  %27 = load %String* %tmp.v14
  store %String %27, %String* %"$tmpForRef13"
  call void @reportError250(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %26, %String* %"$tmpForRef13")
  call void @dtor186(%String* %tmp.v14)
  %28 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %28, i32 0)
  call void @dtor162(i8* %ch)
  ret void

if_end12:                                         ; preds = %dumy_block15, %if_block10
  br label %if_block16

dumy_block15:                                     ; No predecessors!
  br label %if_end12

if_block16:                                       ; preds = %if_end12
  %29 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %30 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %29, i32 0, i32 1
  %31 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %30)
  %32 = icmp ne i8 %31, 39
  br i1 %32, label %if_then17, label %if_end18

if_then17:                                        ; preds = %if_block16
  %33 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @ctorFromCt260(%String* %tmp.v20)
  %34 = load %String* %tmp.v20
  store %String %34, %String* %"$tmpForRef19"
  call void @reportError250(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %33, %String* %"$tmpForRef19")
  call void @dtor186(%String* %tmp.v20)
  br label %if_end18

if_end18:                                         ; preds = %if_then17, %if_block16
  %35 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %36 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %35, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %36)
  %37 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %37, i32 301)
  call void @dtor162(i8* %ch)
  ret void

dumy_block21:                                     ; No predecessors!
  call void @dtor162(i8* %ch)
  ret void
}

; Function Attrs: inlinehint nounwind
define private i1 @hasLessThan(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, i32 %n) #3 {
  %s.addr = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %n.addr = alloca i32
  store i32 %n, i32* %n.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  %3 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %2, i32 0, i32 0
  %4 = load i32* %n.addr
  %5 = call i1 @hasLessThan252(%"RangeWithLookahead[FileRange/rtct]"* %3, i32 %4)
  ret i1 %5
}

; Function Attrs: inlinehint nounwind
define private i1 @hasLessThan252(%"RangeWithLookahead[FileRange/rtct]"* %s, i32 %n) #3 {
  %s.addr = alloca %"RangeWithLookahead[FileRange/rtct]"*
  store %"RangeWithLookahead[FileRange/rtct]"* %s, %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %n.addr = alloca i32
  store i32 %n, i32* %n.addr
  %"$tmpForRef" = alloca i8
  br label %code

code:                                             ; preds = %0
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %1 = load i32* %n.addr
  %2 = load %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %3 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %2, i32 0, i32 1
  %4 = call i64 @size142(%"Vector[Char/rtct]"* %3)
  %5 = call i1 @">="(i32 %1, i64 %4)
  br i1 %5, label %cond.true, label %cond.false

while_body:                                       ; preds = %cond.end
  %6 = load %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %7 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %6, i32 0, i32 1
  %8 = load %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %9 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %8, i32 0, i32 0
  %10 = call i8 @"post_++159"(%FileRange* %9)
  store i8 %10, i8* %"$tmpForRef"
  call void @"+="(%"Vector[Char/rtct]"* %7, i8* %"$tmpForRef")
  br label %while_step

while_step:                                       ; preds = %while_body
  br label %while_block

while_end:                                        ; preds = %cond.end
  %11 = load i32* %n.addr
  %12 = load %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %13 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %12, i32 0, i32 1
  %14 = call i64 @size142(%"Vector[Char/rtct]"* %13)
  %15 = call i1 @">="(i32 %11, i64 %14)
  ret i1 %15

cond.true:                                        ; preds = %while_block
  %16 = load %"RangeWithLookahead[FileRange/rtct]"** %s.addr
  %17 = getelementptr inbounds %"RangeWithLookahead[FileRange/rtct]"* %16, i32 0, i32 0
  %18 = call i1 @"pre_!!"(%FileRange* %17)
  br label %cond.end

cond.false:                                       ; preds = %while_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ %18, %cond.true ], [ false, %cond.false ]
  br i1 %cond.res, label %while_body, label %while_end
}

; Function Attrs: inlinehint nounwind
define private void @ctorFromCt253(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [43 x i8]
  %const.struct = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  store [43 x i8] c"End of file found inside character literal\00", [43 x i8]* %const.bytes
  %2 = getelementptr inbounds [43 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [43 x i8]* %const.bytes, i32 0, i32 42
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  call void @ctor251(%String* %1, %StringRef* %"$tmpForRef")
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctorFromCt254(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [26 x i8]
  %const.struct = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  store [26 x i8] c"Invalid character literal\00", [26 x i8]* %const.bytes
  %2 = getelementptr inbounds [26 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [26 x i8]* %const.bytes, i32 0, i32 25
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  call void @ctor251(%String* %1, %StringRef* %"$tmpForRef")
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @"+=255"(%String* %"$this", i8 %value) #2 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %value.addr = alloca i8
  store i8 %value, i8* %value.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  %2 = load i8* %value.addr
  call void @pushBack256(%String* %1, i8 %2)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @pushBack256(%String* %"$this", i8 %value) #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %value.addr = alloca i8
  store i8 %value, i8* %value.addr
  %t = alloca i64
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  br label %if_block

if_block:                                         ; preds = %code
  %1 = load %String** %"$this.addr"
  %2 = getelementptr inbounds %String* %1, i32 0, i32 1
  %3 = load %String** %"$this.addr"
  %4 = getelementptr inbounds %String* %3, i32 0, i32 2
  %5 = load %"RawPtr[Char/rtct]"* %4
  %6 = call i1 @"==148"(%"RawPtr[Char/rtct]"* %2, %"RawPtr[Char/rtct]" %5)
  br i1 %6, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %7 = load %String** %"$this.addr"
  %8 = call i64 @capacity172(%String* %7)
  %9 = call i64 @"*158"(i32 2, i64 %8)
  store i64 %9, i64* %t
  br label %if_block1

if_end:                                           ; preds = %if_end3, %if_block
  %10 = load i8* %value.addr
  %11 = load %String** %"$this.addr"
  %12 = getelementptr inbounds %String* %11, i32 0, i32 1
  %13 = call i8* @value(%"RawPtr[Char/rtct]"* %12)
  store i8 %10, i8* %13
  %14 = load %String** %"$this.addr"
  %15 = getelementptr inbounds %String* %14, i32 0, i32 1
  %16 = load %String** %"$this.addr"
  %17 = getelementptr inbounds %String* %16, i32 0, i32 1
  call void @advance149(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %17)
  %18 = load %"RawPtr[Char/rtct]"* %"$tmpC"
  call void @"=144"(%"RawPtr[Char/rtct]"* %15, %"RawPtr[Char/rtct]" %18)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  ret void

if_block1:                                        ; preds = %if_then
  %19 = load i64* %t
  %20 = call i1 @"<"(i64 %19, i32 2)
  br i1 %20, label %if_then2, label %if_end3

if_then2:                                         ; preds = %if_block1
  store i64 2, i64* %t
  br label %if_end3

if_end3:                                          ; preds = %if_then2, %if_block1
  %21 = load %String** %"$this.addr"
  %22 = load i64* %t
  call void @reserve171(%String* %21, i64 %22)
  call void @dtor31(i64* %t)
  br label %if_end
}

; Function Attrs: noinline nounwind
define i1 @checkEscapeChar(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s) #4 {
  %s.addr = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %ch = alloca i8
  %"$tmpForRef" = alloca %String
  %tmp.v = alloca %String
  %charVal = alloca i32
  %tmp.v39 = alloca i8
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  %3 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %2)
  store i8 %3, i8* %ch
  br label %if_block

if_block:                                         ; preds = %code
  %4 = load i8* %ch
  %5 = icmp eq i8 %4, 92
  br i1 %5, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %6 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %7 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %6, i32 0, i32 1
  %8 = call i8 @"pre_++249"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %7)
  store i8 %8, i8* %ch
  br label %if_block1

if_end:                                           ; preds = %dumy_block40, %if_block
  call void @dtor162(i8* %ch)
  ret i1 false

if_block1:                                        ; preds = %if_then
  %9 = load i8* %ch
  %10 = icmp eq i8 %9, 114
  br i1 %10, label %if_then2, label %if_else

if_then2:                                         ; preds = %if_block1
  %11 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %12 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %11, i32 0, i32 2
  %13 = getelementptr inbounds %Token* %12, i32 0, i32 2
  call void @"+=255"(%String* %13, i8 13)
  br label %if_end3

if_else:                                          ; preds = %if_block1
  br label %if_block4

if_end3:                                          ; preds = %if_end7, %if_then2
  %14 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %15 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %14, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %15)
  call void @dtor162(i8* %ch)
  ret i1 true

if_block4:                                        ; preds = %if_else
  %16 = load i8* %ch
  %17 = icmp eq i8 %16, 110
  br i1 %17, label %if_then5, label %if_else6

if_then5:                                         ; preds = %if_block4
  %18 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %19 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %18, i32 0, i32 2
  %20 = getelementptr inbounds %Token* %19, i32 0, i32 2
  call void @"+=255"(%String* %20, i8 10)
  br label %if_end7

if_else6:                                         ; preds = %if_block4
  br label %if_block8

if_end7:                                          ; preds = %if_end11, %if_then5
  br label %if_end3

if_block8:                                        ; preds = %if_else6
  %21 = load i8* %ch
  %22 = icmp eq i8 %21, 98
  br i1 %22, label %if_then9, label %if_else10

if_then9:                                         ; preds = %if_block8
  %23 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %24 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %23, i32 0, i32 2
  %25 = getelementptr inbounds %Token* %24, i32 0, i32 2
  call void @"+=255"(%String* %25, i8 8)
  br label %if_end11

if_else10:                                        ; preds = %if_block8
  br label %if_block12

if_end11:                                         ; preds = %if_end15, %if_then9
  br label %if_end7

if_block12:                                       ; preds = %if_else10
  %26 = load i8* %ch
  %27 = icmp eq i8 %26, 102
  br i1 %27, label %if_then13, label %if_else14

if_then13:                                        ; preds = %if_block12
  %28 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %29 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %28, i32 0, i32 2
  %30 = getelementptr inbounds %Token* %29, i32 0, i32 2
  call void @"+=255"(%String* %30, i8 12)
  br label %if_end15

if_else14:                                        ; preds = %if_block12
  br label %if_block16

if_end15:                                         ; preds = %if_end19, %if_then13
  br label %if_end11

if_block16:                                       ; preds = %if_else14
  %31 = load i8* %ch
  %32 = icmp eq i8 %31, 116
  br i1 %32, label %if_then17, label %if_else18

if_then17:                                        ; preds = %if_block16
  %33 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %34 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %33, i32 0, i32 2
  %35 = getelementptr inbounds %Token* %34, i32 0, i32 2
  call void @"+=255"(%String* %35, i8 9)
  br label %if_end19

if_else18:                                        ; preds = %if_block16
  br label %if_block20

if_end19:                                         ; preds = %if_end23, %if_then17
  br label %if_end15

if_block20:                                       ; preds = %if_else18
  %36 = load i8* %ch
  %37 = icmp eq i8 %36, 92
  br i1 %37, label %if_then21, label %if_else22

if_then21:                                        ; preds = %if_block20
  %38 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %39 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %38, i32 0, i32 2
  %40 = getelementptr inbounds %Token* %39, i32 0, i32 2
  call void @"+=255"(%String* %40, i8 92)
  br label %if_end23

if_else22:                                        ; preds = %if_block20
  br label %if_block24

if_end23:                                         ; preds = %if_end27, %if_then21
  br label %if_end19

if_block24:                                       ; preds = %if_else22
  %41 = load i8* %ch
  %42 = icmp eq i8 %41, 39
  br i1 %42, label %if_then25, label %if_else26

if_then25:                                        ; preds = %if_block24
  %43 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %44 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %43, i32 0, i32 2
  %45 = getelementptr inbounds %Token* %44, i32 0, i32 2
  call void @"+=255"(%String* %45, i8 39)
  br label %if_end27

if_else26:                                        ; preds = %if_block24
  br label %if_block28

if_end27:                                         ; preds = %if_end31, %if_then25
  br label %if_end23

if_block28:                                       ; preds = %if_else26
  %46 = load i8* %ch
  %47 = icmp eq i8 %46, 34
  br i1 %47, label %if_then29, label %if_else30

if_then29:                                        ; preds = %if_block28
  %48 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %49 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %48, i32 0, i32 2
  %50 = getelementptr inbounds %Token* %49, i32 0, i32 2
  call void @"+=255"(%String* %50, i8 34)
  br label %if_end31

if_else30:                                        ; preds = %if_block28
  br label %if_block32

if_end31:                                         ; preds = %if_end35, %if_then29
  br label %if_end27

if_block32:                                       ; preds = %if_else30
  %51 = load i8* %ch
  %52 = icmp eq i8 %51, 120
  br i1 %52, label %cond.true, label %cond.false

if_then33:                                        ; preds = %cond.end
  br label %if_block36

if_else34:                                        ; preds = %cond.end
  %53 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %54 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %53, i32 0, i32 2
  %55 = getelementptr inbounds %Token* %54, i32 0, i32 2
  %56 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %57 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %56, i32 0, i32 1
  %58 = call i8 @"pre_++249"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %57)
  call void @"+=255"(%String* %55, i8 %58)
  br label %if_end35

if_end35:                                         ; preds = %if_else34, %if_end38
  br label %if_end31

cond.true:                                        ; preds = %if_block32
  br label %cond.end

cond.false:                                       ; preds = %if_block32
  %59 = load i8* %ch
  %60 = icmp eq i8 %59, 88
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ true, %cond.true ], [ %60, %cond.false ]
  br i1 %cond.res, label %if_then33, label %if_else34

if_block36:                                       ; preds = %if_then33
  %61 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %62 = call i1 @hasLessThan(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %61, i32 2)
  br i1 %62, label %if_then37, label %if_end38

if_then37:                                        ; preds = %if_block36
  %63 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @ctorFromCt257(%String* %tmp.v)
  %64 = load %String* %tmp.v
  store %String %64, %String* %"$tmpForRef"
  call void @reportError250(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %63, %String* %"$tmpForRef")
  call void @dtor186(%String* %tmp.v)
  call void @dtor162(i8* %ch)
  ret i1 true

if_end38:                                         ; preds = %dumy_block, %if_block36
  %65 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %66 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %65, i32 0, i32 1
  %67 = call i8 @"pre_++249"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %66)
  %68 = call i32 @getXdigitVal(i8 %67)
  %69 = call i32 @"*258"(i32 16, i32 %68)
  store i32 %69, i32* %charVal
  %70 = load i32* %charVal
  %71 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %72 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %71, i32 0, i32 1
  %73 = call i8 @"pre_++249"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %72)
  %74 = call i32 @getXdigitVal(i8 %73)
  %75 = call i32 @"+14"(i32 %70, i32 %74)
  store i32 %75, i32* %charVal
  %76 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %77 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %76, i32 0, i32 2
  %78 = getelementptr inbounds %Token* %77, i32 0, i32 2
  %79 = load i32* %charVal
  %80 = trunc i32 %79 to i8
  store i8 %80, i8* %tmp.v39
  %81 = load i8* %tmp.v39
  call void @"+=255"(%String* %78, i8 %81)
  call void @dtor162(i8* %tmp.v39)
  call void @dtor58(i32* %charVal)
  br label %if_end35

dumy_block:                                       ; No predecessors!
  br label %if_end38

dumy_block40:                                     ; No predecessors!
  br label %if_end

dumy_block41:                                     ; No predecessors!
  call void @dtor162(i8* %ch)
  unreachable
}

; Function Attrs: inlinehint nounwind
define private void @ctorFromCt257(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [37 x i8]
  %const.struct = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  store [37 x i8] c"Not enough digits for character code\00", [37 x i8]* %const.bytes
  %2 = getelementptr inbounds [37 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [37 x i8]* %const.bytes, i32 0, i32 36
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  call void @ctor251(%String* %1, %StringRef* %"$tmpForRef")
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i32 @"*258"(i32 %x, i32 %y) #2 {
  %x.addr = alloca i32
  store i32 %x, i32* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i32* %x.addr
  %2 = load i32* %y.addr
  %3 = call i32 @cmn15(i32 %1, i32 %2)
  %4 = load i32* %y.addr
  %5 = load i32* %x.addr
  %6 = call i32 @cmn15(i32 %4, i32 %5)
  %7 = mul i32 %3, %6
  ret i32 %7
}

; Function Attrs: alwaysinline nounwind
define private i32 @getXdigitVal(i8 %c) #2 {
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  %tmp.v = alloca i32
  %tmp.v1 = alloca i32
  %tmp.v2 = alloca i32
  %tmp.v3 = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  %2 = call i1 @isDigit(i8 %1)
  br i1 %2, label %cond_alt1, label %cond_alt2

cond_alt1:                                        ; preds = %code
  %3 = load i8* %c.addr
  call void @_ass_32_8z(i32* %tmp.v, i8 %3)
  %4 = load i32* %tmp.v
  call void @_ass_32_8z(i32* %tmp.v1, i8 48)
  %5 = load i32* %tmp.v1
  %6 = call i32 @-43(i32 %4, i32 %5)
  br label %cond_end

cond_alt2:                                        ; preds = %code
  %7 = load i8* %c.addr
  %8 = call i8 @toLower(i8 %7)
  call void @_ass_32_8z(i32* %tmp.v2, i8 %8)
  %9 = load i32* %tmp.v2
  %10 = call i32 @"+14"(i32 10, i32 %9)
  call void @_ass_32_8z(i32* %tmp.v3, i8 97)
  %11 = load i32* %tmp.v3
  %12 = call i32 @-43(i32 %10, i32 %11)
  br label %cond_end

cond_end:                                         ; preds = %cond_alt2, %cond_alt1
  %cond = phi i32 [ %6, %cond_alt1 ], [ %12, %cond_alt2 ]
  br i1 %2, label %cond_destruct_alt1, label %cond_destruct_alt2

cond_destruct_alt1:                               ; preds = %cond_end
  call void @dtor58(i32* %tmp.v1)
  call void @dtor58(i32* %tmp.v)
  br label %cond_destruct_end

cond_destruct_alt2:                               ; preds = %cond_end
  call void @dtor58(i32* %tmp.v3)
  call void @dtor58(i32* %tmp.v2)
  br label %cond_destruct_end

cond_destruct_end:                                ; preds = %cond_destruct_alt2, %cond_destruct_alt1
  ret i32 %cond

cond_destruct_alt14:                              ; preds = %dumy_block
  call void @dtor58(i32* %tmp.v1)
  call void @dtor58(i32* %tmp.v)
  br label %cond_destruct_end6

cond_destruct_alt25:                              ; preds = %dumy_block
  call void @dtor58(i32* %tmp.v3)
  call void @dtor58(i32* %tmp.v2)
  br label %cond_destruct_end6

cond_destruct_end6:                               ; preds = %cond_destruct_alt25, %cond_destruct_alt14
  unreachable

dumy_block:                                       ; No predecessors!
  br i1 %2, label %cond_destruct_alt14, label %cond_destruct_alt25
}

; Function Attrs: alwaysinline nounwind
define private i1 @isDigit(i8 %c) #2 {
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  %tmp.v = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  call void @_ass_32_8z(i32* %tmp.v, i8 %1)
  %2 = load i32* %tmp.v
  %3 = call i32 @isdigit(i32 %2)
  %4 = call i1 @"!=163"(i32 0, i32 %3)
  call void @dtor58(i32* %tmp.v)
  ret i1 %4

dumy_block:                                       ; No predecessors!
  call void @dtor58(i32* %tmp.v)
  unreachable
}

declare i32 @isdigit(i32)

; Function Attrs: alwaysinline nounwind
define private i8 @toLower(i8 %c) #2 {
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  %tmp.v = alloca i8
  %tmp.v1 = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  call void @_ass_32_8z(i32* %tmp.v1, i8 %1)
  %2 = load i32* %tmp.v1
  %3 = call i32 @tolower(i32 %2)
  %4 = trunc i32 %3 to i8
  store i8 %4, i8* %tmp.v
  %5 = load i8* %tmp.v
  call void @dtor162(i8* %tmp.v)
  call void @dtor58(i32* %tmp.v1)
  ret i8 %5

dumy_block:                                       ; No predecessors!
  call void @dtor162(i8* %tmp.v)
  call void @dtor58(i32* %tmp.v1)
  unreachable
}

declare i32 @tolower(i32)

; Function Attrs: inlinehint nounwind
define private void @advanceAndCapture1(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s) #3 {
  %s.addr = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1, i32 0, i32 2
  %3 = getelementptr inbounds %Token* %2, i32 0, i32 2
  %4 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %5 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %4, i32 0, i32 1
  %6 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %5)
  call void @"+=255"(%String* %3, i8 %6)
  %7 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %8 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %7, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %8)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctorFromCt259(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [43 x i8]
  %const.struct = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  store [43 x i8] c"End of file found inside character literal\00", [43 x i8]* %const.bytes
  %2 = getelementptr inbounds [43 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [43 x i8]* %const.bytes, i32 0, i32 42
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  call void @ctor251(%String* %1, %StringRef* %"$tmpForRef")
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctorFromCt260(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [28 x i8]
  %const.struct = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  store [28 x i8] c"Character literal too large\00", [28 x i8]* %const.bytes
  %2 = getelementptr inbounds [28 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [28 x i8]* %const.bytes, i32 0, i32 27
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  call void @ctor251(%String* %1, %StringRef* %"$tmpForRef")
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @parseString(%TokenType* sret %_result, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s) #3 {
  %_result.addr = alloca %TokenType*
  store %TokenType* %_result, %TokenType** %_result.addr
  %s.addr = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %"$tmpForRef" = alloca %String
  %tmp.v = alloca %String
  %"$tmpForRef7" = alloca %String
  %tmp.v8 = alloca %String
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %2)
  br label %if_block

if_block:                                         ; preds = %code
  %3 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %4 = call i1 @hasLessThan(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %3, i32 1)
  br i1 %4, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %5 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @ctorFromCt261(%String* %tmp.v)
  %6 = load %String* %tmp.v
  store %String %6, %String* %"$tmpForRef"
  call void @reportError250(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %5, %String* %"$tmpForRef")
  call void @dtor186(%String* %tmp.v)
  %7 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %7, i32 0)
  ret void

if_end:                                           ; preds = %dumy_block, %if_block
  br label %while_block

dumy_block:                                       ; No predecessors!
  br label %if_end

while_block:                                      ; preds = %while_step, %if_end
  %8 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %9 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %8, i32 0, i32 1
  %10 = call i1 @"pre_!!247"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %9)
  br i1 %10, label %cond.true, label %cond.false

while_body:                                       ; preds = %cond.end
  br label %if_block1

while_step:                                       ; preds = %if_end3
  br label %while_block

while_end:                                        ; preds = %cond.end
  br label %if_block4

cond.true:                                        ; preds = %while_block
  %11 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %12 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %11, i32 0, i32 1
  %13 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %12)
  %14 = icmp ne i8 %13, 34
  br label %cond.end

cond.false:                                       ; preds = %while_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ %14, %cond.true ], [ false, %cond.false ]
  br i1 %cond.res, label %while_body, label %while_end

if_block1:                                        ; preds = %while_body
  %15 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %16 = call i1 @checkEscapeChar(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %15)
  %17 = xor i1 true, %16
  br i1 %17, label %if_then2, label %if_end3

if_then2:                                         ; preds = %if_block1
  %18 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @advanceAndCapture1(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %18)
  br label %if_end3

if_end3:                                          ; preds = %if_then2, %if_block1
  br label %while_step

if_block4:                                        ; preds = %while_end
  %19 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %20 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %19, i32 0, i32 1
  %21 = call i1 @"pre_!"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %20)
  br i1 %21, label %if_then5, label %if_end6

if_then5:                                         ; preds = %if_block4
  %22 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @ctorFromCt262(%String* %tmp.v8)
  %23 = load %String* %tmp.v8
  store %String %23, %String* %"$tmpForRef7"
  call void @reportError250(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %22, %String* %"$tmpForRef7")
  call void @dtor186(%String* %tmp.v8)
  %24 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %24, i32 0)
  ret void

if_end6:                                          ; preds = %dumy_block9, %if_block4
  %25 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %26 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %25, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %26)
  %27 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %27, i32 302)
  ret void

dumy_block9:                                      ; No predecessors!
  br label %if_end6
}

; Function Attrs: inlinehint nounwind
define private void @ctorFromCt261(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [40 x i8]
  %const.struct = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  store [40 x i8] c"End of file found inside string literal\00", [40 x i8]* %const.bytes
  %2 = getelementptr inbounds [40 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [40 x i8]* %const.bytes, i32 0, i32 39
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  call void @ctor251(%String* %1, %StringRef* %"$tmpForRef")
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctorFromCt262(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [40 x i8]
  %const.struct = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  store [40 x i8] c"End of file found inside string literal\00", [40 x i8]* %const.bytes
  %2 = getelementptr inbounds [40 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [40 x i8]* %const.bytes, i32 0, i32 39
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  call void @ctor251(%String* %1, %StringRef* %"$tmpForRef")
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @parseStringNE(%TokenType* sret %_result, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s) #3 {
  %_result.addr = alloca %TokenType*
  store %TokenType* %_result, %TokenType** %_result.addr
  %s.addr = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %"$tmpForRef" = alloca %String
  %tmp.v = alloca %String
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  call void @advance248(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %2, i64 2)
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %3 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %4 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %3, i32 0, i32 1
  %5 = call i1 @"pre_!!247"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %4)
  br i1 %5, label %cond.true, label %cond.false

while_body:                                       ; preds = %cond.end
  %6 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @advanceAndCapture1(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %6)
  br label %while_step

while_step:                                       ; preds = %while_body
  br label %while_block

while_end:                                        ; preds = %cond.end
  br label %if_block

cond.true:                                        ; preds = %while_block
  %7 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %8 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %7, i32 0, i32 1
  %9 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %8)
  %10 = icmp eq i8 %9, 125
  br i1 %10, label %cond.true1, label %cond.false2

cond.false:                                       ; preds = %while_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.end3
  %cond.res4 = phi i1 [ %14, %cond.end3 ], [ false, %cond.false ]
  br i1 %cond.res4, label %while_body, label %while_end

cond.true1:                                       ; preds = %cond.true
  %11 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %12 = call i8 @peekChar(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %11)
  %13 = icmp eq i8 %12, 62
  br label %cond.end3

cond.false2:                                      ; preds = %cond.true
  br label %cond.end3

cond.end3:                                        ; preds = %cond.false2, %cond.true1
  %cond.res = phi i1 [ %13, %cond.true1 ], [ false, %cond.false2 ]
  %14 = xor i1 true, %cond.res
  br label %cond.end

if_block:                                         ; preds = %while_end
  %15 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %16 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %15, i32 0, i32 1
  %17 = call i1 @"pre_!"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %16)
  br i1 %17, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %18 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @ctorFromCt263(%String* %tmp.v)
  %19 = load %String* %tmp.v
  store %String %19, %String* %"$tmpForRef"
  call void @reportError250(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %18, %String* %"$tmpForRef")
  call void @dtor186(%String* %tmp.v)
  %20 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %20, i32 0)
  ret void

if_end:                                           ; preds = %dumy_block, %if_block
  %21 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %22 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %21, i32 0, i32 1
  call void @advance248(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %22, i64 2)
  %23 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %23, i32 302)
  ret void

dumy_block:                                       ; No predecessors!
  br label %if_end
}

; Function Attrs: inlinehint nounwind
define private void @ctorFromCt263(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [40 x i8]
  %const.struct = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  store [40 x i8] c"End of file found inside string literal\00", [40 x i8]* %const.bytes
  %2 = getelementptr inbounds [40 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [40 x i8]* %const.bytes, i32 0, i32 39
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  call void @ctor251(%String* %1, %StringRef* %"$tmpForRef")
  ret void
}

; Function Attrs: inlinehint nounwind
define private i1 @isOpChar(i8 %c) #3 {
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  %2 = icmp eq i8 %1, 126
  br i1 %2, label %cond.true49, label %cond.false50

cond.true:                                        ; preds = %cond.end3
  br label %cond.end

cond.false:                                       ; preds = %cond.end3
  %3 = load i8* %c.addr
  %4 = icmp eq i8 %3, 42
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res68 = phi i1 [ true, %cond.true ], [ %4, %cond.false ]
  ret i1 %cond.res68

cond.true1:                                       ; preds = %cond.end6
  br label %cond.end3

cond.false2:                                      ; preds = %cond.end6
  %5 = load i8* %c.addr
  %6 = icmp eq i8 %5, 47
  br label %cond.end3

cond.end3:                                        ; preds = %cond.false2, %cond.true1
  %cond.res67 = phi i1 [ true, %cond.true1 ], [ %6, %cond.false2 ]
  br i1 %cond.res67, label %cond.true, label %cond.false

cond.true4:                                       ; preds = %cond.end9
  br label %cond.end6

cond.false5:                                      ; preds = %cond.end9
  %7 = load i8* %c.addr
  %8 = icmp eq i8 %7, 63
  br label %cond.end6

cond.end6:                                        ; preds = %cond.false5, %cond.true4
  %cond.res66 = phi i1 [ true, %cond.true4 ], [ %8, %cond.false5 ]
  br i1 %cond.res66, label %cond.true1, label %cond.false2

cond.true7:                                       ; preds = %cond.end12
  br label %cond.end9

cond.false8:                                      ; preds = %cond.end12
  %9 = load i8* %c.addr
  %10 = icmp eq i8 %9, 62
  br label %cond.end9

cond.end9:                                        ; preds = %cond.false8, %cond.true7
  %cond.res65 = phi i1 [ true, %cond.true7 ], [ %10, %cond.false8 ]
  br i1 %cond.res65, label %cond.true4, label %cond.false5

cond.true10:                                      ; preds = %cond.end15
  br label %cond.end12

cond.false11:                                     ; preds = %cond.end15
  %11 = load i8* %c.addr
  %12 = icmp eq i8 %11, 60
  br label %cond.end12

cond.end12:                                       ; preds = %cond.false11, %cond.true10
  %cond.res64 = phi i1 [ true, %cond.true10 ], [ %12, %cond.false11 ]
  br i1 %cond.res64, label %cond.true7, label %cond.false8

cond.true13:                                      ; preds = %cond.end18
  br label %cond.end15

cond.false14:                                     ; preds = %cond.end18
  %13 = load i8* %c.addr
  %14 = icmp eq i8 %13, 58
  br label %cond.end15

cond.end15:                                       ; preds = %cond.false14, %cond.true13
  %cond.res63 = phi i1 [ true, %cond.true13 ], [ %14, %cond.false14 ]
  br i1 %cond.res63, label %cond.true10, label %cond.false11

cond.true16:                                      ; preds = %cond.end21
  br label %cond.end18

cond.false17:                                     ; preds = %cond.end21
  %15 = load i8* %c.addr
  %16 = icmp eq i8 %15, 92
  br label %cond.end18

cond.end18:                                       ; preds = %cond.false17, %cond.true16
  %cond.res62 = phi i1 [ true, %cond.true16 ], [ %16, %cond.false17 ]
  br i1 %cond.res62, label %cond.true13, label %cond.false14

cond.true19:                                      ; preds = %cond.end24
  br label %cond.end21

cond.false20:                                     ; preds = %cond.end24
  %17 = load i8* %c.addr
  %18 = icmp eq i8 %17, 124
  br label %cond.end21

cond.end21:                                       ; preds = %cond.false20, %cond.true19
  %cond.res61 = phi i1 [ true, %cond.true19 ], [ %18, %cond.false20 ]
  br i1 %cond.res61, label %cond.true16, label %cond.false17

cond.true22:                                      ; preds = %cond.end27
  br label %cond.end24

cond.false23:                                     ; preds = %cond.end27
  %19 = load i8* %c.addr
  %20 = icmp eq i8 %19, 61
  br label %cond.end24

cond.end24:                                       ; preds = %cond.false23, %cond.true22
  %cond.res60 = phi i1 [ true, %cond.true22 ], [ %20, %cond.false23 ]
  br i1 %cond.res60, label %cond.true19, label %cond.false20

cond.true25:                                      ; preds = %cond.end30
  br label %cond.end27

cond.false26:                                     ; preds = %cond.end30
  %21 = load i8* %c.addr
  %22 = icmp eq i8 %21, 43
  br label %cond.end27

cond.end27:                                       ; preds = %cond.false26, %cond.true25
  %cond.res59 = phi i1 [ true, %cond.true25 ], [ %22, %cond.false26 ]
  br i1 %cond.res59, label %cond.true22, label %cond.false23

cond.true28:                                      ; preds = %cond.end33
  br label %cond.end30

cond.false29:                                     ; preds = %cond.end33
  %23 = load i8* %c.addr
  %24 = icmp eq i8 %23, 45
  br label %cond.end30

cond.end30:                                       ; preds = %cond.false29, %cond.true28
  %cond.res58 = phi i1 [ true, %cond.true28 ], [ %24, %cond.false29 ]
  br i1 %cond.res58, label %cond.true25, label %cond.false26

cond.true31:                                      ; preds = %cond.end36
  br label %cond.end33

cond.false32:                                     ; preds = %cond.end36
  %25 = load i8* %c.addr
  %26 = icmp eq i8 %25, 38
  br label %cond.end33

cond.end33:                                       ; preds = %cond.false32, %cond.true31
  %cond.res57 = phi i1 [ true, %cond.true31 ], [ %26, %cond.false32 ]
  br i1 %cond.res57, label %cond.true28, label %cond.false29

cond.true34:                                      ; preds = %cond.end39
  br label %cond.end36

cond.false35:                                     ; preds = %cond.end39
  %27 = load i8* %c.addr
  %28 = icmp eq i8 %27, 94
  br label %cond.end36

cond.end36:                                       ; preds = %cond.false35, %cond.true34
  %cond.res56 = phi i1 [ true, %cond.true34 ], [ %28, %cond.false35 ]
  br i1 %cond.res56, label %cond.true31, label %cond.false32

cond.true37:                                      ; preds = %cond.end42
  br label %cond.end39

cond.false38:                                     ; preds = %cond.end42
  %29 = load i8* %c.addr
  %30 = icmp eq i8 %29, 37
  br label %cond.end39

cond.end39:                                       ; preds = %cond.false38, %cond.true37
  %cond.res55 = phi i1 [ true, %cond.true37 ], [ %30, %cond.false38 ]
  br i1 %cond.res55, label %cond.true34, label %cond.false35

cond.true40:                                      ; preds = %cond.end45
  br label %cond.end42

cond.false41:                                     ; preds = %cond.end45
  %31 = load i8* %c.addr
  %32 = icmp eq i8 %31, 36
  br label %cond.end42

cond.end42:                                       ; preds = %cond.false41, %cond.true40
  %cond.res54 = phi i1 [ true, %cond.true40 ], [ %32, %cond.false41 ]
  br i1 %cond.res54, label %cond.true37, label %cond.false38

cond.true43:                                      ; preds = %cond.end48
  br label %cond.end45

cond.false44:                                     ; preds = %cond.end48
  %33 = load i8* %c.addr
  %34 = icmp eq i8 %33, 35
  br label %cond.end45

cond.end45:                                       ; preds = %cond.false44, %cond.true43
  %cond.res53 = phi i1 [ true, %cond.true43 ], [ %34, %cond.false44 ]
  br i1 %cond.res53, label %cond.true40, label %cond.false41

cond.true46:                                      ; preds = %cond.end51
  br label %cond.end48

cond.false47:                                     ; preds = %cond.end51
  %35 = load i8* %c.addr
  %36 = icmp eq i8 %35, 64
  br label %cond.end48

cond.end48:                                       ; preds = %cond.false47, %cond.true46
  %cond.res52 = phi i1 [ true, %cond.true46 ], [ %36, %cond.false47 ]
  br i1 %cond.res52, label %cond.true43, label %cond.false44

cond.true49:                                      ; preds = %code
  br label %cond.end51

cond.false50:                                     ; preds = %code
  %37 = load i8* %c.addr
  %38 = icmp eq i8 %37, 33
  br label %cond.end51

cond.end51:                                       ; preds = %cond.false50, %cond.true49
  %cond.res = phi i1 [ true, %cond.true49 ], [ %38, %cond.false50 ]
  br i1 %cond.res, label %cond.true46, label %cond.false47
}

; Function Attrs: alwaysinline nounwind
define private i1 @isOpCharDot(i8 %c) #2 {
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  %2 = icmp eq i8 %1, 46
  br i1 %2, label %cond.true, label %cond.false

cond.true:                                        ; preds = %code
  br label %cond.end

cond.false:                                       ; preds = %code
  %3 = load i8* %c.addr
  %4 = call i1 @isOpChar(i8 %3)
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ true, %cond.true ], [ %4, %cond.false ]
  ret i1 %cond.res
}

; Function Attrs: inlinehint nounwind
define private i1 @parseOperator(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s) #3 {
  %s.addr = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %ch = alloca i8
  %funptr = alloca i1 (i8)*
  %i = alloca i32
  %c = alloca i8
  %tmp.v = alloca i32
  %tmp.v4 = alloca i32
  %hasOtherDot = alloca i1
  %funptr8 = alloca i1 (i8)*
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  %3 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %2)
  store i8 %3, i8* %ch
  br label %if_block

if_block:                                         ; preds = %code
  %4 = load i8* %ch
  %5 = call i1 @isOpChar(i8 %4)
  br i1 %5, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %6 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  store i1 (i8)* @isOpCharDot, i1 (i8)** %funptr
  %7 = bitcast i1 (i8)** %funptr to %"FunctionPtr[Bool, Char]"*
  %8 = load %"FunctionPtr[Bool, Char]"* %7
  call void @advanceAndCapture(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %6, %"FunctionPtr[Bool, Char]" %8)
  call void @dtor162(i8* %ch)
  ret i1 true

if_end:                                           ; preds = %dumy_block, %if_block
  br label %if_block1

dumy_block:                                       ; No predecessors!
  br label %if_end

if_block1:                                        ; preds = %if_end
  %9 = load i8* %ch
  %10 = icmp eq i8 %9, 46
  br i1 %10, label %if_then2, label %if_end3

if_then2:                                         ; preds = %if_block1
  store i32 1, i32* %i
  %11 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %12 = load i32* %i
  store i32 %12, i32* %tmp.v
  %13 = load i32* %tmp.v
  %14 = call i8 @peekChar265(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %11, i32 %13)
  store i8 %14, i8* %c
  call void @dtor19(i32* %tmp.v)
  br label %while_block

if_end3:                                          ; preds = %if_end7, %if_block1
  call void @dtor162(i8* %ch)
  ret i1 false

while_block:                                      ; preds = %while_step, %if_then2
  %15 = load i8* %c
  %16 = call i1 @isOpChar(i8 %15)
  br i1 %16, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  %17 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %18 = call i32 @"pre_++13"(i32* %i)
  store i32 %18, i32* %tmp.v4
  %19 = load i32* %tmp.v4
  %20 = call i8 @peekChar265(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %17, i32 %19)
  store i8 %20, i8* %c
  call void @dtor19(i32* %tmp.v4)
  br label %while_step

while_step:                                       ; preds = %while_body
  br label %while_block

while_end:                                        ; preds = %while_block
  %21 = load i8* %c
  %22 = icmp eq i8 %21, 46
  store i1 %22, i1* %hasOtherDot
  br label %if_block5

if_block5:                                        ; preds = %while_end
  %23 = load i1* %hasOtherDot
  br i1 %23, label %if_then6, label %if_end7

if_then6:                                         ; preds = %if_block5
  %24 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  store i1 (i8)* @isOpCharDot, i1 (i8)** %funptr8
  %25 = bitcast i1 (i8)** %funptr8 to %"FunctionPtr[Bool, Char]"*
  %26 = load %"FunctionPtr[Bool, Char]"* %25
  call void @advanceAndCapture(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %24, %"FunctionPtr[Bool, Char]" %26)
  call void @dtor167(i1* %hasOtherDot)
  call void @dtor162(i8* %c)
  call void @dtor58(i32* %i)
  call void @dtor162(i8* %ch)
  ret i1 true

if_end7:                                          ; preds = %dumy_block9, %if_block5
  call void @dtor167(i1* %hasOtherDot)
  call void @dtor162(i8* %c)
  call void @dtor58(i32* %i)
  br label %if_end3

dumy_block9:                                      ; No predecessors!
  br label %if_end7

dumy_block10:                                     ; No predecessors!
  call void @dtor162(i8* %ch)
  unreachable
}

; Function Attrs: inlinehint nounwind
define private void @advanceAndCapture(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, %"FunctionPtr[Bool, Char]" %pred) #3 {
  %s.addr = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %pred.addr = alloca %"FunctionPtr[Bool, Char]"
  store %"FunctionPtr[Bool, Char]" %pred, %"FunctionPtr[Bool, Char]"* %pred.addr
  br label %code

code:                                             ; preds = %0
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  %3 = call i1 @"pre_!!247"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %2)
  br i1 %3, label %cond.true, label %cond.false

while_body:                                       ; preds = %cond.end
  %4 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %5 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %4, i32 0, i32 2
  %6 = getelementptr inbounds %Token* %5, i32 0, i32 2
  %7 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %8 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %7, i32 0, i32 1
  %9 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %8)
  call void @"+=255"(%String* %6, i8 %9)
  br label %while_step

while_step:                                       ; preds = %while_body
  %10 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %11 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %10, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %11)
  br label %while_block

while_end:                                        ; preds = %cond.end
  ret void

cond.true:                                        ; preds = %while_block
  %12 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %13 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %12, i32 0, i32 1
  %14 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %13)
  %15 = call i1 @"()264"(%"FunctionPtr[Bool, Char]"* %pred.addr, i8 %14)
  br label %cond.end

cond.false:                                       ; preds = %while_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ %15, %cond.true ], [ false, %cond.false ]
  br i1 %cond.res, label %while_body, label %while_end
}

; Function Attrs: alwaysinline nounwind
define private i1 @"()264"(%"FunctionPtr[Bool, Char]"* %"$this", i8 %p1) #2 {
  %"$this.addr" = alloca %"FunctionPtr[Bool, Char]"*
  store %"FunctionPtr[Bool, Char]"* %"$this", %"FunctionPtr[Bool, Char]"** %"$this.addr"
  %p1.addr = alloca i8
  store i8 %p1, i8* %p1.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"FunctionPtr[Bool, Char]"** %"$this.addr"
  %2 = load i8* %p1.addr
  %3 = bitcast %"FunctionPtr[Bool, Char]"* %1 to i1 (i8)**
  %4 = load i1 (i8)** %3
  %5 = call i1 %4(i8 %2)
  ret i1 %5
}

; Function Attrs: inlinehint nounwind
define private i8 @peekChar265(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, i32 %n) #3 {
  %s.addr = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %n.addr = alloca i32
  store i32 %n, i32* %n.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  %3 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %2, i32 0, i32 0
  %4 = load i32* %n.addr
  %5 = call i8 @peek(%"RangeWithLookahead[FileRange/rtct]"* %3, i32 %4)
  ret i8 %5
}

; Function Attrs: alwaysinline nounwind
define private i1 @isAlpha(i8 %c) #2 {
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  %tmp.v = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  call void @_ass_32_8z(i32* %tmp.v, i8 %1)
  %2 = load i32* %tmp.v
  %3 = call i32 @isalpha(i32 %2)
  %4 = call i1 @"!=163"(i32 0, i32 %3)
  call void @dtor58(i32* %tmp.v)
  ret i1 %4

dumy_block:                                       ; No predecessors!
  call void @dtor58(i32* %tmp.v)
  unreachable
}

declare i32 @isalpha(i32)

; Function Attrs: noinline nounwind
define void @parseIdentifer(%TokenType* sret %_result, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s) #4 {
  %_result.addr = alloca %TokenType*
  store %TokenType* %_result, %TokenType** %_result.addr
  %s.addr = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %allowSymbolChars = alloca i1
  %firstDot = alloca i1
  %ch = alloca i8
  %data = alloca %StringRef*
  %"$tmpC" = alloca %StringRef
  %const.bytes = alloca [6 x i8]
  %const.struct = alloca %StringRef
  %const.bytes24 = alloca [6 x i8]
  %const.struct25 = alloca %StringRef
  %const.bytes31 = alloca [6 x i8]
  %const.struct32 = alloca %StringRef
  %const.bytes38 = alloca [8 x i8]
  %const.struct39 = alloca %StringRef
  %const.bytes45 = alloca [9 x i8]
  %const.struct46 = alloca %StringRef
  %const.bytes52 = alloca [9 x i8]
  %const.struct53 = alloca %StringRef
  %const.bytes59 = alloca [4 x i8]
  %const.struct60 = alloca %StringRef
  %const.bytes66 = alloca [3 x i8]
  %const.struct67 = alloca %StringRef
  %const.bytes73 = alloca [5 x i8]
  %const.struct74 = alloca %StringRef
  %const.bytes80 = alloca [6 x i8]
  %const.struct81 = alloca %StringRef
  %const.bytes87 = alloca [8 x i8]
  %const.struct88 = alloca %StringRef
  %const.bytes94 = alloca [4 x i8]
  %const.struct95 = alloca %StringRef
  %const.bytes101 = alloca [7 x i8]
  %const.struct102 = alloca %StringRef
  %const.bytes108 = alloca [7 x i8]
  %const.struct109 = alloca %StringRef
  %const.bytes115 = alloca [5 x i8]
  %const.struct116 = alloca %StringRef
  %const.bytes122 = alloca [8 x i8]
  %const.struct123 = alloca %StringRef
  %const.bytes129 = alloca [8 x i8]
  %const.struct130 = alloca %StringRef
  %const.bytes136 = alloca [7 x i8]
  %const.struct137 = alloca %StringRef
  %const.bytes143 = alloca [7 x i8]
  %const.struct144 = alloca %StringRef
  %const.bytes150 = alloca [5 x i8]
  %const.struct151 = alloca %StringRef
  %const.bytes157 = alloca [6 x i8]
  %const.struct158 = alloca %StringRef
  %const.bytes164 = alloca [5 x i8]
  %const.struct165 = alloca %StringRef
  %const.bytes171 = alloca [4 x i8]
  %const.struct172 = alloca %StringRef
  %const.bytes178 = alloca [6 x i8]
  %const.struct179 = alloca %StringRef
  %const.bytes185 = alloca [4 x i8]
  %const.struct186 = alloca %StringRef
  %const.bytes192 = alloca [6 x i8]
  %const.struct193 = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  store i1 false, i1* %allowSymbolChars
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1, i32 0, i32 2
  %3 = getelementptr inbounds %Token* %2, i32 0, i32 2
  %4 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %5 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %4, i32 0, i32 1
  %6 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %5)
  call void @"+=255"(%String* %3, i8 %6)
  %7 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %8 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %7, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %8)
  store i1 true, i1* %firstDot
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %9 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %10 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %9, i32 0, i32 1
  %11 = call i1 @"pre_!!247"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %10)
  br i1 %11, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  %12 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %13 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %12, i32 0, i32 1
  %14 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %13)
  store i8 %14, i8* %ch
  br label %if_block

while_step:                                       ; preds = %if_end7
  %15 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %16 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %15, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %16)
  br label %while_block

while_end:                                        ; preds = %if_then, %while_block
  %17 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %18 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %17, i32 0, i32 2
  %19 = getelementptr inbounds %Token* %18, i32 0, i32 2
  call void @asStringRef(%StringRef* %"$tmpC", %String* %19)
  store %StringRef* %"$tmpC", %StringRef** %data
  call void @dtor78(%StringRef* %"$tmpC")
  br label %if_block16

if_block:                                         ; preds = %while_body
  %20 = load i8* %ch
  %21 = call i1 @isAlpha(i8 %20)
  %22 = xor i1 true, %21
  br i1 %22, label %cond.true1, label %cond.false2

if_then:                                          ; preds = %cond.end
  call void @dtor162(i8* %ch)
  br label %while_end

if_end:                                           ; preds = %dumy_block, %cond.end
  %23 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %24 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %23, i32 0, i32 2
  %25 = getelementptr inbounds %Token* %24, i32 0, i32 2
  %26 = load i8* %ch
  call void @"+=255"(%String* %25, i8 %26)
  br label %if_block5

cond.true:                                        ; preds = %cond.end3
  %27 = load i8* %ch
  %28 = call i1 @isDigit(i8 %27)
  %29 = xor i1 true, %28
  br label %cond.end

cond.false:                                       ; preds = %cond.end3
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res4 = phi i1 [ %29, %cond.true ], [ false, %cond.false ]
  br i1 %cond.res4, label %if_then, label %if_end

cond.true1:                                       ; preds = %if_block
  %30 = load i8* %ch
  %31 = icmp ne i8 %30, 95
  br label %cond.end3

cond.false2:                                      ; preds = %if_block
  br label %cond.end3

cond.end3:                                        ; preds = %cond.false2, %cond.true1
  %cond.res = phi i1 [ %31, %cond.true1 ], [ false, %cond.false2 ]
  br i1 %cond.res, label %cond.true, label %cond.false

dumy_block:                                       ; No predecessors!
  br label %if_end

if_block5:                                        ; preds = %if_end
  %32 = load i8* %ch
  %33 = icmp eq i8 %32, 95
  br i1 %33, label %cond.true8, label %cond.false9

if_then6:                                         ; preds = %cond.end10
  %34 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %35 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %34, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %35)
  br label %if_block12

if_end7:                                          ; preds = %if_end14, %cond.end10
  call void @dtor162(i8* %ch)
  br label %while_step

cond.true8:                                       ; preds = %if_block5
  %36 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %37 = call i8 @peekChar(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %36)
  %38 = call i1 @isOpCharDot(i8 %37)
  br label %cond.end10

cond.false9:                                      ; preds = %if_block5
  br label %cond.end10

cond.end10:                                       ; preds = %cond.false9, %cond.true8
  %cond.res11 = phi i1 [ %38, %cond.true8 ], [ false, %cond.false9 ]
  br i1 %cond.res11, label %if_then6, label %if_end7

if_block12:                                       ; preds = %if_then6
  %39 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %40 = call i1 @parseOperator(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %39)
  br i1 %40, label %if_then13, label %if_end14

if_then13:                                        ; preds = %if_block12
  %41 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %41, i32 299)
  call void @dtor162(i8* %ch)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_end14:                                         ; preds = %dumy_block15, %if_block12
  br label %if_end7

dumy_block15:                                     ; No predecessors!
  br label %if_end14

if_block16:                                       ; preds = %while_end
  %42 = load %StringRef** %data
  store [6 x i8] c"break\00", [6 x i8]* %const.bytes
  %43 = getelementptr inbounds [6 x i8]* %const.bytes, i32 0, i32 0
  %44 = getelementptr inbounds [6 x i8]* %const.bytes, i32 0, i32 5
  %45 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %46 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %43, i8** %45
  store i8* %44, i8** %46
  %47 = load %StringRef* %const.struct
  %48 = call i1 @"==266"(%StringRef* %42, %StringRef %47)
  br i1 %48, label %if_then17, label %if_else

if_then17:                                        ; preds = %if_block16
  %49 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %49, i32 271)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else:                                          ; preds = %if_block16
  br label %if_block20

if_end18:                                         ; preds = %if_end23, %dumy_block19
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

dumy_block19:                                     ; No predecessors!
  br label %if_end18

if_block20:                                       ; preds = %if_else
  %50 = load %StringRef** %data
  store [6 x i8] c"catch\00", [6 x i8]* %const.bytes24
  %51 = getelementptr inbounds [6 x i8]* %const.bytes24, i32 0, i32 0
  %52 = getelementptr inbounds [6 x i8]* %const.bytes24, i32 0, i32 5
  %53 = getelementptr inbounds %StringRef* %const.struct25, i32 0, i32 0
  %54 = getelementptr inbounds %StringRef* %const.struct25, i32 0, i32 1
  store i8* %51, i8** %53
  store i8* %52, i8** %54
  %55 = load %StringRef* %const.struct25
  %56 = call i1 @"==266"(%StringRef* %50, %StringRef %55)
  br i1 %56, label %if_then21, label %if_else22

if_then21:                                        ; preds = %if_block20
  %57 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %57, i32 272)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else22:                                        ; preds = %if_block20
  br label %if_block27

if_end23:                                         ; preds = %if_end30, %dumy_block26
  br label %if_end18

dumy_block26:                                     ; No predecessors!
  br label %if_end23

if_block27:                                       ; preds = %if_else22
  %58 = load %StringRef** %data
  store [6 x i8] c"class\00", [6 x i8]* %const.bytes31
  %59 = getelementptr inbounds [6 x i8]* %const.bytes31, i32 0, i32 0
  %60 = getelementptr inbounds [6 x i8]* %const.bytes31, i32 0, i32 5
  %61 = getelementptr inbounds %StringRef* %const.struct32, i32 0, i32 0
  %62 = getelementptr inbounds %StringRef* %const.struct32, i32 0, i32 1
  store i8* %59, i8** %61
  store i8* %60, i8** %62
  %63 = load %StringRef* %const.struct32
  %64 = call i1 @"==266"(%StringRef* %58, %StringRef %63)
  br i1 %64, label %if_then28, label %if_else29

if_then28:                                        ; preds = %if_block27
  %65 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %65, i32 264)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else29:                                        ; preds = %if_block27
  br label %if_block34

if_end30:                                         ; preds = %if_end37, %dumy_block33
  br label %if_end23

dumy_block33:                                     ; No predecessors!
  br label %if_end30

if_block34:                                       ; preds = %if_else29
  %66 = load %StringRef** %data
  store [8 x i8] c"concept\00", [8 x i8]* %const.bytes38
  %67 = getelementptr inbounds [8 x i8]* %const.bytes38, i32 0, i32 0
  %68 = getelementptr inbounds [8 x i8]* %const.bytes38, i32 0, i32 7
  %69 = getelementptr inbounds %StringRef* %const.struct39, i32 0, i32 0
  %70 = getelementptr inbounds %StringRef* %const.struct39, i32 0, i32 1
  store i8* %67, i8** %69
  store i8* %68, i8** %70
  %71 = load %StringRef* %const.struct39
  %72 = call i1 @"==266"(%StringRef* %66, %StringRef %71)
  br i1 %72, label %if_then35, label %if_else36

if_then35:                                        ; preds = %if_block34
  %73 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %73, i32 265)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else36:                                        ; preds = %if_block34
  br label %if_block41

if_end37:                                         ; preds = %if_end44, %dumy_block40
  br label %if_end30

dumy_block40:                                     ; No predecessors!
  br label %if_end37

if_block41:                                       ; preds = %if_else36
  %74 = load %StringRef** %data
  store [9 x i8] c"continue\00", [9 x i8]* %const.bytes45
  %75 = getelementptr inbounds [9 x i8]* %const.bytes45, i32 0, i32 0
  %76 = getelementptr inbounds [9 x i8]* %const.bytes45, i32 0, i32 8
  %77 = getelementptr inbounds %StringRef* %const.struct46, i32 0, i32 0
  %78 = getelementptr inbounds %StringRef* %const.struct46, i32 0, i32 1
  store i8* %75, i8** %77
  store i8* %76, i8** %78
  %79 = load %StringRef* %const.struct46
  %80 = call i1 @"==266"(%StringRef* %74, %StringRef %79)
  br i1 %80, label %if_then42, label %if_else43

if_then42:                                        ; preds = %if_block41
  %81 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %81, i32 273)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else43:                                        ; preds = %if_block41
  br label %if_block48

if_end44:                                         ; preds = %if_end51, %dumy_block47
  br label %if_end37

dumy_block47:                                     ; No predecessors!
  br label %if_end44

if_block48:                                       ; preds = %if_else43
  %82 = load %StringRef** %data
  store [9 x i8] c"datatype\00", [9 x i8]* %const.bytes52
  %83 = getelementptr inbounds [9 x i8]* %const.bytes52, i32 0, i32 0
  %84 = getelementptr inbounds [9 x i8]* %const.bytes52, i32 0, i32 8
  %85 = getelementptr inbounds %StringRef* %const.struct53, i32 0, i32 0
  %86 = getelementptr inbounds %StringRef* %const.struct53, i32 0, i32 1
  store i8* %83, i8** %85
  store i8* %84, i8** %86
  %87 = load %StringRef* %const.struct53
  %88 = call i1 @"==266"(%StringRef* %82, %StringRef %87)
  br i1 %88, label %if_then49, label %if_else50

if_then49:                                        ; preds = %if_block48
  %89 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %89, i32 266)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else50:                                        ; preds = %if_block48
  br label %if_block55

if_end51:                                         ; preds = %if_end58, %dumy_block54
  br label %if_end44

dumy_block54:                                     ; No predecessors!
  br label %if_end51

if_block55:                                       ; preds = %if_else50
  %90 = load %StringRef** %data
  store [4 x i8] c"fun\00", [4 x i8]* %const.bytes59
  %91 = getelementptr inbounds [4 x i8]* %const.bytes59, i32 0, i32 0
  %92 = getelementptr inbounds [4 x i8]* %const.bytes59, i32 0, i32 3
  %93 = getelementptr inbounds %StringRef* %const.struct60, i32 0, i32 0
  %94 = getelementptr inbounds %StringRef* %const.struct60, i32 0, i32 1
  store i8* %91, i8** %93
  store i8* %92, i8** %94
  %95 = load %StringRef* %const.struct60
  %96 = call i1 @"==266"(%StringRef* %90, %StringRef %95)
  br i1 %96, label %if_then56, label %if_else57

if_then56:                                        ; preds = %if_block55
  %97 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %97, i32 267)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else57:                                        ; preds = %if_block55
  br label %if_block62

if_end58:                                         ; preds = %if_end65, %dumy_block61
  br label %if_end51

dumy_block61:                                     ; No predecessors!
  br label %if_end58

if_block62:                                       ; preds = %if_else57
  %98 = load %StringRef** %data
  store [3 x i8] c"if\00", [3 x i8]* %const.bytes66
  %99 = getelementptr inbounds [3 x i8]* %const.bytes66, i32 0, i32 0
  %100 = getelementptr inbounds [3 x i8]* %const.bytes66, i32 0, i32 2
  %101 = getelementptr inbounds %StringRef* %const.struct67, i32 0, i32 0
  %102 = getelementptr inbounds %StringRef* %const.struct67, i32 0, i32 1
  store i8* %99, i8** %101
  store i8* %100, i8** %102
  %103 = load %StringRef* %const.struct67
  %104 = call i1 @"==266"(%StringRef* %98, %StringRef %103)
  br i1 %104, label %if_then63, label %if_else64

if_then63:                                        ; preds = %if_block62
  %105 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %105, i32 276)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else64:                                        ; preds = %if_block62
  br label %if_block69

if_end65:                                         ; preds = %if_end72, %dumy_block68
  br label %if_end58

dumy_block68:                                     ; No predecessors!
  br label %if_end65

if_block69:                                       ; preds = %if_else64
  %106 = load %StringRef** %data
  store [5 x i8] c"else\00", [5 x i8]* %const.bytes73
  %107 = getelementptr inbounds [5 x i8]* %const.bytes73, i32 0, i32 0
  %108 = getelementptr inbounds [5 x i8]* %const.bytes73, i32 0, i32 4
  %109 = getelementptr inbounds %StringRef* %const.struct74, i32 0, i32 0
  %110 = getelementptr inbounds %StringRef* %const.struct74, i32 0, i32 1
  store i8* %107, i8** %109
  store i8* %108, i8** %110
  %111 = load %StringRef* %const.struct74
  %112 = call i1 @"==266"(%StringRef* %106, %StringRef %111)
  br i1 %112, label %if_then70, label %if_else71

if_then70:                                        ; preds = %if_block69
  %113 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %113, i32 286)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else71:                                        ; preds = %if_block69
  br label %if_block76

if_end72:                                         ; preds = %if_end79, %dumy_block75
  br label %if_end65

dumy_block75:                                     ; No predecessors!
  br label %if_end72

if_block76:                                       ; preds = %if_else71
  %114 = load %StringRef** %data
  store [6 x i8] c"false\00", [6 x i8]* %const.bytes80
  %115 = getelementptr inbounds [6 x i8]* %const.bytes80, i32 0, i32 0
  %116 = getelementptr inbounds [6 x i8]* %const.bytes80, i32 0, i32 5
  %117 = getelementptr inbounds %StringRef* %const.struct81, i32 0, i32 0
  %118 = getelementptr inbounds %StringRef* %const.struct81, i32 0, i32 1
  store i8* %115, i8** %117
  store i8* %116, i8** %118
  %119 = load %StringRef* %const.struct81
  %120 = call i1 @"==266"(%StringRef* %114, %StringRef %119)
  br i1 %120, label %if_then77, label %if_else78

if_then77:                                        ; preds = %if_block76
  %121 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %121, i32 281)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else78:                                        ; preds = %if_block76
  br label %if_block83

if_end79:                                         ; preds = %if_end86, %dumy_block82
  br label %if_end72

dumy_block82:                                     ; No predecessors!
  br label %if_end79

if_block83:                                       ; preds = %if_else78
  %122 = load %StringRef** %data
  store [8 x i8] c"finally\00", [8 x i8]* %const.bytes87
  %123 = getelementptr inbounds [8 x i8]* %const.bytes87, i32 0, i32 0
  %124 = getelementptr inbounds [8 x i8]* %const.bytes87, i32 0, i32 7
  %125 = getelementptr inbounds %StringRef* %const.struct88, i32 0, i32 0
  %126 = getelementptr inbounds %StringRef* %const.struct88, i32 0, i32 1
  store i8* %123, i8** %125
  store i8* %124, i8** %126
  %127 = load %StringRef* %const.struct88
  %128 = call i1 @"==266"(%StringRef* %122, %StringRef %127)
  br i1 %128, label %if_then84, label %if_else85

if_then84:                                        ; preds = %if_block83
  %129 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %129, i32 274)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else85:                                        ; preds = %if_block83
  br label %if_block90

if_end86:                                         ; preds = %if_end93, %dumy_block89
  br label %if_end79

dumy_block89:                                     ; No predecessors!
  br label %if_end86

if_block90:                                       ; preds = %if_else85
  %130 = load %StringRef** %data
  store [4 x i8] c"for\00", [4 x i8]* %const.bytes94
  %131 = getelementptr inbounds [4 x i8]* %const.bytes94, i32 0, i32 0
  %132 = getelementptr inbounds [4 x i8]* %const.bytes94, i32 0, i32 3
  %133 = getelementptr inbounds %StringRef* %const.struct95, i32 0, i32 0
  %134 = getelementptr inbounds %StringRef* %const.struct95, i32 0, i32 1
  store i8* %131, i8** %133
  store i8* %132, i8** %134
  %135 = load %StringRef* %const.struct95
  %136 = call i1 @"==266"(%StringRef* %130, %StringRef %135)
  br i1 %136, label %if_then91, label %if_else92

if_then91:                                        ; preds = %if_block90
  %137 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %137, i32 275)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else92:                                        ; preds = %if_block90
  br label %if_block97

if_end93:                                         ; preds = %if_end100, %dumy_block96
  br label %if_end86

dumy_block96:                                     ; No predecessors!
  br label %if_end93

if_block97:                                       ; preds = %if_else92
  %138 = load %StringRef** %data
  store [7 x i8] c"import\00", [7 x i8]* %const.bytes101
  %139 = getelementptr inbounds [7 x i8]* %const.bytes101, i32 0, i32 0
  %140 = getelementptr inbounds [7 x i8]* %const.bytes101, i32 0, i32 6
  %141 = getelementptr inbounds %StringRef* %const.struct102, i32 0, i32 0
  %142 = getelementptr inbounds %StringRef* %const.struct102, i32 0, i32 1
  store i8* %139, i8** %141
  store i8* %140, i8** %142
  %143 = load %StringRef* %const.struct102
  %144 = call i1 @"==266"(%StringRef* %138, %StringRef %143)
  br i1 %144, label %if_then98, label %if_else99

if_then98:                                        ; preds = %if_block97
  %145 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %145, i32 261)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else99:                                        ; preds = %if_block97
  br label %if_block104

if_end100:                                        ; preds = %if_end107, %dumy_block103
  br label %if_end93

dumy_block103:                                    ; No predecessors!
  br label %if_end100

if_block104:                                      ; preds = %if_else99
  %146 = load %StringRef** %data
  store [7 x i8] c"module\00", [7 x i8]* %const.bytes108
  %147 = getelementptr inbounds [7 x i8]* %const.bytes108, i32 0, i32 0
  %148 = getelementptr inbounds [7 x i8]* %const.bytes108, i32 0, i32 6
  %149 = getelementptr inbounds %StringRef* %const.struct109, i32 0, i32 0
  %150 = getelementptr inbounds %StringRef* %const.struct109, i32 0, i32 1
  store i8* %147, i8** %149
  store i8* %148, i8** %150
  %151 = load %StringRef* %const.struct109
  %152 = call i1 @"==266"(%StringRef* %146, %StringRef %151)
  br i1 %152, label %if_then105, label %if_else106

if_then105:                                       ; preds = %if_block104
  %153 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %153, i32 260)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else106:                                       ; preds = %if_block104
  br label %if_block111

if_end107:                                        ; preds = %if_end114, %dumy_block110
  br label %if_end100

dumy_block110:                                    ; No predecessors!
  br label %if_end107

if_block111:                                      ; preds = %if_else106
  %154 = load %StringRef** %data
  store [5 x i8] c"null\00", [5 x i8]* %const.bytes115
  %155 = getelementptr inbounds [5 x i8]* %const.bytes115, i32 0, i32 0
  %156 = getelementptr inbounds [5 x i8]* %const.bytes115, i32 0, i32 4
  %157 = getelementptr inbounds %StringRef* %const.struct116, i32 0, i32 0
  %158 = getelementptr inbounds %StringRef* %const.struct116, i32 0, i32 1
  store i8* %155, i8** %157
  store i8* %156, i8** %158
  %159 = load %StringRef* %const.struct116
  %160 = call i1 @"==266"(%StringRef* %154, %StringRef %159)
  br i1 %160, label %if_then112, label %if_else113

if_then112:                                       ; preds = %if_block111
  %161 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %161, i32 282)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else113:                                       ; preds = %if_block111
  br label %if_block118

if_end114:                                        ; preds = %if_end121, %dumy_block117
  br label %if_end107

dumy_block117:                                    ; No predecessors!
  br label %if_end114

if_block118:                                      ; preds = %if_else113
  %162 = load %StringRef** %data
  store [8 x i8] c"package\00", [8 x i8]* %const.bytes122
  %163 = getelementptr inbounds [8 x i8]* %const.bytes122, i32 0, i32 0
  %164 = getelementptr inbounds [8 x i8]* %const.bytes122, i32 0, i32 7
  %165 = getelementptr inbounds %StringRef* %const.struct123, i32 0, i32 0
  %166 = getelementptr inbounds %StringRef* %const.struct123, i32 0, i32 1
  store i8* %163, i8** %165
  store i8* %164, i8** %166
  %167 = load %StringRef* %const.struct123
  %168 = call i1 @"==266"(%StringRef* %162, %StringRef %167)
  br i1 %168, label %if_then119, label %if_else120

if_then119:                                       ; preds = %if_block118
  %169 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %169, i32 268)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else120:                                       ; preds = %if_block118
  br label %if_block125

if_end121:                                        ; preds = %if_end128, %dumy_block124
  br label %if_end114

dumy_block124:                                    ; No predecessors!
  br label %if_end121

if_block125:                                      ; preds = %if_else120
  %170 = load %StringRef** %data
  store [8 x i8] c"private\00", [8 x i8]* %const.bytes129
  %171 = getelementptr inbounds [8 x i8]* %const.bytes129, i32 0, i32 0
  %172 = getelementptr inbounds [8 x i8]* %const.bytes129, i32 0, i32 7
  %173 = getelementptr inbounds %StringRef* %const.struct130, i32 0, i32 0
  %174 = getelementptr inbounds %StringRef* %const.struct130, i32 0, i32 1
  store i8* %171, i8** %173
  store i8* %172, i8** %174
  %175 = load %StringRef* %const.struct130
  %176 = call i1 @"==266"(%StringRef* %170, %StringRef %175)
  br i1 %176, label %if_then126, label %if_else127

if_then126:                                       ; preds = %if_block125
  %177 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %177, i32 262)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else127:                                       ; preds = %if_block125
  br label %if_block132

if_end128:                                        ; preds = %if_end135, %dumy_block131
  br label %if_end121

dumy_block131:                                    ; No predecessors!
  br label %if_end128

if_block132:                                      ; preds = %if_else127
  %178 = load %StringRef** %data
  store [7 x i8] c"public\00", [7 x i8]* %const.bytes136
  %179 = getelementptr inbounds [7 x i8]* %const.bytes136, i32 0, i32 0
  %180 = getelementptr inbounds [7 x i8]* %const.bytes136, i32 0, i32 6
  %181 = getelementptr inbounds %StringRef* %const.struct137, i32 0, i32 0
  %182 = getelementptr inbounds %StringRef* %const.struct137, i32 0, i32 1
  store i8* %179, i8** %181
  store i8* %180, i8** %182
  %183 = load %StringRef* %const.struct137
  %184 = call i1 @"==266"(%StringRef* %178, %StringRef %183)
  br i1 %184, label %if_then133, label %if_else134

if_then133:                                       ; preds = %if_block132
  %185 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %185, i32 263)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else134:                                       ; preds = %if_block132
  br label %if_block139

if_end135:                                        ; preds = %if_end142, %dumy_block138
  br label %if_end128

dumy_block138:                                    ; No predecessors!
  br label %if_end135

if_block139:                                      ; preds = %if_else134
  %186 = load %StringRef** %data
  store [7 x i8] c"return\00", [7 x i8]* %const.bytes143
  %187 = getelementptr inbounds [7 x i8]* %const.bytes143, i32 0, i32 0
  %188 = getelementptr inbounds [7 x i8]* %const.bytes143, i32 0, i32 6
  %189 = getelementptr inbounds %StringRef* %const.struct144, i32 0, i32 0
  %190 = getelementptr inbounds %StringRef* %const.struct144, i32 0, i32 1
  store i8* %187, i8** %189
  store i8* %188, i8** %190
  %191 = load %StringRef* %const.struct144
  %192 = call i1 @"==266"(%StringRef* %186, %StringRef %191)
  br i1 %192, label %if_then140, label %if_else141

if_then140:                                       ; preds = %if_block139
  %193 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %193, i32 277)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else141:                                       ; preds = %if_block139
  br label %if_block146

if_end142:                                        ; preds = %if_end149, %dumy_block145
  br label %if_end135

dumy_block145:                                    ; No predecessors!
  br label %if_end142

if_block146:                                      ; preds = %if_else141
  %194 = load %StringRef** %data
  store [5 x i8] c"this\00", [5 x i8]* %const.bytes150
  %195 = getelementptr inbounds [5 x i8]* %const.bytes150, i32 0, i32 0
  %196 = getelementptr inbounds [5 x i8]* %const.bytes150, i32 0, i32 4
  %197 = getelementptr inbounds %StringRef* %const.struct151, i32 0, i32 0
  %198 = getelementptr inbounds %StringRef* %const.struct151, i32 0, i32 1
  store i8* %195, i8** %197
  store i8* %196, i8** %198
  %199 = load %StringRef* %const.struct151
  %200 = call i1 @"==266"(%StringRef* %194, %StringRef %199)
  br i1 %200, label %if_then147, label %if_else148

if_then147:                                       ; preds = %if_block146
  %201 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %201, i32 283)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else148:                                       ; preds = %if_block146
  br label %if_block153

if_end149:                                        ; preds = %if_end156, %dumy_block152
  br label %if_end142

dumy_block152:                                    ; No predecessors!
  br label %if_end149

if_block153:                                      ; preds = %if_else148
  %202 = load %StringRef** %data
  store [6 x i8] c"throw\00", [6 x i8]* %const.bytes157
  %203 = getelementptr inbounds [6 x i8]* %const.bytes157, i32 0, i32 0
  %204 = getelementptr inbounds [6 x i8]* %const.bytes157, i32 0, i32 5
  %205 = getelementptr inbounds %StringRef* %const.struct158, i32 0, i32 0
  %206 = getelementptr inbounds %StringRef* %const.struct158, i32 0, i32 1
  store i8* %203, i8** %205
  store i8* %204, i8** %206
  %207 = load %StringRef* %const.struct158
  %208 = call i1 @"==266"(%StringRef* %202, %StringRef %207)
  br i1 %208, label %if_then154, label %if_else155

if_then154:                                       ; preds = %if_block153
  %209 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %209, i32 278)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else155:                                       ; preds = %if_block153
  br label %if_block160

if_end156:                                        ; preds = %if_end163, %dumy_block159
  br label %if_end149

dumy_block159:                                    ; No predecessors!
  br label %if_end156

if_block160:                                      ; preds = %if_else155
  %210 = load %StringRef** %data
  store [5 x i8] c"true\00", [5 x i8]* %const.bytes164
  %211 = getelementptr inbounds [5 x i8]* %const.bytes164, i32 0, i32 0
  %212 = getelementptr inbounds [5 x i8]* %const.bytes164, i32 0, i32 4
  %213 = getelementptr inbounds %StringRef* %const.struct165, i32 0, i32 0
  %214 = getelementptr inbounds %StringRef* %const.struct165, i32 0, i32 1
  store i8* %211, i8** %213
  store i8* %212, i8** %214
  %215 = load %StringRef* %const.struct165
  %216 = call i1 @"==266"(%StringRef* %210, %StringRef %215)
  br i1 %216, label %if_then161, label %if_else162

if_then161:                                       ; preds = %if_block160
  %217 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %217, i32 284)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else162:                                       ; preds = %if_block160
  br label %if_block167

if_end163:                                        ; preds = %if_end170, %dumy_block166
  br label %if_end156

dumy_block166:                                    ; No predecessors!
  br label %if_end163

if_block167:                                      ; preds = %if_else162
  %218 = load %StringRef** %data
  store [4 x i8] c"try\00", [4 x i8]* %const.bytes171
  %219 = getelementptr inbounds [4 x i8]* %const.bytes171, i32 0, i32 0
  %220 = getelementptr inbounds [4 x i8]* %const.bytes171, i32 0, i32 3
  %221 = getelementptr inbounds %StringRef* %const.struct172, i32 0, i32 0
  %222 = getelementptr inbounds %StringRef* %const.struct172, i32 0, i32 1
  store i8* %219, i8** %221
  store i8* %220, i8** %222
  %223 = load %StringRef* %const.struct172
  %224 = call i1 @"==266"(%StringRef* %218, %StringRef %223)
  br i1 %224, label %if_then168, label %if_else169

if_then168:                                       ; preds = %if_block167
  %225 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %225, i32 279)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else169:                                       ; preds = %if_block167
  br label %if_block174

if_end170:                                        ; preds = %if_end177, %dumy_block173
  br label %if_end163

dumy_block173:                                    ; No predecessors!
  br label %if_end170

if_block174:                                      ; preds = %if_else169
  %226 = load %StringRef** %data
  store [6 x i8] c"using\00", [6 x i8]* %const.bytes178
  %227 = getelementptr inbounds [6 x i8]* %const.bytes178, i32 0, i32 0
  %228 = getelementptr inbounds [6 x i8]* %const.bytes178, i32 0, i32 5
  %229 = getelementptr inbounds %StringRef* %const.struct179, i32 0, i32 0
  %230 = getelementptr inbounds %StringRef* %const.struct179, i32 0, i32 1
  store i8* %227, i8** %229
  store i8* %228, i8** %230
  %231 = load %StringRef* %const.struct179
  %232 = call i1 @"==266"(%StringRef* %226, %StringRef %231)
  br i1 %232, label %if_then175, label %if_else176

if_then175:                                       ; preds = %if_block174
  %233 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %233, i32 269)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else176:                                       ; preds = %if_block174
  br label %if_block181

if_end177:                                        ; preds = %if_end184, %dumy_block180
  br label %if_end170

dumy_block180:                                    ; No predecessors!
  br label %if_end177

if_block181:                                      ; preds = %if_else176
  %234 = load %StringRef** %data
  store [4 x i8] c"var\00", [4 x i8]* %const.bytes185
  %235 = getelementptr inbounds [4 x i8]* %const.bytes185, i32 0, i32 0
  %236 = getelementptr inbounds [4 x i8]* %const.bytes185, i32 0, i32 3
  %237 = getelementptr inbounds %StringRef* %const.struct186, i32 0, i32 0
  %238 = getelementptr inbounds %StringRef* %const.struct186, i32 0, i32 1
  store i8* %235, i8** %237
  store i8* %236, i8** %238
  %239 = load %StringRef* %const.struct186
  %240 = call i1 @"==266"(%StringRef* %234, %StringRef %239)
  br i1 %240, label %if_then182, label %if_else183

if_then182:                                       ; preds = %if_block181
  %241 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %241, i32 270)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else183:                                       ; preds = %if_block181
  br label %if_block188

if_end184:                                        ; preds = %if_end191, %dumy_block187
  br label %if_end177

dumy_block187:                                    ; No predecessors!
  br label %if_end184

if_block188:                                      ; preds = %if_else183
  %242 = load %StringRef** %data
  store [6 x i8] c"while\00", [6 x i8]* %const.bytes192
  %243 = getelementptr inbounds [6 x i8]* %const.bytes192, i32 0, i32 0
  %244 = getelementptr inbounds [6 x i8]* %const.bytes192, i32 0, i32 5
  %245 = getelementptr inbounds %StringRef* %const.struct193, i32 0, i32 0
  %246 = getelementptr inbounds %StringRef* %const.struct193, i32 0, i32 1
  store i8* %243, i8** %245
  store i8* %244, i8** %246
  %247 = load %StringRef* %const.struct193
  %248 = call i1 @"==266"(%StringRef* %242, %StringRef %247)
  br i1 %248, label %if_then189, label %if_else190

if_then189:                                       ; preds = %if_block188
  %249 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %249, i32 280)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else190:                                       ; preds = %if_block188
  %250 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %250, i32 299)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_end191:                                        ; preds = %dumy_block195, %dumy_block194
  br label %if_end184

dumy_block194:                                    ; No predecessors!
  br label %if_end191

dumy_block195:                                    ; No predecessors!
  br label %if_end191
}

; Function Attrs: inlinehint nounwind
define private i1 @"==266"(%StringRef* %"$this", %StringRef %other) #3 {
  %"$this.addr" = alloca %StringRef*
  store %StringRef* %"$this", %StringRef** %"$this.addr"
  %other.addr = alloca %StringRef
  store %StringRef %other, %StringRef* %other.addr
  %s = alloca i64
  %i = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef** %"$this.addr"
  %2 = call i64 @size(%StringRef* %1)
  store i64 %2, i64* %s
  br label %if_block

if_block:                                         ; preds = %code
  %3 = load i64* %s
  %4 = call i64 @size(%StringRef* %other.addr)
  %5 = call i1 @"!=267"(i64 %3, i64 %4)
  br i1 %5, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  call void @dtor31(i64* %s)
  ret i1 false

if_end:                                           ; preds = %dumy_block, %if_block
  store i64 0, i64* %i
  br label %while_block

dumy_block:                                       ; No predecessors!
  br label %if_end

while_block:                                      ; preds = %while_step, %if_end
  %6 = load i64* %i
  %7 = load i64* %s
  %8 = call i1 @"<173"(i64 %6, i64 %7)
  br i1 %8, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  br label %if_block1

while_step:                                       ; preds = %if_end3
  %9 = load i64* %i
  %10 = call i64 @"+74"(i64 %9, i32 1)
  store i64 %10, i64* %i
  br label %while_block

while_end:                                        ; preds = %while_block
  call void @dtor31(i64* %i)
  call void @dtor31(i64* %s)
  ret i1 true

if_block1:                                        ; preds = %while_body
  %11 = load %StringRef** %"$this.addr"
  %12 = load i64* %i
  %13 = call i8* @at(%StringRef* %11, i64 %12)
  %14 = load i8* %13
  %15 = load i64* %i
  %16 = call i8* @at(%StringRef* %other.addr, i64 %15)
  %17 = load i8* %16
  %18 = icmp ne i8 %14, %17
  br i1 %18, label %if_then2, label %if_end3

if_then2:                                         ; preds = %if_block1
  call void @dtor31(i64* %i)
  call void @dtor31(i64* %s)
  ret i1 false

if_end3:                                          ; preds = %dumy_block4, %if_block1
  br label %while_step

dumy_block4:                                      ; No predecessors!
  br label %if_end3

dumy_block5:                                      ; No predecessors!
  call void @dtor31(i64* %i)
  call void @dtor31(i64* %s)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i1 @"!=267"(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn30(i64 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn30(i64 %4, i64 %5)
  %7 = icmp ne i64 %3, %6
  ret i1 %7
}

; Function Attrs: alwaysinline nounwind
define private i8* @at(%StringRef* %"$this", i64 %index) #2 {
  %"$this.addr" = alloca %StringRef*
  store %StringRef* %"$this", %StringRef** %"$this.addr"
  %index.addr = alloca i64
  store i64 %index, i64* %index.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef** %"$this.addr"
  %2 = getelementptr inbounds %StringRef* %1, i32 0, i32 0
  %3 = load i8** %2
  %4 = load i64* %index.addr
  %5 = call i8* @ptrAdd(i8* %3, i64 %4)
  ret i8* %5
}

; Function Attrs: noinline nounwind
define void @parseNumeric(%TokenType* sret %_result, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s) #4 {
  %_result.addr = alloca %TokenType*
  store %TokenType* %_result, %TokenType** %_result.addr
  %s.addr = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %isLong = alloca i1
  %isUnsigned = alloca i1
  %isFloating = alloca i1
  %type = alloca i32
  %ch = alloca i8
  %ch2 = alloca i8
  %ch3 = alloca i8
  %funptr = alloca i1 (i8)*
  %funptr48 = alloca i1 (i8)*
  %"$tmpForRef" = alloca %String
  %tmp.v = alloca %String
  %tmp.v82 = alloca i8
  %"$tmpC" = alloca %StringRef
  %tmp.v83 = alloca %TokenType
  %tmp.v84 = alloca %TokenType
  %tmp.v96 = alloca %TokenType
  %tmp.v97 = alloca %TokenType
  %tmp.v109 = alloca %TokenType
  %tmp.v110 = alloca %TokenType
  br label %code

code:                                             ; preds = %0
  store i1 false, i1* %isLong
  store i1 false, i1* %isUnsigned
  store i1 false, i1* %isFloating
  store i32 0, i32* %type
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  %3 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %2)
  store i8 %3, i8* %ch
  %4 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %5 = call i8 @peekChar(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %4)
  %6 = call i8 @toLower(i8 %5)
  store i8 %6, i8* %ch2
  %7 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %8 = call i8 @peekChar265(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %7, i32 2)
  %9 = call i8 @toLower(i8 %8)
  store i8 %9, i8* %ch3
  br label %if_block

if_block:                                         ; preds = %code
  %10 = load i8* %ch
  %11 = icmp eq i8 %10, 48
  br i1 %11, label %cond.true, label %cond.false

if_then:                                          ; preds = %cond.end
  %12 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %13 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %12, i32 0, i32 1
  call void @advance248(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %13, i64 2)
  %14 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %15 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %14, i32 0, i32 1
  %16 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %17 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %16, i32 0, i32 2
  %18 = getelementptr inbounds %Token* %17, i32 0, i32 2
  %19 = call i64 @consumeDigits(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %15, i64 16, %String* %18)
  %20 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %21 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %20, i32 0, i32 2
  %22 = getelementptr inbounds %Token* %21, i32 0, i32 3
  store i64 %19, i64* %22
  br label %if_end

if_else:                                          ; preds = %cond.end
  br label %if_block1

if_end:                                           ; preds = %if_end4, %if_then
  br label %if_block59

cond.true:                                        ; preds = %if_block
  %23 = load i8* %ch2
  %24 = icmp eq i8 %23, 120
  br label %cond.end

cond.false:                                       ; preds = %if_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ %24, %cond.true ], [ false, %cond.false ]
  br i1 %cond.res, label %if_then, label %if_else

if_block1:                                        ; preds = %if_else
  %25 = load i8* %ch
  %26 = icmp eq i8 %25, 48
  br i1 %26, label %cond.true5, label %cond.false6

if_then2:                                         ; preds = %cond.end7
  %27 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %28 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %27, i32 0, i32 1
  call void @advance248(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %28, i64 2)
  %29 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %30 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %29, i32 0, i32 1
  %31 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %32 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %31, i32 0, i32 2
  %33 = getelementptr inbounds %Token* %32, i32 0, i32 2
  %34 = call i64 @consumeDigits(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %30, i64 2, %String* %33)
  %35 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %36 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %35, i32 0, i32 2
  %37 = getelementptr inbounds %Token* %36, i32 0, i32 3
  store i64 %34, i64* %37
  br label %if_end4

if_else3:                                         ; preds = %cond.end7
  br label %if_block9

if_end4:                                          ; preds = %if_end12, %if_then2
  br label %if_end

cond.true5:                                       ; preds = %if_block1
  %38 = load i8* %ch2
  %39 = icmp eq i8 %38, 98
  br label %cond.end7

cond.false6:                                      ; preds = %if_block1
  br label %cond.end7

cond.end7:                                        ; preds = %cond.false6, %cond.true5
  %cond.res8 = phi i1 [ %39, %cond.true5 ], [ false, %cond.false6 ]
  br i1 %cond.res8, label %if_then2, label %if_else3

if_block9:                                        ; preds = %if_else3
  %40 = load i8* %ch
  %41 = icmp eq i8 %40, 48
  br i1 %41, label %cond.true13, label %cond.false14

if_then10:                                        ; preds = %cond.end15
  %42 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %43 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %42, i32 0, i32 1
  call void @advance248(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %43, i64 1)
  %44 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %45 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %44, i32 0, i32 1
  %46 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %47 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %46, i32 0, i32 2
  %48 = getelementptr inbounds %Token* %47, i32 0, i32 2
  %49 = call i64 @consumeDigits(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %45, i64 8, %String* %48)
  %50 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %51 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %50, i32 0, i32 2
  %52 = getelementptr inbounds %Token* %51, i32 0, i32 3
  store i64 %49, i64* %52
  br label %if_end12

if_else11:                                        ; preds = %cond.end15
  %53 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %54 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %53, i32 0, i32 1
  %55 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %56 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %55, i32 0, i32 2
  %57 = getelementptr inbounds %Token* %56, i32 0, i32 2
  %58 = call i64 @consumeDigits(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %54, i64 10, %String* %57)
  %59 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %60 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %59, i32 0, i32 2
  %61 = getelementptr inbounds %Token* %60, i32 0, i32 3
  store i64 %58, i64* %61
  br label %if_block17

if_end12:                                         ; preds = %if_end19, %if_then10
  br label %if_end4

cond.true13:                                      ; preds = %if_block9
  %62 = load i8* %ch2
  %63 = icmp eq i8 %62, 111
  br label %cond.end15

cond.false14:                                     ; preds = %if_block9
  br label %cond.end15

cond.end15:                                       ; preds = %cond.false14, %cond.true13
  %cond.res16 = phi i1 [ %63, %cond.true13 ], [ false, %cond.false14 ]
  br i1 %cond.res16, label %if_then10, label %if_else11

if_block17:                                       ; preds = %if_else11
  %64 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %65 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %64, i32 0, i32 1
  %66 = call i1 @"pre_!!247"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %65)
  br i1 %66, label %if_then18, label %if_end19

if_then18:                                        ; preds = %if_block17
  %67 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %68 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %67, i32 0, i32 1
  %69 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %68)
  %70 = call i8 @toLower(i8 %69)
  store i8 %70, i8* %ch
  br label %if_block20

if_end19:                                         ; preds = %if_end22, %if_block17
  br label %if_end12

if_block20:                                       ; preds = %if_then18
  %71 = load i8* %ch
  %72 = icmp eq i8 %71, 46
  br i1 %72, label %cond.true32, label %cond.false33

if_then21:                                        ; preds = %cond.end25
  store i1 true, i1* %isFloating
  br label %if_block39

if_end22:                                         ; preds = %if_end51, %cond.end25
  br label %if_end19

cond.true23:                                      ; preds = %cond.end28
  br label %cond.end25

cond.false24:                                     ; preds = %cond.end28
  %73 = load i8* %ch
  %74 = icmp eq i8 %73, 100
  br label %cond.end25

cond.end25:                                       ; preds = %cond.false24, %cond.true23
  %cond.res38 = phi i1 [ true, %cond.true23 ], [ %74, %cond.false24 ]
  br i1 %cond.res38, label %if_then21, label %if_end22

cond.true26:                                      ; preds = %cond.end31
  br label %cond.end28

cond.false27:                                     ; preds = %cond.end31
  %75 = load i8* %ch
  %76 = icmp eq i8 %75, 102
  br label %cond.end28

cond.end28:                                       ; preds = %cond.false27, %cond.true26
  %cond.res37 = phi i1 [ true, %cond.true26 ], [ %76, %cond.false27 ]
  br i1 %cond.res37, label %cond.true23, label %cond.false24

cond.true29:                                      ; preds = %cond.end34
  br label %cond.end31

cond.false30:                                     ; preds = %cond.end34
  %77 = load i8* %ch
  %78 = icmp eq i8 %77, 101
  br label %cond.end31

cond.end31:                                       ; preds = %cond.false30, %cond.true29
  %cond.res36 = phi i1 [ true, %cond.true29 ], [ %78, %cond.false30 ]
  br i1 %cond.res36, label %cond.true26, label %cond.false27

cond.true32:                                      ; preds = %if_block20
  %79 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %80 = call i8 @peekChar(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %79)
  %81 = call i1 @isOpCharDot(i8 %80)
  %82 = xor i1 true, %81
  br label %cond.end34

cond.false33:                                     ; preds = %if_block20
  br label %cond.end34

cond.end34:                                       ; preds = %cond.false33, %cond.true32
  %cond.res35 = phi i1 [ %82, %cond.true32 ], [ false, %cond.false33 ]
  br i1 %cond.res35, label %cond.true29, label %cond.false30

if_block39:                                       ; preds = %if_then21
  %83 = load i8* %ch
  %84 = icmp eq i8 %83, 46
  br i1 %84, label %if_then40, label %if_end41

if_then40:                                        ; preds = %if_block39
  %85 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @advanceAndCapture1(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %85)
  %86 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  store i1 (i8)* @isDigit, i1 (i8)** %funptr
  %87 = bitcast i1 (i8)** %funptr to %"FunctionPtr[Bool/rtct, Char/rtct]"*
  %88 = load %"FunctionPtr[Bool/rtct, Char/rtct]"* %87
  call void @advanceAndCaptureDigit(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %86, %"FunctionPtr[Bool/rtct, Char/rtct]" %88)
  br label %if_end41

if_end41:                                         ; preds = %if_then40, %if_block39
  br label %if_block42

if_block42:                                       ; preds = %if_end41
  %89 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %90 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %89, i32 0, i32 1
  %91 = call i1 @"pre_!!247"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %90)
  br i1 %91, label %if_then43, label %if_end44

if_then43:                                        ; preds = %if_block42
  %92 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %93 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %92, i32 0, i32 1
  %94 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %93)
  %95 = call i8 @toLower(i8 %94)
  store i8 %95, i8* %ch
  br label %if_block45

if_end44:                                         ; preds = %if_end47, %if_block42
  br label %if_block49

if_block45:                                       ; preds = %if_then43
  %96 = load i8* %ch
  %97 = icmp eq i8 %96, 101
  br i1 %97, label %if_then46, label %if_end47

if_then46:                                        ; preds = %if_block45
  %98 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @advanceAndCapture1(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %98)
  %99 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  store i1 (i8)* @isDigit, i1 (i8)** %funptr48
  %100 = bitcast i1 (i8)** %funptr48 to %"FunctionPtr[Bool/rtct, Char/rtct]"*
  %101 = load %"FunctionPtr[Bool/rtct, Char/rtct]"* %100
  call void @advanceAndCaptureDigit(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %99, %"FunctionPtr[Bool/rtct, Char/rtct]" %101)
  br label %if_end47

if_end47:                                         ; preds = %if_then46, %if_block45
  br label %if_end44

if_block49:                                       ; preds = %if_end44
  %102 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %103 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %102, i32 0, i32 1
  %104 = call i1 @"pre_!!247"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %103)
  br i1 %104, label %if_then50, label %if_end51

if_then50:                                        ; preds = %if_block49
  store i1 true, i1* %isLong
  %105 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %106 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %105, i32 0, i32 1
  %107 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %106)
  %108 = call i8 @toLower(i8 %107)
  store i8 %108, i8* %ch
  br label %if_block52

if_end51:                                         ; preds = %if_end55, %if_block49
  br label %if_end22

if_block52:                                       ; preds = %if_then50
  %109 = load i8* %ch
  %110 = icmp eq i8 %109, 102
  br i1 %110, label %if_then53, label %if_else54

if_then53:                                        ; preds = %if_block52
  store i1 false, i1* %isLong
  %111 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %112 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %111, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %112)
  br label %if_end55

if_else54:                                        ; preds = %if_block52
  br label %if_block56

if_end55:                                         ; preds = %if_end58, %if_then53
  br label %if_end51

if_block56:                                       ; preds = %if_else54
  %113 = load i8* %ch
  %114 = icmp eq i8 %113, 100
  br i1 %114, label %if_then57, label %if_end58

if_then57:                                        ; preds = %if_block56
  %115 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %116 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %115, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %116)
  br label %if_end58

if_end58:                                         ; preds = %if_then57, %if_block56
  br label %if_end55

if_block59:                                       ; preds = %if_end
  %117 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %118 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %117, i32 0, i32 2
  %119 = getelementptr inbounds %Token* %118, i32 0, i32 2
  %120 = call i1 @isEmpty273(%String* %119)
  br i1 %120, label %if_then60, label %if_end61

if_then60:                                        ; preds = %if_block59
  %121 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  call void @ctorFromCt274(%String* %tmp.v)
  %122 = load %String* %tmp.v
  store %String %122, %String* %"$tmpForRef"
  call void @reportError250(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %121, %String* %"$tmpForRef")
  call void @dtor186(%String* %tmp.v)
  %123 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %123, i32 0)
  call void @dtor162(i8* %ch3)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  call void @dtor58(i32* %type)
  call void @dtor167(i1* %isFloating)
  call void @dtor167(i1* %isUnsigned)
  call void @dtor167(i1* %isLong)
  ret void

if_end61:                                         ; preds = %dumy_block, %if_block59
  br label %if_block62

dumy_block:                                       ; No predecessors!
  br label %if_end61

if_block62:                                       ; preds = %if_end61
  %124 = load i1* %isFloating
  %125 = xor i1 true, %124
  br i1 %125, label %if_then63, label %if_end64

if_then63:                                        ; preds = %if_block62
  br label %if_block65

if_end64:                                         ; preds = %if_end74, %if_block62
  br label %if_block79

if_block65:                                       ; preds = %if_then63
  %126 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %127 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %126, i32 0, i32 1
  %128 = call i1 @"pre_!!247"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %127)
  br i1 %128, label %cond.true68, label %cond.false69

if_then66:                                        ; preds = %cond.end70
  store i1 true, i1* %isUnsigned
  %129 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %130 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %129, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %130)
  br label %if_end67

if_end67:                                         ; preds = %if_then66, %cond.end70
  br label %if_block72

cond.true68:                                      ; preds = %if_block65
  %131 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %132 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %131, i32 0, i32 1
  %133 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %132)
  %134 = call i8 @toLower(i8 %133)
  %135 = icmp eq i8 %134, 117
  br label %cond.end70

cond.false69:                                     ; preds = %if_block65
  br label %cond.end70

cond.end70:                                       ; preds = %cond.false69, %cond.true68
  %cond.res71 = phi i1 [ %135, %cond.true68 ], [ false, %cond.false69 ]
  br i1 %cond.res71, label %if_then66, label %if_end67

if_block72:                                       ; preds = %if_end67
  %136 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %137 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %136, i32 0, i32 1
  %138 = call i1 @"pre_!!247"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %137)
  br i1 %138, label %cond.true75, label %cond.false76

if_then73:                                        ; preds = %cond.end77
  store i1 true, i1* %isLong
  %139 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %140 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %139, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %140)
  br label %if_end74

if_end74:                                         ; preds = %if_then73, %cond.end77
  br label %if_end64

cond.true75:                                      ; preds = %if_block72
  %141 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %142 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %141, i32 0, i32 1
  %143 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %142)
  %144 = call i8 @toLower(i8 %143)
  %145 = icmp eq i8 %144, 108
  br label %cond.end77

cond.false76:                                     ; preds = %if_block72
  br label %cond.end77

cond.end77:                                       ; preds = %cond.false76, %cond.true75
  %cond.res78 = phi i1 [ %145, %cond.true75 ], [ false, %cond.false76 ]
  br i1 %cond.res78, label %if_then73, label %if_end74

if_block79:                                       ; preds = %if_end64
  %146 = load i1* %isFloating
  br i1 %146, label %if_then80, label %if_end81

if_then80:                                        ; preds = %if_block79
  %147 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %148 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %147, i32 0, i32 2
  %149 = getelementptr inbounds %Token* %148, i32 0, i32 2
  store i8 0, i8* %tmp.v82
  %150 = load i8* %tmp.v82
  call void @"+=255"(%String* %149, i8 %150)
  call void @dtor162(i8* %tmp.v82)
  %151 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %152 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %151, i32 0, i32 2
  %153 = getelementptr inbounds %Token* %152, i32 0, i32 2
  call void @asStringRef(%StringRef* %"$tmpC", %String* %153)
  %154 = load %StringRef* %"$tmpC"
  %155 = call double @asDouble(%StringRef %154)
  %156 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %157 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %156, i32 0, i32 2
  %158 = getelementptr inbounds %Token* %157, i32 0, i32 4
  store double %155, double* %158
  call void @dtor78(%StringRef* %"$tmpC")
  %159 = load %TokenType** %_result.addr
  %160 = load i1* %isLong
  br i1 %160, label %cond_alt1, label %cond_alt2

if_end81:                                         ; preds = %cond_destruct_end87, %if_block79
  br label %if_block89

cond_alt1:                                        ; preds = %if_then80
  call void @ctor224(%TokenType* %tmp.v83, i32 308)
  br label %cond_end

cond_alt2:                                        ; preds = %if_then80
  call void @ctor224(%TokenType* %tmp.v84, i32 307)
  br label %cond_end

cond_end:                                         ; preds = %cond_alt2, %cond_alt1
  %cond = phi %TokenType* [ %tmp.v83, %cond_alt1 ], [ %tmp.v84, %cond_alt2 ]
  call void @ctor275(%TokenType* %159, %TokenType* %cond)
  br i1 %160, label %cond_destruct_alt1, label %cond_destruct_alt2

cond_destruct_alt1:                               ; preds = %cond_end
  call void @dtor201(%TokenType* %tmp.v83)
  br label %cond_destruct_end

cond_destruct_alt2:                               ; preds = %cond_end
  call void @dtor201(%TokenType* %tmp.v84)
  br label %cond_destruct_end

cond_destruct_end:                                ; preds = %cond_destruct_alt2, %cond_destruct_alt1
  call void @dtor162(i8* %ch3)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  call void @dtor58(i32* %type)
  call void @dtor167(i1* %isFloating)
  call void @dtor167(i1* %isUnsigned)
  call void @dtor167(i1* %isLong)
  ret void

cond_destruct_alt185:                             ; preds = %dumy_block88
  call void @dtor201(%TokenType* %tmp.v83)
  br label %cond_destruct_end87

cond_destruct_alt286:                             ; preds = %dumy_block88
  call void @dtor201(%TokenType* %tmp.v84)
  br label %cond_destruct_end87

cond_destruct_end87:                              ; preds = %cond_destruct_alt286, %cond_destruct_alt185
  br label %if_end81

dumy_block88:                                     ; No predecessors!
  br i1 %160, label %cond_destruct_alt185, label %cond_destruct_alt286

if_block89:                                       ; preds = %if_end81
  %161 = load i1* %isUnsigned
  br i1 %161, label %if_then90, label %if_else91

if_then90:                                        ; preds = %if_block89
  %162 = load %TokenType** %_result.addr
  %163 = load i1* %isLong
  br i1 %163, label %cond_alt193, label %cond_alt294

if_else91:                                        ; preds = %if_block89
  %164 = load %TokenType** %_result.addr
  %165 = load i1* %isLong
  br i1 %165, label %cond_alt1106, label %cond_alt2107

if_end92:                                         ; preds = %cond_destruct_end117, %cond_destruct_end104
  call void @dtor162(i8* %ch3)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  call void @dtor58(i32* %type)
  call void @dtor167(i1* %isFloating)
  call void @dtor167(i1* %isUnsigned)
  call void @dtor167(i1* %isLong)
  ret void

cond_alt193:                                      ; preds = %if_then90
  call void @ctor224(%TokenType* %tmp.v96, i32 306)
  br label %cond_end95

cond_alt294:                                      ; preds = %if_then90
  call void @ctor224(%TokenType* %tmp.v97, i32 305)
  br label %cond_end95

cond_end95:                                       ; preds = %cond_alt294, %cond_alt193
  %cond98 = phi %TokenType* [ %tmp.v96, %cond_alt193 ], [ %tmp.v97, %cond_alt294 ]
  call void @ctor275(%TokenType* %162, %TokenType* %cond98)
  br i1 %163, label %cond_destruct_alt199, label %cond_destruct_alt2100

cond_destruct_alt199:                             ; preds = %cond_end95
  call void @dtor201(%TokenType* %tmp.v96)
  br label %cond_destruct_end101

cond_destruct_alt2100:                            ; preds = %cond_end95
  call void @dtor201(%TokenType* %tmp.v97)
  br label %cond_destruct_end101

cond_destruct_end101:                             ; preds = %cond_destruct_alt2100, %cond_destruct_alt199
  call void @dtor162(i8* %ch3)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  call void @dtor58(i32* %type)
  call void @dtor167(i1* %isFloating)
  call void @dtor167(i1* %isUnsigned)
  call void @dtor167(i1* %isLong)
  ret void

cond_destruct_alt1102:                            ; preds = %dumy_block105
  call void @dtor201(%TokenType* %tmp.v96)
  br label %cond_destruct_end104

cond_destruct_alt2103:                            ; preds = %dumy_block105
  call void @dtor201(%TokenType* %tmp.v97)
  br label %cond_destruct_end104

cond_destruct_end104:                             ; preds = %cond_destruct_alt2103, %cond_destruct_alt1102
  br label %if_end92

dumy_block105:                                    ; No predecessors!
  br i1 %163, label %cond_destruct_alt1102, label %cond_destruct_alt2103

cond_alt1106:                                     ; preds = %if_else91
  call void @ctor224(%TokenType* %tmp.v109, i32 304)
  br label %cond_end108

cond_alt2107:                                     ; preds = %if_else91
  call void @ctor224(%TokenType* %tmp.v110, i32 303)
  br label %cond_end108

cond_end108:                                      ; preds = %cond_alt2107, %cond_alt1106
  %cond111 = phi %TokenType* [ %tmp.v109, %cond_alt1106 ], [ %tmp.v110, %cond_alt2107 ]
  call void @ctor275(%TokenType* %164, %TokenType* %cond111)
  br i1 %165, label %cond_destruct_alt1112, label %cond_destruct_alt2113

cond_destruct_alt1112:                            ; preds = %cond_end108
  call void @dtor201(%TokenType* %tmp.v109)
  br label %cond_destruct_end114

cond_destruct_alt2113:                            ; preds = %cond_end108
  call void @dtor201(%TokenType* %tmp.v110)
  br label %cond_destruct_end114

cond_destruct_end114:                             ; preds = %cond_destruct_alt2113, %cond_destruct_alt1112
  call void @dtor162(i8* %ch3)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  call void @dtor58(i32* %type)
  call void @dtor167(i1* %isFloating)
  call void @dtor167(i1* %isUnsigned)
  call void @dtor167(i1* %isLong)
  ret void

cond_destruct_alt1115:                            ; preds = %dumy_block118
  call void @dtor201(%TokenType* %tmp.v109)
  br label %cond_destruct_end117

cond_destruct_alt2116:                            ; preds = %dumy_block118
  call void @dtor201(%TokenType* %tmp.v110)
  br label %cond_destruct_end117

cond_destruct_end117:                             ; preds = %cond_destruct_alt2116, %cond_destruct_alt1115
  br label %if_end92

dumy_block118:                                    ; No predecessors!
  br i1 %165, label %cond_destruct_alt1115, label %cond_destruct_alt2116
}

; Function Attrs: inlinehint nounwind
define private i64 @consumeDigits(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %r, i64 %base, %String* %capture) #3 {
  %r.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %r, %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  %base.addr = alloca i64
  store i64 %base, i64* %base.addr
  %capture.addr = alloca %String*
  store %String* %capture, %String** %capture.addr
  %res = alloca i64
  br label %code

code:                                             ; preds = %0
  store i64 0, i64* %res
  br label %if_block

if_block:                                         ; preds = %code
  %1 = load i64* %base.addr
  %2 = call i1 @"==268"(i64 %1, i32 16)
  br i1 %2, label %if_then, label %if_else

if_then:                                          ; preds = %if_block
  br label %while_block

if_else:                                          ; preds = %if_block
  br label %while_block8

if_end:                                           ; preds = %while_end11, %while_end
  %3 = load i64* %res
  call void @dtor27(i64* %res)
  ret i64 %3

while_block:                                      ; preds = %while_step, %if_then
  %4 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  %5 = call i1 @"pre_!!247"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %4)
  br i1 %5, label %cond.true, label %cond.false

while_body:                                       ; preds = %cond.end
  br label %if_block5

while_step:                                       ; preds = %if_end7
  %6 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %6)
  br label %while_block

while_end:                                        ; preds = %cond.end
  br label %if_end

cond.true:                                        ; preds = %while_block
  %7 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  %8 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %7)
  %9 = call i1 @isXdigit(i8 %8)
  br i1 %9, label %cond.true1, label %cond.false2

cond.false:                                       ; preds = %while_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.end3
  %cond.res4 = phi i1 [ %cond.res, %cond.end3 ], [ false, %cond.false ]
  br i1 %cond.res4, label %while_body, label %while_end

cond.true1:                                       ; preds = %cond.true
  br label %cond.end3

cond.false2:                                      ; preds = %cond.true
  %10 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  %11 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %10)
  %12 = icmp eq i8 %11, 95
  br label %cond.end3

cond.end3:                                        ; preds = %cond.false2, %cond.true1
  %cond.res = phi i1 [ true, %cond.true1 ], [ %12, %cond.false2 ]
  br label %cond.end

if_block5:                                        ; preds = %while_body
  %13 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  %14 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %13)
  %15 = icmp ne i8 %14, 95
  br i1 %15, label %if_then6, label %if_end7

if_then6:                                         ; preds = %if_block5
  %16 = load %String** %capture.addr
  %17 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  %18 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %17)
  call void @"+=255"(%String* %16, i8 %18)
  %19 = load i64* %res
  %20 = load i64* %base.addr
  %21 = call i64 @"*271"(i64 %19, i64 %20)
  store i64 %21, i64* %res
  %22 = load i64* %res
  %23 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  %24 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %23)
  %25 = call i32 @getXdigitVal(i8 %24)
  %26 = call i64 @"+272"(i64 %22, i32 %25)
  store i64 %26, i64* %res
  br label %if_end7

if_end7:                                          ; preds = %if_then6, %if_block5
  br label %while_step

while_block8:                                     ; preds = %while_step10, %if_else
  %27 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  %28 = call i1 @"pre_!!247"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %27)
  br i1 %28, label %cond.true12, label %cond.false13

while_body9:                                      ; preds = %cond.end14
  br label %if_block20

while_step10:                                     ; preds = %if_end22
  %29 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %29)
  br label %while_block8

while_end11:                                      ; preds = %cond.end14
  br label %if_end

cond.true12:                                      ; preds = %while_block8
  %30 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  %31 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %30)
  %32 = call i1 @isXdigit(i8 %31)
  br i1 %32, label %cond.true15, label %cond.false16

cond.false13:                                     ; preds = %while_block8
  br label %cond.end14

cond.end14:                                       ; preds = %cond.false13, %cond.end17
  %cond.res19 = phi i1 [ %cond.res18, %cond.end17 ], [ false, %cond.false13 ]
  br i1 %cond.res19, label %while_body9, label %while_end11

cond.true15:                                      ; preds = %cond.true12
  br label %cond.end17

cond.false16:                                     ; preds = %cond.true12
  %33 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  %34 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %33)
  %35 = icmp eq i8 %34, 95
  br label %cond.end17

cond.end17:                                       ; preds = %cond.false16, %cond.true15
  %cond.res18 = phi i1 [ true, %cond.true15 ], [ %35, %cond.false16 ]
  br label %cond.end14

if_block20:                                       ; preds = %while_body9
  %36 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  %37 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %36)
  %38 = icmp ne i8 %37, 95
  br i1 %38, label %if_then21, label %if_end22

if_then21:                                        ; preds = %if_block20
  %39 = load %String** %capture.addr
  %40 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  %41 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %40)
  call void @"+=255"(%String* %39, i8 %41)
  %42 = load i64* %res
  %43 = load i64* %base.addr
  %44 = call i64 @"*271"(i64 %42, i64 %43)
  store i64 %44, i64* %res
  %45 = load i64* %res
  %46 = load %"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"** %r.addr
  %47 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %46)
  %48 = call i32 @getDigitVal(i8 %47)
  %49 = call i64 @"+272"(i64 %45, i32 %48)
  store i64 %49, i64* %res
  br label %if_end22

if_end22:                                         ; preds = %if_then21, %if_block20
  br label %while_step10

dumy_block:                                       ; No predecessors!
  call void @dtor27(i64* %res)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i1 @"==268"(i64 %x, i32 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i32* %y.addr
  %3 = call i64 @cmn269(i64 %1, i32 %2)
  %4 = load i32* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn270(i32 %4, i64 %5)
  %7 = icmp eq i64 %3, %6
  ret i1 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @cmn269(i64 %x, i32 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  ret i64 %1
}

; Function Attrs: alwaysinline nounwind
define private i64 @cmn270(i32 %x, i64 %y) #2 {
  %x.addr = alloca i32
  store i32 %x, i32* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load i32* %x.addr
  %2 = zext i32 %1 to i64
  store i64 %2, i64* %tmp.v
  %3 = load i64* %tmp.v
  call void @dtor27(i64* %tmp.v)
  ret i64 %3

dumy_block:                                       ; No predecessors!
  call void @dtor27(i64* %tmp.v)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i1 @isXdigit(i8 %c) #2 {
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  %tmp.v = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  call void @_ass_32_8z(i32* %tmp.v, i8 %1)
  %2 = load i32* %tmp.v
  %3 = call i32 @isxdigit(i32 %2)
  %4 = call i1 @"!=163"(i32 0, i32 %3)
  call void @dtor58(i32* %tmp.v)
  ret i1 %4

dumy_block:                                       ; No predecessors!
  call void @dtor58(i32* %tmp.v)
  unreachable
}

declare i32 @isxdigit(i32)

; Function Attrs: alwaysinline nounwind
define private i64 @"*271"(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn26(i64 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn26(i64 %4, i64 %5)
  %7 = mul i64 %3, %6
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i64 @"+272"(i64 %x, i32 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i32* %y.addr
  %3 = call i64 @cmn269(i64 %1, i32 %2)
  %4 = load i32* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn270(i32 %4, i64 %5)
  %7 = add i64 %3, %6
  ret i64 %7
}

; Function Attrs: alwaysinline nounwind
define private i32 @getDigitVal(i8 %c) #2 {
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  %tmp.v = alloca i32
  %tmp.v1 = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  call void @_ass_32_8z(i32* %tmp.v, i8 %1)
  %2 = load i32* %tmp.v
  call void @_ass_32_8z(i32* %tmp.v1, i8 48)
  %3 = load i32* %tmp.v1
  %4 = call i32 @-43(i32 %2, i32 %3)
  call void @dtor58(i32* %tmp.v1)
  call void @dtor58(i32* %tmp.v)
  ret i32 %4

dumy_block:                                       ; No predecessors!
  call void @dtor58(i32* %tmp.v1)
  call void @dtor58(i32* %tmp.v)
  unreachable
}

; Function Attrs: inlinehint nounwind
define private void @advanceAndCaptureDigit(%"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, %"FunctionPtr[Bool/rtct, Char/rtct]" %pred) #3 {
  %s.addr = alloca %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %pred.addr = alloca %"FunctionPtr[Bool/rtct, Char/rtct]"
  store %"FunctionPtr[Bool/rtct, Char/rtct]" %pred, %"FunctionPtr[Bool/rtct, Char/rtct]"* %pred.addr
  br label %code

code:                                             ; preds = %0
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %1 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  %3 = call i1 @"pre_!!247"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %2)
  br i1 %3, label %cond.true, label %cond.false

while_body:                                       ; preds = %cond.end
  br label %if_block

while_step:                                       ; preds = %if_end
  %4 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %5 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %4, i32 0, i32 1
  call void @popFront233(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %5)
  br label %while_block

while_end:                                        ; preds = %cond.end
  ret void

cond.true:                                        ; preds = %while_block
  %6 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %7 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %6, i32 0, i32 1
  %8 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %7)
  %9 = call i1 @"()232"(%"FunctionPtr[Bool/rtct, Char/rtct]"* %pred.addr, i8 %8)
  br i1 %9, label %cond.true1, label %cond.false2

cond.false:                                       ; preds = %while_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.end3
  %cond.res4 = phi i1 [ %cond.res, %cond.end3 ], [ false, %cond.false ]
  br i1 %cond.res4, label %while_body, label %while_end

cond.true1:                                       ; preds = %cond.true
  br label %cond.end3

cond.false2:                                      ; preds = %cond.true
  %10 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %11 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %10, i32 0, i32 1
  %12 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %11)
  %13 = icmp eq i8 %12, 95
  br label %cond.end3

cond.end3:                                        ; preds = %cond.false2, %cond.true1
  %cond.res = phi i1 [ true, %cond.true1 ], [ %13, %cond.false2 ]
  br label %cond.end

if_block:                                         ; preds = %while_body
  %14 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %15 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %14, i32 0, i32 1
  %16 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %15)
  %17 = icmp ne i8 %16, 95
  br i1 %17, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %18 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %19 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %18, i32 0, i32 2
  %20 = getelementptr inbounds %Token* %19, i32 0, i32 2
  %21 = load %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"** %s.addr
  %22 = getelementptr inbounds %"SparrowScanner[FileRange/rtct, ExternalErrorReporter]"* %21, i32 0, i32 1
  %23 = call i8 @"pre_*"(%"LocationSyncCharRange[RangeWithLookahead[FileRange/rtct]]"* %22)
  call void @"+=255"(%String* %20, i8 %23)
  br label %if_end

if_end:                                           ; preds = %if_then, %if_block
  br label %while_step
}

; Function Attrs: alwaysinline nounwind
define private i1 @isEmpty273(%String* %"$this") #2 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  %2 = getelementptr inbounds %String* %1, i32 0, i32 0
  %3 = load %String** %"$this.addr"
  %4 = getelementptr inbounds %String* %3, i32 0, i32 1
  %5 = load %"RawPtr[Char/rtct]"* %4
  %6 = call i1 @"==148"(%"RawPtr[Char/rtct]"* %2, %"RawPtr[Char/rtct]" %5)
  ret i1 %6
}

; Function Attrs: inlinehint nounwind
define private void @ctorFromCt274(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [24 x i8]
  %const.struct = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  store [24 x i8] c"Invalid numeric literal\00", [24 x i8]* %const.bytes
  %2 = getelementptr inbounds [24 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [24 x i8]* %const.bytes, i32 0, i32 23
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  call void @ctor251(%String* %1, %StringRef* %"$tmpForRef")
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor275(%TokenType* %"$this", %TokenType* %other) #2 {
  %"$this.addr" = alloca %TokenType*
  store %TokenType* %"$this", %TokenType** %"$this.addr"
  %other.addr = alloca %TokenType*
  store %TokenType* %other, %TokenType** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %TokenType** %other.addr
  %2 = getelementptr inbounds %TokenType* %1, i32 0, i32 0
  %3 = load i32* %2
  %4 = load %TokenType** %"$this.addr"
  %5 = getelementptr inbounds %TokenType* %4, i32 0, i32 0
  store i32 %3, i32* %5
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @toString276(%String* sret %_result, %StringRef %a1, i8 %a2, %StringRef %a3, i32 %a4) #3 {
  %_result.addr = alloca %String*
  store %String* %_result, %String** %_result.addr
  %a1.addr = alloca %StringRef
  store %StringRef %a1, %StringRef* %a1.addr
  %a2.addr = alloca i8
  store i8 %a2, i8* %a2.addr
  %a3.addr = alloca %StringRef
  store %StringRef %a3, %StringRef* %a3.addr
  %a4.addr = alloca i32
  store i32 %a4, i32* %a4.addr
  %s = alloca %StringOutputStream
  br label %code

code:                                             ; preds = %0
  call void @ctor169(%StringOutputStream* %s)
  %1 = call %StringOutputStream* @"<<"(%StringOutputStream* %s, %StringRef* %a1.addr)
  %2 = call %StringOutputStream* @"<<280"(%StringOutputStream* %1, i8* %a2.addr)
  %3 = call %StringOutputStream* @"<<"(%StringOutputStream* %2, %StringRef* %a3.addr)
  %4 = call %StringOutputStream* @"<<277"(%StringOutputStream* %3, i32* %a4.addr)
  %5 = load %String** %_result.addr
  %6 = getelementptr inbounds %StringOutputStream* %s, i32 0, i32 0
  call void @ctor183(%String* %5, %String* %6)
  call void @dtor185(%StringOutputStream* %s)
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor185(%StringOutputStream* %s)
  ret void
}

; Function Attrs: inlinehint nounwind
define private %StringOutputStream* @"<<277"(%StringOutputStream* %s, i32* %x) #3 {
  %s.addr = alloca %StringOutputStream*
  store %StringOutputStream* %s, %StringOutputStream** %s.addr
  %x.addr = alloca i32*
  store i32* %x, i32** %x.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %StringOutputStream** %s.addr
  %2 = load i32** %x.addr
  %3 = load i32* %2
  call void @"<<<278"(%StringOutputStream* %1, i32 %3)
  %4 = load %StringOutputStream** %s.addr
  ret %StringOutputStream* %4
}

; Function Attrs: alwaysinline nounwind
define private void @"<<<278"(%StringOutputStream* %"$this", i32 %x) #2 {
  %"$this.addr" = alloca %StringOutputStream*
  store %StringOutputStream* %"$this", %StringOutputStream** %"$this.addr"
  %x.addr = alloca i32
  store i32 %x, i32* %x.addr
  %"$tmpC" = alloca %"ContiguousMemoryRange[Char/rtct]"
  %"$tmpC1" = alloca %String
  br label %code

code:                                             ; preds = %0
  %1 = load %StringOutputStream** %"$this.addr"
  %2 = getelementptr inbounds %StringOutputStream* %1, i32 0, i32 0
  %3 = load i32* %x.addr
  call void @intToString(%String* %"$tmpC1", i32 %3)
  call void @all279(%"ContiguousMemoryRange[Char/rtct]"* %"$tmpC", %String* %"$tmpC1")
  %4 = load %"ContiguousMemoryRange[Char/rtct]"* %"$tmpC"
  call void @append(%String* %2, %"ContiguousMemoryRange[Char/rtct]" %4)
  call void @dtor182(%"ContiguousMemoryRange[Char/rtct]"* %"$tmpC")
  call void @dtor186(%String* %"$tmpC1")
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @all279(%"ContiguousMemoryRange[Char/rtct]"* sret %_result, %String* %"$this") #2 {
  %_result.addr = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %_result, %"ContiguousMemoryRange[Char/rtct]"** %_result.addr
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"ContiguousMemoryRange[Char/rtct]"** %_result.addr
  %2 = load %String** %"$this.addr"
  %3 = getelementptr inbounds %String* %2, i32 0, i32 0
  %4 = load %"RawPtr[Char/rtct]"* %3
  %5 = load %String** %"$this.addr"
  %6 = getelementptr inbounds %String* %5, i32 0, i32 1
  %7 = load %"RawPtr[Char/rtct]"* %6
  call void @ctor181(%"ContiguousMemoryRange[Char/rtct]"* %1, %"RawPtr[Char/rtct]" %4, %"RawPtr[Char/rtct]" %7)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @intToString(%String* sret %_result, i32 %x) #3 {
  %_result.addr = alloca %String*
  store %String* %_result, %String** %_result.addr
  %x.addr = alloca i32
  store i32 %x, i32* %x.addr
  %buf = alloca [12 x i8]
  %"$tmpC" = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = bitcast [12 x i8]* %buf to i8*
  store i8 0, i8* %1
  %2 = load i32* %x.addr
  %3 = bitcast [12 x i8]* %buf to i8*
  call void @_Int_to_CString(i32 %2, i8* %3)
  %4 = load %String** %_result.addr
  %5 = bitcast [12 x i8]* %buf to i8*
  call void @_String_fromCString(%StringRef* %"$tmpC", i8* %5)
  call void @ctor251(%String* %4, %StringRef* %"$tmpC")
  call void @dtor78(%StringRef* %"$tmpC")
  %6 = bitcast [12 x i8]* %buf to i8*
  call void @dtor(i8* %6)
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor78(%StringRef* %"$tmpC")
  %7 = bitcast [12 x i8]* %buf to i8*
  call void @dtor(i8* %7)
  ret void
}

; Function Attrs: inlinehint nounwind
define private %StringOutputStream* @"<<280"(%StringOutputStream* %s, i8* %x) #3 {
  %s.addr = alloca %StringOutputStream*
  store %StringOutputStream* %s, %StringOutputStream** %s.addr
  %x.addr = alloca i8*
  store i8* %x, i8** %x.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %StringOutputStream** %s.addr
  %2 = load i8** %x.addr
  %3 = load i8* %2
  call void @"<<<281"(%StringOutputStream* %1, i8 %3)
  %4 = load %StringOutputStream** %s.addr
  ret %StringOutputStream* %4
}

; Function Attrs: alwaysinline nounwind
define private void @"<<<281"(%StringOutputStream* %"$this", i8 %x) #2 {
  %"$this.addr" = alloca %StringOutputStream*
  store %StringOutputStream* %"$this", %StringOutputStream** %"$this.addr"
  %x.addr = alloca i8
  store i8 %x, i8* %x.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %StringOutputStream** %"$this.addr"
  %2 = getelementptr inbounds %StringOutputStream* %1, i32 0, i32 0
  %3 = load i8* %x.addr
  call void @append282(%String* %2, i8 %3)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @append282(%String* %"$this", i8 %value) #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %value.addr = alloca i8
  store i8 %value, i8* %value.addr
  %tmp.v = alloca %"ContiguousMemoryRange[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  %2 = load i8* %value.addr
  %3 = load %String** %"$this.addr"
  %4 = getelementptr inbounds %String* %3, i32 0, i32 1
  %5 = load %"RawPtr[Char/rtct]"* %4
  %6 = load %String** %"$this.addr"
  %7 = getelementptr inbounds %String* %6, i32 0, i32 1
  %8 = load %"RawPtr[Char/rtct]"* %7
  call void @ctor181(%"ContiguousMemoryRange[Char/rtct]"* %tmp.v, %"RawPtr[Char/rtct]" %5, %"RawPtr[Char/rtct]" %8)
  %9 = load %"ContiguousMemoryRange[Char/rtct]"* %tmp.v
  call void @insertBefore283(%String* %1, i8 %2, %"ContiguousMemoryRange[Char/rtct]" %9)
  call void @dtor182(%"ContiguousMemoryRange[Char/rtct]"* %tmp.v)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @insertBefore283(%String* %"$this", i8 %value, %"ContiguousMemoryRange[Char/rtct]" %pos) #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %value.addr = alloca i8
  store i8 %value, i8* %value.addr
  %pos.addr = alloca %"ContiguousMemoryRange[Char/rtct]"
  store %"ContiguousMemoryRange[Char/rtct]" %pos, %"ContiguousMemoryRange[Char/rtct]"* %pos.addr
  %posCount = alloca i64
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  %p = alloca %"RawPtr[Char/rtct]"
  %q = alloca %"RawPtr[Char/rtct]"
  %tmp.v = alloca i64
  %"$tmpC1" = alloca %"RawPtr[Char/rtct]"
  %"$tmpC2" = alloca %"RawPtr[Char/rtct]"
  %"$tmpC3" = alloca %"RawPtr[Char/rtct]"
  %"$tmpC4" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  call void @frontPtr(%"RawPtr[Char/rtct]"* %"$tmpC", %"ContiguousMemoryRange[Char/rtct]"* %pos.addr)
  %1 = load %String** %"$this.addr"
  %2 = getelementptr inbounds %String* %1, i32 0, i32 0
  %3 = load %"RawPtr[Char/rtct]"* %2
  %4 = call i64 @diff(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]" %3)
  store i64 %4, i64* %posCount
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  %5 = load %String** %"$this.addr"
  %6 = load %String** %"$this.addr"
  %7 = call i64 @size174(%String* %6)
  %8 = call i64 @"+74"(i64 %7, i32 1)
  call void @reserve171(%String* %5, i64 %8)
  %9 = load %String** %"$this.addr"
  %10 = getelementptr inbounds %String* %9, i32 0, i32 1
  call void @advance175(%"RawPtr[Char/rtct]"* %p, %"RawPtr[Char/rtct]"* %10, i64 -1)
  %11 = load %String** %"$this.addr"
  %12 = getelementptr inbounds %String* %11, i32 0, i32 0
  %13 = load i64* %posCount
  store i64 1, i64* %tmp.v
  %14 = load i64* %tmp.v
  %15 = call i64 @-53(i64 %13, i64 %14)
  call void @advance175(%"RawPtr[Char/rtct]"* %q, %"RawPtr[Char/rtct]"* %12, i64 %15)
  call void @dtor35(i64* %tmp.v)
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %16 = load %"RawPtr[Char/rtct]"* %q
  %17 = call i1 @"==148"(%"RawPtr[Char/rtct]"* %p, %"RawPtr[Char/rtct]" %16)
  %18 = xor i1 true, %17
  br i1 %18, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  %19 = call i8* @value(%"RawPtr[Char/rtct]"* %p)
  %20 = load i8* %19
  call void @advance149(%"RawPtr[Char/rtct]"* %"$tmpC1", %"RawPtr[Char/rtct]"* %p)
  %21 = call i8* @value(%"RawPtr[Char/rtct]"* %"$tmpC1")
  store i8 %20, i8* %21
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC1")
  %22 = call i8* @value(%"RawPtr[Char/rtct]"* %p)
  call void @dtor162(i8* %22)
  br label %while_step

while_step:                                       ; preds = %while_body
  call void @advance175(%"RawPtr[Char/rtct]"* %"$tmpC2", %"RawPtr[Char/rtct]"* %p, i64 -1)
  %23 = load %"RawPtr[Char/rtct]"* %"$tmpC2"
  call void @"=144"(%"RawPtr[Char/rtct]"* %p, %"RawPtr[Char/rtct]" %23)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC2")
  br label %while_block

while_end:                                        ; preds = %while_block
  %24 = load i8* %value.addr
  call void @advance149(%"RawPtr[Char/rtct]"* %"$tmpC3", %"RawPtr[Char/rtct]"* %p)
  %25 = call i8* @value(%"RawPtr[Char/rtct]"* %"$tmpC3")
  store i8 %24, i8* %25
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC3")
  %26 = load %String** %"$this.addr"
  %27 = getelementptr inbounds %String* %26, i32 0, i32 1
  %28 = load %String** %"$this.addr"
  %29 = getelementptr inbounds %String* %28, i32 0, i32 1
  call void @advance149(%"RawPtr[Char/rtct]"* %"$tmpC4", %"RawPtr[Char/rtct]"* %29)
  %30 = load %"RawPtr[Char/rtct]"* %"$tmpC4"
  call void @"=144"(%"RawPtr[Char/rtct]"* %27, %"RawPtr[Char/rtct]" %30)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC4")
  call void @dtor146(%"RawPtr[Char/rtct]"* %q)
  call void @dtor146(%"RawPtr[Char/rtct]"* %p)
  call void @dtor35(i64* %posCount)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i1 @"==284"(%TokenType* %"$this", %TokenType* %other) #2 {
  %"$this.addr" = alloca %TokenType*
  store %TokenType* %"$this", %TokenType** %"$this.addr"
  %other.addr = alloca %TokenType*
  store %TokenType* %other, %TokenType** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %TokenType** %"$this.addr"
  %2 = getelementptr inbounds %TokenType* %1, i32 0, i32 0
  %3 = load i32* %2
  %4 = load %TokenType** %other.addr
  %5 = getelementptr inbounds %TokenType* %4, i32 0, i32 0
  %6 = load i32* %5
  %7 = call i1 @"==285"(i32 %3, i32 %6)
  ret i1 %7
}

; Function Attrs: alwaysinline nounwind
define private i1 @"==285"(i32 %x, i32 %y) #2 {
  %x.addr = alloca i32
  store i32 %x, i32* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i32* %x.addr
  %2 = load i32* %y.addr
  %3 = call i32 @cmn15(i32 %1, i32 %2)
  %4 = load i32* %y.addr
  %5 = load i32* %x.addr
  %6 = call i32 @cmn15(i32 %4, i32 %5)
  %7 = icmp eq i32 %3, %6
  ret i1 %7
}

; Function Attrs: alwaysinline nounwind
define private void @ctor286(%Token* %"$this", %Token* %other) #2 {
  %"$this.addr" = alloca %Token*
  store %Token* %"$this", %Token** %"$this.addr"
  %other.addr = alloca %Token*
  store %Token* %other, %Token** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %Token** %"$this.addr"
  %2 = getelementptr inbounds %Token* %1, i32 0, i32 0
  %3 = load %Token** %other.addr
  %4 = getelementptr inbounds %Token* %3, i32 0, i32 0
  call void @ctor134(%Location* %2, %Location* %4)
  %5 = load %Token** %"$this.addr"
  %6 = getelementptr inbounds %Token* %5, i32 0, i32 1
  %7 = load %Token** %other.addr
  %8 = getelementptr inbounds %Token* %7, i32 0, i32 1
  call void @ctor275(%TokenType* %6, %TokenType* %8)
  %9 = load %Token** %"$this.addr"
  %10 = getelementptr inbounds %Token* %9, i32 0, i32 2
  %11 = load %Token** %other.addr
  %12 = getelementptr inbounds %Token* %11, i32 0, i32 2
  call void @ctor183(%String* %10, %String* %12)
  %13 = load %Token** %other.addr
  %14 = getelementptr inbounds %Token* %13, i32 0, i32 3
  %15 = load i64* %14
  %16 = load %Token** %"$this.addr"
  %17 = getelementptr inbounds %Token* %16, i32 0, i32 3
  store i64 %15, i64* %17
  %18 = load %Token** %other.addr
  %19 = getelementptr inbounds %Token* %18, i32 0, i32 4
  %20 = load double* %19
  %21 = load %Token** %"$this.addr"
  %22 = getelementptr inbounds %Token* %21, i32 0, i32 4
  store double %20, double* %22
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @"post_++287"(%Token* sret %_result, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %r) #3 {
  %_result.addr = alloca %Token*
  store %Token* %_result, %Token** %_result.addr
  %r.addr = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %r, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %r.addr
  %res = alloca %Token
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %r.addr
  call void @front288(%Token* %res, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1)
  %2 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %r.addr
  call void @popFront289(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %2)
  %3 = load %Token** %_result.addr
  call void @ctor286(%Token* %3, %Token* %res)
  call void @dtor200(%Token* %res)
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor200(%Token* %res)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @front288(%Token* sret %_result, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s) #3 {
  %_result.addr = alloca %Token*
  store %Token* %_result, %Token** %_result.addr
  %s.addr = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  br label %code

code:                                             ; preds = %0
  br label %if_block

if_block:                                         ; preds = %code
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1, i32 0, i32 3
  %3 = load i1* %2
  %4 = xor i1 true, %3
  br i1 %4, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %5 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %6 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %5, i32 0, i32 3
  store i1 true, i1* %6
  %7 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @popFront289(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %7)
  br label %if_end

if_end:                                           ; preds = %if_then, %if_block
  %8 = load %Token** %_result.addr
  %9 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %10 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %9, i32 0, i32 2
  call void @ctor286(%Token* %8, %Token* %10)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @popFront289(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s) #3 {
  %s.addr = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %"$tmpC" = alloca %TokenType
  %tmp.v = alloca %TokenType
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1, i32 0, i32 2
  %3 = getelementptr inbounds %Token* %2, i32 0, i32 1
  %4 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @nextToken290(%TokenType* %"$tmpC", %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %4)
  call void @"=215"(%TokenType* %3, %TokenType* %"$tmpC")
  call void @dtor201(%TokenType* %"$tmpC")
  %5 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %6 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %5, i32 0, i32 2
  %7 = getelementptr inbounds %Token* %6, i32 0, i32 0
  %8 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %9 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %8, i32 0, i32 1
  %10 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %9, i32 0, i32 1
  %11 = load %Location** %10
  call void @"=211"(%Location* %7, %Location* %11)
  %12 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %13 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %12, i32 0, i32 2
  %14 = getelementptr inbounds %Token* %13, i32 0, i32 1
  call void @ctor224(%TokenType* %tmp.v, i32 0)
  %15 = call i1 @"==284"(%TokenType* %14, %TokenType* %tmp.v)
  %16 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %17 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %16, i32 0, i32 4
  store i1 %15, i1* %17
  call void @dtor201(%TokenType* %tmp.v)
  ret void
}

; Function Attrs: noinline nounwind
define void @nextToken290(%TokenType* sret %_result, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s) #4 {
  %_result.addr = alloca %TokenType*
  store %TokenType* %_result, %TokenType** %_result.addr
  %s.addr = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %loc = alloca %Location*
  %ch = alloca i8
  %ch2 = alloca i8
  %funptr = alloca i1 (i8)*
  %tmp.v = alloca %"$lambdaEnclosure.0"
  %c1 = alloca i8
  %c2 = alloca i8
  %"$tmpForRef" = alloca %String
  %tmp.v34 = alloca %String
  %"$tmpC" = alloca %String
  %const.bytes = alloca [27 x i8]
  %const.struct = alloca %StringRef
  %const.bytes140 = alloca [2 x i8]
  %const.struct141 = alloca %StringRef
  %tmp.v142 = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  %3 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %2, i32 0, i32 1
  %4 = load %Location** %3
  store %Location* %4, %Location** %loc
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  br i1 true, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  %5 = load %Location** %loc
  call void @stepOver(%Location* %5)
  %6 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %7 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %6, i32 0, i32 2
  %8 = getelementptr inbounds %Token* %7, i32 0, i32 2
  call void @clear(%String* %8)
  br label %if_block

while_step:                                       ; preds = %dumy_block143, %if_end33, %if_end10, %if_then2
  br label %while_block

while_end:                                        ; preds = %while_block
  ret void

if_block:                                         ; preds = %while_body
  %9 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %10 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %9, i32 0, i32 1
  %11 = call i1 @isEmpty291(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %10)
  br i1 %11, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %12 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %12, i32 0)
  ret void

if_end:                                           ; preds = %dumy_block, %if_block
  %13 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %14 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %13, i32 0, i32 1
  %15 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %14)
  store i8 %15, i8* %ch
  %16 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %17 = call i8 @peekChar296(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %16)
  store i8 %17, i8* %ch2
  br label %if_block1

dumy_block:                                       ; No predecessors!
  br label %if_end

if_block1:                                        ; preds = %if_end
  %18 = load i8* %ch
  %19 = call i1 @isSpace(i8 %18)
  br i1 %19, label %if_then2, label %if_end3

if_then2:                                         ; preds = %if_block1
  %20 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %21 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %20, i32 0, i32 1
  store i1 (i8)* @isSpace, i1 (i8)** %funptr
  %22 = bitcast i1 (i8)** %funptr to %"FunctionPtr[Bool/rtct, Char/rtct]"*
  %23 = load %"FunctionPtr[Bool/rtct, Char/rtct]"* %22
  call void @advanceIf298(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %21, %"FunctionPtr[Bool/rtct, Char/rtct]" %23)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  br label %while_step

if_end3:                                          ; preds = %dumy_block4, %if_block1
  br label %if_block5

dumy_block4:                                      ; No predecessors!
  br label %if_end3

if_block5:                                        ; preds = %if_end3
  %24 = load i8* %ch
  %25 = icmp eq i8 %24, 47
  br i1 %25, label %cond.true, label %cond.false

if_then6:                                         ; preds = %cond.end
  %26 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %27 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %26, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %27)
  %28 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %29 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %28, i32 0, i32 1
  call void @ctor304(%"$lambdaEnclosure.0"* %tmp.v)
  %30 = load %"$lambdaEnclosure.0"* %tmp.v
  call void @advanceIf302(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %29, %"$lambdaEnclosure.0" %30)
  call void @dtor305(%"$lambdaEnclosure.0"* %tmp.v)
  br label %if_block8

if_end7:                                          ; preds = %dumy_block11, %cond.end
  br label %if_block12

cond.true:                                        ; preds = %if_block5
  %31 = load i8* %ch2
  %32 = icmp eq i8 %31, 47
  br label %cond.end

cond.false:                                       ; preds = %if_block5
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ %32, %cond.true ], [ false, %cond.false ]
  br i1 %cond.res, label %if_then6, label %if_end7

if_block8:                                        ; preds = %if_then6
  %33 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %34 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %33, i32 0, i32 1
  %35 = call i1 @"pre_!!306"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %34)
  br i1 %35, label %if_then9, label %if_end10

if_then9:                                         ; preds = %if_block8
  %36 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %37 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %36, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %37)
  br label %if_end10

if_end10:                                         ; preds = %if_then9, %if_block8
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  br label %while_step

dumy_block11:                                     ; No predecessors!
  br label %if_end7

if_block12:                                       ; preds = %if_end7
  %38 = load i8* %ch
  %39 = icmp eq i8 %38, 47
  br i1 %39, label %cond.true15, label %cond.false16

if_then13:                                        ; preds = %cond.end17
  %40 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %41 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %40, i32 0, i32 1
  call void @advance307(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %41, i64 2)
  store i8 97, i8* %c1
  store i8 97, i8* %c2
  br label %while_block19

if_end14:                                         ; preds = %dumy_block36, %cond.end17
  br label %if_block37

cond.true15:                                      ; preds = %if_block12
  %42 = load i8* %ch2
  %43 = icmp eq i8 %42, 42
  br label %cond.end17

cond.false16:                                     ; preds = %if_block12
  br label %cond.end17

cond.end17:                                       ; preds = %cond.false16, %cond.true15
  %cond.res18 = phi i1 [ %43, %cond.true15 ], [ false, %cond.false16 ]
  br i1 %cond.res18, label %if_then13, label %if_end14

while_block19:                                    ; preds = %while_step21, %if_then13
  %44 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %45 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %44, i32 0, i32 1
  %46 = call i1 @"pre_!!306"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %45)
  br i1 %46, label %cond.true23, label %cond.false24

while_body20:                                     ; preds = %cond.end25
  %47 = load i8* %c2
  store i8 %47, i8* %c1
  %48 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %49 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %48, i32 0, i32 1
  %50 = call i8 @"pre_++308"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %49)
  store i8 %50, i8* %c2
  br label %while_step21

while_step21:                                     ; preds = %while_body20
  br label %while_block19

while_end22:                                      ; preds = %cond.end25
  br label %if_block31

cond.true23:                                      ; preds = %while_block19
  %51 = load i8* %c1
  %52 = icmp ne i8 %51, 42
  br i1 %52, label %cond.true26, label %cond.false27

cond.false24:                                     ; preds = %while_block19
  br label %cond.end25

cond.end25:                                       ; preds = %cond.false24, %cond.end28
  %cond.res30 = phi i1 [ %cond.res29, %cond.end28 ], [ false, %cond.false24 ]
  br i1 %cond.res30, label %while_body20, label %while_end22

cond.true26:                                      ; preds = %cond.true23
  br label %cond.end28

cond.false27:                                     ; preds = %cond.true23
  %53 = load i8* %c2
  %54 = icmp ne i8 %53, 47
  br label %cond.end28

cond.end28:                                       ; preds = %cond.false27, %cond.true26
  %cond.res29 = phi i1 [ true, %cond.true26 ], [ %54, %cond.false27 ]
  br label %cond.end25

if_block31:                                       ; preds = %while_end22
  %55 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %56 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %55, i32 0, i32 1
  %57 = call i1 @"pre_!309"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %56)
  br i1 %57, label %if_then32, label %if_end33

if_then32:                                        ; preds = %if_block31
  %58 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @ctorFromCt311(%String* %tmp.v34)
  %59 = load %String* %tmp.v34
  store %String %59, %String* %"$tmpForRef"
  call void @reportError310(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %58, %String* %"$tmpForRef")
  call void @dtor186(%String* %tmp.v34)
  %60 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %60, i32 0)
  call void @dtor162(i8* %c2)
  call void @dtor162(i8* %c1)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end33:                                         ; preds = %dumy_block35, %if_block31
  %61 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %62 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %61, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %62)
  call void @dtor162(i8* %c2)
  call void @dtor162(i8* %c1)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  br label %while_step

dumy_block35:                                     ; No predecessors!
  br label %if_end33

dumy_block36:                                     ; No predecessors!
  call void @dtor162(i8* %c2)
  call void @dtor162(i8* %c1)
  br label %if_end14

if_block37:                                       ; preds = %if_end14
  %63 = load i8* %ch
  %64 = icmp eq i8 %63, 39
  br i1 %64, label %if_then38, label %if_end39

if_then38:                                        ; preds = %if_block37
  %65 = load %TokenType** %_result.addr
  %66 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @parseCharacter312(%TokenType* %65, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %66)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end39:                                         ; preds = %dumy_block40, %if_block37
  br label %if_block41

dumy_block40:                                     ; No predecessors!
  br label %if_end39

if_block41:                                       ; preds = %if_end39
  %67 = load i8* %ch
  %68 = icmp eq i8 %67, 34
  br i1 %68, label %if_then42, label %if_end43

if_then42:                                        ; preds = %if_block41
  %69 = load %TokenType** %_result.addr
  %70 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @parseString322(%TokenType* %69, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %70)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end43:                                         ; preds = %dumy_block44, %if_block41
  br label %if_block45

dumy_block44:                                     ; No predecessors!
  br label %if_end43

if_block45:                                       ; preds = %if_end43
  %71 = load i8* %ch
  %72 = icmp eq i8 %71, 60
  br i1 %72, label %cond.true48, label %cond.false49

if_then46:                                        ; preds = %cond.end50
  %73 = load %TokenType** %_result.addr
  %74 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @parseStringNE325(%TokenType* %73, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %74)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end47:                                         ; preds = %dumy_block52, %cond.end50
  br label %if_block53

cond.true48:                                      ; preds = %if_block45
  %75 = load i8* %ch2
  %76 = icmp eq i8 %75, 123
  br label %cond.end50

cond.false49:                                     ; preds = %if_block45
  br label %cond.end50

cond.end50:                                       ; preds = %cond.false49, %cond.true48
  %cond.res51 = phi i1 [ %76, %cond.true48 ], [ false, %cond.false49 ]
  br i1 %cond.res51, label %if_then46, label %if_end47

dumy_block52:                                     ; No predecessors!
  br label %if_end47

if_block53:                                       ; preds = %if_end47
  %77 = load i8* %ch
  %78 = icmp eq i8 %77, 123
  br i1 %78, label %if_then54, label %if_end55

if_then54:                                        ; preds = %if_block53
  %79 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %80 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %79, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %80)
  %81 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %81, i32 287)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end55:                                         ; preds = %dumy_block56, %if_block53
  br label %if_block57

dumy_block56:                                     ; No predecessors!
  br label %if_end55

if_block57:                                       ; preds = %if_end55
  %82 = load i8* %ch
  %83 = icmp eq i8 %82, 123
  br i1 %83, label %if_then58, label %if_end59

if_then58:                                        ; preds = %if_block57
  %84 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %85 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %84, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %85)
  %86 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %86, i32 287)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end59:                                         ; preds = %dumy_block60, %if_block57
  br label %if_block61

dumy_block60:                                     ; No predecessors!
  br label %if_end59

if_block61:                                       ; preds = %if_end59
  %87 = load i8* %ch
  %88 = icmp eq i8 %87, 125
  br i1 %88, label %if_then62, label %if_end63

if_then62:                                        ; preds = %if_block61
  %89 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %90 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %89, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %90)
  %91 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %91, i32 288)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end63:                                         ; preds = %dumy_block64, %if_block61
  br label %if_block65

dumy_block64:                                     ; No predecessors!
  br label %if_end63

if_block65:                                       ; preds = %if_end63
  %92 = load i8* %ch
  %93 = icmp eq i8 %92, 91
  br i1 %93, label %if_then66, label %if_end67

if_then66:                                        ; preds = %if_block65
  %94 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %95 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %94, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %95)
  %96 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %96, i32 289)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end67:                                         ; preds = %dumy_block68, %if_block65
  br label %if_block69

dumy_block68:                                     ; No predecessors!
  br label %if_end67

if_block69:                                       ; preds = %if_end67
  %97 = load i8* %ch
  %98 = icmp eq i8 %97, 93
  br i1 %98, label %if_then70, label %if_end71

if_then70:                                        ; preds = %if_block69
  %99 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %100 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %99, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %100)
  %101 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %101, i32 290)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end71:                                         ; preds = %dumy_block72, %if_block69
  br label %if_block73

dumy_block72:                                     ; No predecessors!
  br label %if_end71

if_block73:                                       ; preds = %if_end71
  %102 = load i8* %ch
  %103 = icmp eq i8 %102, 40
  br i1 %103, label %if_then74, label %if_end75

if_then74:                                        ; preds = %if_block73
  %104 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %105 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %104, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %105)
  %106 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %106, i32 291)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end75:                                         ; preds = %dumy_block76, %if_block73
  br label %if_block77

dumy_block76:                                     ; No predecessors!
  br label %if_end75

if_block77:                                       ; preds = %if_end75
  %107 = load i8* %ch
  %108 = icmp eq i8 %107, 41
  br i1 %108, label %if_then78, label %if_end79

if_then78:                                        ; preds = %if_block77
  %109 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %110 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %109, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %110)
  %111 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %111, i32 292)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end79:                                         ; preds = %dumy_block80, %if_block77
  br label %if_block81

dumy_block80:                                     ; No predecessors!
  br label %if_end79

if_block81:                                       ; preds = %if_end79
  %112 = load i8* %ch
  %113 = icmp eq i8 %112, 59
  br i1 %113, label %if_then82, label %if_end83

if_then82:                                        ; preds = %if_block81
  %114 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %115 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %114, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %115)
  %116 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %116, i32 294)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end83:                                         ; preds = %dumy_block84, %if_block81
  br label %if_block85

dumy_block84:                                     ; No predecessors!
  br label %if_end83

if_block85:                                       ; preds = %if_end83
  %117 = load i8* %ch
  %118 = icmp eq i8 %117, 44
  br i1 %118, label %if_then86, label %if_end87

if_then86:                                        ; preds = %if_block85
  %119 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %120 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %119, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %120)
  %121 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %121, i32 295)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end87:                                         ; preds = %dumy_block88, %if_block85
  br label %if_block89

dumy_block88:                                     ; No predecessors!
  br label %if_end87

if_block89:                                       ; preds = %if_end87
  %122 = load i8* %ch
  %123 = icmp eq i8 %122, 96
  br i1 %123, label %if_then90, label %if_end91

if_then90:                                        ; preds = %if_block89
  %124 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %125 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %124, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %125)
  %126 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %126, i32 297)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end91:                                         ; preds = %dumy_block92, %if_block89
  br label %if_block93

dumy_block92:                                     ; No predecessors!
  br label %if_end91

if_block93:                                       ; preds = %if_end91
  %127 = load i8* %ch
  %128 = icmp eq i8 %127, 58
  br i1 %128, label %cond.true96, label %cond.false97

if_then94:                                        ; preds = %cond.end98
  %129 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %130 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %129, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %130)
  %131 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %131, i32 293)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end95:                                         ; preds = %dumy_block100, %cond.end98
  br label %if_block101

cond.true96:                                      ; preds = %if_block93
  %132 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %133 = call i8 @peekChar296(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %132)
  %134 = call i1 @isOpChar(i8 %133)
  %135 = xor i1 true, %134
  br label %cond.end98

cond.false97:                                     ; preds = %if_block93
  br label %cond.end98

cond.end98:                                       ; preds = %cond.false97, %cond.true96
  %cond.res99 = phi i1 [ %135, %cond.true96 ], [ false, %cond.false97 ]
  br i1 %cond.res99, label %if_then94, label %if_end95

dumy_block100:                                    ; No predecessors!
  br label %if_end95

if_block101:                                      ; preds = %if_end95
  %136 = load i8* %ch
  %137 = icmp eq i8 %136, 61
  br i1 %137, label %cond.true104, label %cond.false105

if_then102:                                       ; preds = %cond.end106
  %138 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %139 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %138, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %139)
  %140 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %140, i32 298)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end103:                                        ; preds = %dumy_block108, %cond.end106
  br label %if_block109

cond.true104:                                     ; preds = %if_block101
  %141 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %142 = call i8 @peekChar296(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %141)
  %143 = call i1 @isOpChar(i8 %142)
  %144 = xor i1 true, %143
  br label %cond.end106

cond.false105:                                    ; preds = %if_block101
  br label %cond.end106

cond.end106:                                      ; preds = %cond.false105, %cond.true104
  %cond.res107 = phi i1 [ %144, %cond.true104 ], [ false, %cond.false105 ]
  br i1 %cond.res107, label %if_then102, label %if_end103

dumy_block108:                                    ; No predecessors!
  br label %if_end103

if_block109:                                      ; preds = %if_end103
  %145 = load i8* %ch
  %146 = call i1 @isOpCharDot(i8 %145)
  br i1 %146, label %if_then110, label %if_end111

if_then110:                                       ; preds = %if_block109
  br label %if_block112

if_end111:                                        ; preds = %if_end114, %if_block109
  br label %if_block116

if_block112:                                      ; preds = %if_then110
  %147 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %148 = call i1 @parseOperator327(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %147)
  br i1 %148, label %if_then113, label %if_end114

if_then113:                                       ; preds = %if_block112
  %149 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %149, i32 300)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end114:                                        ; preds = %dumy_block115, %if_block112
  br label %if_end111

dumy_block115:                                    ; No predecessors!
  br label %if_end114

if_block116:                                      ; preds = %if_end111
  %150 = load i8* %ch
  %151 = icmp eq i8 %150, 46
  br i1 %151, label %cond.true119, label %cond.false120

if_then117:                                       ; preds = %cond.end121
  %152 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %153 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %152, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %153)
  %154 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %154, i32 296)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end118:                                        ; preds = %dumy_block123, %cond.end121
  br label %if_block124

cond.true119:                                     ; preds = %if_block116
  %155 = load i8* %ch2
  %156 = call i1 @isDigit(i8 %155)
  %157 = xor i1 true, %156
  br label %cond.end121

cond.false120:                                    ; preds = %if_block116
  br label %cond.end121

cond.end121:                                      ; preds = %cond.false120, %cond.true119
  %cond.res122 = phi i1 [ %157, %cond.true119 ], [ false, %cond.false120 ]
  br i1 %cond.res122, label %if_then117, label %if_end118

dumy_block123:                                    ; No predecessors!
  br label %if_end118

if_block124:                                      ; preds = %if_end118
  %158 = load i8* %ch
  %159 = call i1 @isAlpha(i8 %158)
  br i1 %159, label %cond.true127, label %cond.false128

if_then125:                                       ; preds = %cond.end129
  %160 = load %TokenType** %_result.addr
  %161 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @parseIdentifer330(%TokenType* %160, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %161)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end126:                                        ; preds = %dumy_block131, %cond.end129
  br label %if_block132

cond.true127:                                     ; preds = %if_block124
  br label %cond.end129

cond.false128:                                    ; preds = %if_block124
  %162 = load i8* %ch
  %163 = icmp eq i8 %162, 95
  br label %cond.end129

cond.end129:                                      ; preds = %cond.false128, %cond.true127
  %cond.res130 = phi i1 [ true, %cond.true127 ], [ %163, %cond.false128 ]
  br i1 %cond.res130, label %if_then125, label %if_end126

dumy_block131:                                    ; No predecessors!
  br label %if_end126

if_block132:                                      ; preds = %if_end126
  %164 = load i8* %ch
  %165 = call i1 @isDigit(i8 %164)
  br i1 %165, label %cond.true135, label %cond.false136

if_then133:                                       ; preds = %cond.end137
  %166 = load %TokenType** %_result.addr
  %167 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @parseNumeric331(%TokenType* %166, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %167)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

if_end134:                                        ; preds = %dumy_block139, %cond.end137
  %168 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  store [27 x i8] c"Invalid character found: '\00", [27 x i8]* %const.bytes
  %169 = getelementptr inbounds [27 x i8]* %const.bytes, i32 0, i32 0
  %170 = getelementptr inbounds [27 x i8]* %const.bytes, i32 0, i32 26
  %171 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %172 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %169, i8** %171
  store i8* %170, i8** %172
  %173 = load %StringRef* %const.struct
  %174 = load i8* %ch
  store [2 x i8] c"'\00", [2 x i8]* %const.bytes140
  %175 = getelementptr inbounds [2 x i8]* %const.bytes140, i32 0, i32 0
  %176 = getelementptr inbounds [2 x i8]* %const.bytes140, i32 0, i32 1
  %177 = getelementptr inbounds %StringRef* %const.struct141, i32 0, i32 0
  %178 = getelementptr inbounds %StringRef* %const.struct141, i32 0, i32 1
  store i8* %175, i8** %177
  store i8* %176, i8** %178
  %179 = load %StringRef* %const.struct141
  %180 = load i8* %ch
  call void @_ass_32_8z(i32* %tmp.v142, i8 %180)
  %181 = load i32* %tmp.v142
  call void @toString276(%String* %"$tmpC", %StringRef %173, i8 %174, %StringRef %179, i32 %181)
  call void @reportError310(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %168, %String* %"$tmpC")
  call void @dtor186(%String* %"$tmpC")
  call void @dtor58(i32* %tmp.v142)
  %182 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %183 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %182, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %183)
  %184 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %184, i32 0)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  ret void

cond.true135:                                     ; preds = %if_block132
  br label %cond.end137

cond.false136:                                    ; preds = %if_block132
  %185 = load i8* %ch
  %186 = icmp eq i8 %185, 46
  br label %cond.end137

cond.end137:                                      ; preds = %cond.false136, %cond.true135
  %cond.res138 = phi i1 [ true, %cond.true135 ], [ %186, %cond.false136 ]
  br i1 %cond.res138, label %if_then133, label %if_end134

dumy_block139:                                    ; No predecessors!
  br label %if_end134

dumy_block143:                                    ; No predecessors!
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  br label %while_step
}

; Function Attrs: alwaysinline nounwind
define private i1 @isEmpty291(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %s) #2 {
  %s.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %s, %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %s.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %s.addr
  %2 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %1, i32 0, i32 0
  %3 = call i1 @isEmpty292(%"RangeWithLookahead[StringRef/rtct]"* %2)
  ret i1 %3
}

; Function Attrs: alwaysinline nounwind
define private i1 @isEmpty292(%"RangeWithLookahead[StringRef/rtct]"* %s) #2 {
  %s.addr = alloca %"RangeWithLookahead[StringRef/rtct]"*
  store %"RangeWithLookahead[StringRef/rtct]"* %s, %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  %2 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %1, i32 0, i32 1
  %3 = call i1 @isEmpty223(%"Vector[Char/rtct]"* %2)
  ret i1 %3
}

; Function Attrs: alwaysinline nounwind
define private i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %r) #2 {
  %r.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %r, %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  %2 = call i8 @front294(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %1)
  ret i8 %2
}

; Function Attrs: alwaysinline nounwind
define private i8 @front294(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %s) #2 {
  %s.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %s, %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %s.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %s.addr
  %2 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %1, i32 0, i32 0
  %3 = call i8 @front295(%"RangeWithLookahead[StringRef/rtct]"* %2)
  ret i8 %3
}

; Function Attrs: alwaysinline nounwind
define private i8 @front295(%"RangeWithLookahead[StringRef/rtct]"* %s) #2 {
  %s.addr = alloca %"RangeWithLookahead[StringRef/rtct]"*
  store %"RangeWithLookahead[StringRef/rtct]"* %s, %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  %2 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %1, i32 0, i32 1
  %3 = call i8* @front227(%"Vector[Char/rtct]"* %2)
  %4 = load i8* %3
  ret i8 %4
}

; Function Attrs: inlinehint nounwind
define private i8 @peekChar296(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s) #3 {
  %s.addr = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  %3 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %2, i32 0, i32 0
  %4 = call i8 @peek297(%"RangeWithLookahead[StringRef/rtct]"* %3, i32 1)
  ret i8 %4
}

; Function Attrs: inlinehint nounwind
define private i8 @peek297(%"RangeWithLookahead[StringRef/rtct]"* %s, i32 %n) #3 {
  %s.addr = alloca %"RangeWithLookahead[StringRef/rtct]"*
  store %"RangeWithLookahead[StringRef/rtct]"* %s, %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  %n.addr = alloca i32
  store i32 %n, i32* %n.addr
  %tmp.v = alloca i64
  %tmp.v1 = alloca i8
  br label %code

code:                                             ; preds = %0
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %1 = load i32* %n.addr
  %2 = load %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  %3 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %2, i32 0, i32 1
  %4 = call i64 @size142(%"Vector[Char/rtct]"* %3)
  %5 = call i1 @">="(i32 %1, i64 %4)
  br i1 %5, label %cond.true, label %cond.false

while_body:                                       ; preds = %cond.end
  %6 = load %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  %7 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %6, i32 0, i32 1
  %8 = load %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  %9 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %8, i32 0, i32 0
  %10 = call i8* @"post_++193"(%StringRef* %9)
  call void @"+="(%"Vector[Char/rtct]"* %7, i8* %10)
  br label %while_step

while_step:                                       ; preds = %while_body
  br label %while_block

while_end:                                        ; preds = %cond.end
  br label %if_block

cond.true:                                        ; preds = %while_block
  %11 = load %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  %12 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %11, i32 0, i32 0
  %13 = call i1 @"pre_!!192"(%StringRef* %12)
  br label %cond.end

cond.false:                                       ; preds = %while_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ %13, %cond.true ], [ false, %cond.false ]
  br i1 %cond.res, label %while_body, label %while_end

if_block:                                         ; preds = %while_end
  %14 = load i32* %n.addr
  %15 = load %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  %16 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %15, i32 0, i32 1
  %17 = call i64 @size142(%"Vector[Char/rtct]"* %16)
  %18 = call i1 @"<230"(i32 %14, i64 %17)
  br i1 %18, label %if_then, label %if_else

if_then:                                          ; preds = %if_block
  %19 = load %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  %20 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %19, i32 0, i32 1
  %21 = load i32* %n.addr
  %22 = zext i32 %21 to i64
  store i64 %22, i64* %tmp.v
  %23 = load i64* %tmp.v
  %24 = call i8* @"()231"(%"Vector[Char/rtct]"* %20, i64 %23)
  %25 = load i8* %24
  call void @dtor31(i64* %tmp.v)
  ret i8 %25

if_else:                                          ; preds = %if_block
  store i8 0, i8* %tmp.v1
  %26 = load i8* %tmp.v1
  call void @dtor162(i8* %tmp.v1)
  ret i8 %26

if_end:                                           ; preds = %dumy_block2, %dumy_block
  unreachable

dumy_block:                                       ; No predecessors!
  call void @dtor31(i64* %tmp.v)
  br label %if_end

dumy_block2:                                      ; No predecessors!
  call void @dtor162(i8* %tmp.v1)
  br label %if_end
}

; Function Attrs: inlinehint nounwind
define private void @advanceIf298(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %range, %"FunctionPtr[Bool/rtct, Char/rtct]" %pred) #3 {
  %range.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %range, %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %range.addr
  %pred.addr = alloca %"FunctionPtr[Bool/rtct, Char/rtct]"
  store %"FunctionPtr[Bool/rtct, Char/rtct]" %pred, %"FunctionPtr[Bool/rtct, Char/rtct]"* %pred.addr
  br label %code

code:                                             ; preds = %0
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %range.addr
  %2 = call i1 @isEmpty291(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %1)
  %3 = xor i1 true, %2
  br i1 %3, label %cond.true, label %cond.false

while_body:                                       ; preds = %cond.end
  %4 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %range.addr
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %4)
  br label %while_step

while_step:                                       ; preds = %while_body
  br label %while_block

while_end:                                        ; preds = %cond.end
  ret void

cond.true:                                        ; preds = %while_block
  %5 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %range.addr
  %6 = call i8 @front294(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %5)
  %7 = call i1 @"()232"(%"FunctionPtr[Bool/rtct, Char/rtct]"* %pred.addr, i8 %6)
  br label %cond.end

cond.false:                                       ; preds = %while_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ %7, %cond.true ], [ false, %cond.false ]
  br i1 %cond.res, label %while_body, label %while_end
}

; Function Attrs: inlinehint nounwind
define private void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %s) #3 {
  %s.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %s, %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %s.addr
  br label %code

code:                                             ; preds = %0
  br label %if_block

if_block:                                         ; preds = %code
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %s.addr
  %2 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %1, i32 0, i32 0
  %3 = call i8 @"pre_*300"(%"RangeWithLookahead[StringRef/rtct]"* %2)
  %4 = icmp eq i8 %3, 10
  br i1 %4, label %if_then, label %if_else

if_then:                                          ; preds = %if_block
  %5 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %s.addr
  %6 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %5, i32 0, i32 1
  %7 = load %Location** %6
  call void @addLines(%Location* %7, i32 1)
  br label %if_end

if_else:                                          ; preds = %if_block
  %8 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %s.addr
  %9 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %8, i32 0, i32 1
  %10 = load %Location** %9
  call void @addColumns(%Location* %10, i32 1)
  br label %if_end

if_end:                                           ; preds = %if_else, %if_then
  %11 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %s.addr
  %12 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %11, i32 0, i32 0
  call void @popFront301(%"RangeWithLookahead[StringRef/rtct]"* %12)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i8 @"pre_*300"(%"RangeWithLookahead[StringRef/rtct]"* %r) #2 {
  %r.addr = alloca %"RangeWithLookahead[StringRef/rtct]"*
  store %"RangeWithLookahead[StringRef/rtct]"* %r, %"RangeWithLookahead[StringRef/rtct]"** %r.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"RangeWithLookahead[StringRef/rtct]"** %r.addr
  %2 = call i8 @front295(%"RangeWithLookahead[StringRef/rtct]"* %1)
  ret i8 %2
}

; Function Attrs: inlinehint nounwind
define private void @popFront301(%"RangeWithLookahead[StringRef/rtct]"* %s) #3 {
  %s.addr = alloca %"RangeWithLookahead[StringRef/rtct]"*
  store %"RangeWithLookahead[StringRef/rtct]"* %s, %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  %2 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %1, i32 0, i32 1
  call void @remove236(%"Vector[Char/rtct]"* %2, i64 0)
  br label %if_block

if_block:                                         ; preds = %code
  %3 = load %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  %4 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %3, i32 0, i32 1
  %5 = call i1 @isEmpty223(%"Vector[Char/rtct]"* %4)
  br i1 %5, label %cond.true, label %cond.false

if_then:                                          ; preds = %cond.end
  %6 = load %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  %7 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %6, i32 0, i32 1
  %8 = load %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  %9 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %8, i32 0, i32 0
  %10 = call i8* @"post_++193"(%StringRef* %9)
  call void @"+="(%"Vector[Char/rtct]"* %7, i8* %10)
  br label %if_end

if_end:                                           ; preds = %if_then, %cond.end
  ret void

cond.true:                                        ; preds = %if_block
  %11 = load %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  %12 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %11, i32 0, i32 0
  %13 = call i1 @"pre_!!192"(%StringRef* %12)
  br label %cond.end

cond.false:                                       ; preds = %if_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ %13, %cond.true ], [ false, %cond.false ]
  br i1 %cond.res, label %if_then, label %if_end
}

; Function Attrs: inlinehint nounwind
define private void @advanceIf302(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %range, %"$lambdaEnclosure.0" %pred) #3 {
  %range.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %range, %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %range.addr
  %pred.addr = alloca %"$lambdaEnclosure.0"
  store %"$lambdaEnclosure.0" %pred, %"$lambdaEnclosure.0"* %pred.addr
  br label %code

code:                                             ; preds = %0
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %range.addr
  %2 = call i1 @isEmpty291(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %1)
  %3 = xor i1 true, %2
  br i1 %3, label %cond.true, label %cond.false

while_body:                                       ; preds = %cond.end
  %4 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %range.addr
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %4)
  br label %while_step

while_step:                                       ; preds = %while_body
  br label %while_block

while_end:                                        ; preds = %cond.end
  ret void

cond.true:                                        ; preds = %while_block
  %5 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %range.addr
  %6 = call i8 @front294(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %5)
  %7 = call i1 @"()303"(%"$lambdaEnclosure.0"* %pred.addr, i8 %6)
  br label %cond.end

cond.false:                                       ; preds = %while_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ %7, %cond.true ], [ false, %cond.false ]
  br i1 %cond.res, label %while_body, label %while_end
}

; Function Attrs: alwaysinline nounwind
define private i1 @"()303"(%"$lambdaEnclosure.0"* %"$this", i8 %c) #2 {
  %"$this.addr" = alloca %"$lambdaEnclosure.0"*
  store %"$lambdaEnclosure.0"* %"$this", %"$lambdaEnclosure.0"** %"$this.addr"
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  %2 = icmp ne i8 %1, 10
  ret i1 %2
}

; Function Attrs: alwaysinline nounwind
define private void @ctor304(%"$lambdaEnclosure.0"* %"$this") #2 {
  %"$this.addr" = alloca %"$lambdaEnclosure.0"*
  store %"$lambdaEnclosure.0"* %"$this", %"$lambdaEnclosure.0"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor305(%"$lambdaEnclosure.0"* %"$this") #2 {
  %"$this.addr" = alloca %"$lambdaEnclosure.0"*
  store %"$lambdaEnclosure.0"* %"$this", %"$lambdaEnclosure.0"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i1 @"pre_!!306"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %r) #2 {
  %r.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %r, %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  %2 = call i1 @isEmpty291(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %1)
  %3 = xor i1 true, %2
  ret i1 %3
}

; Function Attrs: inlinehint nounwind
define private void @advance307(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %range, i64 %n) #3 {
  %range.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %range, %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %range.addr
  %n.addr = alloca i64
  store i64 %n, i64* %n.addr
  br label %code

code:                                             ; preds = %0
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %1 = load i64* %n.addr
  %2 = call i1 @">"(i64 %1, i32 0)
  br i1 %2, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  %3 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %range.addr
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %3)
  br label %while_step

while_step:                                       ; preds = %while_body
  %4 = load i64* %n.addr
  %5 = call i64 @-240(i64 %4, i32 1)
  store i64 %5, i64* %n.addr
  br label %while_block

while_end:                                        ; preds = %while_block
  ret void
}

; Function Attrs: inlinehint nounwind
define private i8 @"pre_++308"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %r) #3 {
  %r.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %r, %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %1)
  %2 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  %3 = call i8 @front294(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %2)
  ret i8 %3
}

; Function Attrs: alwaysinline nounwind
define private i1 @"pre_!309"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %r) #2 {
  %r.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %r, %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  %2 = call i1 @isEmpty291(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %1)
  ret i1 %2
}

; Function Attrs: inlinehint nounwind
define private void @reportError310(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, %String* %msg) #3 {
  %s.addr = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %msg.addr = alloca %String*
  store %String* %msg, %String** %msg.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1, i32 0, i32 5
  %3 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %4 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %3, i32 0, i32 1
  %5 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %4, i32 0, i32 1
  %6 = load %Location** %5
  %7 = load %Location* %6
  %8 = load %String** %msg.addr
  call void @reportError(%ExternalErrorReporter* %2, %Location %7, %String* %8)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctorFromCt311(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [33 x i8]
  %const.struct = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  store [33 x i8] c"End of file found inside comment\00", [33 x i8]* %const.bytes
  %2 = getelementptr inbounds [33 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [33 x i8]* %const.bytes, i32 0, i32 32
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  call void @ctor251(%String* %1, %StringRef* %"$tmpForRef")
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @parseCharacter312(%TokenType* sret %_result, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s) #3 {
  %_result.addr = alloca %TokenType*
  store %TokenType* %_result, %TokenType** %_result.addr
  %s.addr = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %"$tmpForRef" = alloca %String
  %tmp.v = alloca %String
  %ch = alloca i8
  %"$tmpForRef4" = alloca %String
  %tmp.v5 = alloca %String
  %"$tmpForRef13" = alloca %String
  %tmp.v14 = alloca %String
  %"$tmpForRef19" = alloca %String
  %tmp.v20 = alloca %String
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %2)
  br label %if_block

if_block:                                         ; preds = %code
  %3 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %4 = call i1 @hasLessThan313(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %3, i32 2)
  br i1 %4, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %5 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @ctorFromCt315(%String* %tmp.v)
  %6 = load %String* %tmp.v
  store %String %6, %String* %"$tmpForRef"
  call void @reportError310(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %5, %String* %"$tmpForRef")
  call void @dtor186(%String* %tmp.v)
  %7 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %7, i32 0)
  ret void

if_end:                                           ; preds = %dumy_block, %if_block
  %8 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %9 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %8, i32 0, i32 1
  %10 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %9)
  store i8 %10, i8* %ch
  br label %if_block1

dumy_block:                                       ; No predecessors!
  br label %if_end

if_block1:                                        ; preds = %if_end
  %11 = load i8* %ch
  %12 = icmp eq i8 %11, 39
  br i1 %12, label %if_then2, label %if_else

if_then2:                                         ; preds = %if_block1
  %13 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @ctorFromCt316(%String* %tmp.v5)
  %14 = load %String* %tmp.v5
  store %String %14, %String* %"$tmpForRef4"
  call void @reportError310(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %13, %String* %"$tmpForRef4")
  call void @dtor186(%String* %tmp.v5)
  %15 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %16 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %15, i32 0, i32 2
  %17 = getelementptr inbounds %Token* %16, i32 0, i32 2
  call void @"+=255"(%String* %17, i8 63)
  %18 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %18, i32 301)
  call void @dtor162(i8* %ch)
  ret void

if_else:                                          ; preds = %if_block1
  br label %if_block7

if_end3:                                          ; preds = %if_end9, %dumy_block6
  br label %if_block10

dumy_block6:                                      ; No predecessors!
  br label %if_end3

if_block7:                                        ; preds = %if_else
  %19 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %20 = call i1 @checkEscapeChar317(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %19)
  %21 = xor i1 true, %20
  br i1 %21, label %if_then8, label %if_end9

if_then8:                                         ; preds = %if_block7
  %22 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @advanceAndCapture1319(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %22)
  br label %if_end9

if_end9:                                          ; preds = %if_then8, %if_block7
  br label %if_end3

if_block10:                                       ; preds = %if_end3
  %23 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %24 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %23, i32 0, i32 1
  %25 = call i1 @"pre_!309"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %24)
  br i1 %25, label %if_then11, label %if_end12

if_then11:                                        ; preds = %if_block10
  %26 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @ctorFromCt320(%String* %tmp.v14)
  %27 = load %String* %tmp.v14
  store %String %27, %String* %"$tmpForRef13"
  call void @reportError310(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %26, %String* %"$tmpForRef13")
  call void @dtor186(%String* %tmp.v14)
  %28 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %28, i32 0)
  call void @dtor162(i8* %ch)
  ret void

if_end12:                                         ; preds = %dumy_block15, %if_block10
  br label %if_block16

dumy_block15:                                     ; No predecessors!
  br label %if_end12

if_block16:                                       ; preds = %if_end12
  %29 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %30 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %29, i32 0, i32 1
  %31 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %30)
  %32 = icmp ne i8 %31, 39
  br i1 %32, label %if_then17, label %if_end18

if_then17:                                        ; preds = %if_block16
  %33 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @ctorFromCt321(%String* %tmp.v20)
  %34 = load %String* %tmp.v20
  store %String %34, %String* %"$tmpForRef19"
  call void @reportError310(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %33, %String* %"$tmpForRef19")
  call void @dtor186(%String* %tmp.v20)
  br label %if_end18

if_end18:                                         ; preds = %if_then17, %if_block16
  %35 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %36 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %35, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %36)
  %37 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %37, i32 301)
  call void @dtor162(i8* %ch)
  ret void

dumy_block21:                                     ; No predecessors!
  call void @dtor162(i8* %ch)
  ret void
}

; Function Attrs: inlinehint nounwind
define private i1 @hasLessThan313(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, i32 %n) #3 {
  %s.addr = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %n.addr = alloca i32
  store i32 %n, i32* %n.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  %3 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %2, i32 0, i32 0
  %4 = load i32* %n.addr
  %5 = call i1 @hasLessThan314(%"RangeWithLookahead[StringRef/rtct]"* %3, i32 %4)
  ret i1 %5
}

; Function Attrs: inlinehint nounwind
define private i1 @hasLessThan314(%"RangeWithLookahead[StringRef/rtct]"* %s, i32 %n) #3 {
  %s.addr = alloca %"RangeWithLookahead[StringRef/rtct]"*
  store %"RangeWithLookahead[StringRef/rtct]"* %s, %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  %n.addr = alloca i32
  store i32 %n, i32* %n.addr
  br label %code

code:                                             ; preds = %0
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %1 = load i32* %n.addr
  %2 = load %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  %3 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %2, i32 0, i32 1
  %4 = call i64 @size142(%"Vector[Char/rtct]"* %3)
  %5 = call i1 @">="(i32 %1, i64 %4)
  br i1 %5, label %cond.true, label %cond.false

while_body:                                       ; preds = %cond.end
  %6 = load %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  %7 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %6, i32 0, i32 1
  %8 = load %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  %9 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %8, i32 0, i32 0
  %10 = call i8* @"post_++193"(%StringRef* %9)
  call void @"+="(%"Vector[Char/rtct]"* %7, i8* %10)
  br label %while_step

while_step:                                       ; preds = %while_body
  br label %while_block

while_end:                                        ; preds = %cond.end
  %11 = load i32* %n.addr
  %12 = load %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  %13 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %12, i32 0, i32 1
  %14 = call i64 @size142(%"Vector[Char/rtct]"* %13)
  %15 = call i1 @">="(i32 %11, i64 %14)
  ret i1 %15

cond.true:                                        ; preds = %while_block
  %16 = load %"RangeWithLookahead[StringRef/rtct]"** %s.addr
  %17 = getelementptr inbounds %"RangeWithLookahead[StringRef/rtct]"* %16, i32 0, i32 0
  %18 = call i1 @"pre_!!192"(%StringRef* %17)
  br label %cond.end

cond.false:                                       ; preds = %while_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ %18, %cond.true ], [ false, %cond.false ]
  br i1 %cond.res, label %while_body, label %while_end
}

; Function Attrs: inlinehint nounwind
define private void @ctorFromCt315(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [43 x i8]
  %const.struct = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  store [43 x i8] c"End of file found inside character literal\00", [43 x i8]* %const.bytes
  %2 = getelementptr inbounds [43 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [43 x i8]* %const.bytes, i32 0, i32 42
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  call void @ctor251(%String* %1, %StringRef* %"$tmpForRef")
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctorFromCt316(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [26 x i8]
  %const.struct = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  store [26 x i8] c"Invalid character literal\00", [26 x i8]* %const.bytes
  %2 = getelementptr inbounds [26 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [26 x i8]* %const.bytes, i32 0, i32 25
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  call void @ctor251(%String* %1, %StringRef* %"$tmpForRef")
  ret void
}

; Function Attrs: noinline nounwind
define i1 @checkEscapeChar317(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s) #4 {
  %s.addr = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %ch = alloca i8
  %"$tmpForRef" = alloca %String
  %tmp.v = alloca %String
  %charVal = alloca i32
  %tmp.v39 = alloca i8
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  %3 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %2)
  store i8 %3, i8* %ch
  br label %if_block

if_block:                                         ; preds = %code
  %4 = load i8* %ch
  %5 = icmp eq i8 %4, 92
  br i1 %5, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %6 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %7 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %6, i32 0, i32 1
  %8 = call i8 @"pre_++308"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %7)
  store i8 %8, i8* %ch
  br label %if_block1

if_end:                                           ; preds = %dumy_block40, %if_block
  call void @dtor162(i8* %ch)
  ret i1 false

if_block1:                                        ; preds = %if_then
  %9 = load i8* %ch
  %10 = icmp eq i8 %9, 114
  br i1 %10, label %if_then2, label %if_else

if_then2:                                         ; preds = %if_block1
  %11 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %12 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %11, i32 0, i32 2
  %13 = getelementptr inbounds %Token* %12, i32 0, i32 2
  call void @"+=255"(%String* %13, i8 13)
  br label %if_end3

if_else:                                          ; preds = %if_block1
  br label %if_block4

if_end3:                                          ; preds = %if_end7, %if_then2
  %14 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %15 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %14, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %15)
  call void @dtor162(i8* %ch)
  ret i1 true

if_block4:                                        ; preds = %if_else
  %16 = load i8* %ch
  %17 = icmp eq i8 %16, 110
  br i1 %17, label %if_then5, label %if_else6

if_then5:                                         ; preds = %if_block4
  %18 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %19 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %18, i32 0, i32 2
  %20 = getelementptr inbounds %Token* %19, i32 0, i32 2
  call void @"+=255"(%String* %20, i8 10)
  br label %if_end7

if_else6:                                         ; preds = %if_block4
  br label %if_block8

if_end7:                                          ; preds = %if_end11, %if_then5
  br label %if_end3

if_block8:                                        ; preds = %if_else6
  %21 = load i8* %ch
  %22 = icmp eq i8 %21, 98
  br i1 %22, label %if_then9, label %if_else10

if_then9:                                         ; preds = %if_block8
  %23 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %24 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %23, i32 0, i32 2
  %25 = getelementptr inbounds %Token* %24, i32 0, i32 2
  call void @"+=255"(%String* %25, i8 8)
  br label %if_end11

if_else10:                                        ; preds = %if_block8
  br label %if_block12

if_end11:                                         ; preds = %if_end15, %if_then9
  br label %if_end7

if_block12:                                       ; preds = %if_else10
  %26 = load i8* %ch
  %27 = icmp eq i8 %26, 102
  br i1 %27, label %if_then13, label %if_else14

if_then13:                                        ; preds = %if_block12
  %28 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %29 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %28, i32 0, i32 2
  %30 = getelementptr inbounds %Token* %29, i32 0, i32 2
  call void @"+=255"(%String* %30, i8 12)
  br label %if_end15

if_else14:                                        ; preds = %if_block12
  br label %if_block16

if_end15:                                         ; preds = %if_end19, %if_then13
  br label %if_end11

if_block16:                                       ; preds = %if_else14
  %31 = load i8* %ch
  %32 = icmp eq i8 %31, 116
  br i1 %32, label %if_then17, label %if_else18

if_then17:                                        ; preds = %if_block16
  %33 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %34 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %33, i32 0, i32 2
  %35 = getelementptr inbounds %Token* %34, i32 0, i32 2
  call void @"+=255"(%String* %35, i8 9)
  br label %if_end19

if_else18:                                        ; preds = %if_block16
  br label %if_block20

if_end19:                                         ; preds = %if_end23, %if_then17
  br label %if_end15

if_block20:                                       ; preds = %if_else18
  %36 = load i8* %ch
  %37 = icmp eq i8 %36, 92
  br i1 %37, label %if_then21, label %if_else22

if_then21:                                        ; preds = %if_block20
  %38 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %39 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %38, i32 0, i32 2
  %40 = getelementptr inbounds %Token* %39, i32 0, i32 2
  call void @"+=255"(%String* %40, i8 92)
  br label %if_end23

if_else22:                                        ; preds = %if_block20
  br label %if_block24

if_end23:                                         ; preds = %if_end27, %if_then21
  br label %if_end19

if_block24:                                       ; preds = %if_else22
  %41 = load i8* %ch
  %42 = icmp eq i8 %41, 39
  br i1 %42, label %if_then25, label %if_else26

if_then25:                                        ; preds = %if_block24
  %43 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %44 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %43, i32 0, i32 2
  %45 = getelementptr inbounds %Token* %44, i32 0, i32 2
  call void @"+=255"(%String* %45, i8 39)
  br label %if_end27

if_else26:                                        ; preds = %if_block24
  br label %if_block28

if_end27:                                         ; preds = %if_end31, %if_then25
  br label %if_end23

if_block28:                                       ; preds = %if_else26
  %46 = load i8* %ch
  %47 = icmp eq i8 %46, 34
  br i1 %47, label %if_then29, label %if_else30

if_then29:                                        ; preds = %if_block28
  %48 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %49 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %48, i32 0, i32 2
  %50 = getelementptr inbounds %Token* %49, i32 0, i32 2
  call void @"+=255"(%String* %50, i8 34)
  br label %if_end31

if_else30:                                        ; preds = %if_block28
  br label %if_block32

if_end31:                                         ; preds = %if_end35, %if_then29
  br label %if_end27

if_block32:                                       ; preds = %if_else30
  %51 = load i8* %ch
  %52 = icmp eq i8 %51, 120
  br i1 %52, label %cond.true, label %cond.false

if_then33:                                        ; preds = %cond.end
  br label %if_block36

if_else34:                                        ; preds = %cond.end
  %53 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %54 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %53, i32 0, i32 2
  %55 = getelementptr inbounds %Token* %54, i32 0, i32 2
  %56 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %57 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %56, i32 0, i32 1
  %58 = call i8 @"pre_++308"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %57)
  call void @"+=255"(%String* %55, i8 %58)
  br label %if_end35

if_end35:                                         ; preds = %if_else34, %if_end38
  br label %if_end31

cond.true:                                        ; preds = %if_block32
  br label %cond.end

cond.false:                                       ; preds = %if_block32
  %59 = load i8* %ch
  %60 = icmp eq i8 %59, 88
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ true, %cond.true ], [ %60, %cond.false ]
  br i1 %cond.res, label %if_then33, label %if_else34

if_block36:                                       ; preds = %if_then33
  %61 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %62 = call i1 @hasLessThan313(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %61, i32 2)
  br i1 %62, label %if_then37, label %if_end38

if_then37:                                        ; preds = %if_block36
  %63 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @ctorFromCt318(%String* %tmp.v)
  %64 = load %String* %tmp.v
  store %String %64, %String* %"$tmpForRef"
  call void @reportError310(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %63, %String* %"$tmpForRef")
  call void @dtor186(%String* %tmp.v)
  call void @dtor162(i8* %ch)
  ret i1 true

if_end38:                                         ; preds = %dumy_block, %if_block36
  %65 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %66 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %65, i32 0, i32 1
  %67 = call i8 @"pre_++308"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %66)
  %68 = call i32 @getXdigitVal(i8 %67)
  %69 = call i32 @"*258"(i32 16, i32 %68)
  store i32 %69, i32* %charVal
  %70 = load i32* %charVal
  %71 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %72 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %71, i32 0, i32 1
  %73 = call i8 @"pre_++308"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %72)
  %74 = call i32 @getXdigitVal(i8 %73)
  %75 = call i32 @"+14"(i32 %70, i32 %74)
  store i32 %75, i32* %charVal
  %76 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %77 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %76, i32 0, i32 2
  %78 = getelementptr inbounds %Token* %77, i32 0, i32 2
  %79 = load i32* %charVal
  %80 = trunc i32 %79 to i8
  store i8 %80, i8* %tmp.v39
  %81 = load i8* %tmp.v39
  call void @"+=255"(%String* %78, i8 %81)
  call void @dtor162(i8* %tmp.v39)
  call void @dtor58(i32* %charVal)
  br label %if_end35

dumy_block:                                       ; No predecessors!
  br label %if_end38

dumy_block40:                                     ; No predecessors!
  br label %if_end

dumy_block41:                                     ; No predecessors!
  call void @dtor162(i8* %ch)
  unreachable
}

; Function Attrs: inlinehint nounwind
define private void @ctorFromCt318(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [37 x i8]
  %const.struct = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  store [37 x i8] c"Not enough digits for character code\00", [37 x i8]* %const.bytes
  %2 = getelementptr inbounds [37 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [37 x i8]* %const.bytes, i32 0, i32 36
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  call void @ctor251(%String* %1, %StringRef* %"$tmpForRef")
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @advanceAndCapture1319(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s) #3 {
  %s.addr = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1, i32 0, i32 2
  %3 = getelementptr inbounds %Token* %2, i32 0, i32 2
  %4 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %5 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %4, i32 0, i32 1
  %6 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %5)
  call void @"+=255"(%String* %3, i8 %6)
  %7 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %8 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %7, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %8)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctorFromCt320(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [43 x i8]
  %const.struct = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  store [43 x i8] c"End of file found inside character literal\00", [43 x i8]* %const.bytes
  %2 = getelementptr inbounds [43 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [43 x i8]* %const.bytes, i32 0, i32 42
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  call void @ctor251(%String* %1, %StringRef* %"$tmpForRef")
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctorFromCt321(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [28 x i8]
  %const.struct = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  store [28 x i8] c"Character literal too large\00", [28 x i8]* %const.bytes
  %2 = getelementptr inbounds [28 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [28 x i8]* %const.bytes, i32 0, i32 27
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  call void @ctor251(%String* %1, %StringRef* %"$tmpForRef")
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @parseString322(%TokenType* sret %_result, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s) #3 {
  %_result.addr = alloca %TokenType*
  store %TokenType* %_result, %TokenType** %_result.addr
  %s.addr = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %"$tmpForRef" = alloca %String
  %tmp.v = alloca %String
  %"$tmpForRef7" = alloca %String
  %tmp.v8 = alloca %String
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %2)
  br label %if_block

if_block:                                         ; preds = %code
  %3 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %4 = call i1 @hasLessThan313(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %3, i32 1)
  br i1 %4, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %5 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @ctorFromCt323(%String* %tmp.v)
  %6 = load %String* %tmp.v
  store %String %6, %String* %"$tmpForRef"
  call void @reportError310(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %5, %String* %"$tmpForRef")
  call void @dtor186(%String* %tmp.v)
  %7 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %7, i32 0)
  ret void

if_end:                                           ; preds = %dumy_block, %if_block
  br label %while_block

dumy_block:                                       ; No predecessors!
  br label %if_end

while_block:                                      ; preds = %while_step, %if_end
  %8 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %9 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %8, i32 0, i32 1
  %10 = call i1 @"pre_!!306"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %9)
  br i1 %10, label %cond.true, label %cond.false

while_body:                                       ; preds = %cond.end
  br label %if_block1

while_step:                                       ; preds = %if_end3
  br label %while_block

while_end:                                        ; preds = %cond.end
  br label %if_block4

cond.true:                                        ; preds = %while_block
  %11 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %12 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %11, i32 0, i32 1
  %13 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %12)
  %14 = icmp ne i8 %13, 34
  br label %cond.end

cond.false:                                       ; preds = %while_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ %14, %cond.true ], [ false, %cond.false ]
  br i1 %cond.res, label %while_body, label %while_end

if_block1:                                        ; preds = %while_body
  %15 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %16 = call i1 @checkEscapeChar317(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %15)
  %17 = xor i1 true, %16
  br i1 %17, label %if_then2, label %if_end3

if_then2:                                         ; preds = %if_block1
  %18 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @advanceAndCapture1319(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %18)
  br label %if_end3

if_end3:                                          ; preds = %if_then2, %if_block1
  br label %while_step

if_block4:                                        ; preds = %while_end
  %19 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %20 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %19, i32 0, i32 1
  %21 = call i1 @"pre_!309"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %20)
  br i1 %21, label %if_then5, label %if_end6

if_then5:                                         ; preds = %if_block4
  %22 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @ctorFromCt324(%String* %tmp.v8)
  %23 = load %String* %tmp.v8
  store %String %23, %String* %"$tmpForRef7"
  call void @reportError310(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %22, %String* %"$tmpForRef7")
  call void @dtor186(%String* %tmp.v8)
  %24 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %24, i32 0)
  ret void

if_end6:                                          ; preds = %dumy_block9, %if_block4
  %25 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %26 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %25, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %26)
  %27 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %27, i32 302)
  ret void

dumy_block9:                                      ; No predecessors!
  br label %if_end6
}

; Function Attrs: inlinehint nounwind
define private void @ctorFromCt323(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [40 x i8]
  %const.struct = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  store [40 x i8] c"End of file found inside string literal\00", [40 x i8]* %const.bytes
  %2 = getelementptr inbounds [40 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [40 x i8]* %const.bytes, i32 0, i32 39
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  call void @ctor251(%String* %1, %StringRef* %"$tmpForRef")
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ctorFromCt324(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [40 x i8]
  %const.struct = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  store [40 x i8] c"End of file found inside string literal\00", [40 x i8]* %const.bytes
  %2 = getelementptr inbounds [40 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [40 x i8]* %const.bytes, i32 0, i32 39
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  call void @ctor251(%String* %1, %StringRef* %"$tmpForRef")
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @parseStringNE325(%TokenType* sret %_result, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s) #3 {
  %_result.addr = alloca %TokenType*
  store %TokenType* %_result, %TokenType** %_result.addr
  %s.addr = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %"$tmpForRef" = alloca %String
  %tmp.v = alloca %String
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  call void @advance307(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %2, i64 2)
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %3 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %4 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %3, i32 0, i32 1
  %5 = call i1 @"pre_!!306"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %4)
  br i1 %5, label %cond.true, label %cond.false

while_body:                                       ; preds = %cond.end
  %6 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @advanceAndCapture1319(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %6)
  br label %while_step

while_step:                                       ; preds = %while_body
  br label %while_block

while_end:                                        ; preds = %cond.end
  br label %if_block

cond.true:                                        ; preds = %while_block
  %7 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %8 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %7, i32 0, i32 1
  %9 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %8)
  %10 = icmp eq i8 %9, 125
  br i1 %10, label %cond.true1, label %cond.false2

cond.false:                                       ; preds = %while_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.end3
  %cond.res4 = phi i1 [ %14, %cond.end3 ], [ false, %cond.false ]
  br i1 %cond.res4, label %while_body, label %while_end

cond.true1:                                       ; preds = %cond.true
  %11 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %12 = call i8 @peekChar296(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %11)
  %13 = icmp eq i8 %12, 62
  br label %cond.end3

cond.false2:                                      ; preds = %cond.true
  br label %cond.end3

cond.end3:                                        ; preds = %cond.false2, %cond.true1
  %cond.res = phi i1 [ %13, %cond.true1 ], [ false, %cond.false2 ]
  %14 = xor i1 true, %cond.res
  br label %cond.end

if_block:                                         ; preds = %while_end
  %15 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %16 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %15, i32 0, i32 1
  %17 = call i1 @"pre_!309"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %16)
  br i1 %17, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %18 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @ctorFromCt326(%String* %tmp.v)
  %19 = load %String* %tmp.v
  store %String %19, %String* %"$tmpForRef"
  call void @reportError310(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %18, %String* %"$tmpForRef")
  call void @dtor186(%String* %tmp.v)
  %20 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %20, i32 0)
  ret void

if_end:                                           ; preds = %dumy_block, %if_block
  %21 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %22 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %21, i32 0, i32 1
  call void @advance307(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %22, i64 2)
  %23 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %23, i32 302)
  ret void

dumy_block:                                       ; No predecessors!
  br label %if_end
}

; Function Attrs: inlinehint nounwind
define private void @ctorFromCt326(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [40 x i8]
  %const.struct = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  store [40 x i8] c"End of file found inside string literal\00", [40 x i8]* %const.bytes
  %2 = getelementptr inbounds [40 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [40 x i8]* %const.bytes, i32 0, i32 39
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  call void @ctor251(%String* %1, %StringRef* %"$tmpForRef")
  ret void
}

; Function Attrs: inlinehint nounwind
define private i1 @parseOperator327(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s) #3 {
  %s.addr = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %ch = alloca i8
  %funptr = alloca i1 (i8)*
  %i = alloca i32
  %c = alloca i8
  %tmp.v = alloca i32
  %tmp.v4 = alloca i32
  %hasOtherDot = alloca i1
  %funptr8 = alloca i1 (i8)*
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  %3 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %2)
  store i8 %3, i8* %ch
  br label %if_block

if_block:                                         ; preds = %code
  %4 = load i8* %ch
  %5 = call i1 @isOpChar(i8 %4)
  br i1 %5, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %6 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  store i1 (i8)* @isOpCharDot, i1 (i8)** %funptr
  %7 = bitcast i1 (i8)** %funptr to %"FunctionPtr[Bool, Char]"*
  %8 = load %"FunctionPtr[Bool, Char]"* %7
  call void @advanceAndCapture328(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %6, %"FunctionPtr[Bool, Char]" %8)
  call void @dtor162(i8* %ch)
  ret i1 true

if_end:                                           ; preds = %dumy_block, %if_block
  br label %if_block1

dumy_block:                                       ; No predecessors!
  br label %if_end

if_block1:                                        ; preds = %if_end
  %9 = load i8* %ch
  %10 = icmp eq i8 %9, 46
  br i1 %10, label %if_then2, label %if_end3

if_then2:                                         ; preds = %if_block1
  store i32 1, i32* %i
  %11 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %12 = load i32* %i
  store i32 %12, i32* %tmp.v
  %13 = load i32* %tmp.v
  %14 = call i8 @peekChar329(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %11, i32 %13)
  store i8 %14, i8* %c
  call void @dtor19(i32* %tmp.v)
  br label %while_block

if_end3:                                          ; preds = %if_end7, %if_block1
  call void @dtor162(i8* %ch)
  ret i1 false

while_block:                                      ; preds = %while_step, %if_then2
  %15 = load i8* %c
  %16 = call i1 @isOpChar(i8 %15)
  br i1 %16, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  %17 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %18 = call i32 @"pre_++13"(i32* %i)
  store i32 %18, i32* %tmp.v4
  %19 = load i32* %tmp.v4
  %20 = call i8 @peekChar329(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %17, i32 %19)
  store i8 %20, i8* %c
  call void @dtor19(i32* %tmp.v4)
  br label %while_step

while_step:                                       ; preds = %while_body
  br label %while_block

while_end:                                        ; preds = %while_block
  %21 = load i8* %c
  %22 = icmp eq i8 %21, 46
  store i1 %22, i1* %hasOtherDot
  br label %if_block5

if_block5:                                        ; preds = %while_end
  %23 = load i1* %hasOtherDot
  br i1 %23, label %if_then6, label %if_end7

if_then6:                                         ; preds = %if_block5
  %24 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  store i1 (i8)* @isOpCharDot, i1 (i8)** %funptr8
  %25 = bitcast i1 (i8)** %funptr8 to %"FunctionPtr[Bool, Char]"*
  %26 = load %"FunctionPtr[Bool, Char]"* %25
  call void @advanceAndCapture328(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %24, %"FunctionPtr[Bool, Char]" %26)
  call void @dtor167(i1* %hasOtherDot)
  call void @dtor162(i8* %c)
  call void @dtor58(i32* %i)
  call void @dtor162(i8* %ch)
  ret i1 true

if_end7:                                          ; preds = %dumy_block9, %if_block5
  call void @dtor167(i1* %hasOtherDot)
  call void @dtor162(i8* %c)
  call void @dtor58(i32* %i)
  br label %if_end3

dumy_block9:                                      ; No predecessors!
  br label %if_end7

dumy_block10:                                     ; No predecessors!
  call void @dtor162(i8* %ch)
  unreachable
}

; Function Attrs: inlinehint nounwind
define private void @advanceAndCapture328(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, %"FunctionPtr[Bool, Char]" %pred) #3 {
  %s.addr = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %pred.addr = alloca %"FunctionPtr[Bool, Char]"
  store %"FunctionPtr[Bool, Char]" %pred, %"FunctionPtr[Bool, Char]"* %pred.addr
  br label %code

code:                                             ; preds = %0
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  %3 = call i1 @"pre_!!306"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %2)
  br i1 %3, label %cond.true, label %cond.false

while_body:                                       ; preds = %cond.end
  %4 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %5 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %4, i32 0, i32 2
  %6 = getelementptr inbounds %Token* %5, i32 0, i32 2
  %7 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %8 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %7, i32 0, i32 1
  %9 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %8)
  call void @"+=255"(%String* %6, i8 %9)
  br label %while_step

while_step:                                       ; preds = %while_body
  %10 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %11 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %10, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %11)
  br label %while_block

while_end:                                        ; preds = %cond.end
  ret void

cond.true:                                        ; preds = %while_block
  %12 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %13 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %12, i32 0, i32 1
  %14 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %13)
  %15 = call i1 @"()264"(%"FunctionPtr[Bool, Char]"* %pred.addr, i8 %14)
  br label %cond.end

cond.false:                                       ; preds = %while_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ %15, %cond.true ], [ false, %cond.false ]
  br i1 %cond.res, label %while_body, label %while_end
}

; Function Attrs: inlinehint nounwind
define private i8 @peekChar329(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, i32 %n) #3 {
  %s.addr = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %n.addr = alloca i32
  store i32 %n, i32* %n.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  %3 = getelementptr inbounds %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %2, i32 0, i32 0
  %4 = load i32* %n.addr
  %5 = call i8 @peek297(%"RangeWithLookahead[StringRef/rtct]"* %3, i32 %4)
  ret i8 %5
}

; Function Attrs: noinline nounwind
define void @parseIdentifer330(%TokenType* sret %_result, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s) #4 {
  %_result.addr = alloca %TokenType*
  store %TokenType* %_result, %TokenType** %_result.addr
  %s.addr = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %allowSymbolChars = alloca i1
  %firstDot = alloca i1
  %ch = alloca i8
  %data = alloca %StringRef*
  %"$tmpC" = alloca %StringRef
  %const.bytes = alloca [6 x i8]
  %const.struct = alloca %StringRef
  %const.bytes24 = alloca [6 x i8]
  %const.struct25 = alloca %StringRef
  %const.bytes31 = alloca [6 x i8]
  %const.struct32 = alloca %StringRef
  %const.bytes38 = alloca [8 x i8]
  %const.struct39 = alloca %StringRef
  %const.bytes45 = alloca [9 x i8]
  %const.struct46 = alloca %StringRef
  %const.bytes52 = alloca [9 x i8]
  %const.struct53 = alloca %StringRef
  %const.bytes59 = alloca [4 x i8]
  %const.struct60 = alloca %StringRef
  %const.bytes66 = alloca [3 x i8]
  %const.struct67 = alloca %StringRef
  %const.bytes73 = alloca [5 x i8]
  %const.struct74 = alloca %StringRef
  %const.bytes80 = alloca [6 x i8]
  %const.struct81 = alloca %StringRef
  %const.bytes87 = alloca [8 x i8]
  %const.struct88 = alloca %StringRef
  %const.bytes94 = alloca [4 x i8]
  %const.struct95 = alloca %StringRef
  %const.bytes101 = alloca [7 x i8]
  %const.struct102 = alloca %StringRef
  %const.bytes108 = alloca [7 x i8]
  %const.struct109 = alloca %StringRef
  %const.bytes115 = alloca [5 x i8]
  %const.struct116 = alloca %StringRef
  %const.bytes122 = alloca [8 x i8]
  %const.struct123 = alloca %StringRef
  %const.bytes129 = alloca [8 x i8]
  %const.struct130 = alloca %StringRef
  %const.bytes136 = alloca [7 x i8]
  %const.struct137 = alloca %StringRef
  %const.bytes143 = alloca [7 x i8]
  %const.struct144 = alloca %StringRef
  %const.bytes150 = alloca [5 x i8]
  %const.struct151 = alloca %StringRef
  %const.bytes157 = alloca [6 x i8]
  %const.struct158 = alloca %StringRef
  %const.bytes164 = alloca [5 x i8]
  %const.struct165 = alloca %StringRef
  %const.bytes171 = alloca [4 x i8]
  %const.struct172 = alloca %StringRef
  %const.bytes178 = alloca [6 x i8]
  %const.struct179 = alloca %StringRef
  %const.bytes185 = alloca [4 x i8]
  %const.struct186 = alloca %StringRef
  %const.bytes192 = alloca [6 x i8]
  %const.struct193 = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  store i1 false, i1* %allowSymbolChars
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1, i32 0, i32 2
  %3 = getelementptr inbounds %Token* %2, i32 0, i32 2
  %4 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %5 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %4, i32 0, i32 1
  %6 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %5)
  call void @"+=255"(%String* %3, i8 %6)
  %7 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %8 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %7, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %8)
  store i1 true, i1* %firstDot
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %9 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %10 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %9, i32 0, i32 1
  %11 = call i1 @"pre_!!306"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %10)
  br i1 %11, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  %12 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %13 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %12, i32 0, i32 1
  %14 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %13)
  store i8 %14, i8* %ch
  br label %if_block

while_step:                                       ; preds = %if_end7
  %15 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %16 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %15, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %16)
  br label %while_block

while_end:                                        ; preds = %if_then, %while_block
  %17 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %18 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %17, i32 0, i32 2
  %19 = getelementptr inbounds %Token* %18, i32 0, i32 2
  call void @asStringRef(%StringRef* %"$tmpC", %String* %19)
  store %StringRef* %"$tmpC", %StringRef** %data
  call void @dtor78(%StringRef* %"$tmpC")
  br label %if_block16

if_block:                                         ; preds = %while_body
  %20 = load i8* %ch
  %21 = call i1 @isAlpha(i8 %20)
  %22 = xor i1 true, %21
  br i1 %22, label %cond.true1, label %cond.false2

if_then:                                          ; preds = %cond.end
  call void @dtor162(i8* %ch)
  br label %while_end

if_end:                                           ; preds = %dumy_block, %cond.end
  %23 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %24 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %23, i32 0, i32 2
  %25 = getelementptr inbounds %Token* %24, i32 0, i32 2
  %26 = load i8* %ch
  call void @"+=255"(%String* %25, i8 %26)
  br label %if_block5

cond.true:                                        ; preds = %cond.end3
  %27 = load i8* %ch
  %28 = call i1 @isDigit(i8 %27)
  %29 = xor i1 true, %28
  br label %cond.end

cond.false:                                       ; preds = %cond.end3
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res4 = phi i1 [ %29, %cond.true ], [ false, %cond.false ]
  br i1 %cond.res4, label %if_then, label %if_end

cond.true1:                                       ; preds = %if_block
  %30 = load i8* %ch
  %31 = icmp ne i8 %30, 95
  br label %cond.end3

cond.false2:                                      ; preds = %if_block
  br label %cond.end3

cond.end3:                                        ; preds = %cond.false2, %cond.true1
  %cond.res = phi i1 [ %31, %cond.true1 ], [ false, %cond.false2 ]
  br i1 %cond.res, label %cond.true, label %cond.false

dumy_block:                                       ; No predecessors!
  br label %if_end

if_block5:                                        ; preds = %if_end
  %32 = load i8* %ch
  %33 = icmp eq i8 %32, 95
  br i1 %33, label %cond.true8, label %cond.false9

if_then6:                                         ; preds = %cond.end10
  %34 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %35 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %34, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %35)
  br label %if_block12

if_end7:                                          ; preds = %if_end14, %cond.end10
  call void @dtor162(i8* %ch)
  br label %while_step

cond.true8:                                       ; preds = %if_block5
  %36 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %37 = call i8 @peekChar296(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %36)
  %38 = call i1 @isOpCharDot(i8 %37)
  br label %cond.end10

cond.false9:                                      ; preds = %if_block5
  br label %cond.end10

cond.end10:                                       ; preds = %cond.false9, %cond.true8
  %cond.res11 = phi i1 [ %38, %cond.true8 ], [ false, %cond.false9 ]
  br i1 %cond.res11, label %if_then6, label %if_end7

if_block12:                                       ; preds = %if_then6
  %39 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %40 = call i1 @parseOperator327(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %39)
  br i1 %40, label %if_then13, label %if_end14

if_then13:                                        ; preds = %if_block12
  %41 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %41, i32 299)
  call void @dtor162(i8* %ch)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_end14:                                         ; preds = %dumy_block15, %if_block12
  br label %if_end7

dumy_block15:                                     ; No predecessors!
  br label %if_end14

if_block16:                                       ; preds = %while_end
  %42 = load %StringRef** %data
  store [6 x i8] c"break\00", [6 x i8]* %const.bytes
  %43 = getelementptr inbounds [6 x i8]* %const.bytes, i32 0, i32 0
  %44 = getelementptr inbounds [6 x i8]* %const.bytes, i32 0, i32 5
  %45 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %46 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %43, i8** %45
  store i8* %44, i8** %46
  %47 = load %StringRef* %const.struct
  %48 = call i1 @"==266"(%StringRef* %42, %StringRef %47)
  br i1 %48, label %if_then17, label %if_else

if_then17:                                        ; preds = %if_block16
  %49 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %49, i32 271)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else:                                          ; preds = %if_block16
  br label %if_block20

if_end18:                                         ; preds = %if_end23, %dumy_block19
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

dumy_block19:                                     ; No predecessors!
  br label %if_end18

if_block20:                                       ; preds = %if_else
  %50 = load %StringRef** %data
  store [6 x i8] c"catch\00", [6 x i8]* %const.bytes24
  %51 = getelementptr inbounds [6 x i8]* %const.bytes24, i32 0, i32 0
  %52 = getelementptr inbounds [6 x i8]* %const.bytes24, i32 0, i32 5
  %53 = getelementptr inbounds %StringRef* %const.struct25, i32 0, i32 0
  %54 = getelementptr inbounds %StringRef* %const.struct25, i32 0, i32 1
  store i8* %51, i8** %53
  store i8* %52, i8** %54
  %55 = load %StringRef* %const.struct25
  %56 = call i1 @"==266"(%StringRef* %50, %StringRef %55)
  br i1 %56, label %if_then21, label %if_else22

if_then21:                                        ; preds = %if_block20
  %57 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %57, i32 272)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else22:                                        ; preds = %if_block20
  br label %if_block27

if_end23:                                         ; preds = %if_end30, %dumy_block26
  br label %if_end18

dumy_block26:                                     ; No predecessors!
  br label %if_end23

if_block27:                                       ; preds = %if_else22
  %58 = load %StringRef** %data
  store [6 x i8] c"class\00", [6 x i8]* %const.bytes31
  %59 = getelementptr inbounds [6 x i8]* %const.bytes31, i32 0, i32 0
  %60 = getelementptr inbounds [6 x i8]* %const.bytes31, i32 0, i32 5
  %61 = getelementptr inbounds %StringRef* %const.struct32, i32 0, i32 0
  %62 = getelementptr inbounds %StringRef* %const.struct32, i32 0, i32 1
  store i8* %59, i8** %61
  store i8* %60, i8** %62
  %63 = load %StringRef* %const.struct32
  %64 = call i1 @"==266"(%StringRef* %58, %StringRef %63)
  br i1 %64, label %if_then28, label %if_else29

if_then28:                                        ; preds = %if_block27
  %65 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %65, i32 264)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else29:                                        ; preds = %if_block27
  br label %if_block34

if_end30:                                         ; preds = %if_end37, %dumy_block33
  br label %if_end23

dumy_block33:                                     ; No predecessors!
  br label %if_end30

if_block34:                                       ; preds = %if_else29
  %66 = load %StringRef** %data
  store [8 x i8] c"concept\00", [8 x i8]* %const.bytes38
  %67 = getelementptr inbounds [8 x i8]* %const.bytes38, i32 0, i32 0
  %68 = getelementptr inbounds [8 x i8]* %const.bytes38, i32 0, i32 7
  %69 = getelementptr inbounds %StringRef* %const.struct39, i32 0, i32 0
  %70 = getelementptr inbounds %StringRef* %const.struct39, i32 0, i32 1
  store i8* %67, i8** %69
  store i8* %68, i8** %70
  %71 = load %StringRef* %const.struct39
  %72 = call i1 @"==266"(%StringRef* %66, %StringRef %71)
  br i1 %72, label %if_then35, label %if_else36

if_then35:                                        ; preds = %if_block34
  %73 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %73, i32 265)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else36:                                        ; preds = %if_block34
  br label %if_block41

if_end37:                                         ; preds = %if_end44, %dumy_block40
  br label %if_end30

dumy_block40:                                     ; No predecessors!
  br label %if_end37

if_block41:                                       ; preds = %if_else36
  %74 = load %StringRef** %data
  store [9 x i8] c"continue\00", [9 x i8]* %const.bytes45
  %75 = getelementptr inbounds [9 x i8]* %const.bytes45, i32 0, i32 0
  %76 = getelementptr inbounds [9 x i8]* %const.bytes45, i32 0, i32 8
  %77 = getelementptr inbounds %StringRef* %const.struct46, i32 0, i32 0
  %78 = getelementptr inbounds %StringRef* %const.struct46, i32 0, i32 1
  store i8* %75, i8** %77
  store i8* %76, i8** %78
  %79 = load %StringRef* %const.struct46
  %80 = call i1 @"==266"(%StringRef* %74, %StringRef %79)
  br i1 %80, label %if_then42, label %if_else43

if_then42:                                        ; preds = %if_block41
  %81 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %81, i32 273)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else43:                                        ; preds = %if_block41
  br label %if_block48

if_end44:                                         ; preds = %if_end51, %dumy_block47
  br label %if_end37

dumy_block47:                                     ; No predecessors!
  br label %if_end44

if_block48:                                       ; preds = %if_else43
  %82 = load %StringRef** %data
  store [9 x i8] c"datatype\00", [9 x i8]* %const.bytes52
  %83 = getelementptr inbounds [9 x i8]* %const.bytes52, i32 0, i32 0
  %84 = getelementptr inbounds [9 x i8]* %const.bytes52, i32 0, i32 8
  %85 = getelementptr inbounds %StringRef* %const.struct53, i32 0, i32 0
  %86 = getelementptr inbounds %StringRef* %const.struct53, i32 0, i32 1
  store i8* %83, i8** %85
  store i8* %84, i8** %86
  %87 = load %StringRef* %const.struct53
  %88 = call i1 @"==266"(%StringRef* %82, %StringRef %87)
  br i1 %88, label %if_then49, label %if_else50

if_then49:                                        ; preds = %if_block48
  %89 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %89, i32 266)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else50:                                        ; preds = %if_block48
  br label %if_block55

if_end51:                                         ; preds = %if_end58, %dumy_block54
  br label %if_end44

dumy_block54:                                     ; No predecessors!
  br label %if_end51

if_block55:                                       ; preds = %if_else50
  %90 = load %StringRef** %data
  store [4 x i8] c"fun\00", [4 x i8]* %const.bytes59
  %91 = getelementptr inbounds [4 x i8]* %const.bytes59, i32 0, i32 0
  %92 = getelementptr inbounds [4 x i8]* %const.bytes59, i32 0, i32 3
  %93 = getelementptr inbounds %StringRef* %const.struct60, i32 0, i32 0
  %94 = getelementptr inbounds %StringRef* %const.struct60, i32 0, i32 1
  store i8* %91, i8** %93
  store i8* %92, i8** %94
  %95 = load %StringRef* %const.struct60
  %96 = call i1 @"==266"(%StringRef* %90, %StringRef %95)
  br i1 %96, label %if_then56, label %if_else57

if_then56:                                        ; preds = %if_block55
  %97 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %97, i32 267)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else57:                                        ; preds = %if_block55
  br label %if_block62

if_end58:                                         ; preds = %if_end65, %dumy_block61
  br label %if_end51

dumy_block61:                                     ; No predecessors!
  br label %if_end58

if_block62:                                       ; preds = %if_else57
  %98 = load %StringRef** %data
  store [3 x i8] c"if\00", [3 x i8]* %const.bytes66
  %99 = getelementptr inbounds [3 x i8]* %const.bytes66, i32 0, i32 0
  %100 = getelementptr inbounds [3 x i8]* %const.bytes66, i32 0, i32 2
  %101 = getelementptr inbounds %StringRef* %const.struct67, i32 0, i32 0
  %102 = getelementptr inbounds %StringRef* %const.struct67, i32 0, i32 1
  store i8* %99, i8** %101
  store i8* %100, i8** %102
  %103 = load %StringRef* %const.struct67
  %104 = call i1 @"==266"(%StringRef* %98, %StringRef %103)
  br i1 %104, label %if_then63, label %if_else64

if_then63:                                        ; preds = %if_block62
  %105 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %105, i32 276)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else64:                                        ; preds = %if_block62
  br label %if_block69

if_end65:                                         ; preds = %if_end72, %dumy_block68
  br label %if_end58

dumy_block68:                                     ; No predecessors!
  br label %if_end65

if_block69:                                       ; preds = %if_else64
  %106 = load %StringRef** %data
  store [5 x i8] c"else\00", [5 x i8]* %const.bytes73
  %107 = getelementptr inbounds [5 x i8]* %const.bytes73, i32 0, i32 0
  %108 = getelementptr inbounds [5 x i8]* %const.bytes73, i32 0, i32 4
  %109 = getelementptr inbounds %StringRef* %const.struct74, i32 0, i32 0
  %110 = getelementptr inbounds %StringRef* %const.struct74, i32 0, i32 1
  store i8* %107, i8** %109
  store i8* %108, i8** %110
  %111 = load %StringRef* %const.struct74
  %112 = call i1 @"==266"(%StringRef* %106, %StringRef %111)
  br i1 %112, label %if_then70, label %if_else71

if_then70:                                        ; preds = %if_block69
  %113 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %113, i32 286)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else71:                                        ; preds = %if_block69
  br label %if_block76

if_end72:                                         ; preds = %if_end79, %dumy_block75
  br label %if_end65

dumy_block75:                                     ; No predecessors!
  br label %if_end72

if_block76:                                       ; preds = %if_else71
  %114 = load %StringRef** %data
  store [6 x i8] c"false\00", [6 x i8]* %const.bytes80
  %115 = getelementptr inbounds [6 x i8]* %const.bytes80, i32 0, i32 0
  %116 = getelementptr inbounds [6 x i8]* %const.bytes80, i32 0, i32 5
  %117 = getelementptr inbounds %StringRef* %const.struct81, i32 0, i32 0
  %118 = getelementptr inbounds %StringRef* %const.struct81, i32 0, i32 1
  store i8* %115, i8** %117
  store i8* %116, i8** %118
  %119 = load %StringRef* %const.struct81
  %120 = call i1 @"==266"(%StringRef* %114, %StringRef %119)
  br i1 %120, label %if_then77, label %if_else78

if_then77:                                        ; preds = %if_block76
  %121 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %121, i32 281)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else78:                                        ; preds = %if_block76
  br label %if_block83

if_end79:                                         ; preds = %if_end86, %dumy_block82
  br label %if_end72

dumy_block82:                                     ; No predecessors!
  br label %if_end79

if_block83:                                       ; preds = %if_else78
  %122 = load %StringRef** %data
  store [8 x i8] c"finally\00", [8 x i8]* %const.bytes87
  %123 = getelementptr inbounds [8 x i8]* %const.bytes87, i32 0, i32 0
  %124 = getelementptr inbounds [8 x i8]* %const.bytes87, i32 0, i32 7
  %125 = getelementptr inbounds %StringRef* %const.struct88, i32 0, i32 0
  %126 = getelementptr inbounds %StringRef* %const.struct88, i32 0, i32 1
  store i8* %123, i8** %125
  store i8* %124, i8** %126
  %127 = load %StringRef* %const.struct88
  %128 = call i1 @"==266"(%StringRef* %122, %StringRef %127)
  br i1 %128, label %if_then84, label %if_else85

if_then84:                                        ; preds = %if_block83
  %129 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %129, i32 274)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else85:                                        ; preds = %if_block83
  br label %if_block90

if_end86:                                         ; preds = %if_end93, %dumy_block89
  br label %if_end79

dumy_block89:                                     ; No predecessors!
  br label %if_end86

if_block90:                                       ; preds = %if_else85
  %130 = load %StringRef** %data
  store [4 x i8] c"for\00", [4 x i8]* %const.bytes94
  %131 = getelementptr inbounds [4 x i8]* %const.bytes94, i32 0, i32 0
  %132 = getelementptr inbounds [4 x i8]* %const.bytes94, i32 0, i32 3
  %133 = getelementptr inbounds %StringRef* %const.struct95, i32 0, i32 0
  %134 = getelementptr inbounds %StringRef* %const.struct95, i32 0, i32 1
  store i8* %131, i8** %133
  store i8* %132, i8** %134
  %135 = load %StringRef* %const.struct95
  %136 = call i1 @"==266"(%StringRef* %130, %StringRef %135)
  br i1 %136, label %if_then91, label %if_else92

if_then91:                                        ; preds = %if_block90
  %137 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %137, i32 275)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else92:                                        ; preds = %if_block90
  br label %if_block97

if_end93:                                         ; preds = %if_end100, %dumy_block96
  br label %if_end86

dumy_block96:                                     ; No predecessors!
  br label %if_end93

if_block97:                                       ; preds = %if_else92
  %138 = load %StringRef** %data
  store [7 x i8] c"import\00", [7 x i8]* %const.bytes101
  %139 = getelementptr inbounds [7 x i8]* %const.bytes101, i32 0, i32 0
  %140 = getelementptr inbounds [7 x i8]* %const.bytes101, i32 0, i32 6
  %141 = getelementptr inbounds %StringRef* %const.struct102, i32 0, i32 0
  %142 = getelementptr inbounds %StringRef* %const.struct102, i32 0, i32 1
  store i8* %139, i8** %141
  store i8* %140, i8** %142
  %143 = load %StringRef* %const.struct102
  %144 = call i1 @"==266"(%StringRef* %138, %StringRef %143)
  br i1 %144, label %if_then98, label %if_else99

if_then98:                                        ; preds = %if_block97
  %145 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %145, i32 261)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else99:                                        ; preds = %if_block97
  br label %if_block104

if_end100:                                        ; preds = %if_end107, %dumy_block103
  br label %if_end93

dumy_block103:                                    ; No predecessors!
  br label %if_end100

if_block104:                                      ; preds = %if_else99
  %146 = load %StringRef** %data
  store [7 x i8] c"module\00", [7 x i8]* %const.bytes108
  %147 = getelementptr inbounds [7 x i8]* %const.bytes108, i32 0, i32 0
  %148 = getelementptr inbounds [7 x i8]* %const.bytes108, i32 0, i32 6
  %149 = getelementptr inbounds %StringRef* %const.struct109, i32 0, i32 0
  %150 = getelementptr inbounds %StringRef* %const.struct109, i32 0, i32 1
  store i8* %147, i8** %149
  store i8* %148, i8** %150
  %151 = load %StringRef* %const.struct109
  %152 = call i1 @"==266"(%StringRef* %146, %StringRef %151)
  br i1 %152, label %if_then105, label %if_else106

if_then105:                                       ; preds = %if_block104
  %153 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %153, i32 260)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else106:                                       ; preds = %if_block104
  br label %if_block111

if_end107:                                        ; preds = %if_end114, %dumy_block110
  br label %if_end100

dumy_block110:                                    ; No predecessors!
  br label %if_end107

if_block111:                                      ; preds = %if_else106
  %154 = load %StringRef** %data
  store [5 x i8] c"null\00", [5 x i8]* %const.bytes115
  %155 = getelementptr inbounds [5 x i8]* %const.bytes115, i32 0, i32 0
  %156 = getelementptr inbounds [5 x i8]* %const.bytes115, i32 0, i32 4
  %157 = getelementptr inbounds %StringRef* %const.struct116, i32 0, i32 0
  %158 = getelementptr inbounds %StringRef* %const.struct116, i32 0, i32 1
  store i8* %155, i8** %157
  store i8* %156, i8** %158
  %159 = load %StringRef* %const.struct116
  %160 = call i1 @"==266"(%StringRef* %154, %StringRef %159)
  br i1 %160, label %if_then112, label %if_else113

if_then112:                                       ; preds = %if_block111
  %161 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %161, i32 282)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else113:                                       ; preds = %if_block111
  br label %if_block118

if_end114:                                        ; preds = %if_end121, %dumy_block117
  br label %if_end107

dumy_block117:                                    ; No predecessors!
  br label %if_end114

if_block118:                                      ; preds = %if_else113
  %162 = load %StringRef** %data
  store [8 x i8] c"package\00", [8 x i8]* %const.bytes122
  %163 = getelementptr inbounds [8 x i8]* %const.bytes122, i32 0, i32 0
  %164 = getelementptr inbounds [8 x i8]* %const.bytes122, i32 0, i32 7
  %165 = getelementptr inbounds %StringRef* %const.struct123, i32 0, i32 0
  %166 = getelementptr inbounds %StringRef* %const.struct123, i32 0, i32 1
  store i8* %163, i8** %165
  store i8* %164, i8** %166
  %167 = load %StringRef* %const.struct123
  %168 = call i1 @"==266"(%StringRef* %162, %StringRef %167)
  br i1 %168, label %if_then119, label %if_else120

if_then119:                                       ; preds = %if_block118
  %169 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %169, i32 268)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else120:                                       ; preds = %if_block118
  br label %if_block125

if_end121:                                        ; preds = %if_end128, %dumy_block124
  br label %if_end114

dumy_block124:                                    ; No predecessors!
  br label %if_end121

if_block125:                                      ; preds = %if_else120
  %170 = load %StringRef** %data
  store [8 x i8] c"private\00", [8 x i8]* %const.bytes129
  %171 = getelementptr inbounds [8 x i8]* %const.bytes129, i32 0, i32 0
  %172 = getelementptr inbounds [8 x i8]* %const.bytes129, i32 0, i32 7
  %173 = getelementptr inbounds %StringRef* %const.struct130, i32 0, i32 0
  %174 = getelementptr inbounds %StringRef* %const.struct130, i32 0, i32 1
  store i8* %171, i8** %173
  store i8* %172, i8** %174
  %175 = load %StringRef* %const.struct130
  %176 = call i1 @"==266"(%StringRef* %170, %StringRef %175)
  br i1 %176, label %if_then126, label %if_else127

if_then126:                                       ; preds = %if_block125
  %177 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %177, i32 262)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else127:                                       ; preds = %if_block125
  br label %if_block132

if_end128:                                        ; preds = %if_end135, %dumy_block131
  br label %if_end121

dumy_block131:                                    ; No predecessors!
  br label %if_end128

if_block132:                                      ; preds = %if_else127
  %178 = load %StringRef** %data
  store [7 x i8] c"public\00", [7 x i8]* %const.bytes136
  %179 = getelementptr inbounds [7 x i8]* %const.bytes136, i32 0, i32 0
  %180 = getelementptr inbounds [7 x i8]* %const.bytes136, i32 0, i32 6
  %181 = getelementptr inbounds %StringRef* %const.struct137, i32 0, i32 0
  %182 = getelementptr inbounds %StringRef* %const.struct137, i32 0, i32 1
  store i8* %179, i8** %181
  store i8* %180, i8** %182
  %183 = load %StringRef* %const.struct137
  %184 = call i1 @"==266"(%StringRef* %178, %StringRef %183)
  br i1 %184, label %if_then133, label %if_else134

if_then133:                                       ; preds = %if_block132
  %185 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %185, i32 263)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else134:                                       ; preds = %if_block132
  br label %if_block139

if_end135:                                        ; preds = %if_end142, %dumy_block138
  br label %if_end128

dumy_block138:                                    ; No predecessors!
  br label %if_end135

if_block139:                                      ; preds = %if_else134
  %186 = load %StringRef** %data
  store [7 x i8] c"return\00", [7 x i8]* %const.bytes143
  %187 = getelementptr inbounds [7 x i8]* %const.bytes143, i32 0, i32 0
  %188 = getelementptr inbounds [7 x i8]* %const.bytes143, i32 0, i32 6
  %189 = getelementptr inbounds %StringRef* %const.struct144, i32 0, i32 0
  %190 = getelementptr inbounds %StringRef* %const.struct144, i32 0, i32 1
  store i8* %187, i8** %189
  store i8* %188, i8** %190
  %191 = load %StringRef* %const.struct144
  %192 = call i1 @"==266"(%StringRef* %186, %StringRef %191)
  br i1 %192, label %if_then140, label %if_else141

if_then140:                                       ; preds = %if_block139
  %193 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %193, i32 277)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else141:                                       ; preds = %if_block139
  br label %if_block146

if_end142:                                        ; preds = %if_end149, %dumy_block145
  br label %if_end135

dumy_block145:                                    ; No predecessors!
  br label %if_end142

if_block146:                                      ; preds = %if_else141
  %194 = load %StringRef** %data
  store [5 x i8] c"this\00", [5 x i8]* %const.bytes150
  %195 = getelementptr inbounds [5 x i8]* %const.bytes150, i32 0, i32 0
  %196 = getelementptr inbounds [5 x i8]* %const.bytes150, i32 0, i32 4
  %197 = getelementptr inbounds %StringRef* %const.struct151, i32 0, i32 0
  %198 = getelementptr inbounds %StringRef* %const.struct151, i32 0, i32 1
  store i8* %195, i8** %197
  store i8* %196, i8** %198
  %199 = load %StringRef* %const.struct151
  %200 = call i1 @"==266"(%StringRef* %194, %StringRef %199)
  br i1 %200, label %if_then147, label %if_else148

if_then147:                                       ; preds = %if_block146
  %201 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %201, i32 283)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else148:                                       ; preds = %if_block146
  br label %if_block153

if_end149:                                        ; preds = %if_end156, %dumy_block152
  br label %if_end142

dumy_block152:                                    ; No predecessors!
  br label %if_end149

if_block153:                                      ; preds = %if_else148
  %202 = load %StringRef** %data
  store [6 x i8] c"throw\00", [6 x i8]* %const.bytes157
  %203 = getelementptr inbounds [6 x i8]* %const.bytes157, i32 0, i32 0
  %204 = getelementptr inbounds [6 x i8]* %const.bytes157, i32 0, i32 5
  %205 = getelementptr inbounds %StringRef* %const.struct158, i32 0, i32 0
  %206 = getelementptr inbounds %StringRef* %const.struct158, i32 0, i32 1
  store i8* %203, i8** %205
  store i8* %204, i8** %206
  %207 = load %StringRef* %const.struct158
  %208 = call i1 @"==266"(%StringRef* %202, %StringRef %207)
  br i1 %208, label %if_then154, label %if_else155

if_then154:                                       ; preds = %if_block153
  %209 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %209, i32 278)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else155:                                       ; preds = %if_block153
  br label %if_block160

if_end156:                                        ; preds = %if_end163, %dumy_block159
  br label %if_end149

dumy_block159:                                    ; No predecessors!
  br label %if_end156

if_block160:                                      ; preds = %if_else155
  %210 = load %StringRef** %data
  store [5 x i8] c"true\00", [5 x i8]* %const.bytes164
  %211 = getelementptr inbounds [5 x i8]* %const.bytes164, i32 0, i32 0
  %212 = getelementptr inbounds [5 x i8]* %const.bytes164, i32 0, i32 4
  %213 = getelementptr inbounds %StringRef* %const.struct165, i32 0, i32 0
  %214 = getelementptr inbounds %StringRef* %const.struct165, i32 0, i32 1
  store i8* %211, i8** %213
  store i8* %212, i8** %214
  %215 = load %StringRef* %const.struct165
  %216 = call i1 @"==266"(%StringRef* %210, %StringRef %215)
  br i1 %216, label %if_then161, label %if_else162

if_then161:                                       ; preds = %if_block160
  %217 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %217, i32 284)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else162:                                       ; preds = %if_block160
  br label %if_block167

if_end163:                                        ; preds = %if_end170, %dumy_block166
  br label %if_end156

dumy_block166:                                    ; No predecessors!
  br label %if_end163

if_block167:                                      ; preds = %if_else162
  %218 = load %StringRef** %data
  store [4 x i8] c"try\00", [4 x i8]* %const.bytes171
  %219 = getelementptr inbounds [4 x i8]* %const.bytes171, i32 0, i32 0
  %220 = getelementptr inbounds [4 x i8]* %const.bytes171, i32 0, i32 3
  %221 = getelementptr inbounds %StringRef* %const.struct172, i32 0, i32 0
  %222 = getelementptr inbounds %StringRef* %const.struct172, i32 0, i32 1
  store i8* %219, i8** %221
  store i8* %220, i8** %222
  %223 = load %StringRef* %const.struct172
  %224 = call i1 @"==266"(%StringRef* %218, %StringRef %223)
  br i1 %224, label %if_then168, label %if_else169

if_then168:                                       ; preds = %if_block167
  %225 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %225, i32 279)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else169:                                       ; preds = %if_block167
  br label %if_block174

if_end170:                                        ; preds = %if_end177, %dumy_block173
  br label %if_end163

dumy_block173:                                    ; No predecessors!
  br label %if_end170

if_block174:                                      ; preds = %if_else169
  %226 = load %StringRef** %data
  store [6 x i8] c"using\00", [6 x i8]* %const.bytes178
  %227 = getelementptr inbounds [6 x i8]* %const.bytes178, i32 0, i32 0
  %228 = getelementptr inbounds [6 x i8]* %const.bytes178, i32 0, i32 5
  %229 = getelementptr inbounds %StringRef* %const.struct179, i32 0, i32 0
  %230 = getelementptr inbounds %StringRef* %const.struct179, i32 0, i32 1
  store i8* %227, i8** %229
  store i8* %228, i8** %230
  %231 = load %StringRef* %const.struct179
  %232 = call i1 @"==266"(%StringRef* %226, %StringRef %231)
  br i1 %232, label %if_then175, label %if_else176

if_then175:                                       ; preds = %if_block174
  %233 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %233, i32 269)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else176:                                       ; preds = %if_block174
  br label %if_block181

if_end177:                                        ; preds = %if_end184, %dumy_block180
  br label %if_end170

dumy_block180:                                    ; No predecessors!
  br label %if_end177

if_block181:                                      ; preds = %if_else176
  %234 = load %StringRef** %data
  store [4 x i8] c"var\00", [4 x i8]* %const.bytes185
  %235 = getelementptr inbounds [4 x i8]* %const.bytes185, i32 0, i32 0
  %236 = getelementptr inbounds [4 x i8]* %const.bytes185, i32 0, i32 3
  %237 = getelementptr inbounds %StringRef* %const.struct186, i32 0, i32 0
  %238 = getelementptr inbounds %StringRef* %const.struct186, i32 0, i32 1
  store i8* %235, i8** %237
  store i8* %236, i8** %238
  %239 = load %StringRef* %const.struct186
  %240 = call i1 @"==266"(%StringRef* %234, %StringRef %239)
  br i1 %240, label %if_then182, label %if_else183

if_then182:                                       ; preds = %if_block181
  %241 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %241, i32 270)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else183:                                       ; preds = %if_block181
  br label %if_block188

if_end184:                                        ; preds = %if_end191, %dumy_block187
  br label %if_end177

dumy_block187:                                    ; No predecessors!
  br label %if_end184

if_block188:                                      ; preds = %if_else183
  %242 = load %StringRef** %data
  store [6 x i8] c"while\00", [6 x i8]* %const.bytes192
  %243 = getelementptr inbounds [6 x i8]* %const.bytes192, i32 0, i32 0
  %244 = getelementptr inbounds [6 x i8]* %const.bytes192, i32 0, i32 5
  %245 = getelementptr inbounds %StringRef* %const.struct193, i32 0, i32 0
  %246 = getelementptr inbounds %StringRef* %const.struct193, i32 0, i32 1
  store i8* %243, i8** %245
  store i8* %244, i8** %246
  %247 = load %StringRef* %const.struct193
  %248 = call i1 @"==266"(%StringRef* %242, %StringRef %247)
  br i1 %248, label %if_then189, label %if_else190

if_then189:                                       ; preds = %if_block188
  %249 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %249, i32 280)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_else190:                                       ; preds = %if_block188
  %250 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %250, i32 299)
  call void @dtor167(i1* %firstDot)
  call void @dtor167(i1* %allowSymbolChars)
  ret void

if_end191:                                        ; preds = %dumy_block195, %dumy_block194
  br label %if_end184

dumy_block194:                                    ; No predecessors!
  br label %if_end191

dumy_block195:                                    ; No predecessors!
  br label %if_end191
}

; Function Attrs: noinline nounwind
define void @parseNumeric331(%TokenType* sret %_result, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s) #4 {
  %_result.addr = alloca %TokenType*
  store %TokenType* %_result, %TokenType** %_result.addr
  %s.addr = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %isLong = alloca i1
  %isUnsigned = alloca i1
  %isFloating = alloca i1
  %type = alloca i32
  %ch = alloca i8
  %ch2 = alloca i8
  %ch3 = alloca i8
  %funptr = alloca i1 (i8)*
  %funptr48 = alloca i1 (i8)*
  %"$tmpForRef" = alloca %String
  %tmp.v = alloca %String
  %tmp.v82 = alloca i8
  %"$tmpC" = alloca %StringRef
  %tmp.v83 = alloca %TokenType
  %tmp.v84 = alloca %TokenType
  %tmp.v96 = alloca %TokenType
  %tmp.v97 = alloca %TokenType
  %tmp.v109 = alloca %TokenType
  %tmp.v110 = alloca %TokenType
  br label %code

code:                                             ; preds = %0
  store i1 false, i1* %isLong
  store i1 false, i1* %isUnsigned
  store i1 false, i1* %isFloating
  store i32 0, i32* %type
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  %3 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %2)
  store i8 %3, i8* %ch
  %4 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %5 = call i8 @peekChar296(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %4)
  %6 = call i8 @toLower(i8 %5)
  store i8 %6, i8* %ch2
  %7 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %8 = call i8 @peekChar329(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %7, i32 2)
  %9 = call i8 @toLower(i8 %8)
  store i8 %9, i8* %ch3
  br label %if_block

if_block:                                         ; preds = %code
  %10 = load i8* %ch
  %11 = icmp eq i8 %10, 48
  br i1 %11, label %cond.true, label %cond.false

if_then:                                          ; preds = %cond.end
  %12 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %13 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %12, i32 0, i32 1
  call void @advance307(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %13, i64 2)
  %14 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %15 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %14, i32 0, i32 1
  %16 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %17 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %16, i32 0, i32 2
  %18 = getelementptr inbounds %Token* %17, i32 0, i32 2
  %19 = call i64 @consumeDigits332(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %15, i64 16, %String* %18)
  %20 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %21 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %20, i32 0, i32 2
  %22 = getelementptr inbounds %Token* %21, i32 0, i32 3
  store i64 %19, i64* %22
  br label %if_end

if_else:                                          ; preds = %cond.end
  br label %if_block1

if_end:                                           ; preds = %if_end4, %if_then
  br label %if_block59

cond.true:                                        ; preds = %if_block
  %23 = load i8* %ch2
  %24 = icmp eq i8 %23, 120
  br label %cond.end

cond.false:                                       ; preds = %if_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ %24, %cond.true ], [ false, %cond.false ]
  br i1 %cond.res, label %if_then, label %if_else

if_block1:                                        ; preds = %if_else
  %25 = load i8* %ch
  %26 = icmp eq i8 %25, 48
  br i1 %26, label %cond.true5, label %cond.false6

if_then2:                                         ; preds = %cond.end7
  %27 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %28 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %27, i32 0, i32 1
  call void @advance307(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %28, i64 2)
  %29 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %30 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %29, i32 0, i32 1
  %31 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %32 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %31, i32 0, i32 2
  %33 = getelementptr inbounds %Token* %32, i32 0, i32 2
  %34 = call i64 @consumeDigits332(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %30, i64 2, %String* %33)
  %35 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %36 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %35, i32 0, i32 2
  %37 = getelementptr inbounds %Token* %36, i32 0, i32 3
  store i64 %34, i64* %37
  br label %if_end4

if_else3:                                         ; preds = %cond.end7
  br label %if_block9

if_end4:                                          ; preds = %if_end12, %if_then2
  br label %if_end

cond.true5:                                       ; preds = %if_block1
  %38 = load i8* %ch2
  %39 = icmp eq i8 %38, 98
  br label %cond.end7

cond.false6:                                      ; preds = %if_block1
  br label %cond.end7

cond.end7:                                        ; preds = %cond.false6, %cond.true5
  %cond.res8 = phi i1 [ %39, %cond.true5 ], [ false, %cond.false6 ]
  br i1 %cond.res8, label %if_then2, label %if_else3

if_block9:                                        ; preds = %if_else3
  %40 = load i8* %ch
  %41 = icmp eq i8 %40, 48
  br i1 %41, label %cond.true13, label %cond.false14

if_then10:                                        ; preds = %cond.end15
  %42 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %43 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %42, i32 0, i32 1
  call void @advance307(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %43, i64 1)
  %44 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %45 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %44, i32 0, i32 1
  %46 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %47 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %46, i32 0, i32 2
  %48 = getelementptr inbounds %Token* %47, i32 0, i32 2
  %49 = call i64 @consumeDigits332(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %45, i64 8, %String* %48)
  %50 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %51 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %50, i32 0, i32 2
  %52 = getelementptr inbounds %Token* %51, i32 0, i32 3
  store i64 %49, i64* %52
  br label %if_end12

if_else11:                                        ; preds = %cond.end15
  %53 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %54 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %53, i32 0, i32 1
  %55 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %56 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %55, i32 0, i32 2
  %57 = getelementptr inbounds %Token* %56, i32 0, i32 2
  %58 = call i64 @consumeDigits332(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %54, i64 10, %String* %57)
  %59 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %60 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %59, i32 0, i32 2
  %61 = getelementptr inbounds %Token* %60, i32 0, i32 3
  store i64 %58, i64* %61
  br label %if_block17

if_end12:                                         ; preds = %if_end19, %if_then10
  br label %if_end4

cond.true13:                                      ; preds = %if_block9
  %62 = load i8* %ch2
  %63 = icmp eq i8 %62, 111
  br label %cond.end15

cond.false14:                                     ; preds = %if_block9
  br label %cond.end15

cond.end15:                                       ; preds = %cond.false14, %cond.true13
  %cond.res16 = phi i1 [ %63, %cond.true13 ], [ false, %cond.false14 ]
  br i1 %cond.res16, label %if_then10, label %if_else11

if_block17:                                       ; preds = %if_else11
  %64 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %65 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %64, i32 0, i32 1
  %66 = call i1 @"pre_!!306"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %65)
  br i1 %66, label %if_then18, label %if_end19

if_then18:                                        ; preds = %if_block17
  %67 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %68 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %67, i32 0, i32 1
  %69 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %68)
  %70 = call i8 @toLower(i8 %69)
  store i8 %70, i8* %ch
  br label %if_block20

if_end19:                                         ; preds = %if_end22, %if_block17
  br label %if_end12

if_block20:                                       ; preds = %if_then18
  %71 = load i8* %ch
  %72 = icmp eq i8 %71, 46
  br i1 %72, label %cond.true32, label %cond.false33

if_then21:                                        ; preds = %cond.end25
  store i1 true, i1* %isFloating
  br label %if_block39

if_end22:                                         ; preds = %if_end51, %cond.end25
  br label %if_end19

cond.true23:                                      ; preds = %cond.end28
  br label %cond.end25

cond.false24:                                     ; preds = %cond.end28
  %73 = load i8* %ch
  %74 = icmp eq i8 %73, 100
  br label %cond.end25

cond.end25:                                       ; preds = %cond.false24, %cond.true23
  %cond.res38 = phi i1 [ true, %cond.true23 ], [ %74, %cond.false24 ]
  br i1 %cond.res38, label %if_then21, label %if_end22

cond.true26:                                      ; preds = %cond.end31
  br label %cond.end28

cond.false27:                                     ; preds = %cond.end31
  %75 = load i8* %ch
  %76 = icmp eq i8 %75, 102
  br label %cond.end28

cond.end28:                                       ; preds = %cond.false27, %cond.true26
  %cond.res37 = phi i1 [ true, %cond.true26 ], [ %76, %cond.false27 ]
  br i1 %cond.res37, label %cond.true23, label %cond.false24

cond.true29:                                      ; preds = %cond.end34
  br label %cond.end31

cond.false30:                                     ; preds = %cond.end34
  %77 = load i8* %ch
  %78 = icmp eq i8 %77, 101
  br label %cond.end31

cond.end31:                                       ; preds = %cond.false30, %cond.true29
  %cond.res36 = phi i1 [ true, %cond.true29 ], [ %78, %cond.false30 ]
  br i1 %cond.res36, label %cond.true26, label %cond.false27

cond.true32:                                      ; preds = %if_block20
  %79 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %80 = call i8 @peekChar296(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %79)
  %81 = call i1 @isOpCharDot(i8 %80)
  %82 = xor i1 true, %81
  br label %cond.end34

cond.false33:                                     ; preds = %if_block20
  br label %cond.end34

cond.end34:                                       ; preds = %cond.false33, %cond.true32
  %cond.res35 = phi i1 [ %82, %cond.true32 ], [ false, %cond.false33 ]
  br i1 %cond.res35, label %cond.true29, label %cond.false30

if_block39:                                       ; preds = %if_then21
  %83 = load i8* %ch
  %84 = icmp eq i8 %83, 46
  br i1 %84, label %if_then40, label %if_end41

if_then40:                                        ; preds = %if_block39
  %85 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @advanceAndCapture1319(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %85)
  %86 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  store i1 (i8)* @isDigit, i1 (i8)** %funptr
  %87 = bitcast i1 (i8)** %funptr to %"FunctionPtr[Bool/rtct, Char/rtct]"*
  %88 = load %"FunctionPtr[Bool/rtct, Char/rtct]"* %87
  call void @advanceAndCaptureDigit333(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %86, %"FunctionPtr[Bool/rtct, Char/rtct]" %88)
  br label %if_end41

if_end41:                                         ; preds = %if_then40, %if_block39
  br label %if_block42

if_block42:                                       ; preds = %if_end41
  %89 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %90 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %89, i32 0, i32 1
  %91 = call i1 @"pre_!!306"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %90)
  br i1 %91, label %if_then43, label %if_end44

if_then43:                                        ; preds = %if_block42
  %92 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %93 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %92, i32 0, i32 1
  %94 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %93)
  %95 = call i8 @toLower(i8 %94)
  store i8 %95, i8* %ch
  br label %if_block45

if_end44:                                         ; preds = %if_end47, %if_block42
  br label %if_block49

if_block45:                                       ; preds = %if_then43
  %96 = load i8* %ch
  %97 = icmp eq i8 %96, 101
  br i1 %97, label %if_then46, label %if_end47

if_then46:                                        ; preds = %if_block45
  %98 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @advanceAndCapture1319(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %98)
  %99 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  store i1 (i8)* @isDigit, i1 (i8)** %funptr48
  %100 = bitcast i1 (i8)** %funptr48 to %"FunctionPtr[Bool/rtct, Char/rtct]"*
  %101 = load %"FunctionPtr[Bool/rtct, Char/rtct]"* %100
  call void @advanceAndCaptureDigit333(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %99, %"FunctionPtr[Bool/rtct, Char/rtct]" %101)
  br label %if_end47

if_end47:                                         ; preds = %if_then46, %if_block45
  br label %if_end44

if_block49:                                       ; preds = %if_end44
  %102 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %103 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %102, i32 0, i32 1
  %104 = call i1 @"pre_!!306"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %103)
  br i1 %104, label %if_then50, label %if_end51

if_then50:                                        ; preds = %if_block49
  store i1 true, i1* %isLong
  %105 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %106 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %105, i32 0, i32 1
  %107 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %106)
  %108 = call i8 @toLower(i8 %107)
  store i8 %108, i8* %ch
  br label %if_block52

if_end51:                                         ; preds = %if_end55, %if_block49
  br label %if_end22

if_block52:                                       ; preds = %if_then50
  %109 = load i8* %ch
  %110 = icmp eq i8 %109, 102
  br i1 %110, label %if_then53, label %if_else54

if_then53:                                        ; preds = %if_block52
  store i1 false, i1* %isLong
  %111 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %112 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %111, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %112)
  br label %if_end55

if_else54:                                        ; preds = %if_block52
  br label %if_block56

if_end55:                                         ; preds = %if_end58, %if_then53
  br label %if_end51

if_block56:                                       ; preds = %if_else54
  %113 = load i8* %ch
  %114 = icmp eq i8 %113, 100
  br i1 %114, label %if_then57, label %if_end58

if_then57:                                        ; preds = %if_block56
  %115 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %116 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %115, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %116)
  br label %if_end58

if_end58:                                         ; preds = %if_then57, %if_block56
  br label %if_end55

if_block59:                                       ; preds = %if_end
  %117 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %118 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %117, i32 0, i32 2
  %119 = getelementptr inbounds %Token* %118, i32 0, i32 2
  %120 = call i1 @isEmpty273(%String* %119)
  br i1 %120, label %if_then60, label %if_end61

if_then60:                                        ; preds = %if_block59
  %121 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  call void @ctorFromCt334(%String* %tmp.v)
  %122 = load %String* %tmp.v
  store %String %122, %String* %"$tmpForRef"
  call void @reportError310(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %121, %String* %"$tmpForRef")
  call void @dtor186(%String* %tmp.v)
  %123 = load %TokenType** %_result.addr
  call void @ctor224(%TokenType* %123, i32 0)
  call void @dtor162(i8* %ch3)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  call void @dtor58(i32* %type)
  call void @dtor167(i1* %isFloating)
  call void @dtor167(i1* %isUnsigned)
  call void @dtor167(i1* %isLong)
  ret void

if_end61:                                         ; preds = %dumy_block, %if_block59
  br label %if_block62

dumy_block:                                       ; No predecessors!
  br label %if_end61

if_block62:                                       ; preds = %if_end61
  %124 = load i1* %isFloating
  %125 = xor i1 true, %124
  br i1 %125, label %if_then63, label %if_end64

if_then63:                                        ; preds = %if_block62
  br label %if_block65

if_end64:                                         ; preds = %if_end74, %if_block62
  br label %if_block79

if_block65:                                       ; preds = %if_then63
  %126 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %127 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %126, i32 0, i32 1
  %128 = call i1 @"pre_!!306"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %127)
  br i1 %128, label %cond.true68, label %cond.false69

if_then66:                                        ; preds = %cond.end70
  store i1 true, i1* %isUnsigned
  %129 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %130 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %129, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %130)
  br label %if_end67

if_end67:                                         ; preds = %if_then66, %cond.end70
  br label %if_block72

cond.true68:                                      ; preds = %if_block65
  %131 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %132 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %131, i32 0, i32 1
  %133 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %132)
  %134 = call i8 @toLower(i8 %133)
  %135 = icmp eq i8 %134, 117
  br label %cond.end70

cond.false69:                                     ; preds = %if_block65
  br label %cond.end70

cond.end70:                                       ; preds = %cond.false69, %cond.true68
  %cond.res71 = phi i1 [ %135, %cond.true68 ], [ false, %cond.false69 ]
  br i1 %cond.res71, label %if_then66, label %if_end67

if_block72:                                       ; preds = %if_end67
  %136 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %137 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %136, i32 0, i32 1
  %138 = call i1 @"pre_!!306"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %137)
  br i1 %138, label %cond.true75, label %cond.false76

if_then73:                                        ; preds = %cond.end77
  store i1 true, i1* %isLong
  %139 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %140 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %139, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %140)
  br label %if_end74

if_end74:                                         ; preds = %if_then73, %cond.end77
  br label %if_end64

cond.true75:                                      ; preds = %if_block72
  %141 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %142 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %141, i32 0, i32 1
  %143 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %142)
  %144 = call i8 @toLower(i8 %143)
  %145 = icmp eq i8 %144, 108
  br label %cond.end77

cond.false76:                                     ; preds = %if_block72
  br label %cond.end77

cond.end77:                                       ; preds = %cond.false76, %cond.true75
  %cond.res78 = phi i1 [ %145, %cond.true75 ], [ false, %cond.false76 ]
  br i1 %cond.res78, label %if_then73, label %if_end74

if_block79:                                       ; preds = %if_end64
  %146 = load i1* %isFloating
  br i1 %146, label %if_then80, label %if_end81

if_then80:                                        ; preds = %if_block79
  %147 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %148 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %147, i32 0, i32 2
  %149 = getelementptr inbounds %Token* %148, i32 0, i32 2
  store i8 0, i8* %tmp.v82
  %150 = load i8* %tmp.v82
  call void @"+=255"(%String* %149, i8 %150)
  call void @dtor162(i8* %tmp.v82)
  %151 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %152 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %151, i32 0, i32 2
  %153 = getelementptr inbounds %Token* %152, i32 0, i32 2
  call void @asStringRef(%StringRef* %"$tmpC", %String* %153)
  %154 = load %StringRef* %"$tmpC"
  %155 = call double @asDouble(%StringRef %154)
  %156 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %157 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %156, i32 0, i32 2
  %158 = getelementptr inbounds %Token* %157, i32 0, i32 4
  store double %155, double* %158
  call void @dtor78(%StringRef* %"$tmpC")
  %159 = load %TokenType** %_result.addr
  %160 = load i1* %isLong
  br i1 %160, label %cond_alt1, label %cond_alt2

if_end81:                                         ; preds = %cond_destruct_end87, %if_block79
  br label %if_block89

cond_alt1:                                        ; preds = %if_then80
  call void @ctor224(%TokenType* %tmp.v83, i32 308)
  br label %cond_end

cond_alt2:                                        ; preds = %if_then80
  call void @ctor224(%TokenType* %tmp.v84, i32 307)
  br label %cond_end

cond_end:                                         ; preds = %cond_alt2, %cond_alt1
  %cond = phi %TokenType* [ %tmp.v83, %cond_alt1 ], [ %tmp.v84, %cond_alt2 ]
  call void @ctor275(%TokenType* %159, %TokenType* %cond)
  br i1 %160, label %cond_destruct_alt1, label %cond_destruct_alt2

cond_destruct_alt1:                               ; preds = %cond_end
  call void @dtor201(%TokenType* %tmp.v83)
  br label %cond_destruct_end

cond_destruct_alt2:                               ; preds = %cond_end
  call void @dtor201(%TokenType* %tmp.v84)
  br label %cond_destruct_end

cond_destruct_end:                                ; preds = %cond_destruct_alt2, %cond_destruct_alt1
  call void @dtor162(i8* %ch3)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  call void @dtor58(i32* %type)
  call void @dtor167(i1* %isFloating)
  call void @dtor167(i1* %isUnsigned)
  call void @dtor167(i1* %isLong)
  ret void

cond_destruct_alt185:                             ; preds = %dumy_block88
  call void @dtor201(%TokenType* %tmp.v83)
  br label %cond_destruct_end87

cond_destruct_alt286:                             ; preds = %dumy_block88
  call void @dtor201(%TokenType* %tmp.v84)
  br label %cond_destruct_end87

cond_destruct_end87:                              ; preds = %cond_destruct_alt286, %cond_destruct_alt185
  br label %if_end81

dumy_block88:                                     ; No predecessors!
  br i1 %160, label %cond_destruct_alt185, label %cond_destruct_alt286

if_block89:                                       ; preds = %if_end81
  %161 = load i1* %isUnsigned
  br i1 %161, label %if_then90, label %if_else91

if_then90:                                        ; preds = %if_block89
  %162 = load %TokenType** %_result.addr
  %163 = load i1* %isLong
  br i1 %163, label %cond_alt193, label %cond_alt294

if_else91:                                        ; preds = %if_block89
  %164 = load %TokenType** %_result.addr
  %165 = load i1* %isLong
  br i1 %165, label %cond_alt1106, label %cond_alt2107

if_end92:                                         ; preds = %cond_destruct_end117, %cond_destruct_end104
  call void @dtor162(i8* %ch3)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  call void @dtor58(i32* %type)
  call void @dtor167(i1* %isFloating)
  call void @dtor167(i1* %isUnsigned)
  call void @dtor167(i1* %isLong)
  ret void

cond_alt193:                                      ; preds = %if_then90
  call void @ctor224(%TokenType* %tmp.v96, i32 306)
  br label %cond_end95

cond_alt294:                                      ; preds = %if_then90
  call void @ctor224(%TokenType* %tmp.v97, i32 305)
  br label %cond_end95

cond_end95:                                       ; preds = %cond_alt294, %cond_alt193
  %cond98 = phi %TokenType* [ %tmp.v96, %cond_alt193 ], [ %tmp.v97, %cond_alt294 ]
  call void @ctor275(%TokenType* %162, %TokenType* %cond98)
  br i1 %163, label %cond_destruct_alt199, label %cond_destruct_alt2100

cond_destruct_alt199:                             ; preds = %cond_end95
  call void @dtor201(%TokenType* %tmp.v96)
  br label %cond_destruct_end101

cond_destruct_alt2100:                            ; preds = %cond_end95
  call void @dtor201(%TokenType* %tmp.v97)
  br label %cond_destruct_end101

cond_destruct_end101:                             ; preds = %cond_destruct_alt2100, %cond_destruct_alt199
  call void @dtor162(i8* %ch3)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  call void @dtor58(i32* %type)
  call void @dtor167(i1* %isFloating)
  call void @dtor167(i1* %isUnsigned)
  call void @dtor167(i1* %isLong)
  ret void

cond_destruct_alt1102:                            ; preds = %dumy_block105
  call void @dtor201(%TokenType* %tmp.v96)
  br label %cond_destruct_end104

cond_destruct_alt2103:                            ; preds = %dumy_block105
  call void @dtor201(%TokenType* %tmp.v97)
  br label %cond_destruct_end104

cond_destruct_end104:                             ; preds = %cond_destruct_alt2103, %cond_destruct_alt1102
  br label %if_end92

dumy_block105:                                    ; No predecessors!
  br i1 %163, label %cond_destruct_alt1102, label %cond_destruct_alt2103

cond_alt1106:                                     ; preds = %if_else91
  call void @ctor224(%TokenType* %tmp.v109, i32 304)
  br label %cond_end108

cond_alt2107:                                     ; preds = %if_else91
  call void @ctor224(%TokenType* %tmp.v110, i32 303)
  br label %cond_end108

cond_end108:                                      ; preds = %cond_alt2107, %cond_alt1106
  %cond111 = phi %TokenType* [ %tmp.v109, %cond_alt1106 ], [ %tmp.v110, %cond_alt2107 ]
  call void @ctor275(%TokenType* %164, %TokenType* %cond111)
  br i1 %165, label %cond_destruct_alt1112, label %cond_destruct_alt2113

cond_destruct_alt1112:                            ; preds = %cond_end108
  call void @dtor201(%TokenType* %tmp.v109)
  br label %cond_destruct_end114

cond_destruct_alt2113:                            ; preds = %cond_end108
  call void @dtor201(%TokenType* %tmp.v110)
  br label %cond_destruct_end114

cond_destruct_end114:                             ; preds = %cond_destruct_alt2113, %cond_destruct_alt1112
  call void @dtor162(i8* %ch3)
  call void @dtor162(i8* %ch2)
  call void @dtor162(i8* %ch)
  call void @dtor58(i32* %type)
  call void @dtor167(i1* %isFloating)
  call void @dtor167(i1* %isUnsigned)
  call void @dtor167(i1* %isLong)
  ret void

cond_destruct_alt1115:                            ; preds = %dumy_block118
  call void @dtor201(%TokenType* %tmp.v109)
  br label %cond_destruct_end117

cond_destruct_alt2116:                            ; preds = %dumy_block118
  call void @dtor201(%TokenType* %tmp.v110)
  br label %cond_destruct_end117

cond_destruct_end117:                             ; preds = %cond_destruct_alt2116, %cond_destruct_alt1115
  br label %if_end92

dumy_block118:                                    ; No predecessors!
  br i1 %165, label %cond_destruct_alt1115, label %cond_destruct_alt2116
}

; Function Attrs: inlinehint nounwind
define private i64 @consumeDigits332(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %r, i64 %base, %String* %capture) #3 {
  %r.addr = alloca %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"*
  store %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %r, %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  %base.addr = alloca i64
  store i64 %base, i64* %base.addr
  %capture.addr = alloca %String*
  store %String* %capture, %String** %capture.addr
  %res = alloca i64
  br label %code

code:                                             ; preds = %0
  store i64 0, i64* %res
  br label %if_block

if_block:                                         ; preds = %code
  %1 = load i64* %base.addr
  %2 = call i1 @"==268"(i64 %1, i32 16)
  br i1 %2, label %if_then, label %if_else

if_then:                                          ; preds = %if_block
  br label %while_block

if_else:                                          ; preds = %if_block
  br label %while_block8

if_end:                                           ; preds = %while_end11, %while_end
  %3 = load i64* %res
  call void @dtor27(i64* %res)
  ret i64 %3

while_block:                                      ; preds = %while_step, %if_then
  %4 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  %5 = call i1 @"pre_!!306"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %4)
  br i1 %5, label %cond.true, label %cond.false

while_body:                                       ; preds = %cond.end
  br label %if_block5

while_step:                                       ; preds = %if_end7
  %6 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %6)
  br label %while_block

while_end:                                        ; preds = %cond.end
  br label %if_end

cond.true:                                        ; preds = %while_block
  %7 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  %8 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %7)
  %9 = call i1 @isXdigit(i8 %8)
  br i1 %9, label %cond.true1, label %cond.false2

cond.false:                                       ; preds = %while_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.end3
  %cond.res4 = phi i1 [ %cond.res, %cond.end3 ], [ false, %cond.false ]
  br i1 %cond.res4, label %while_body, label %while_end

cond.true1:                                       ; preds = %cond.true
  br label %cond.end3

cond.false2:                                      ; preds = %cond.true
  %10 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  %11 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %10)
  %12 = icmp eq i8 %11, 95
  br label %cond.end3

cond.end3:                                        ; preds = %cond.false2, %cond.true1
  %cond.res = phi i1 [ true, %cond.true1 ], [ %12, %cond.false2 ]
  br label %cond.end

if_block5:                                        ; preds = %while_body
  %13 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  %14 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %13)
  %15 = icmp ne i8 %14, 95
  br i1 %15, label %if_then6, label %if_end7

if_then6:                                         ; preds = %if_block5
  %16 = load %String** %capture.addr
  %17 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  %18 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %17)
  call void @"+=255"(%String* %16, i8 %18)
  %19 = load i64* %res
  %20 = load i64* %base.addr
  %21 = call i64 @"*271"(i64 %19, i64 %20)
  store i64 %21, i64* %res
  %22 = load i64* %res
  %23 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  %24 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %23)
  %25 = call i32 @getXdigitVal(i8 %24)
  %26 = call i64 @"+272"(i64 %22, i32 %25)
  store i64 %26, i64* %res
  br label %if_end7

if_end7:                                          ; preds = %if_then6, %if_block5
  br label %while_step

while_block8:                                     ; preds = %while_step10, %if_else
  %27 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  %28 = call i1 @"pre_!!306"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %27)
  br i1 %28, label %cond.true12, label %cond.false13

while_body9:                                      ; preds = %cond.end14
  br label %if_block20

while_step10:                                     ; preds = %if_end22
  %29 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %29)
  br label %while_block8

while_end11:                                      ; preds = %cond.end14
  br label %if_end

cond.true12:                                      ; preds = %while_block8
  %30 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  %31 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %30)
  %32 = call i1 @isXdigit(i8 %31)
  br i1 %32, label %cond.true15, label %cond.false16

cond.false13:                                     ; preds = %while_block8
  br label %cond.end14

cond.end14:                                       ; preds = %cond.false13, %cond.end17
  %cond.res19 = phi i1 [ %cond.res18, %cond.end17 ], [ false, %cond.false13 ]
  br i1 %cond.res19, label %while_body9, label %while_end11

cond.true15:                                      ; preds = %cond.true12
  br label %cond.end17

cond.false16:                                     ; preds = %cond.true12
  %33 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  %34 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %33)
  %35 = icmp eq i8 %34, 95
  br label %cond.end17

cond.end17:                                       ; preds = %cond.false16, %cond.true15
  %cond.res18 = phi i1 [ true, %cond.true15 ], [ %35, %cond.false16 ]
  br label %cond.end14

if_block20:                                       ; preds = %while_body9
  %36 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  %37 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %36)
  %38 = icmp ne i8 %37, 95
  br i1 %38, label %if_then21, label %if_end22

if_then21:                                        ; preds = %if_block20
  %39 = load %String** %capture.addr
  %40 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  %41 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %40)
  call void @"+=255"(%String* %39, i8 %41)
  %42 = load i64* %res
  %43 = load i64* %base.addr
  %44 = call i64 @"*271"(i64 %42, i64 %43)
  store i64 %44, i64* %res
  %45 = load i64* %res
  %46 = load %"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"** %r.addr
  %47 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %46)
  %48 = call i32 @getDigitVal(i8 %47)
  %49 = call i64 @"+272"(i64 %45, i32 %48)
  store i64 %49, i64* %res
  br label %if_end22

if_end22:                                         ; preds = %if_then21, %if_block20
  br label %while_step10

dumy_block:                                       ; No predecessors!
  call void @dtor27(i64* %res)
  unreachable
}

; Function Attrs: inlinehint nounwind
define private void @advanceAndCaptureDigit333(%"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, %"FunctionPtr[Bool/rtct, Char/rtct]" %pred) #3 {
  %s.addr = alloca %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"*
  store %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %s, %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %pred.addr = alloca %"FunctionPtr[Bool/rtct, Char/rtct]"
  store %"FunctionPtr[Bool/rtct, Char/rtct]" %pred, %"FunctionPtr[Bool/rtct, Char/rtct]"* %pred.addr
  br label %code

code:                                             ; preds = %0
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %1 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %2 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %1, i32 0, i32 1
  %3 = call i1 @"pre_!!306"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %2)
  br i1 %3, label %cond.true, label %cond.false

while_body:                                       ; preds = %cond.end
  br label %if_block

while_step:                                       ; preds = %if_end
  %4 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %5 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %4, i32 0, i32 1
  call void @popFront299(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %5)
  br label %while_block

while_end:                                        ; preds = %cond.end
  ret void

cond.true:                                        ; preds = %while_block
  %6 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %7 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %6, i32 0, i32 1
  %8 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %7)
  %9 = call i1 @"()232"(%"FunctionPtr[Bool/rtct, Char/rtct]"* %pred.addr, i8 %8)
  br i1 %9, label %cond.true1, label %cond.false2

cond.false:                                       ; preds = %while_block
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.end3
  %cond.res4 = phi i1 [ %cond.res, %cond.end3 ], [ false, %cond.false ]
  br i1 %cond.res4, label %while_body, label %while_end

cond.true1:                                       ; preds = %cond.true
  br label %cond.end3

cond.false2:                                      ; preds = %cond.true
  %10 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %11 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %10, i32 0, i32 1
  %12 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %11)
  %13 = icmp eq i8 %12, 95
  br label %cond.end3

cond.end3:                                        ; preds = %cond.false2, %cond.true1
  %cond.res = phi i1 [ true, %cond.true1 ], [ %13, %cond.false2 ]
  br label %cond.end

if_block:                                         ; preds = %while_body
  %14 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %15 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %14, i32 0, i32 1
  %16 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %15)
  %17 = icmp ne i8 %16, 95
  br i1 %17, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  %18 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %19 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %18, i32 0, i32 2
  %20 = getelementptr inbounds %Token* %19, i32 0, i32 2
  %21 = load %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"** %s.addr
  %22 = getelementptr inbounds %"SparrowScanner[StringRef/rtct, ExternalErrorReporter]"* %21, i32 0, i32 1
  %23 = call i8 @"pre_*293"(%"LocationSyncCharRange[RangeWithLookahead[StringRef/rtct]]"* %22)
  call void @"+=255"(%String* %20, i8 %23)
  br label %if_end

if_end:                                           ; preds = %if_then, %if_block
  br label %while_step
}

; Function Attrs: inlinehint nounwind
define private void @ctorFromCt334(%String* %"$this") #3 {
  %"$this.addr" = alloca %String*
  store %String* %"$this", %String** %"$this.addr"
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [24 x i8]
  %const.struct = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %"$this.addr"
  store [24 x i8] c"Invalid numeric literal\00", [24 x i8]* %const.bytes
  %2 = getelementptr inbounds [24 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [24 x i8]* %const.bytes, i32 0, i32 23
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  call void @ctor251(%String* %1, %StringRef* %"$tmpForRef")
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i1 @isOctalDigit(i8 %c) #2 {
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  %2 = icmp sle i8 48, %1
  br i1 %2, label %cond.true, label %cond.false

cond.true:                                        ; preds = %code
  %3 = load i8* %c.addr
  %4 = icmp sle i8 %3, 55
  br label %cond.end

cond.false:                                       ; preds = %code
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ %4, %cond.true ], [ false, %cond.false ]
  ret i1 %cond.res
}

; Function Attrs: alwaysinline nounwind
define private i1 @isBinDigit(i8 %c) #2 {
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  %2 = icmp eq i8 %1, 48
  br i1 %2, label %cond.true, label %cond.false

cond.true:                                        ; preds = %code
  br label %cond.end

cond.false:                                       ; preds = %code
  %3 = load i8* %c.addr
  %4 = icmp eq i8 %3, 49
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond.res = phi i1 [ true, %cond.true ], [ %4, %cond.false ]
  ret i1 %cond.res
}

; Function Attrs: inlinehint nounwind
define private void @mkLocation(%Location* sret %_result) #3 {
  %_result.addr = alloca %Location*
  store %Location* %_result, %Location** %_result.addr
  %res = alloca %Location
  br label %code

code:                                             ; preds = %0
  call void @ctor112(%Location* %res)
  call void @setOne(%Location* %res)
  %1 = load %Location** %_result.addr
  call void @ctor134(%Location* %1, %Location* %res)
  call void @dtor202(%Location* %res)
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor202(%Location* %res)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @setOne(%Location* %l) #3 {
  %l.addr = alloca %Location*
  store %Location* %l, %Location** %l.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %Location** %l.addr
  %2 = getelementptr inbounds %Location* %1, i32 0, i32 1
  %3 = getelementptr inbounds %LineCol* %2, i32 0, i32 0
  store i32 1, i32* %3
  %4 = load %Location** %l.addr
  %5 = getelementptr inbounds %Location* %4, i32 0, i32 1
  %6 = getelementptr inbounds %LineCol* %5, i32 0, i32 1
  store i32 1, i32* %6
  %7 = load %Location** %l.addr
  %8 = getelementptr inbounds %Location* %7, i32 0, i32 2
  %9 = getelementptr inbounds %LineCol* %8, i32 0, i32 0
  store i32 1, i32* %9
  %10 = load %Location** %l.addr
  %11 = getelementptr inbounds %Location* %10, i32 0, i32 2
  %12 = getelementptr inbounds %LineCol* %11, i32 0, i32 1
  store i32 1, i32* %12
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @mkLocation335(%Location* sret %_result, %SourceCode %sourceCode) #3 {
  %_result.addr = alloca %Location*
  store %Location* %_result, %Location** %_result.addr
  %sourceCode.addr = alloca %SourceCode
  store %SourceCode %sourceCode, %SourceCode* %sourceCode.addr
  %res = alloca %Location
  br label %code

code:                                             ; preds = %0
  call void @ctor112(%Location* %res)
  %1 = getelementptr inbounds %Location* %res, i32 0, i32 0
  call void @"=212"(%SourceCode* %1, %SourceCode* %sourceCode.addr)
  call void @setOne(%Location* %res)
  %2 = load %Location** %_result.addr
  call void @ctor134(%Location* %2, %Location* %res)
  call void @dtor202(%Location* %res)
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor202(%Location* %res)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @copyStart(%Location* %l, %Location* %other) #3 {
  %l.addr = alloca %Location*
  store %Location* %l, %Location** %l.addr
  %other.addr = alloca %Location*
  store %Location* %other, %Location** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %Location** %l.addr
  %2 = getelementptr inbounds %Location* %1, i32 0, i32 1
  %3 = load %Location** %other.addr
  %4 = getelementptr inbounds %Location* %3, i32 0, i32 1
  call void @"=214"(%LineCol* %2, %LineCol* %4)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @copyEnd(%Location* %l, %Location* %other) #3 {
  %l.addr = alloca %Location*
  store %Location* %l, %Location** %l.addr
  %other.addr = alloca %Location*
  store %Location* %other, %Location** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %Location** %l.addr
  %2 = getelementptr inbounds %Location* %1, i32 0, i32 2
  %3 = load %Location** %other.addr
  %4 = getelementptr inbounds %Location* %3, i32 0, i32 2
  call void @"=214"(%LineCol* %2, %LineCol* %4)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @setAsStartOf(%Location* %l, %Location* %other) #3 {
  %l.addr = alloca %Location*
  store %Location* %l, %Location** %l.addr
  %other.addr = alloca %Location*
  store %Location* %other, %Location** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %Location** %l.addr
  %2 = getelementptr inbounds %Location* %1, i32 0, i32 1
  %3 = load %Location** %other.addr
  %4 = getelementptr inbounds %Location* %3, i32 0, i32 1
  call void @"=214"(%LineCol* %2, %LineCol* %4)
  %5 = load %Location** %l.addr
  %6 = getelementptr inbounds %Location* %5, i32 0, i32 2
  %7 = load %Location** %other.addr
  %8 = getelementptr inbounds %Location* %7, i32 0, i32 1
  call void @"=214"(%LineCol* %6, %LineCol* %8)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @setAsEndOf(%Location* %l, %Location* %other) #3 {
  %l.addr = alloca %Location*
  store %Location* %l, %Location** %l.addr
  %other.addr = alloca %Location*
  store %Location* %other, %Location** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %Location** %l.addr
  %2 = getelementptr inbounds %Location* %1, i32 0, i32 1
  %3 = load %Location** %other.addr
  %4 = getelementptr inbounds %Location* %3, i32 0, i32 2
  call void @"=214"(%LineCol* %2, %LineCol* %4)
  %5 = load %Location** %l.addr
  %6 = getelementptr inbounds %Location* %5, i32 0, i32 2
  %7 = load %Location** %other.addr
  %8 = getelementptr inbounds %Location* %7, i32 0, i32 2
  call void @"=214"(%LineCol* %6, %LineCol* %8)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @"+336"(%String* sret %_result, %String %x, %String %y) #3 {
  %_result.addr = alloca %String*
  store %String* %_result, %String** %_result.addr
  %x.addr = alloca %String
  store %String %x, %String* %x.addr
  %y.addr = alloca %String
  store %String %y, %String* %y.addr
  %sz1 = alloca i64
  %sz2 = alloca i64
  %res = alloca %String
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = call i64 @size174(%String* %x.addr)
  store i64 %1, i64* %sz1
  %2 = call i64 @size174(%String* %y.addr)
  store i64 %2, i64* %sz2
  %3 = load i64* %sz1
  %4 = load i64* %sz2
  %5 = call i64 @"+29"(i64 %3, i64 %4)
  call void @ctor184(%String* %res, i64 %5)
  %6 = getelementptr inbounds %String* %res, i32 0, i32 0
  %7 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %6)
  %8 = getelementptr inbounds %String* %x.addr, i32 0, i32 0
  %9 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %8)
  %10 = load i64* %sz1
  call void @_spr_memcpy(i8* %7, i8* %9, i64 %10)
  %11 = getelementptr inbounds %String* %res, i32 0, i32 0
  %12 = load i64* %sz1
  call void @advance(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %11, i64 %12)
  %13 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %"$tmpC")
  %14 = getelementptr inbounds %String* %y.addr, i32 0, i32 0
  %15 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %14)
  %16 = load i64* %sz2
  call void @_spr_memcpy(i8* %13, i8* %15, i64 %16)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  %17 = load %String** %_result.addr
  call void @ctor183(%String* %17, %String* %res)
  call void @dtor186(%String* %res)
  call void @dtor31(i64* %sz2)
  call void @dtor31(i64* %sz1)
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor186(%String* %res)
  call void @dtor31(i64* %sz2)
  call void @dtor31(i64* %sz1)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @"+337"(%String* sret %_result, %String %x, %StringRef %y) #3 {
  %_result.addr = alloca %String*
  store %String* %_result, %String** %_result.addr
  %x.addr = alloca %String
  store %String %x, %String* %x.addr
  %y.addr = alloca %StringRef
  store %StringRef %y, %StringRef* %y.addr
  %sz1 = alloca i64
  %sz2 = alloca i64
  %res = alloca %String
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = call i64 @size174(%String* %x.addr)
  store i64 %1, i64* %sz1
  %2 = call i64 @size(%StringRef* %y.addr)
  store i64 %2, i64* %sz2
  %3 = load i64* %sz1
  %4 = load i64* %sz2
  %5 = call i64 @"+29"(i64 %3, i64 %4)
  call void @ctor184(%String* %res, i64 %5)
  %6 = getelementptr inbounds %String* %res, i32 0, i32 0
  %7 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %6)
  %8 = getelementptr inbounds %String* %x.addr, i32 0, i32 0
  %9 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %8)
  %10 = load i64* %sz1
  call void @_spr_memcpy(i8* %7, i8* %9, i64 %10)
  %11 = getelementptr inbounds %String* %res, i32 0, i32 0
  %12 = load i64* %sz1
  call void @advance(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %11, i64 %12)
  %13 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %"$tmpC")
  %14 = getelementptr inbounds %StringRef* %y.addr, i32 0, i32 0
  %15 = load i8** %14
  %16 = load i64* %sz2
  call void @_spr_memcpy(i8* %13, i8* %15, i64 %16)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  %17 = load %String** %_result.addr
  call void @ctor183(%String* %17, %String* %res)
  call void @dtor186(%String* %res)
  call void @dtor31(i64* %sz2)
  call void @dtor31(i64* %sz1)
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor186(%String* %res)
  call void @dtor31(i64* %sz2)
  call void @dtor31(i64* %sz1)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @ulongToString(%String* sret %_result, i64 %x) #3 {
  %_result.addr = alloca %String*
  store %String* %_result, %String** %_result.addr
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %buf = alloca [16 x i8]
  %"$tmpC" = alloca %StringRef
  br label %code

code:                                             ; preds = %0
  %1 = bitcast [16 x i8]* %buf to i8*
  store i8 0, i8* %1
  %2 = load i64* %x.addr
  %3 = bitcast [16 x i8]* %buf to i8*
  call void @_ULong_to_CString(i64 %2, i8* %3)
  %4 = load %String** %_result.addr
  %5 = bitcast [16 x i8]* %buf to i8*
  call void @_String_fromCString(%StringRef* %"$tmpC", i8* %5)
  call void @ctor251(%String* %4, %StringRef* %"$tmpC")
  call void @dtor78(%StringRef* %"$tmpC")
  %6 = bitcast [16 x i8]* %buf to i8*
  call void @dtor(i8* %6)
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor78(%StringRef* %"$tmpC")
  %7 = bitcast [16 x i8]* %buf to i8*
  call void @dtor(i8* %7)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @toString338(%String* sret %_result) #2 {
  %_result.addr = alloca %String*
  store %String* %_result, %String** %_result.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %String** %_result.addr
  call void @ctor123(%String* %1)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i8* @back339(%"ContiguousMemoryRange[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %"$this", %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %1, i32 0, i32 1
  call void @advance175(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %2, i64 -1)
  %3 = call i8* @value(%"RawPtr[Char/rtct]"* %"$tmpC")
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  ret i8* %3

dumy_block:                                       ; No predecessors!
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i8* @"()340"(%"ContiguousMemoryRange[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %"$this", %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %1, i32 0, i32 0
  %3 = call i8* @value(%"RawPtr[Char/rtct]"* %2)
  ret i8* %3
}

; Function Attrs: alwaysinline nounwind
define private i8* @"()341"(%"ContiguousMemoryRange[Char/rtct]"* %"$this", i64 %n) #2 {
  %"$this.addr" = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %"$this", %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %n.addr = alloca i64
  store i64 %n, i64* %n.addr
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %1, i32 0, i32 0
  %3 = load i64* %n.addr
  store i64 %3, i64* %tmp.v
  %4 = load i64* %tmp.v
  call void @advance175(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %2, i64 %4)
  %5 = call i8* @value(%"RawPtr[Char/rtct]"* %"$tmpC")
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  call void @dtor35(i64* %tmp.v)
  ret i8* %5

dumy_block:                                       ; No predecessors!
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  call void @dtor35(i64* %tmp.v)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private void @popBack342(%"ContiguousMemoryRange[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %"$this", %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %1, i32 0, i32 1
  %3 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"ContiguousMemoryRange[Char/rtct]"* %3, i32 0, i32 1
  call void @advance175(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %4, i64 -1)
  %5 = load %"RawPtr[Char/rtct]"* %"$tmpC"
  call void @"=144"(%"RawPtr[Char/rtct]"* %2, %"RawPtr[Char/rtct]" %5)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i8* @"pre_++343"(%"ContiguousMemoryRange[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %"$this", %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  call void @popFront180(%"ContiguousMemoryRange[Char/rtct]"* %1)
  %2 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %3 = call i8* @front179(%"ContiguousMemoryRange[Char/rtct]"* %2)
  ret i8* %3
}

; Function Attrs: alwaysinline nounwind
define private i8* @"post_++344"(%"ContiguousMemoryRange[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %"$this", %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %res = alloca i8*
  br label %code

code:                                             ; preds = %0
  %1 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  %2 = call i8* @front179(%"ContiguousMemoryRange[Char/rtct]"* %1)
  store i8* %2, i8** %res
  %3 = load %"ContiguousMemoryRange[Char/rtct]"** %"$this.addr"
  call void @popFront180(%"ContiguousMemoryRange[Char/rtct]"* %3)
  %4 = load i8** %res
  ret i8* %4
}

; Function Attrs: inlinehint nounwind
define private void @fromRef(%"RawPtr[Char/rtct]"* sret %_result, i8* %ptr) #3 {
  %_result.addr = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %_result, %"RawPtr[Char/rtct]"** %_result.addr
  %ptr.addr = alloca i8*
  store i8* %ptr, i8** %ptr.addr
  %res = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  call void @ctor120(%"RawPtr[Char/rtct]"* %res)
  %1 = load i8** %ptr.addr
  %2 = getelementptr inbounds %"RawPtr[Char/rtct]"* %res, i32 0, i32 0
  store i8* %1, i8** %2
  %3 = load %"RawPtr[Char/rtct]"** %_result.addr
  call void @ctor145(%"RawPtr[Char/rtct]"* %3, %"RawPtr[Char/rtct]"* %res)
  call void @dtor146(%"RawPtr[Char/rtct]"* %res)
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor146(%"RawPtr[Char/rtct]"* %res)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i1 @isNull(%"RawPtr[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]"** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %"RawPtr[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"RawPtr[Char/rtct]"* %1, i32 0, i32 0
  %3 = load i8** %2
  %4 = call i1 @_opRefEQ(i8* %3, i8* null)
  ret i1 %4
}

; Function Attrs: alwaysinline nounwind
define private i1 @"<345"(%"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]" %other) #2 {
  %"$this.addr" = alloca %"RawPtr[Char/rtct]"*
  store %"RawPtr[Char/rtct]"* %"$this", %"RawPtr[Char/rtct]"** %"$this.addr"
  %other.addr = alloca %"RawPtr[Char/rtct]"
  store %"RawPtr[Char/rtct]" %other, %"RawPtr[Char/rtct]"* %other.addr
  %tmp.v = alloca i64
  br label %code

code:                                             ; preds = %0
  %1 = load %"RawPtr[Char/rtct]"** %"$this.addr"
  %2 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %1)
  %3 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %other.addr)
  %4 = call i64 @ptrDiff(i8* %2, i8* %3)
  store i64 0, i64* %tmp.v
  %5 = load i64* %tmp.v
  %6 = call i1 @"<346"(i64 %4, i64 %5)
  call void @dtor35(i64* %tmp.v)
  ret i1 %6

dumy_block:                                       ; No predecessors!
  call void @dtor35(i64* %tmp.v)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i1 @"<346"(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn34(i64 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn34(i64 %4, i64 %5)
  %7 = icmp slt i64 %3, %6
  ret i1 %7
}

; Function Attrs: nounwind readnone
declare float @llvm.sqrt.f32(float) #5

; Function Attrs: nounwind readnone
declare float @llvm.sin.f32(float) #5

; Function Attrs: nounwind readnone
declare float @llvm.cos.f32(float) #5

; Function Attrs: nounwind readnone
declare float @llvm.powi.f32(float, i32) #5

; Function Attrs: nounwind readnone
declare float @llvm.pow.f32(float, float) #5

; Function Attrs: nounwind readnone
declare float @llvm.exp.f32(float) #5

; Function Attrs: nounwind readnone
declare float @llvm.exp2.f32(float) #5

; Function Attrs: nounwind readnone
declare float @llvm.log.f32(float) #5

; Function Attrs: nounwind readnone
declare float @llvm.log10.f32(float) #5

; Function Attrs: nounwind readnone
declare float @llvm.log2.f32(float) #5

; Function Attrs: nounwind readnone
declare float @llvm.fabs.f32(float) #5

; Function Attrs: nounwind readnone
declare float @llvm.floor.f32(float) #5

; Function Attrs: nounwind readnone
declare float @llvm.ceil.f32(float) #5

declare float @llvm.truc.f32(float)

; Function Attrs: nounwind readnone
declare float @llvm.rint.f32(float) #5

; Function Attrs: nounwind readnone
declare double @llvm.sqrt.f64(double) #5

; Function Attrs: nounwind readnone
declare double @llvm.sin.f64(double) #5

; Function Attrs: nounwind readnone
declare double @llvm.cos.f64(double) #5

; Function Attrs: nounwind readnone
declare double @llvm.powi.f64(double, i32) #5

; Function Attrs: nounwind readnone
declare double @llvm.pow.f64(double, double) #5

; Function Attrs: nounwind readnone
declare double @llvm.exp.f64(double) #5

; Function Attrs: nounwind readnone
declare double @llvm.exp2.f64(double) #5

; Function Attrs: nounwind readnone
declare double @llvm.log.f64(double) #5

; Function Attrs: nounwind readnone
declare double @llvm.log10.f64(double) #5

; Function Attrs: nounwind readnone
declare double @llvm.log2.f64(double) #5

; Function Attrs: nounwind readnone
declare double @llvm.fabs.f64(double) #5

; Function Attrs: nounwind readnone
declare double @llvm.floor.f64(double) #5

; Function Attrs: nounwind readnone
declare double @llvm.ceil.f64(double) #5

declare double @llvm.truc.f64(double)

; Function Attrs: nounwind readnone
declare double @llvm.rint.f64(double) #5

; Function Attrs: alwaysinline nounwind
define private i1 @isAlnum(i8 %c) #2 {
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  %tmp.v = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  call void @_ass_32_8z(i32* %tmp.v, i8 %1)
  %2 = load i32* %tmp.v
  %3 = call i32 @isalnum(i32 %2)
  %4 = call i1 @"!=163"(i32 0, i32 %3)
  call void @dtor58(i32* %tmp.v)
  ret i1 %4

dumy_block:                                       ; No predecessors!
  call void @dtor58(i32* %tmp.v)
  unreachable
}

declare i32 @isalnum(i32)

; Function Attrs: alwaysinline nounwind
define private i1 @isBlank(i8 %c) #2 {
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  %tmp.v = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  call void @_ass_32_8z(i32* %tmp.v, i8 %1)
  %2 = load i32* %tmp.v
  %3 = call i32 @isblank(i32 %2)
  %4 = call i1 @"!=163"(i32 0, i32 %3)
  call void @dtor58(i32* %tmp.v)
  ret i1 %4

dumy_block:                                       ; No predecessors!
  call void @dtor58(i32* %tmp.v)
  unreachable
}

declare i32 @isblank(i32)

; Function Attrs: alwaysinline nounwind
define private i1 @isCntrl(i8 %c) #2 {
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  %tmp.v = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  call void @_ass_32_8z(i32* %tmp.v, i8 %1)
  %2 = load i32* %tmp.v
  %3 = call i32 @iscntrl(i32 %2)
  %4 = call i1 @"!=163"(i32 0, i32 %3)
  call void @dtor58(i32* %tmp.v)
  ret i1 %4

dumy_block:                                       ; No predecessors!
  call void @dtor58(i32* %tmp.v)
  unreachable
}

declare i32 @iscntrl(i32)

; Function Attrs: alwaysinline nounwind
define private i1 @isGraph(i8 %c) #2 {
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  %tmp.v = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  call void @_ass_32_8z(i32* %tmp.v, i8 %1)
  %2 = load i32* %tmp.v
  %3 = call i32 @isgraph(i32 %2)
  %4 = call i1 @"!=163"(i32 0, i32 %3)
  call void @dtor58(i32* %tmp.v)
  ret i1 %4

dumy_block:                                       ; No predecessors!
  call void @dtor58(i32* %tmp.v)
  unreachable
}

declare i32 @isgraph(i32)

; Function Attrs: alwaysinline nounwind
define private i1 @isLower(i8 %c) #2 {
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  %tmp.v = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  call void @_ass_32_8z(i32* %tmp.v, i8 %1)
  %2 = load i32* %tmp.v
  %3 = call i32 @islower(i32 %2)
  %4 = call i1 @"!=163"(i32 0, i32 %3)
  call void @dtor58(i32* %tmp.v)
  ret i1 %4

dumy_block:                                       ; No predecessors!
  call void @dtor58(i32* %tmp.v)
  unreachable
}

declare i32 @islower(i32)

; Function Attrs: alwaysinline nounwind
define private i1 @isPrint(i8 %c) #2 {
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  %tmp.v = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  call void @_ass_32_8z(i32* %tmp.v, i8 %1)
  %2 = load i32* %tmp.v
  %3 = call i32 @isprint(i32 %2)
  %4 = call i1 @"!=163"(i32 0, i32 %3)
  call void @dtor58(i32* %tmp.v)
  ret i1 %4

dumy_block:                                       ; No predecessors!
  call void @dtor58(i32* %tmp.v)
  unreachable
}

declare i32 @isprint(i32)

; Function Attrs: alwaysinline nounwind
define private i1 @isPunct(i8 %c) #2 {
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  %tmp.v = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  call void @_ass_32_8z(i32* %tmp.v, i8 %1)
  %2 = load i32* %tmp.v
  %3 = call i32 @ispunct(i32 %2)
  %4 = call i1 @"!=163"(i32 0, i32 %3)
  call void @dtor58(i32* %tmp.v)
  ret i1 %4

dumy_block:                                       ; No predecessors!
  call void @dtor58(i32* %tmp.v)
  unreachable
}

declare i32 @ispunct(i32)

; Function Attrs: alwaysinline nounwind
define private i1 @isUpper(i8 %c) #2 {
  %c.addr = alloca i8
  store i8 %c, i8* %c.addr
  %tmp.v = alloca i32
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %c.addr
  call void @_ass_32_8z(i32* %tmp.v, i8 %1)
  %2 = load i32* %tmp.v
  %3 = call i32 @isupper(i32 %2)
  %4 = call i1 @"!=163"(i32 0, i32 %3)
  call void @dtor58(i32* %tmp.v)
  ret i1 %4

dumy_block:                                       ; No predecessors!
  call void @dtor58(i32* %tmp.v)
  unreachable
}

declare i32 @isupper(i32)

; Function Attrs: inlinehint nounwind
define private void @ctor347(%"Vector[Char/rtct]"* %"$this", i64 %size) #3 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %size.addr = alloca i64
  store i64 %size, i64* %size.addr
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  %"$tmpC1" = alloca %"RawPtr[Char/rtct]"
  %p = alloca %"RawPtr[Char/rtct]"
  %"$tmpC2" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"Vector[Char/rtct]"* %1, i32 0, i32 0
  call void @ctor120(%"RawPtr[Char/rtct]"* %2)
  %3 = load %"Vector[Char/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"Vector[Char/rtct]"* %3, i32 0, i32 1
  call void @ctor120(%"RawPtr[Char/rtct]"* %4)
  %5 = load %"Vector[Char/rtct]"** %"$this.addr"
  %6 = getelementptr inbounds %"Vector[Char/rtct]"* %5, i32 0, i32 2
  call void @ctor120(%"RawPtr[Char/rtct]"* %6)
  %7 = load %"Vector[Char/rtct]"** %"$this.addr"
  %8 = getelementptr inbounds %"Vector[Char/rtct]"* %7, i32 0, i32 0
  %9 = load i64* %size.addr
  call void @alloc(%"RawPtr[Char/rtct]"* %"$tmpC", i64 %9)
  %10 = load %"RawPtr[Char/rtct]"* %"$tmpC"
  call void @"=144"(%"RawPtr[Char/rtct]"* %8, %"RawPtr[Char/rtct]" %10)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  %11 = load %"Vector[Char/rtct]"** %"$this.addr"
  %12 = getelementptr inbounds %"Vector[Char/rtct]"* %11, i32 0, i32 1
  %13 = load %"Vector[Char/rtct]"** %"$this.addr"
  %14 = getelementptr inbounds %"Vector[Char/rtct]"* %13, i32 0, i32 0
  %15 = load i64* %size.addr
  call void @advance(%"RawPtr[Char/rtct]"* %"$tmpC1", %"RawPtr[Char/rtct]"* %14, i64 %15)
  %16 = load %"RawPtr[Char/rtct]"* %"$tmpC1"
  call void @"=144"(%"RawPtr[Char/rtct]"* %12, %"RawPtr[Char/rtct]" %16)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC1")
  %17 = load %"Vector[Char/rtct]"** %"$this.addr"
  %18 = getelementptr inbounds %"Vector[Char/rtct]"* %17, i32 0, i32 2
  %19 = load %"Vector[Char/rtct]"** %"$this.addr"
  %20 = getelementptr inbounds %"Vector[Char/rtct]"* %19, i32 0, i32 1
  %21 = load %"RawPtr[Char/rtct]"* %20
  call void @"=144"(%"RawPtr[Char/rtct]"* %18, %"RawPtr[Char/rtct]" %21)
  %22 = load %"Vector[Char/rtct]"** %"$this.addr"
  %23 = getelementptr inbounds %"Vector[Char/rtct]"* %22, i32 0, i32 0
  call void @ctor145(%"RawPtr[Char/rtct]"* %p, %"RawPtr[Char/rtct]"* %23)
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %24 = load %"Vector[Char/rtct]"** %"$this.addr"
  %25 = getelementptr inbounds %"Vector[Char/rtct]"* %24, i32 0, i32 1
  %26 = load %"RawPtr[Char/rtct]"* %25
  %27 = call i1 @"==148"(%"RawPtr[Char/rtct]"* %p, %"RawPtr[Char/rtct]" %26)
  %28 = xor i1 true, %27
  br i1 %28, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  %29 = call i8* @value(%"RawPtr[Char/rtct]"* %p)
  store i8 0, i8* %29
  br label %while_step

while_step:                                       ; preds = %while_body
  call void @advance149(%"RawPtr[Char/rtct]"* %"$tmpC2", %"RawPtr[Char/rtct]"* %p)
  %30 = load %"RawPtr[Char/rtct]"* %"$tmpC2"
  call void @"=144"(%"RawPtr[Char/rtct]"* %p, %"RawPtr[Char/rtct]" %30)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC2")
  br label %while_block

while_end:                                        ; preds = %while_block
  call void @dtor146(%"RawPtr[Char/rtct]"* %p)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @swap348(%"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"* %other) #3 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %other.addr = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %other, %"Vector[Char/rtct]"** %other.addr
  %tmp = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"Vector[Char/rtct]"* %1, i32 0, i32 0
  call void @ctor145(%"RawPtr[Char/rtct]"* %tmp, %"RawPtr[Char/rtct]"* %2)
  %3 = load %"Vector[Char/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"Vector[Char/rtct]"* %3, i32 0, i32 0
  %5 = load %"Vector[Char/rtct]"** %other.addr
  %6 = getelementptr inbounds %"Vector[Char/rtct]"* %5, i32 0, i32 0
  %7 = load %"RawPtr[Char/rtct]"* %6
  call void @"=144"(%"RawPtr[Char/rtct]"* %4, %"RawPtr[Char/rtct]" %7)
  %8 = load %"Vector[Char/rtct]"** %other.addr
  %9 = getelementptr inbounds %"Vector[Char/rtct]"* %8, i32 0, i32 0
  %10 = load %"RawPtr[Char/rtct]"* %tmp
  call void @"=144"(%"RawPtr[Char/rtct]"* %9, %"RawPtr[Char/rtct]" %10)
  %11 = load %"Vector[Char/rtct]"** %"$this.addr"
  %12 = getelementptr inbounds %"Vector[Char/rtct]"* %11, i32 0, i32 1
  %13 = load %"RawPtr[Char/rtct]"* %12
  call void @"=144"(%"RawPtr[Char/rtct]"* %tmp, %"RawPtr[Char/rtct]" %13)
  %14 = load %"Vector[Char/rtct]"** %"$this.addr"
  %15 = getelementptr inbounds %"Vector[Char/rtct]"* %14, i32 0, i32 1
  %16 = load %"Vector[Char/rtct]"** %other.addr
  %17 = getelementptr inbounds %"Vector[Char/rtct]"* %16, i32 0, i32 1
  %18 = load %"RawPtr[Char/rtct]"* %17
  call void @"=144"(%"RawPtr[Char/rtct]"* %15, %"RawPtr[Char/rtct]" %18)
  %19 = load %"Vector[Char/rtct]"** %other.addr
  %20 = getelementptr inbounds %"Vector[Char/rtct]"* %19, i32 0, i32 1
  %21 = load %"RawPtr[Char/rtct]"* %tmp
  call void @"=144"(%"RawPtr[Char/rtct]"* %20, %"RawPtr[Char/rtct]" %21)
  %22 = load %"Vector[Char/rtct]"** %"$this.addr"
  %23 = getelementptr inbounds %"Vector[Char/rtct]"* %22, i32 0, i32 2
  %24 = load %"RawPtr[Char/rtct]"* %23
  call void @"=144"(%"RawPtr[Char/rtct]"* %tmp, %"RawPtr[Char/rtct]" %24)
  %25 = load %"Vector[Char/rtct]"** %"$this.addr"
  %26 = getelementptr inbounds %"Vector[Char/rtct]"* %25, i32 0, i32 2
  %27 = load %"Vector[Char/rtct]"** %other.addr
  %28 = getelementptr inbounds %"Vector[Char/rtct]"* %27, i32 0, i32 2
  %29 = load %"RawPtr[Char/rtct]"* %28
  call void @"=144"(%"RawPtr[Char/rtct]"* %26, %"RawPtr[Char/rtct]" %29)
  %30 = load %"Vector[Char/rtct]"** %other.addr
  %31 = getelementptr inbounds %"Vector[Char/rtct]"* %30, i32 0, i32 2
  %32 = load %"RawPtr[Char/rtct]"* %tmp
  call void @"=144"(%"RawPtr[Char/rtct]"* %31, %"RawPtr[Char/rtct]" %32)
  call void @dtor146(%"RawPtr[Char/rtct]"* %tmp)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i8* @at349(%"Vector[Char/rtct]"* %"$this", i64 %index) #2 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %index.addr = alloca i64
  store i64 %index, i64* %index.addr
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"Vector[Char/rtct]"* %1, i32 0, i32 0
  %3 = load i64* %index.addr
  call void @advance(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %2, i64 %3)
  %4 = call i8* @value(%"RawPtr[Char/rtct]"* %"$tmpC")
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  ret i8* %4

dumy_block:                                       ; No predecessors!
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i8* @back350(%"Vector[Char/rtct]"* %"$this") #2 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"Vector[Char/rtct]"* %1, i32 0, i32 1
  call void @advance175(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %2, i64 -1)
  %3 = call i8* @value(%"RawPtr[Char/rtct]"* %"$tmpC")
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  ret i8* %3

dumy_block:                                       ; No predecessors!
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private void @subrange(%"ContiguousMemoryRange[Char/rtct]"* sret %_result, %"Vector[Char/rtct]"* %"$this", i64 %index, i64 %num) #2 {
  %_result.addr = alloca %"ContiguousMemoryRange[Char/rtct]"*
  store %"ContiguousMemoryRange[Char/rtct]"* %_result, %"ContiguousMemoryRange[Char/rtct]"** %_result.addr
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %index.addr = alloca i64
  store i64 %index, i64* %index.addr
  %num.addr = alloca i64
  store i64 %num, i64* %num.addr
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  %"$tmpC1" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %"ContiguousMemoryRange[Char/rtct]"** %_result.addr
  %2 = load %"Vector[Char/rtct]"** %"$this.addr"
  %3 = getelementptr inbounds %"Vector[Char/rtct]"* %2, i32 0, i32 0
  %4 = load i64* %index.addr
  call void @advance(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %3, i64 %4)
  %5 = load %"RawPtr[Char/rtct]"* %"$tmpC"
  %6 = load %"Vector[Char/rtct]"** %"$this.addr"
  %7 = getelementptr inbounds %"Vector[Char/rtct]"* %6, i32 0, i32 0
  %8 = load i64* %index.addr
  %9 = load i64* %num.addr
  %10 = call i64 @"+29"(i64 %8, i64 %9)
  call void @advance(%"RawPtr[Char/rtct]"* %"$tmpC1", %"RawPtr[Char/rtct]"* %7, i64 %10)
  %11 = load %"RawPtr[Char/rtct]"* %"$tmpC1"
  call void @ctor181(%"ContiguousMemoryRange[Char/rtct]"* %1, %"RawPtr[Char/rtct]" %5, %"RawPtr[Char/rtct]" %11)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC1")
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC1")
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @insertBefore351(%"Vector[Char/rtct]"* %"$this", i8* %value, %"ContiguousMemoryRange[Char/rtct]" %pos) #3 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %value.addr = alloca i8*
  store i8* %value, i8** %value.addr
  %pos.addr = alloca %"ContiguousMemoryRange[Char/rtct]"
  store %"ContiguousMemoryRange[Char/rtct]" %pos, %"ContiguousMemoryRange[Char/rtct]"* %pos.addr
  %posCount = alloca i64
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  %p = alloca %"RawPtr[Char/rtct]"
  %q = alloca %"RawPtr[Char/rtct]"
  %"$tmpC1" = alloca %"RawPtr[Char/rtct]"
  %"$tmpC2" = alloca %"RawPtr[Char/rtct]"
  %"$tmpC3" = alloca %"RawPtr[Char/rtct]"
  %"$tmpC4" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  call void @frontPtr(%"RawPtr[Char/rtct]"* %"$tmpC", %"ContiguousMemoryRange[Char/rtct]"* %pos.addr)
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"Vector[Char/rtct]"* %1, i32 0, i32 0
  %3 = load %"RawPtr[Char/rtct]"* %2
  %4 = call i64 @diff(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]" %3)
  store i64 %4, i64* %posCount
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  %5 = load %"Vector[Char/rtct]"** %"$this.addr"
  %6 = load %"Vector[Char/rtct]"** %"$this.addr"
  %7 = call i64 @size142(%"Vector[Char/rtct]"* %6)
  %8 = call i64 @"+74"(i64 %7, i32 1)
  call void @reserve(%"Vector[Char/rtct]"* %5, i64 %8)
  %9 = load %"Vector[Char/rtct]"** %"$this.addr"
  %10 = getelementptr inbounds %"Vector[Char/rtct]"* %9, i32 0, i32 1
  call void @advance175(%"RawPtr[Char/rtct]"* %p, %"RawPtr[Char/rtct]"* %10, i64 -1)
  %11 = load %"Vector[Char/rtct]"** %"$this.addr"
  %12 = getelementptr inbounds %"Vector[Char/rtct]"* %11, i32 0, i32 0
  %13 = load i64* %posCount
  %14 = call i64 @-352(i64 %13, i32 1)
  call void @advance175(%"RawPtr[Char/rtct]"* %q, %"RawPtr[Char/rtct]"* %12, i64 %14)
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %15 = load %"RawPtr[Char/rtct]"* %q
  %16 = call i1 @"==148"(%"RawPtr[Char/rtct]"* %p, %"RawPtr[Char/rtct]" %15)
  %17 = xor i1 true, %16
  br i1 %17, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  %18 = call i8* @value(%"RawPtr[Char/rtct]"* %p)
  %19 = load i8* %18
  call void @advance149(%"RawPtr[Char/rtct]"* %"$tmpC1", %"RawPtr[Char/rtct]"* %p)
  %20 = call i8* @value(%"RawPtr[Char/rtct]"* %"$tmpC1")
  store i8 %19, i8* %20
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC1")
  %21 = call i8* @value(%"RawPtr[Char/rtct]"* %p)
  call void @dtor162(i8* %21)
  br label %while_step

while_step:                                       ; preds = %while_body
  call void @advance175(%"RawPtr[Char/rtct]"* %"$tmpC2", %"RawPtr[Char/rtct]"* %p, i64 -1)
  %22 = load %"RawPtr[Char/rtct]"* %"$tmpC2"
  call void @"=144"(%"RawPtr[Char/rtct]"* %p, %"RawPtr[Char/rtct]" %22)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC2")
  br label %while_block

while_end:                                        ; preds = %while_block
  %23 = load i8** %value.addr
  %24 = load i8* %23
  call void @advance149(%"RawPtr[Char/rtct]"* %"$tmpC3", %"RawPtr[Char/rtct]"* %p)
  %25 = call i8* @value(%"RawPtr[Char/rtct]"* %"$tmpC3")
  store i8 %24, i8* %25
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC3")
  %26 = load %"Vector[Char/rtct]"** %"$this.addr"
  %27 = getelementptr inbounds %"Vector[Char/rtct]"* %26, i32 0, i32 1
  %28 = load %"Vector[Char/rtct]"** %"$this.addr"
  %29 = getelementptr inbounds %"Vector[Char/rtct]"* %28, i32 0, i32 1
  call void @advance149(%"RawPtr[Char/rtct]"* %"$tmpC4", %"RawPtr[Char/rtct]"* %29)
  %30 = load %"RawPtr[Char/rtct]"* %"$tmpC4"
  call void @"=144"(%"RawPtr[Char/rtct]"* %27, %"RawPtr[Char/rtct]" %30)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC4")
  call void @dtor146(%"RawPtr[Char/rtct]"* %q)
  call void @dtor146(%"RawPtr[Char/rtct]"* %p)
  call void @dtor35(i64* %posCount)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private i64 @-352(i64 %x, i32 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i32
  store i32 %y, i32* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i32* %y.addr
  %3 = call i64 @cmn130(i64 %1, i32 %2)
  %4 = load i32* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn131(i32 %4, i64 %5)
  %7 = call i64 @_DiffType_opMinus(i64 %3, i64 %6)
  ret i64 %7
}

; Function Attrs: inlinehint nounwind
define private void @insertAfter(%"Vector[Char/rtct]"* %"$this", i8* %value, %"ContiguousMemoryRange[Char/rtct]" %pos) #3 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %value.addr = alloca i8*
  store i8* %value, i8** %value.addr
  %pos.addr = alloca %"ContiguousMemoryRange[Char/rtct]"
  store %"ContiguousMemoryRange[Char/rtct]" %pos, %"ContiguousMemoryRange[Char/rtct]"* %pos.addr
  %tmp.v = alloca %"ContiguousMemoryRange[Char/rtct]"
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = load i8** %value.addr
  call void @backPtr(%"RawPtr[Char/rtct]"* %"$tmpC", %"ContiguousMemoryRange[Char/rtct]"* %pos.addr)
  %3 = load %"RawPtr[Char/rtct]"* %"$tmpC"
  %4 = load %"Vector[Char/rtct]"** %"$this.addr"
  %5 = getelementptr inbounds %"Vector[Char/rtct]"* %4, i32 0, i32 1
  %6 = load %"RawPtr[Char/rtct]"* %5
  call void @ctor181(%"ContiguousMemoryRange[Char/rtct]"* %tmp.v, %"RawPtr[Char/rtct]" %3, %"RawPtr[Char/rtct]" %6)
  %7 = load %"ContiguousMemoryRange[Char/rtct]"* %tmp.v
  call void @insertBefore351(%"Vector[Char/rtct]"* %1, i8* %2, %"ContiguousMemoryRange[Char/rtct]" %7)
  call void @dtor182(%"ContiguousMemoryRange[Char/rtct]"* %tmp.v)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @popBack353(%"Vector[Char/rtct]"* %"$this") #3 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = getelementptr inbounds %"Vector[Char/rtct]"* %1, i32 0, i32 1
  %3 = load %"Vector[Char/rtct]"** %"$this.addr"
  %4 = getelementptr inbounds %"Vector[Char/rtct]"* %3, i32 0, i32 1
  call void @advance175(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %4, i64 -1)
  %5 = load %"RawPtr[Char/rtct]"* %"$tmpC"
  call void @"=144"(%"RawPtr[Char/rtct]"* %2, %"RawPtr[Char/rtct]" %5)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  %6 = load %"Vector[Char/rtct]"** %"$this.addr"
  %7 = getelementptr inbounds %"Vector[Char/rtct]"* %6, i32 0, i32 1
  %8 = call i8* @value(%"RawPtr[Char/rtct]"* %7)
  call void @dtor162(i8* %8)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @resize(%"Vector[Char/rtct]"* %"$this", i64 %n) #3 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %n.addr = alloca i64
  store i64 %n, i64* %n.addr
  %oldSize = alloca i64
  %newEnd = alloca %"RawPtr[Char/rtct]"
  %"$tmpC" = alloca %"RawPtr[Char/rtct]"
  %newEnd4 = alloca %"RawPtr[Char/rtct]"
  %"$tmpC9" = alloca %"RawPtr[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = call i64 @size142(%"Vector[Char/rtct]"* %1)
  store i64 %2, i64* %oldSize
  br label %if_block

if_block:                                         ; preds = %code
  %3 = load i64* %n.addr
  %4 = load i64* %oldSize
  %5 = call i1 @"==354"(i64 %3, i64 %4)
  br i1 %5, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  call void @dtor31(i64* %oldSize)
  ret void

if_end:                                           ; preds = %dumy_block, %if_block
  br label %if_block1

dumy_block:                                       ; No predecessors!
  br label %if_end

if_block1:                                        ; preds = %if_end
  %6 = load i64* %n.addr
  %7 = load i64* %oldSize
  %8 = call i1 @"<173"(i64 %6, i64 %7)
  br i1 %8, label %if_then2, label %if_else

if_then2:                                         ; preds = %if_block1
  %9 = load %"Vector[Char/rtct]"** %"$this.addr"
  %10 = getelementptr inbounds %"Vector[Char/rtct]"* %9, i32 0, i32 0
  %11 = load i64* %n.addr
  call void @advance(%"RawPtr[Char/rtct]"* %newEnd, %"RawPtr[Char/rtct]"* %10, i64 %11)
  br label %while_block

if_else:                                          ; preds = %if_block1
  %12 = load %"Vector[Char/rtct]"** %"$this.addr"
  %13 = load i64* %n.addr
  call void @reserve(%"Vector[Char/rtct]"* %12, i64 %13)
  %14 = load %"Vector[Char/rtct]"** %"$this.addr"
  %15 = getelementptr inbounds %"Vector[Char/rtct]"* %14, i32 0, i32 0
  %16 = load i64* %n.addr
  call void @advance(%"RawPtr[Char/rtct]"* %newEnd4, %"RawPtr[Char/rtct]"* %15, i64 %16)
  br label %while_block5

if_end3:                                          ; preds = %while_end8, %while_end
  call void @dtor31(i64* %oldSize)
  ret void

while_block:                                      ; preds = %while_step, %if_then2
  %17 = load %"Vector[Char/rtct]"** %"$this.addr"
  %18 = getelementptr inbounds %"Vector[Char/rtct]"* %17, i32 0, i32 1
  %19 = load %"RawPtr[Char/rtct]"* %newEnd
  %20 = call i1 @"==148"(%"RawPtr[Char/rtct]"* %18, %"RawPtr[Char/rtct]" %19)
  %21 = xor i1 true, %20
  br i1 %21, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  %22 = load %"Vector[Char/rtct]"** %"$this.addr"
  %23 = getelementptr inbounds %"Vector[Char/rtct]"* %22, i32 0, i32 1
  %24 = load %"Vector[Char/rtct]"** %"$this.addr"
  %25 = getelementptr inbounds %"Vector[Char/rtct]"* %24, i32 0, i32 1
  call void @advance175(%"RawPtr[Char/rtct]"* %"$tmpC", %"RawPtr[Char/rtct]"* %25, i64 -1)
  %26 = load %"RawPtr[Char/rtct]"* %"$tmpC"
  call void @"=144"(%"RawPtr[Char/rtct]"* %23, %"RawPtr[Char/rtct]" %26)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC")
  %27 = load %"Vector[Char/rtct]"** %"$this.addr"
  %28 = getelementptr inbounds %"Vector[Char/rtct]"* %27, i32 0, i32 1
  %29 = call i8* @value(%"RawPtr[Char/rtct]"* %28)
  call void @dtor162(i8* %29)
  br label %while_step

while_step:                                       ; preds = %while_body
  br label %while_block

while_end:                                        ; preds = %while_block
  call void @dtor146(%"RawPtr[Char/rtct]"* %newEnd)
  br label %if_end3

while_block5:                                     ; preds = %while_step7, %if_else
  %30 = load %"Vector[Char/rtct]"** %"$this.addr"
  %31 = getelementptr inbounds %"Vector[Char/rtct]"* %30, i32 0, i32 1
  %32 = load %"RawPtr[Char/rtct]"* %newEnd4
  %33 = call i1 @"==148"(%"RawPtr[Char/rtct]"* %31, %"RawPtr[Char/rtct]" %32)
  %34 = xor i1 true, %33
  br i1 %34, label %while_body6, label %while_end8

while_body6:                                      ; preds = %while_block5
  %35 = load %"Vector[Char/rtct]"** %"$this.addr"
  %36 = getelementptr inbounds %"Vector[Char/rtct]"* %35, i32 0, i32 1
  %37 = call i8* @value(%"RawPtr[Char/rtct]"* %36)
  store i8 0, i8* %37
  %38 = load %"Vector[Char/rtct]"** %"$this.addr"
  %39 = getelementptr inbounds %"Vector[Char/rtct]"* %38, i32 0, i32 1
  %40 = load %"Vector[Char/rtct]"** %"$this.addr"
  %41 = getelementptr inbounds %"Vector[Char/rtct]"* %40, i32 0, i32 1
  call void @advance149(%"RawPtr[Char/rtct]"* %"$tmpC9", %"RawPtr[Char/rtct]"* %41)
  %42 = load %"RawPtr[Char/rtct]"* %"$tmpC9"
  call void @"=144"(%"RawPtr[Char/rtct]"* %39, %"RawPtr[Char/rtct]" %42)
  call void @dtor146(%"RawPtr[Char/rtct]"* %"$tmpC9")
  br label %while_step7

while_step7:                                      ; preds = %while_body6
  br label %while_block5

while_end8:                                       ; preds = %while_block5
  call void @dtor146(%"RawPtr[Char/rtct]"* %newEnd4)
  br label %if_end3
}

; Function Attrs: alwaysinline nounwind
define private i1 @"==354"(i64 %x, i64 %y) #2 {
  %x.addr = alloca i64
  store i64 %x, i64* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i64* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn30(i64 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i64* %x.addr
  %6 = call i64 @cmn30(i64 %4, i64 %5)
  %7 = icmp eq i64 %3, %6
  ret i1 %7
}

; Function Attrs: inlinehint nounwind
define private void @clear355(%"Vector[Char/rtct]"* %"$this") #3 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %tmp.v = alloca %"ContiguousMemoryRange[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = load %"Vector[Char/rtct]"** %"$this.addr"
  %3 = getelementptr inbounds %"Vector[Char/rtct]"* %2, i32 0, i32 0
  %4 = load %"RawPtr[Char/rtct]"* %3
  %5 = load %"Vector[Char/rtct]"** %"$this.addr"
  %6 = getelementptr inbounds %"Vector[Char/rtct]"* %5, i32 0, i32 1
  %7 = load %"RawPtr[Char/rtct]"* %6
  call void @ctor181(%"ContiguousMemoryRange[Char/rtct]"* %tmp.v, %"RawPtr[Char/rtct]" %4, %"RawPtr[Char/rtct]" %7)
  %8 = load %"ContiguousMemoryRange[Char/rtct]"* %tmp.v
  call void @remove241(%"Vector[Char/rtct]"* %1, %"ContiguousMemoryRange[Char/rtct]" %8)
  call void @dtor182(%"ContiguousMemoryRange[Char/rtct]"* %tmp.v)
  ret void
}

; Function Attrs: inlinehint nounwind
define private %"Vector[Char/rtct]"* @"=356"(%"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"* %other) #3 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %other.addr = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %other, %"Vector[Char/rtct]"** %other.addr
  %tmp = alloca %"Vector[Char/rtct]"
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %other.addr
  call void @ctor141(%"Vector[Char/rtct]"* %tmp, %"Vector[Char/rtct]"* %1)
  %2 = load %"Vector[Char/rtct]"** %"$this.addr"
  call void @swap348(%"Vector[Char/rtct]"* %tmp, %"Vector[Char/rtct]"* %2)
  %3 = load %"Vector[Char/rtct]"** %"$this.addr"
  call void @dtor165(%"Vector[Char/rtct]"* %tmp)
  ret %"Vector[Char/rtct]"* %3

dumy_block:                                       ; No predecessors!
  call void @dtor165(%"Vector[Char/rtct]"* %tmp)
  unreachable
}

; Function Attrs: inlinehint nounwind
define private i1 @"==357"(%"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"* %other) #3 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %other.addr = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %other, %"Vector[Char/rtct]"** %other.addr
  %i = alloca i32
  %s = alloca i64
  %tmp.v = alloca i64
  %tmp.v4 = alloca i64
  br label %code

code:                                             ; preds = %0
  br label %if_block

if_block:                                         ; preds = %code
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = call i64 @size142(%"Vector[Char/rtct]"* %1)
  %3 = load %"Vector[Char/rtct]"** %other.addr
  %4 = call i64 @size142(%"Vector[Char/rtct]"* %3)
  %5 = call i1 @"!=267"(i64 %2, i64 %4)
  br i1 %5, label %if_then, label %if_end

if_then:                                          ; preds = %if_block
  ret i1 false

if_end:                                           ; preds = %dumy_block, %if_block
  store i32 0, i32* %i
  %6 = load %"Vector[Char/rtct]"** %"$this.addr"
  %7 = call i64 @size142(%"Vector[Char/rtct]"* %6)
  store i64 %7, i64* %s
  br label %while_block

dumy_block:                                       ; No predecessors!
  br label %if_end

while_block:                                      ; preds = %while_step, %if_end
  %8 = load i32* %i
  %9 = load i64* %s
  %10 = call i1 @"<358"(i32 %8, i64 %9)
  br i1 %10, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  br label %if_block1

while_step:                                       ; preds = %if_end3
  %11 = load i32* %i
  %12 = call i32 @"+14"(i32 %11, i32 1)
  store i32 %12, i32* %i
  br label %while_block

while_end:                                        ; preds = %while_block
  call void @dtor31(i64* %s)
  call void @dtor58(i32* %i)
  ret i1 true

if_block1:                                        ; preds = %while_body
  %13 = load %"Vector[Char/rtct]"** %"$this.addr"
  %14 = load i32* %i
  %15 = zext i32 %14 to i64
  store i64 %15, i64* %tmp.v
  %16 = load i64* %tmp.v
  %17 = call i8* @at349(%"Vector[Char/rtct]"* %13, i64 %16)
  %18 = load i8* %17
  %19 = load %"Vector[Char/rtct]"** %other.addr
  %20 = load i32* %i
  %21 = zext i32 %20 to i64
  store i64 %21, i64* %tmp.v4
  %22 = load i64* %tmp.v4
  %23 = call i8* @at349(%"Vector[Char/rtct]"* %19, i64 %22)
  %24 = load i8* %23
  %25 = icmp eq i8 %18, %24
  %26 = xor i1 true, %25
  br i1 %26, label %if_then2, label %if_end3

if_then2:                                         ; preds = %if_block1
  call void @dtor31(i64* %tmp.v4)
  call void @dtor31(i64* %tmp.v)
  call void @dtor31(i64* %s)
  call void @dtor58(i32* %i)
  ret i1 false

if_end3:                                          ; preds = %dumy_block5, %if_block1
  call void @dtor31(i64* %tmp.v4)
  call void @dtor31(i64* %tmp.v)
  br label %while_step

dumy_block5:                                      ; No predecessors!
  br label %if_end3

dumy_block6:                                      ; No predecessors!
  call void @dtor31(i64* %s)
  call void @dtor58(i32* %i)
  unreachable
}

; Function Attrs: alwaysinline nounwind
define private i1 @"<358"(i32 %x, i64 %y) #2 {
  %x.addr = alloca i32
  store i32 %x, i32* %x.addr
  %y.addr = alloca i64
  store i64 %y, i64* %y.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i32* %x.addr
  %2 = load i64* %y.addr
  %3 = call i64 @cmn76(i32 %1, i64 %2)
  %4 = load i64* %y.addr
  %5 = load i32* %x.addr
  %6 = call i64 @cmn75(i64 %4, i32 %5)
  %7 = icmp slt i64 %3, %6
  ret i1 %7
}

; Function Attrs: alwaysinline nounwind
define private i1 @"!=359"(%"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"* %other) #2 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %other.addr = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %other, %"Vector[Char/rtct]"** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %"Vector[Char/rtct]"** %"$this.addr"
  %2 = load %"Vector[Char/rtct]"** %other.addr
  %3 = call i1 @"==357"(%"Vector[Char/rtct]"* %1, %"Vector[Char/rtct]"* %2)
  %4 = xor i1 true, %3
  ret i1 %4
}

; Function Attrs: inlinehint nounwind
define private void @dumpThis(%"Vector[Char/rtct]"* %"$this", %StringRef %prefix) #3 {
  %"$this.addr" = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %"$this", %"Vector[Char/rtct]"** %"$this.addr"
  %prefix.addr = alloca %StringRef
  store %StringRef %prefix, %StringRef* %prefix.addr
  %"$tmpForRef" = alloca %StringRef
  %const.bytes = alloca [9 x i8]
  %const.struct = alloca %StringRef
  %"$tmpForRef1" = alloca %StringRef
  %const.bytes2 = alloca [23 x i8]
  %const.struct3 = alloca %StringRef
  %"$tmpForRef4" = alloca %StringRef
  %const.bytes5 = alloca [3 x i8]
  %const.struct6 = alloca %StringRef
  %"$tmpC" = alloca %StreamRefWrapperHelperClass
  %"$tmpForRef7" = alloca %StringRef
  %const.bytes8 = alloca [11 x i8]
  %const.struct9 = alloca %StringRef
  %tmp.v = alloca i32
  %"$tmpForRef10" = alloca %StringRef
  %const.bytes11 = alloca [12 x i8]
  %const.struct12 = alloca %StringRef
  %"$tmpC13" = alloca %StreamRefWrapperHelperClass
  %"$tmpForRef14" = alloca %StringRef
  %const.bytes15 = alloca [10 x i8]
  %const.struct16 = alloca %StringRef
  %"$tmpC17" = alloca %StreamRefWrapperHelperClass
  %"$tmpForRef18" = alloca %StringRef
  %const.bytes19 = alloca [17 x i8]
  %const.struct20 = alloca %StringRef
  %"$tmpC21" = alloca %StreamRefWrapperHelperClass
  br label %code

code:                                             ; preds = %0
  %1 = call %ConsoleOutputStream* @"<<365"(%ConsoleOutputStream* @cout, %StringRef* %prefix.addr)
  store [9 x i8] c"dumping \00", [9 x i8]* %const.bytes
  %2 = getelementptr inbounds [9 x i8]* %const.bytes, i32 0, i32 0
  %3 = getelementptr inbounds [9 x i8]* %const.bytes, i32 0, i32 8
  %4 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 0
  %5 = getelementptr inbounds %StringRef* %const.struct, i32 0, i32 1
  store i8* %2, i8** %4
  store i8* %3, i8** %5
  %6 = load %StringRef* %const.struct
  store %StringRef %6, %StringRef* %"$tmpForRef"
  %7 = call %ConsoleOutputStream* @"<<365"(%ConsoleOutputStream* %1, %StringRef* %"$tmpForRef")
  store [23 x i8] c"Vector[Char/rtct]/rtct\00", [23 x i8]* %const.bytes2
  %8 = getelementptr inbounds [23 x i8]* %const.bytes2, i32 0, i32 0
  %9 = getelementptr inbounds [23 x i8]* %const.bytes2, i32 0, i32 22
  %10 = getelementptr inbounds %StringRef* %const.struct3, i32 0, i32 0
  %11 = getelementptr inbounds %StringRef* %const.struct3, i32 0, i32 1
  store i8* %8, i8** %10
  store i8* %9, i8** %11
  %12 = load %StringRef* %const.struct3
  store %StringRef %12, %StringRef* %"$tmpForRef1"
  %13 = call %ConsoleOutputStream* @"<<365"(%ConsoleOutputStream* %7, %StringRef* %"$tmpForRef1")
  store [3 x i8] c": \00", [3 x i8]* %const.bytes5
  %14 = getelementptr inbounds [3 x i8]* %const.bytes5, i32 0, i32 0
  %15 = getelementptr inbounds [3 x i8]* %const.bytes5, i32 0, i32 2
  %16 = getelementptr inbounds %StringRef* %const.struct6, i32 0, i32 0
  %17 = getelementptr inbounds %StringRef* %const.struct6, i32 0, i32 1
  store i8* %14, i8** %16
  store i8* %15, i8** %17
  %18 = load %StringRef* %const.struct6
  store %StringRef %18, %StringRef* %"$tmpForRef4"
  %19 = call %ConsoleOutputStream* @"<<365"(%ConsoleOutputStream* %13, %StringRef* %"$tmpForRef4")
  %20 = load %"Vector[Char/rtct]"** %"$this.addr"
  call void @mkStreamRefWrapper(%StreamRefWrapperHelperClass* %"$tmpC", %"Vector[Char/rtct]"* %20)
  %21 = call %ConsoleOutputStream* @"<<363"(%ConsoleOutputStream* %19, %StreamRefWrapperHelperClass* %"$tmpC")
  %22 = call %ConsoleOutputStream* @"<<360"(%ConsoleOutputStream* %21, %EndLineHelperClass* @endl)
  call void @dtor369(%StreamRefWrapperHelperClass* %"$tmpC")
  store [11 x i8] c"    size: \00", [11 x i8]* %const.bytes8
  %23 = getelementptr inbounds [11 x i8]* %const.bytes8, i32 0, i32 0
  %24 = getelementptr inbounds [11 x i8]* %const.bytes8, i32 0, i32 10
  %25 = getelementptr inbounds %StringRef* %const.struct9, i32 0, i32 0
  %26 = getelementptr inbounds %StringRef* %const.struct9, i32 0, i32 1
  store i8* %23, i8** %25
  store i8* %24, i8** %26
  %27 = load %StringRef* %const.struct9
  store %StringRef %27, %StringRef* %"$tmpForRef7"
  %28 = call %ConsoleOutputStream* @"<<365"(%ConsoleOutputStream* @cout, %StringRef* %"$tmpForRef7")
  %29 = load %"Vector[Char/rtct]"** %"$this.addr"
  %30 = call i64 @size142(%"Vector[Char/rtct]"* %29)
  %31 = trunc i64 %30 to i32
  store i32 %31, i32* %tmp.v
  %32 = call %ConsoleOutputStream* @"<<370"(%ConsoleOutputStream* %28, i32* %tmp.v)
  %33 = call %ConsoleOutputStream* @"<<360"(%ConsoleOutputStream* %32, %EndLineHelperClass* @endl)
  call void @dtor58(i32* %tmp.v)
  store [12 x i8] c"    begin: \00", [12 x i8]* %const.bytes11
  %34 = getelementptr inbounds [12 x i8]* %const.bytes11, i32 0, i32 0
  %35 = getelementptr inbounds [12 x i8]* %const.bytes11, i32 0, i32 11
  %36 = getelementptr inbounds %StringRef* %const.struct12, i32 0, i32 0
  %37 = getelementptr inbounds %StringRef* %const.struct12, i32 0, i32 1
  store i8* %34, i8** %36
  store i8* %35, i8** %37
  %38 = load %StringRef* %const.struct12
  store %StringRef %38, %StringRef* %"$tmpForRef10"
  %39 = call %ConsoleOutputStream* @"<<365"(%ConsoleOutputStream* @cout, %StringRef* %"$tmpForRef10")
  %40 = load %"Vector[Char/rtct]"** %"$this.addr"
  %41 = getelementptr inbounds %"Vector[Char/rtct]"* %40, i32 0, i32 0
  %42 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %41)
  call void @mkStreamRefWrapper372(%StreamRefWrapperHelperClass* %"$tmpC13", i8* %42)
  %43 = call %ConsoleOutputStream* @"<<363"(%ConsoleOutputStream* %39, %StreamRefWrapperHelperClass* %"$tmpC13")
  %44 = call %ConsoleOutputStream* @"<<360"(%ConsoleOutputStream* %43, %EndLineHelperClass* @endl)
  call void @dtor369(%StreamRefWrapperHelperClass* %"$tmpC13")
  store [10 x i8] c"    end: \00", [10 x i8]* %const.bytes15
  %45 = getelementptr inbounds [10 x i8]* %const.bytes15, i32 0, i32 0
  %46 = getelementptr inbounds [10 x i8]* %const.bytes15, i32 0, i32 9
  %47 = getelementptr inbounds %StringRef* %const.struct16, i32 0, i32 0
  %48 = getelementptr inbounds %StringRef* %const.struct16, i32 0, i32 1
  store i8* %45, i8** %47
  store i8* %46, i8** %48
  %49 = load %StringRef* %const.struct16
  store %StringRef %49, %StringRef* %"$tmpForRef14"
  %50 = call %ConsoleOutputStream* @"<<365"(%ConsoleOutputStream* @cout, %StringRef* %"$tmpForRef14")
  %51 = load %"Vector[Char/rtct]"** %"$this.addr"
  %52 = getelementptr inbounds %"Vector[Char/rtct]"* %51, i32 0, i32 1
  %53 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %52)
  call void @mkStreamRefWrapper372(%StreamRefWrapperHelperClass* %"$tmpC17", i8* %53)
  %54 = call %ConsoleOutputStream* @"<<363"(%ConsoleOutputStream* %50, %StreamRefWrapperHelperClass* %"$tmpC17")
  %55 = call %ConsoleOutputStream* @"<<360"(%ConsoleOutputStream* %54, %EndLineHelperClass* @endl)
  call void @dtor369(%StreamRefWrapperHelperClass* %"$tmpC17")
  store [17 x i8] c"    endOfStore: \00", [17 x i8]* %const.bytes19
  %56 = getelementptr inbounds [17 x i8]* %const.bytes19, i32 0, i32 0
  %57 = getelementptr inbounds [17 x i8]* %const.bytes19, i32 0, i32 16
  %58 = getelementptr inbounds %StringRef* %const.struct20, i32 0, i32 0
  %59 = getelementptr inbounds %StringRef* %const.struct20, i32 0, i32 1
  store i8* %56, i8** %58
  store i8* %57, i8** %59
  %60 = load %StringRef* %const.struct20
  store %StringRef %60, %StringRef* %"$tmpForRef18"
  %61 = call %ConsoleOutputStream* @"<<365"(%ConsoleOutputStream* @cout, %StringRef* %"$tmpForRef18")
  %62 = load %"Vector[Char/rtct]"** %"$this.addr"
  %63 = getelementptr inbounds %"Vector[Char/rtct]"* %62, i32 0, i32 2
  %64 = call i8* @bytePtr(%"RawPtr[Char/rtct]"* %63)
  call void @mkStreamRefWrapper372(%StreamRefWrapperHelperClass* %"$tmpC21", i8* %64)
  %65 = call %ConsoleOutputStream* @"<<363"(%ConsoleOutputStream* %61, %StreamRefWrapperHelperClass* %"$tmpC21")
  %66 = call %ConsoleOutputStream* @"<<360"(%ConsoleOutputStream* %65, %EndLineHelperClass* @endl)
  call void @dtor369(%StreamRefWrapperHelperClass* %"$tmpC21")
  ret void
}

; Function Attrs: inlinehint nounwind
define private %ConsoleOutputStream* @"<<360"(%ConsoleOutputStream* %s, %EndLineHelperClass* %x) #3 {
  %s.addr = alloca %ConsoleOutputStream*
  store %ConsoleOutputStream* %s, %ConsoleOutputStream** %s.addr
  %x.addr = alloca %EndLineHelperClass*
  store %EndLineHelperClass* %x, %EndLineHelperClass** %x.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %EndLineHelperClass** %x.addr
  %2 = load %ConsoleOutputStream** %s.addr
  call void @">>"(%EndLineHelperClass* %1, %ConsoleOutputStream* %2)
  %3 = load %ConsoleOutputStream** %s.addr
  ret %ConsoleOutputStream* %3
}

; Function Attrs: inlinehint nounwind
define private void @">>"(%EndLineHelperClass* %"$this", %ConsoleOutputStream* %os) #3 {
  %"$this.addr" = alloca %EndLineHelperClass*
  store %EndLineHelperClass* %"$this", %EndLineHelperClass** %"$this.addr"
  %os.addr = alloca %ConsoleOutputStream*
  store %ConsoleOutputStream* %os, %ConsoleOutputStream** %os.addr
  %"$tmpForRef" = alloca i8
  br label %code

code:                                             ; preds = %0
  %1 = load %ConsoleOutputStream** %os.addr
  store i8 10, i8* %"$tmpForRef"
  %2 = call %ConsoleOutputStream* @"<<361"(%ConsoleOutputStream* %1, i8* %"$tmpForRef")
  ret void
}

; Function Attrs: inlinehint nounwind
define private %ConsoleOutputStream* @"<<361"(%ConsoleOutputStream* %s, i8* %x) #3 {
  %s.addr = alloca %ConsoleOutputStream*
  store %ConsoleOutputStream* %s, %ConsoleOutputStream** %s.addr
  %x.addr = alloca i8*
  store i8* %x, i8** %x.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %ConsoleOutputStream** %s.addr
  %2 = load i8** %x.addr
  %3 = load i8* %2
  call void @"<<<362"(%ConsoleOutputStream* %1, i8 %3)
  %4 = load %ConsoleOutputStream** %s.addr
  ret %ConsoleOutputStream* %4
}

; Function Attrs: alwaysinline nounwind
define private void @"<<<362"(%ConsoleOutputStream* %"$this", i8 %x) #2 {
  %"$this.addr" = alloca %ConsoleOutputStream*
  store %ConsoleOutputStream* %"$this", %ConsoleOutputStream** %"$this.addr"
  %x.addr = alloca i8
  store i8 %x, i8* %x.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i8* %x.addr
  call void @writeChar(i8 %1)
  ret void
}

; Function Attrs: inlinehint nounwind
define private %ConsoleOutputStream* @"<<363"(%ConsoleOutputStream* %s, %StreamRefWrapperHelperClass* %x) #3 {
  %s.addr = alloca %ConsoleOutputStream*
  store %ConsoleOutputStream* %s, %ConsoleOutputStream** %s.addr
  %x.addr = alloca %StreamRefWrapperHelperClass*
  store %StreamRefWrapperHelperClass* %x, %StreamRefWrapperHelperClass** %x.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %StreamRefWrapperHelperClass** %x.addr
  %2 = load %StreamRefWrapperHelperClass* %1
  %3 = load %ConsoleOutputStream** %s.addr
  call void @">>364"(%StreamRefWrapperHelperClass %2, %ConsoleOutputStream* %3)
  %4 = load %ConsoleOutputStream** %s.addr
  ret %ConsoleOutputStream* %4
}

; Function Attrs: inlinehint nounwind
define private void @">>364"(%StreamRefWrapperHelperClass %p, %ConsoleOutputStream* %os) #3 {
  %p.addr = alloca %StreamRefWrapperHelperClass
  store %StreamRefWrapperHelperClass %p, %StreamRefWrapperHelperClass* %p.addr
  %os.addr = alloca %ConsoleOutputStream*
  store %ConsoleOutputStream* %os, %ConsoleOutputStream** %os.addr
  br label %code

code:                                             ; preds = %0
  %1 = getelementptr inbounds %StreamRefWrapperHelperClass* %p.addr, i32 0, i32 0
  %2 = load i8** %1
  call void @writePointer(i8* %2)
  ret void
}

; Function Attrs: inlinehint nounwind
define private %ConsoleOutputStream* @"<<365"(%ConsoleOutputStream* %s, %StringRef* %x) #3 {
  %s.addr = alloca %ConsoleOutputStream*
  store %ConsoleOutputStream* %s, %ConsoleOutputStream** %s.addr
  %x.addr = alloca %StringRef*
  store %StringRef* %x, %StringRef** %x.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %ConsoleOutputStream** %s.addr
  %2 = load %StringRef** %x.addr
  %3 = load %StringRef* %2
  call void @"<<<366"(%ConsoleOutputStream* %1, %StringRef %3)
  %4 = load %ConsoleOutputStream** %s.addr
  ret %ConsoleOutputStream* %4
}

; Function Attrs: alwaysinline nounwind
define private void @"<<<366"(%ConsoleOutputStream* %"$this", %StringRef %x) #2 {
  %"$this.addr" = alloca %ConsoleOutputStream*
  store %ConsoleOutputStream* %"$this", %ConsoleOutputStream** %"$this.addr"
  %x.addr = alloca %StringRef
  store %StringRef %x, %StringRef* %x.addr
  %"$rangeVar" = alloca %StringRef
  %c = alloca i8*
  br label %code

code:                                             ; preds = %0
  %1 = load %StringRef* %x.addr
  call void @ctor77(%StringRef* %"$rangeVar", %StringRef %1)
  br label %while_block

while_block:                                      ; preds = %while_step, %code
  %2 = call i1 @isEmpty128(%StringRef* %"$rangeVar")
  %3 = xor i1 true, %2
  br i1 %3, label %while_body, label %while_end

while_body:                                       ; preds = %while_block
  %4 = call i8* @front194(%StringRef* %"$rangeVar")
  store i8* %4, i8** %c
  %5 = load i8** %c
  %6 = load i8* %5
  call void @writeChar(i8 %6)
  br label %while_step

while_step:                                       ; preds = %while_body
  call void @popFront195(%StringRef* %"$rangeVar")
  br label %while_block

while_end:                                        ; preds = %while_block
  call void @dtor78(%StringRef* %"$rangeVar")
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @mkStreamRefWrapper(%StreamRefWrapperHelperClass* sret %_result, %"Vector[Char/rtct]"* %x) #3 {
  %_result.addr = alloca %StreamRefWrapperHelperClass*
  store %StreamRefWrapperHelperClass* %_result, %StreamRefWrapperHelperClass** %_result.addr
  %x.addr = alloca %"Vector[Char/rtct]"*
  store %"Vector[Char/rtct]"* %x, %"Vector[Char/rtct]"** %x.addr
  %res = alloca %StreamRefWrapperHelperClass
  br label %code

code:                                             ; preds = %0
  call void @ctor367(%StreamRefWrapperHelperClass* %res)
  %1 = load %"Vector[Char/rtct]"** %x.addr
  %2 = bitcast %"Vector[Char/rtct]"* %1 to i8*
  %3 = getelementptr inbounds %StreamRefWrapperHelperClass* %res, i32 0, i32 0
  store i8* %2, i8** %3
  %4 = load %StreamRefWrapperHelperClass** %_result.addr
  call void @ctor368(%StreamRefWrapperHelperClass* %4, %StreamRefWrapperHelperClass* %res)
  call void @dtor369(%StreamRefWrapperHelperClass* %res)
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor369(%StreamRefWrapperHelperClass* %res)
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor367(%StreamRefWrapperHelperClass* %"$this") #2 {
  %"$this.addr" = alloca %StreamRefWrapperHelperClass*
  store %StreamRefWrapperHelperClass* %"$this", %StreamRefWrapperHelperClass** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  %1 = load %StreamRefWrapperHelperClass** %"$this.addr"
  %2 = getelementptr inbounds %StreamRefWrapperHelperClass* %1, i32 0, i32 0
  store i8* null, i8** %2
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @ctor368(%StreamRefWrapperHelperClass* %"$this", %StreamRefWrapperHelperClass* %other) #2 {
  %"$this.addr" = alloca %StreamRefWrapperHelperClass*
  store %StreamRefWrapperHelperClass* %"$this", %StreamRefWrapperHelperClass** %"$this.addr"
  %other.addr = alloca %StreamRefWrapperHelperClass*
  store %StreamRefWrapperHelperClass* %other, %StreamRefWrapperHelperClass** %other.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %StreamRefWrapperHelperClass** %other.addr
  %2 = getelementptr inbounds %StreamRefWrapperHelperClass* %1, i32 0, i32 0
  %3 = load i8** %2
  %4 = load %StreamRefWrapperHelperClass** %"$this.addr"
  %5 = getelementptr inbounds %StreamRefWrapperHelperClass* %4, i32 0, i32 0
  store i8* %3, i8** %5
  ret void
}

; Function Attrs: alwaysinline nounwind
define private void @dtor369(%StreamRefWrapperHelperClass* %"$this") #2 {
  %"$this.addr" = alloca %StreamRefWrapperHelperClass*
  store %StreamRefWrapperHelperClass* %"$this", %StreamRefWrapperHelperClass** %"$this.addr"
  br label %code

code:                                             ; preds = %0
  ret void
}

; Function Attrs: inlinehint nounwind
define private %ConsoleOutputStream* @"<<370"(%ConsoleOutputStream* %s, i32* %x) #3 {
  %s.addr = alloca %ConsoleOutputStream*
  store %ConsoleOutputStream* %s, %ConsoleOutputStream** %s.addr
  %x.addr = alloca i32*
  store i32* %x, i32** %x.addr
  br label %code

code:                                             ; preds = %0
  %1 = load %ConsoleOutputStream** %s.addr
  %2 = load i32** %x.addr
  %3 = load i32* %2
  call void @"<<<371"(%ConsoleOutputStream* %1, i32 %3)
  %4 = load %ConsoleOutputStream** %s.addr
  ret %ConsoleOutputStream* %4
}

; Function Attrs: alwaysinline nounwind
define private void @"<<<371"(%ConsoleOutputStream* %"$this", i32 %x) #2 {
  %"$this.addr" = alloca %ConsoleOutputStream*
  store %ConsoleOutputStream* %"$this", %ConsoleOutputStream** %"$this.addr"
  %x.addr = alloca i32
  store i32 %x, i32* %x.addr
  br label %code

code:                                             ; preds = %0
  %1 = load i32* %x.addr
  call void @writeInt(i32 %1)
  ret void
}

; Function Attrs: inlinehint nounwind
define private void @mkStreamRefWrapper372(%StreamRefWrapperHelperClass* sret %_result, i8* %x) #3 {
  %_result.addr = alloca %StreamRefWrapperHelperClass*
  store %StreamRefWrapperHelperClass* %_result, %StreamRefWrapperHelperClass** %_result.addr
  %x.addr = alloca i8*
  store i8* %x, i8** %x.addr
  %res = alloca %StreamRefWrapperHelperClass
  br label %code

code:                                             ; preds = %0
  call void @ctor367(%StreamRefWrapperHelperClass* %res)
  %1 = load i8** %x.addr
  %2 = getelementptr inbounds %StreamRefWrapperHelperClass* %res, i32 0, i32 0
  store i8* %1, i8** %2
  %3 = load %StreamRefWrapperHelperClass** %_result.addr
  call void @ctor368(%StreamRefWrapperHelperClass* %3, %StreamRefWrapperHelperClass* %res)
  call void @dtor369(%StreamRefWrapperHelperClass* %res)
  ret void

dumy_block:                                       ; No predecessors!
  call void @dtor369(%StreamRefWrapperHelperClass* %res)
  ret void
}

declare void @exit(i32)

; Function Attrs: inlinehint nounwind
define private i32 @systemCall(%StringRef %cmd) #3 {
  %cmd.addr = alloca %StringRef
  store %StringRef %cmd, %StringRef* %cmd.addr
  br label %code

code:                                             ; preds = %0
  %1 = call i8* @cStr(%StringRef* %cmd.addr)
  %2 = call i32 bitcast (void (i8*)* @system to i32 (i8*)*)(i8* %1)
  ret i32 %2
}

declare i8* @fgets(i8*, i32, i8*)

declare i32 @fread(i8*, i32, i32, i8*)

attributes #0 = { alwaysinline }
attributes #1 = { nounwind }
attributes #2 = { alwaysinline nounwind }
attributes #3 = { inlinehint nounwind }
attributes #4 = { noinline nounwind }
attributes #5 = { nounwind readnone }
