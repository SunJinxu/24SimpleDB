#include "stdio.h"
#include "stdlib.h"
#include <string.h>

#define DB_PROMPT "24SimpleDb > "
#define true 1
#define false 0
#define DB_EXIT_SIGN ".exit"

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
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

/**
 * Statement类型枚举
*/
typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType;

/**
 * 信息缓存结构体
*/
typedef struct InputBuffer {
    char *buffer;   // 内存
    size_t bufferSize;  // 内存长度（用于getline函数出参）
    ssize_t inputSize;  // 有效长度
} InputBuffer;

/**
 * 打印提示消息
*/
void printPrompt() {
    printf(DB_PROMPT);
}

/**
 * 创建并返回一个信息缓存指针
*/
InputBuffer *createInputBuffer() {
    InputBuffer *ptr = (InputBuffer *)malloc(sizeof(InputBuffer));
    ptr->buffer = NULL;
    ptr->bufferSize = 0;
    ptr->inputSize = 0;
    return ptr;
}

/**
 * 销毁信息缓存
*/
void destroyInputBuffer(InputBuffer *inputBuffer) {
    free(inputBuffer->buffer);
    free(inputBuffer);
}

/**
 * 读取命令行输入至缓存中
*/
void readCliInput(InputBuffer *inputBuffer) {
    ssize_t readCliLen = getline(&(inputBuffer->buffer), &(inputBuffer->bufferSize), stdin);
    if (readCliLen <= 0) {
        printf("invalide cli read!");
        exit(EXIT_FAILURE);
    }
    // 忽视最后的换行符
    inputBuffer->inputSize = readCliLen - 1;
    inputBuffer->buffer[readCliLen - 1] = '\0';
}

/**
 * 处理meta-commands
*/
MetaCommandResult executeMetaCommand(InputBuffer *InputBuffer) {
    if (strcmp(InputBuffer->buffer, ".exit") == 0) {
        exit(EXIT_SUCCESS);
    }
    return META_COMMAND_UNRECOGNIZED_COMMAND;
}

/**
 * Statement结构体
*/
typedef struct {
    StatementType statementType;
} Statement;

/**
 * Statement处理方法
*/
PrepareResult prepareStatement(InputBuffer *inputBuffer, Statement *statement) {
    if (strncmp(inputBuffer->buffer, "insert", 6) == 0) {
        statement->statementType = STATEMENT_INSERT;
        return PREPARE_SUCCESS;
    } else if (strcmp(inputBuffer->buffer, "select") == 0) {
        statement->statementType = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

/**
 * Statement执行方法
*/
void executeStatement(Statement *statement) {
    switch (statement->statementType) {
        case (STATEMENT_INSERT):
            printf("This is a insert statement\n");
            break;
        case (STATEMENT_SELECT):
            printf("This is a select statement\n");
            break;
        default:
            break;
    }
}

int main() {
    InputBuffer *userInput = createInputBuffer();
    printf("welcome to use 24SimpleDb!\n");

    while (true) {
        printPrompt();
        readCliInput(userInput);

        // 处理meta_command
        if (userInput->buffer[0] == '.') {
            MetaCommandResult metaRet = executeMetaCommand(userInput);
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
        PrepareResult prepareRet = prepareStatement(userInput, &statement);
        switch (prepareRet) {
            case PREPARE_SUCCESS:
                executeStatement(&statement);
                break;
            case PREPARE_UNRECOGNIZED_STATEMENT:
                printf("Prepare statement failed '%s'\n", userInput->buffer);
                break;
        }
    }

    return 0;
}