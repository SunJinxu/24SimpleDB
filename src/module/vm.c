#include <string.h>
#include "db.h"
#include "cursor.h"
#include "vm.h"

/**
 * Row的打印方法
*/
void PrintRow(Row *row) {
    printf("ID: %d, USERNAME: %s, EMAIL: %s\n", row->id, row->username, row->email);
}

/**
 * 处理meta-commands
*/
MetaCommandResult ExecuteMetaCommand(InputBuffer *InputBuffer, Table *table) {
    if (strcmp(InputBuffer->buffer, ".exit") == 0) {
        DbClose(table);
        exit(EXIT_SUCCESS);
    }
    return META_COMMAND_UNRECOGNIZED_COMMAND;
}

/**
 * insert执行函数（插入至table最末尾）
*/
ExecuteResult ExecuteInsert(Statement *statement, Table *table) {
    if (table->rowNum >= TABLE_MAX_ROWS) {
        return EXECUTE_TABLE_FULL;
    }
    Row *row = &(statement->rowToInsert);
    Cursor *cursor = TableEnd(table);
    void *slot = CursorValue(cursor);
    SerializeRow(row, slot);
    table->rowNum++;
    free(cursor);
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