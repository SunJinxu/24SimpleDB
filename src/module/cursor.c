#include "db.h"
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
