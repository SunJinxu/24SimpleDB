#ifndef DB_COMMON_H
#define DB_COMMON_H

#include <stdio.h>
#include <stdlib.h>

#define DB_PROMPT "24SimpleDb > "
#define DB_EXIT_SIGN ".exit"

#define TABLE_MAX_PAGES 100
static const uint32_t PAGE_SIZE  = 4096;    // 与大部分操作系统的内存页大小相同

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
 * 准备sql语句的返回值枚举类
*/
typedef enum {
    PREPARE_SUCCESS,
    PREPARE_SYNTAX_ERROR,   // 解析错误
    PREPARE_STRING_TOO_LONG,    // 超出字段限制
    PREPARE_NEGATIVE_ID,    // 负数id
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

#endif