#include <string.h>
#include "stdlib.h"
#include "compiler.h"

/**
 * 创建并返回一个信息缓存指针
*/
InputBuffer *CreateInputBuffer() {
    InputBuffer *ptr = (InputBuffer *)malloc(sizeof(InputBuffer));
    ptr->buffer = NULL;
    ptr->bufferSize = 0;
    ptr->inputSize = 0;
    return ptr;
}

/**
 * 销毁信息缓存
*/
void DestroyInputBuffer(InputBuffer *inputBuffer) {
    free(inputBuffer->buffer);
    free(inputBuffer);
}

/**
 * 读取命令行输入至缓存中
*/
void ReadCliInput(InputBuffer *inputBuffer) {
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
 * Statement处理方法
*/
PrepareResult PrepareStatement(InputBuffer *inputBuffer, Statement *statement) {
    if (strncmp(inputBuffer->buffer, "insert", 6) == 0) {
        statement->statementType = STATEMENT_INSERT;
        int parsedArgc = sscanf(inputBuffer->buffer, "insert %d %s %s",
                                &(statement->rowToInsert.id),
                                statement->rowToInsert.username,
                                statement->rowToInsert.email);
        if (parsedArgc < 3) {
            return PREPARE_SYNTAX_ERROR;
        }
        return PREPARE_SUCCESS;
    } else if (strcmp(inputBuffer->buffer, "select") == 0) {
        statement->statementType = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}