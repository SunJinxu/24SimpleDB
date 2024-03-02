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
* `meta-commands`  Non-SQL中的概念，通常以`.`开头; 表示需要系统对这些命令进行处理。
* `普通command` 普通command，是需要SQL Compile解析，并由VM执行的命令
  * SQL Compile: 将command解析为Statement结构(模拟bytecode)
  * Virtural Machine: 执行Statement(bytecode)

`Statement` 为可以被VM执行的结构，实际DB中为bytecode。其中含有`type`类型表示其类型

## 3. An In-Memory, Append-Only, Single-Table Database
本节目标是构建一个简单DB，具有以下功能：
* 插入一行，或者打印全部行
* 仅在内存中保存数据
* 一个单体，硬编码表 

*table形式如下*：
|column|type|
|:---:|:---:|
|id|integer|
|username|varchar(32)|
|email|varchar(255)|


*table的实现如下*:
* 将Row存放在成块的称为Page的内存中
* 每个Page都将尽可能存放满Row
* 每个页面都将行数据序列化为一种紧凑的表示形式
* Pages只有在需要时才会被分配
* 维护一个指向Pages的固定大小array

*sql语句形式如下*：
* `insert` -- 插入一条记录

  `insert 1 cstack foo@bar.com`
* `select` -- 列举全部记录
  
  `select`

## 4 Our first tests(and bugs)
### 4.1 使用GoogleTest搭建自己的测试框架
* 将googletest作为子模块导入至本地项目的include的文件夹中

  `git submodule add https://github.com/google/googletest.git include/googletest`
* 修改根目录的`CMakeLists.txt`文件

    ```
    add_subdirectory(test)
    add_subdirectory(include/googletest)
    ```
* 新建test文件夹
  * 创建db_test.cpp作为测试文件，头部`#include <gtest/gtest.h>`
  * 添加`CMakeLists.txt`文件，加入如下内容
    ```
    cmake_minimum_required(VERSION 3.10)

    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED True)

    set(CMAKE_BUILD_TYPE Debug)

    add_executable(dbTest db_test.cpp)
    target_link_libraries(dbTest gtest_main)
    target_include_directories(dbTest PRIVATE ${GTEST_INCLUDE_DIRS})
    ```
* 重新编译整个项目
* 增加新的launch.json
### 4.2 编写自己的测试用例（TODO）