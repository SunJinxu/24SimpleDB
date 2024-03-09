#ifndef DB_VISUAL_MACHINE_H
#define DB_VISUAL_MACHINE_H

#include "common.h"
#include "cursor.h"
#include "table.h"

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

/**
 * 插入一个叶子节点
*/
void LeafNodeInsert(Cursor *cursor, uint32_t key, Row *value);

/**
 * 分裂和生成新node的算法
*/
void LeafNodeSplitAndInsert(Cursor *cursor, uint32_t key, Row *value);

#endif