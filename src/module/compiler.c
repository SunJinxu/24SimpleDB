#include <string.h>
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
PrepareResult PrepareInsert(InputBuffer *inputBuffer, Statement *statement) {
    statement->statementType = STATEMENT_INSERT;

    char* keyword = strtok(inputBuffer->buffer, " ");
    char* id = strtok(NULL, " ");
    char* username = strtok(NULL, " ");
    char* email = strtok(NULL, " ");

    if (id == NULL || username == NULL || email == NULL) {
        return PREPARE_SYNTAX_ERROR;
    }

    int rowId = atoi(id);
    if (rowId < 0) {
        return PREPARE_NEGATIVE_ID;
    }
    if (strlen(username) > COLUMN_USERNAME_SIZE) {
        return PREPARE_STRING_TOO_LONG;
    }
    if (strlen(email) > COLUMN_EMAIL_SIZE) {
        return PREPARE_STRING_TOO_LONG;
    }

    statement->rowToInsert.id = rowId;
    strcpy(statement->rowToInsert.username, username);
    strcpy(statement->rowToInsert.email, email);

    return PREPARE_SUCCESS;
}

PrepareResult PrepareStatement(InputBuffer *inputBuffer, Statement *statement) {
    if (strncmp(inputBuffer->buffer, "insert", 6) == 0) {
        return PrepareInsert(inputBuffer, statement);
    } else if (strcmp(inputBuffer->buffer, "select") == 0) {
        statement->statementType = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}