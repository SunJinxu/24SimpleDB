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


void InternalNodeSplitAndInsert(Table *table, uint32_t parentPageNum, uint32_t childPageNum);

/**
 * 创建一个新节点，以右侧节点为参数，分配一个新page存放左侧节点，过程中rootpage始终是0号页面
*/
void CreateNewRoot(Table *table, uint32_t rightChildPageNum);

void UpdateInternalNodeKey(void *node, uint32_t oldKey, uint32_t newKey);

#endif