# a-simple-compiler

北理2022级计科编译原理实验lab2 个人实验记录



## 项目简介

将一个简单的类似c语言的代码转化为可执行x86汇编代码。

代码量挺大的，即使项目完成过程极度依赖ai，但还是发上来了。项目纪念意义大于实用意义。



## 项目结构

Lexer: 词法分析器

Parser: 语法分析器

CodeGen: 汇编代码生成



## 使用方式

使用cmake启动

```bash
mkdir build
cd ./build
cmake ..
./Compilerlab02 yourfile.c
```
