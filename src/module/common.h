#ifndef DB_COMMON_H
#define DB_COMMON_H

#include <stdio.h>

#define DB_PROMPT "24SimpleDb > "
#define DB_EXIT_SIGN ".exit"

#define true 1
#define false 0

#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute) // 计算某个属性的占用内存的size

/**
 * 信息缓存结构体
*/
typedef struct InputBuffer {
    char *buffer;   // 内存
    size_t bufferSize;  // 内存长度（用于getline函数出参）
    ssize_t inputSize;  // 有效长度
} InputBuffer;

/**
 * VM执行结果
*/
typedef enum {
    EXECUTE_TABLE_FULL,
    EXECUTE_SUCCESS,
    EXECUTE_FAIL
} ExecuteResult;

/**
 * meta_commands返回值枚举类
*/
typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

/**
 * 准备sql语句的返回值枚举类`
*/
typedef enum {
    PREPARE_SUCCESS,
    PREPARE_SYNTAX_ERROR,   // 解析错误
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

#endif