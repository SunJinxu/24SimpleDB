#include <unistd.h>
#include <string.h>
#include "pager.h"
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
    void *page = GetPage(table->pager, pageNum);
    uint32_t rowOffset = rowNum % ROWS_PER_PAGE;
    uint32_t byteOffset = rowOffset * ROW_SIZE;
    return page + byteOffset;
}

/**
 * Row与CompactedRow之间的互相转换方法
*/
void SerializeRow(Row *row, void *compacted) {
    memcpy((char *)compacted, &(row->id), ID_SIZE);
    strncpy(compacted + USERNAME_OFFSET, row->username, USERNAME_SIZE);
    strncpy(compacted + EMAIL_OFFSET, row->email, EMAIL_SIZE);
}

void DeserializeRow(void *compacted, Row *row) {
    memcpy(&(row->id), (char *)compacted, ID_SIZE);
    memcpy(&(row->username), (char *)compacted + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(row->email), (char *)compacted + EMAIL_OFFSET, EMAIL_SIZE);
}

/**
 * 文件名打开DB
*/
Table *DbOpen(const char *fileName) {
    Pager *pager = PagerOpen(fileName);
    uint32_t rowNum = pager->fileLength / ROW_SIZE;

    Table *table = (Table *)malloc(sizeof(Table));
    table->rowNum = rowNum;
    table->pager = pager;
    return table;
}

/**
 * 关闭DB，并刷新到文件
*/
void DbClose(Table *table) {
    Pager *pager = table->pager;
    uint32_t pageFullNum = table->rowNum / ROWS_PER_PAGE;
    for (uint32_t i = 0; i < pageFullNum; i++) {
        if (pager->pages[i] == NULL) {
            continue;
        }
        PagerFlush(pager, i, PAGE_SIZE);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }

    // 最后可能有一个未满的页
    uint32_t remainRowNum = table->rowNum % ROWS_PER_PAGE;
    if (remainRowNum > 0) {
        if (pager->pages[pageFullNum] != NULL) {
            PagerFlush(pager, pageFullNum, remainRowNum * ROW_SIZE);
            free(pager->pages[pageFullNum]);
            pager->pages[pageFullNum] = NULL;
        }
    }

    int result = close(pager->fileDescriptor);
    if (result == -1) {
        printf("close db failed, exit!\n");
        exit(EXIT_FAILURE);
    }

    free(pager);
    free(table);
}