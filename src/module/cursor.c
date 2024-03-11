#include <string.h>
#include "table.h"
#include "b_tree.h"
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
 * InternalNode的查找包含指定key的child
 * 二分查找
*/
uint32_t InternalNodeFindChild(void *node, uint32_t key) {
    uint32_t keyNums = *InternalNodeKeyNums(node);
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
    return minIndex;
}

/**
 * InternalNode的二分查找过程
 * 需要注意的是，internal node的每个cell中的key的值，都是cell中child节点的key最大值
*/
Cursor *InternalNodeFind(Table* table, uint32_t pageNum, uint32_t key) {
    void *node = GetPage(table->pager, pageNum);
    uint32_t childIndex = InternalNodeFindChild(node, key);
    uint32_t childNum = *InternalNodeChild(node, childIndex);
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

void InternalNodeInsert(Table *table, uint32_t parentPageNum, uint32_t childPageNum) {
    void *parent = GetPage(table->pager, parentPageNum);
    void *child = GetPage(table->pager, childPageNum);
    // 获取到child在parent internal node中应该插入的index位置
    uint32_t childMaxKey = GetNodeMaxKey(table->pager, child);
    uint32_t index = InternalNodeFindChild(parent, childMaxKey);
    uint32_t originKeyNums = *InternalNodeKeyNums(parent);

    // case1: 插入节点导致InternalNode分裂的情况
    if (originKeyNums >= INTERNAL_NODE_MAX_CELLS) {
        InternalNodeSplitAndInsert(table, parentPageNum, childPageNum);
        return;
    }

    // case2: 插入节点不导致InternalNode分裂的情况
    // 根据child中的max key的大小选择不同的插入方式
    uint32_t rightChildPageNum = *InternalNodeRightChild(parent);   // 大于最大key的child page num
    // 一个拥有最右子节点为INVALID_PAGE_NUM的parent节点,是一个空节点
    if (rightChildPageNum == INVALID_PAGE_NUM) {    // 直接将child作为parent节点的最右子节点的方法
        *InternalNodeRightChild(parent) = childPageNum;
        return;
    }

    void *rightChild = GetPage(table->pager, rightChildPageNum);
    *InternalNodeKeyNums(parent) = originKeyNums+ 1;
    if (childMaxKey > GetNodeMaxKey(table->pager, rightChild)) {  // childMaxKey大于父节点最右侧子节点，将原右子节点移动至新CELL中
        *InternalNodeChild(parent, originKeyNums) = rightChildPageNum;
        *InternalNodeKey(parent, originKeyNums) = GetNodeMaxKey(table->pager, rightChild);
        *InternalNodeRightChild(parent) = childPageNum;
    } else {    // 如果没有大于最右侧子节点
        /* 旧cell移动，为新cell分配空间 */
        for (uint32_t i = originKeyNums; i > index; i--) {
            void *destination = InternalNodeCell(parent, i);
            void *source = InternalNodeCell(parent, i - 1);
            memcpy(destination, source, INTERNAL_NODE_CELL_SIZE);
        }
        *InternalNodeChild(parent, index) = childPageNum;
        *InternalNodeKey(parent, index) = childMaxKey;
    }
}