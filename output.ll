declare i32 @getint()
declare void @putint(i32)
declare void @putstr(i8*)
  @str_1 = constant [2 x i8] c" \00"
  @str_2 = constant [2 x i8] c" \00"
  @str_3 = constant [2 x i8] c" \00"
  @str_4 = constant [2 x i8] c"\0a\00"
  @str_5 = constant [10 x i8] c"20373493\0a\00"
define void @hanoi(i32 %x1, i32 %x3, i32 %x5, i32 %x7) {
  %x8 = alloca i32
  %x6 = alloca i32
  %x4 = alloca i32
  %x2 = alloca i32
  store i32 %x1, i32* %x2
  store i32 %x3, i32* %x4
  store i32 %x5, i32* %x6
  store i32 %x7, i32* %x8
  %x9 = load i32, i32* %x2
  %x10 = load i32, i32* %x4
  %x11 = load i32, i32* %x6
  %x12 = load i32, i32* %x8
  call void @putint(i32 %x9)
  %x13 = getelementptr inbounds [2 x i8], [2 x i8]* @str_1, i32 0, i32 0
  call void @putstr(i8* %x13)
  call void @putint(i32 %x10)
  %x14 = getelementptr inbounds [2 x i8], [2 x i8]* @str_2, i32 0, i32 0
  call void @putstr(i8* %x14)
  call void @putint(i32 %x11)
  %x15 = getelementptr inbounds [2 x i8], [2 x i8]* @str_3, i32 0, i32 0
  call void @putstr(i8* %x15)
  call void @putint(i32 %x12)
  %x16 = getelementptr inbounds [2 x i8], [2 x i8]* @str_4, i32 0, i32 0
  call void @putstr(i8* %x16)
  %x17 = load i32, i32* %x8
  %x18 = icmp eq i32 %x17, 0
  br i1 %x18, label %br_1, label %br_2
br_1:
  ret void
  br label %br_2
br_2:
  %x19 = load i32, i32* %x2
  %x20 = load i32, i32* %x6
  %x21 = load i32, i32* %x4
  %x22 = load i32, i32* %x8
  %x23 = sub i32 %x22, 1
  call void @hanoi(i32 %x19, i32 %x20, i32 %x21, i32 %x23)
  %x24 = load i32, i32* %x4
  %x25 = load i32, i32* %x2
  %x26 = load i32, i32* %x6
  %x27 = load i32, i32* %x8
  %x28 = sub i32 %x27, 1
  call void @hanoi(i32 %x24, i32 %x25, i32 %x26, i32 %x28)
  ret void
}
define i32 @main() {
  %x29 = alloca i32
  %x30 = call i32 @getint()
  store i32 %x30, i32* %x29
  %x31 = getelementptr inbounds [10 x i8], [10 x i8]* @str_5, i32 0, i32 0
  call void @putstr(i8* %x31)
  %x32 = load i32, i32* %x29
  call void @hanoi(i32 1, i32 2, i32 3, i32 %x32)
  ret i32 0
}
