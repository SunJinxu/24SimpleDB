#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include "module/common.h"
#include "module/compiler.h"
#include "module/vm.h"
#include "module/db.h"

/**
 * 打印提示消息
*/
void PrintDbPrompt() {
    printf(DB_PROMPT);
}

int main() {
    InputBuffer *userInput = CreateInputBuffer();
    Table *table = CreateTable();
    printf("welcome to use 24SimpleDb!\n");

    while (true) {
        PrintDbPrompt();
        ReadCliInput(userInput);

        // 处理meta_command
        if (userInput->buffer[0] == '.') {
            MetaCommandResult metaRet = ExecuteMetaCommand(userInput);
            switch (metaRet) {
                case META_COMMAND_SUCCESS:
                    break;
                case META_COMMAND_UNRECOGNIZED_COMMAND:
                    printf("Unrecognized command '%s'\n", userInput->buffer);
                    break;
            }
            continue;
        }
        
        // 处理statement
        Statement statement;
        PrepareResult prepareRet = PrepareStatement(userInput, &statement);
        switch (prepareRet) {
            case PREPARE_SUCCESS:
                switch (ExecuteStatement(&statement, table)) {
                    case EXECUTE_SUCCESS:
                        printf("executed.\n");
                        break;
                    case EXECUTE_FAIL:
                        printf("error! execute failed.\n");
                        break;
                    case EXECUTE_TABLE_FULL:
                        printf("error! table full.\n");
                        break;
                }
                break;
            case PREPARE_STRING_TOO_LONG:
                printf("Input string too long error '%s'\n", userInput->buffer);
                break;
            case PREPARE_SYNTAX_ERROR:
                printf("Prepare Syntax error '%s'\n", userInput->buffer);
                break;
            case PREPARE_NEGATIVE_ID:
                printf("id must no be negative! \n");
                break;
            case PREPARE_UNRECOGNIZED_STATEMENT:
                printf("Prepare statement failed '%s'\n", userInput->buffer);
                break;
        }
    }

    DestroyTable(table);
    DestroyInputBuffer(userInput);
    return 0;
}