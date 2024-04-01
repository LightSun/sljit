

## sljit
- SLJIT_IMM: immediate data(立即数)
- SLJIT_S0，SLJIT_S1... : save register. 一般函数参数存于此
- SLJIT_R0，SLJIT_R1... : temp/Scratch register 临时寄存器

## TODO1
- array. string
- if,else if ...else
- for/switch/while.
- self-method.

## TODO
- 实现常用指令
- 可变参数实现
- 函数内联 (抽象寄存器索引-DataStack+local_stack)
- 多线程(async/await)
- 泛型
- 绑定c/c++对象.
- 通过ffi导入so模块。
- 访问控制？ private/protected/public

