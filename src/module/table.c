#include <unistd.h>
#include <string.h>
#include "pager.h"
#include "b_tree.h"
#include "table.h"


// /**
//  * 根据指定的rowNum返回对应的rowSlot(槽)指针
// */
// void *RowSlot(Table *table, uint32_t rowNum) {
//     uint32_t pageNum = rowNum / ROWS_PER_PAGE;  // 可以判断出任何一个row都会精准匹配到对应的page中
//     void *page = GetPage(table->pager, pageNum);
//     uint32_t rowOffset = rowNum % ROWS_PER_PAGE;
//     uint32_t byteOffset = rowOffset * ROW_SIZE;
//     return page + byteOffset;
// }

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

    Table *table = (Table *)malloc(sizeof(Table));
    table->rootPageNum = 0;
    table->pager = pager;

    if (pager->pageNums == 0) {
        void *rootNode = GetPage(pager, 0);
        InitializeLeafNode(rootNode);
        SetNodeRoot(rootNode, true);
    }
    return table;
}

/**
 * 关闭DB，并刷新到文件
*/
void DbClose(Table *table) {
    Pager *pager = table->pager;
    for (uint32_t i = 0; i < pager->pageNums; i++) {
        if (pager->pages[i] == NULL) {
            continue;
        }
        PagerFlush(pager, i);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }

    int result = close(pager->fileDescriptor);
    if (result == -1) {
        printf("close db failed, exit!\n");
        exit(EXIT_FAILURE);
    }

    free(pager);
    free(table);
}