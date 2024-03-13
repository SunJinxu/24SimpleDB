#include <string.h>
#include "util.h"
#include "table.h"
#include "cursor.h"
#include "b_tree.h"
#include "b_tree_helper.h"
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
        PrintTree(table->pager, 0, 0);
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


