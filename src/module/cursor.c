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

/**
 * InternalNode的二分查找过程
 * 需要注意的是，internal node的每个cell中的key的值，都是cell中child节点的key最大值
*/
Cursor *InternalNodeFind(Table* table, uint32_t pageNum, uint32_t key) {
    void *node = GetPage(table->pager, pageNum);
    uint32_t keyNums = *InternalNodeKeyNums(node);
    // 二分查找确定需要搜寻的子节点
    uint32_t minIndex = 0;
    uint32_t maxIndex = keyNums;
    while (minIndex != maxIndex) {
        uint32_t index = (minIndex + maxIndex) >> 1;
        uint32_t keyToRight = *InternalNodeKey(node, index);
        if (keyToRight >= key) {
            maxIndex = index;
        } else {
            minIndex = index + 1;
        }
    }
    // 根据子节点类型决定下一步查找动作
    uint32_t childNum = *InternalNodeChild(node, minIndex);
    void *child = GetPage(table->pager, childNum);
    switch(GetNodeType(child)) {
        case NODE_LEAF:
            return LeafNodeFind(table, childNum, key);
        case NODE_INTERNAL:
            return InternalNodeFind(table, childNum, key);
    }
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
        cursor->endOfTable = true;
    }
}

void *CursorValue(Cursor *cursor) {
    uint32_t pageNum = cursor->pageNum;
    void *page = GetPage(cursor->table->pager, pageNum);
    return LeafNodeValue(page, cursor->cellNum);
}
