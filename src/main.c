#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include "module/common.h"
#include "module/util.h"
#include "module/compiler.h"
#include "module/vm.h"
#include "module/table.h"
#include "module/pager.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Must input a db filename, exit!\n");
        exit(EXIT_FAILURE);
    }

    printf("welcome to use 24SimpleDb!\n");

    const char *dbFileName = argv[1];
    Table *table = DbOpen(dbFileName);

    InputBuffer *userInput = CreateInputBuffer();
    while (true) {
        PrintDbPrompt();
        ReadCliInput(userInput);

        // 处理meta_command
        if (userInput->buffer[0] == '.') {
            MetaCommandResult metaRet = ExecuteMetaCommand(userInput, table);
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
                    case EXECUTE_DUPLICATE_KEY:
                        printf("error! duplicate key.\n");
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

    DbClose(table);
    DestroyInputBuffer(userInput);
    return 0;
}