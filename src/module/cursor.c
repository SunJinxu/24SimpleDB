#include <string.h>
#include "table.h"
#include "b_tree.h"
#include "b_tree_helper.h"
#include "vm.h"
#include "cursor.h"

Cursor *TableStart(Table *table) {
    Cursor *cursor = TableFind(table, 0);

    void *node = GetPage(table->pager, cursor->pageNum);
    uint32_t cellNums = *LeafNodeCellNums(node);
    cursor->endOfTable = (cellNums == 0);
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


Cursor *TableFind(Table *table, uint32_t key) {
    uint32_t rootPageNum = table->rootPageNum;
    void* rootNode = GetPage(table->pager, rootPageNum);

    if (GetNodeType(rootNode) == NODE_LEAF) {
        return LeafNodeFind(table, rootPageNum, key);
    } else {
        return InternalNodeFind(table, rootPageNum, key);
    }
}

void CursorAdvance(Cursor *cursor) {
    uint32_t pageNum = cursor->pageNum;
    void *node = GetPage(cursor->table->pager, pageNum);
    cursor->cellNum++;
    if (cursor->cellNum >= *(LeafNodeCellNums(node))) {
        // 跳转至下一个sibling
        uint32_t nextPageNum = *LeafNodeNextLeaf(node);
        if (nextPageNum == 0) {
            // 到达当前leaf最右侧节点
            cursor->endOfTable = true;
        } else {
            cursor->pageNum = nextPageNum;
            cursor->cellNum = 0;
        }
    }
}

void *CursorValue(Cursor *cursor) {
    uint32_t pageNum = cursor->pageNum;
    void *page = GetPage(cursor->table->pager, pageNum);
    return LeafNodeValue(page, cursor->cellNum);
}