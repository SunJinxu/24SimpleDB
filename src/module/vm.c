#include <string.h>
#include "vm.h"

/**
 * 处理meta-commands
*/
MetaCommandResult ExecuteMetaCommand(InputBuffer *InputBuffer) {
    if (strcmp(InputBuffer->buffer, ".exit") == 0) {
        exit(EXIT_SUCCESS);
    }
    return META_COMMAND_UNRECOGNIZED_COMMAND;
}


ExecuteResult ExecuteInsert(Statement *statement, Table *table) {
    if (table->rowNum >= TABLE_MAX_ROWS) {
        return EXECUTE_TABLE_FULL;
    }
    Row *row = &(statement->rowToInsert);
    void *slot = RowSlot(table, table->rowNum);
    SerializeRow(row, slot);
    table->rowNum++;
    return EXECUTE_SUCCESS;
}

ExecuteResult ExecuteSelect(Statement *statement, Table *table) {
    for (uint32_t i = 0; i < table->rowNum; i++) {
        Row row;
        DeserializeRow(RowSlot(table, i), &row);
        PrintRow(&row);
    }
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