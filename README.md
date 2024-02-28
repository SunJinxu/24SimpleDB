# 24SimpleDB
a simple db inspired by sqlite

## 1. Making a Simple REPL(read-execute-print loop)
关键点在与实现三个主要组件：
* prompt打印
* 控制台输入消息接收
* 消息接收处理逻辑

其中，prompt打印可以打印我们自己的`24SimpleDb`标识；控制台消息接收使用自定义的结构体进行读取和存储；消息处理逻辑则用if_else结构进行匹配。

## 2. SQL Compiler and Virtual Machine
SQL中的command分为两种: 一种为`meta-commands`, 另一种为`普通command`
* `meta-commands`  Non-SQL中的概念，通常以`.`开头; 表示需要系统对这些命令进行处理
* `普通command` 普通command，是需要SQL Compile解析，并由VM执行的命令
  * SQL Compile: 将command解析为Statement结构(模拟bytecode)
  * Virtural Machine: 执行Statement(bytecode)

`Statement` 为可以被VM执行的结构，实际DB中为bytecode。其中含有`type`类型表示其类型