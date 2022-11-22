declare i32 @getint()
declare void @putint(i32)
declare void @putstr(i8*)
    @normalConst1 = constant i32 10
    @normalConst2 = constant i32 12
    @normalConst3 = constant i32 13
    @normalConst4 = constant i32 14
    @normalVar1 = global i32 21
    @normalVar2 = global i32 22
    @normalVar3 = global i32 23
    @cnt = global i32 zeroinitializer
    @str_1 = constant [10 x i8] c"19373341\0a\00"
define void @start() {
    %x1 = load i32, i32* @cnt
    %x2 = add i32 %x1, 1
    store i32 %x2, i32* @cnt
    %x3 = getelementptr inbounds [10 x i8], [10 x i8]* @str_1, i32 0, i32 0
    call void @putstr(i8* %x3)
    ret void
}
define i32 @main() {
    br label %br_1
    br_1:
    %x4 = icmp ne i32 1, 0
    br i1 %x4, label %br_2, label %br_3
    br_2:
    call void @start()
    %x5 = load i32, i32* @cnt
    %x6 = icmp eq i32 %x5, 10
    br i1 %x6, label %br_4, label %br_5
    br_4:
    br label %br_3
    br label %br_5
    br_5:
    br label %br_1
    br_3:
    ret i32 0
}
