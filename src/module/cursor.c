#include "table.h"
#include "b_tree.h"
#include "cursor.h"

Cursor *TableStart(Table *table) {
    Cursor *cursor = (Cursor *)malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->pageNum = table->rootPageNum;
    cursor->cellNum = 0;
    void *rootNode = GetPage(table->pager, table->rootPageNum);
    uint32_t nodeNumCells = *(LeafNodeCellNums(rootNode));
    cursor->endOfTable = (nodeNumCells == 0);
    return cursor;
}

Cursor *TableEnd(Table *table) {
    Cursor *cursor = (Cursor *)malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->pageNum = table->rootPageNum;
    void *rootNode = GetPage(table->pager, table->rootPageNum);
    uint32_t nodeNumCells = *(LeafNodeCellNums(rootNode));
    cursor->cellNum = nodeNumCells;
    cursor->endOfTable = true;
    return cursor;
}

/**
 * 指定table的pageNum对应的node中寻找key对应的cursor
 * algorithm: Binary Search
*/
Cursor *LeafNodeFind(Table *table, uint32_t pageNum, uint32_t key) {
    void* node = GetPage(table->pager, pageNum);
    uint32_t numCells = *LeafNodeCellNums(node);
    Cursor* cursor = malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->pageNum = pageNum;
    // Binary search
    uint32_t min_index = 0;
    uint32_t one_past_max_index = numCells;
    while (one_past_max_index != min_index) {
        uint32_t index = (min_index + one_past_max_index) / 2;
        uint32_t key_at_index = *LeafNodeKey(node, index);
        if (key == key_at_index) {
        cursor->cellNum = index;
        return cursor;
        }
        if (key < key_at_index) {
            one_past_max_index = index;
        } else {
            min_index = index + 1;
        }
    }
    cursor->cellNum = min_index;
    return cursor;
}

Cursor *TableFind(Table *table, uint32_t key) {
    uint32_t rootPageNum = table->rootPageNum;
    void* rootNode = GetPage(table->pager, rootPageNum);

    if (GetNodeType(rootNode) == NODE_LEAF) {
        return LeafNodeFind(table, rootPageNum, key);
    } else {
        printf("Need to implement searching an internal node\n");
        exit(EXIT_FAILURE);
    }
}

void CursorAdvance(Cursor *cursor) {
    uint32_t pageNum = cursor->pageNum;
    void *node = GetPage(cursor->table->pager, pageNum);
    cursor->cellNum++;
    if (cursor->cellNum >= *(LeafNodeCellNums(node))) {
        cursor->endOfTable = true;
    }
}

void *CursorValue(Cursor *cursor) {
    uint32_t pageNum = cursor->pageNum;
    void *page = GetPage(cursor->table->pager, pageNum);
    return LeafNodeValue(page, cursor->cellNum);
}
