#include "stdio.h"
#include "stdlib.h"
#include <string.h>

#define DB_PROMPT "24SimpleDb > "
#define true 1
#define false 0
#define DB_EXIT_SIGN ".exit"
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100

#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute) // 计算某个属性的占用内存的size

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
    PREPARE_SYNTAX_ERROR,   // 解析错误
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
 * 处理meta-commands
*/
MetaCommandResult ExecuteMetaCommand(InputBuffer *InputBuffer) {
    if (strcmp(InputBuffer->buffer, ".exit") == 0) {
        exit(EXIT_SUCCESS);
    }
    return META_COMMAND_UNRECOGNIZED_COMMAND;
}

/**
 * 表的行
*/
typedef struct {
    int id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;

// 定义Row的一些属性
const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

// 定义Table一些属性
const uint32_t PAGE_SIZE = 4096;    // 与大部分操作系统的内存页大小相同
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

/**
 * Table结构体
*/
typedef struct {
    uint32_t rowNum;
    void *pages[TABLE_MAX_PAGES];
} Table;

/**
 * 根据指定的rowNum返回对应的rowSlot(槽)指针
*/
void *RowSlot(Table *table, uint32_t rowNum) {
    uint32_t pageNum = rowNum / ROWS_PER_PAGE;  // 可以判断出任何一个row都会精准匹配到对应的page中
    if (table->pages[pageNum] == NULL) {
        table->pages[pageNum] = malloc(PAGE_SIZE);
    }
    uint32_t rowOffset = rowNum / ROWS_PER_PAGE;
    uint32_t byteOffset = rowOffset * ROW_SIZE;
    return (table->pages[pageNum]) + byteOffset;
}

/**
 * Statement结构体
*/
typedef struct {
    StatementType statementType;
    Row rowToInsert;
} Statement;

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

/**
 * Row与CompactedRow之间的互相转换方法
*/
void SerializeRow(Row *row, void *compacted) {
    memcpy(compacted, &(row->id), ID_SIZE);
    memcpy(compacted + USERNAME_OFFSET, &(row->username), USERNAME_SIZE);
    memcpy(compacted + EMAIL_OFFSET, &(row->email), EMAIL_SIZE);
}

void DeserializeRow(void *compacted, Row *row) {
    memcpy(&(row->id), compacted, ID_SIZE);
    memcpy(&(row->username), compacted + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(row->email), compacted + EMAIL_OFFSET, EMAIL_SIZE);
}

/**
 * Row的打印方法
*/
void PrintRow(Row *row) {
    printf("ID: %d, USERNAME: %s, EMAIL: %s", row->id, row->username, row->email);
}

/**
 * 几种Statement执行方法
*/
typedef enum {
    EXECUTE_TABLE_FULL,
    EXECUTE_SUCCESS,
    EXECUTE_FAIL
} ExecuteResult;

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

/**
 * Table创建与销毁方法
*/
Table *CreateTable() {
    Table *table = (Table *)malloc(sizeof(Table));
    table->rowNum = 0;
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        table->pages[i] = NULL;
    }
    return table;
}

void DestroyTable(Table *table) {
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        free(table->pages[i]);
    }
    free(table);
}

int main() {
    InputBuffer *userInput = CreateInputBuffer();
    Table *table = CreateTable();
    printf("welcome to use 24SimpleDb!\n");

    while (true) {
        printPrompt();
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
            case PREPARE_SYNTAX_ERROR:
                printf("Prepare Syntax error '%s'\n", userInput->buffer);
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