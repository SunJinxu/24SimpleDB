#include "stdlib.h"
#include <stdlib.h>
#include <string.h>
#include "db.h"

/**
 * Row的打印方法
*/
void PrintRow(Row *row) {
    printf("ID: %d, USERNAME: %s, EMAIL: %s\n", row->id, row->username, row->email);
}

/**
 * 根据指定的rowNum返回对应的rowSlot(槽)指针
*/
void *RowSlot(Table *table, uint32_t rowNum) {
    uint32_t pageNum = rowNum / ROWS_PER_PAGE;  // 可以判断出任何一个row都会精准匹配到对应的page中
    if (table->pages[pageNum] == NULL) {
        table->pages[pageNum] = malloc(PAGE_SIZE);
    }
    uint32_t rowOffset = rowNum % ROWS_PER_PAGE;
    uint32_t byteOffset = rowOffset * ROW_SIZE;
    return (char *)(table->pages[pageNum]) + byteOffset;
}

/**
 * Row与CompactedRow之间的互相转换方法
*/
void SerializeRow(Row *row, void *compacted) {
    memcpy((char *)compacted, &(row->id), ID_SIZE);
    memcpy((char *)compacted + USERNAME_OFFSET, &(row->username), USERNAME_SIZE);
    memcpy((char *)compacted + EMAIL_OFFSET, &(row->email), EMAIL_SIZE);
}

void DeserializeRow(void *compacted, Row *row) {
    memcpy(&(row->id), (char *)compacted, ID_SIZE);
    memcpy(&(row->username), (char *)compacted + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(row->email), (char *)compacted + EMAIL_OFFSET, EMAIL_SIZE);
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