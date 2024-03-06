#include <string.h>
#include "util.h"
#include "db.h"
#include "cursor.h"
#include "b_tree.h"
#include "vm.h"

/**
 * 处理meta-commands
*/
MetaCommandResult ExecuteMetaCommand(InputBuffer *InputBuffer, Table *table) {
    if (strcmp(InputBuffer->buffer, ".exit") == 0) {
        DbClose(table);
        exit(EXIT_SUCCESS);
    } else if (strcmp(InputBuffer->buffer, ".constants") == 0) {
        printf("Constants: \n");
        PrintConstants();
        return META_COMMAND_SUCCESS;
    } else if(strcmp(InputBuffer->buffer, ".btree") == 0) {
        printf("Tree:\n");
        PrintLeafNode(GetPage(table->pager, table->rootPageNum));
        return META_COMMAND_SUCCESS;
    }
    return META_COMMAND_UNRECOGNIZED_COMMAND;
}

/**
 * insert执行函数（插入至table最末尾）
*/
ExecuteResult ExecuteInsert(Statement *statement, Table *table) {
    void *node = GetPage(table->pager, table->rootPageNum);
    uint32_t cellNums = *LeafNodeCellNums(node);
    if (cellNums >= LEAF_NODE_MAX_CELLS) {
        return EXECUTE_TABLE_FULL;
    }

    Row* rowToInsert = &(statement->rowToInsert);
    uint32_t keyToInsert = rowToInsert->id;
    Cursor* cursor = TableFind(table, keyToInsert);

    if (cursor->cellNum < cellNums) {
        uint32_t ketAtIndex = *LeafNodeKey(node, cursor->cellNum);
        if (ketAtIndex == keyToInsert) {
            return EXECUTE_DUPLICATE_KEY;
        }
    }

    LeafNodeInsert(cursor, rowToInsert->id, rowToInsert);
    return EXECUTE_SUCCESS;
}

/**
 * select执行函数（打印table中所有row）
*/
ExecuteResult ExecuteSelect(Statement *statement, Table *table) {
    Cursor *cursor = TableStart(table);
    Row row;
    while (!(cursor->endOfTable)) {
        void *slot = CursorValue(cursor);
        DeserializeRow(slot, &row);
        PrintRow(&row);
        CursorAdvance(cursor);
    }
    free(cursor);
    return EXECUTE_SUCCESS;
}

/**
 * Statement执行方法
*/
ExecuteResult ExecuteStatement(Statement *statement, Table *table) {
    switch (statement->statementType) {
        case (STATEMENT_INSERT):
            return ExecuteInsert(statement, table);
        case (STATEMENT_SELECT):
            return ExecuteSelect(statement, table);
    }
    return EXECUTE_FAIL;
}

/**
 * 插入一个叶子节点
*/
void LeafNodeInsert(Cursor *cursor, uint32_t key, Row *value) {
    void *node = GetPage(cursor->table->pager, cursor->pageNum);

    uint32_t cellNums = *LeafNodeCellNums(node);
    if (cellNums >= LEAF_NODE_MAX_CELLS) {
        printf("current leaf node is full, exit!");
        exit(EXIT_FAILURE);
    }

    if (cursor->cellNum < cellNums) {   // 数据后移
        for (uint32_t i = cellNums; i > cursor->cellNum; i--) {
            memcpy(LeafNodeCell(node, i), LeafNodeCell(node, i - 1), LEAF_NODE_CELL_SIZE);
        }
    }

    *(LeafNodeCellNums(node)) += 1;
    *(LeafNodeKey(node, cursor->cellNum)) = key;
    SerializeRow(value, LeafNodeValue(node, cursor->cellNum));
}