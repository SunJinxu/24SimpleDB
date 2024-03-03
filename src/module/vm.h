#ifndef DB_VISUAL_MACHINE_H
#define DB_VISUAL_MACHINE_H

#include "common.h"
#include "db.h"

/**
 * Statement类型枚举
*/
typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType;

/**
 * Statement结构体
*/
typedef struct {
    StatementType statementType;
    Row rowToInsert;
} Statement;

/**
 * 处理meta-commands
*/
MetaCommandResult ExecuteMetaCommand(InputBuffer *InputBuffer, Table *table);

/**
 * Statement执行方法
*/
ExecuteResult ExecuteStatement(Statement *statement, Table *table);

#endif