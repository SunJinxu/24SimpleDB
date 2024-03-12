#include <string.h>
#include "table.h"
#include "b_tree.h"
#include "b_tree_helper.h"

void LeafNodeInsert(Cursor *cursor, uint32_t key, Row *value) {
    void *node = GetPage(cursor->table->pager, cursor->pageNum);

    uint32_t cellNums = *LeafNodeCellNums(node);
    // node full
    if (cellNums >= LEAF_NODE_MAX_CELLS) {
        LeafNodeSplitAndInsert(cursor, key, value);
        return;
    }

    if (cursor->cellNum < cellNums) {   // 数据后移
        for (uint32_t i = cellNums; i > cursor->cellNum; i--) {
            memcpy(LeafNodeCell(node, i), LeafNodeCell(node, i - 1), LEAF_NODE_CELL_SIZE);
        }
    }

    *(LeafNodeCellNums(node)) += 1;
    *(LeafNodeKey(node, cursor->cellNum)) = key;
    SerializeRow(value, LeafNodeValue(node, cursor->cellNum));
}

void CreateNewRoot(Table *table, uint32_t rightChildPageNum) {
    // 处理对root节点的分割
    // old root拷贝至新page，成为左子节点
    // 重新初始化root page来包含新的root节点
    // 新root节点指向两个孩子
    void *root = GetPage(table->pager, table->rootPageNum);
    void *rightChild = GetPage(table->pager, rightChildPageNum);
    uint32_t leftChildPageNum = GetUnusedPageNum(table->pager);
    void *leftChild = GetPage(table->pager, leftChildPageNum);
    if (GetNodeType(root) == NODE_INTERNAL) {
        InitializeInternalNode(rightChild);
        InitializeInternalNode(leftChild);
    }
    memcpy(leftChild, root, PAGE_SIZE);
    SetNodeRoot(leftChild, false);
    if (GetNodeType(leftChild) == NODE_INTERNAL) {
        void* child;
        for (int i = 0; i < *InternalNodeKeyNums(leftChild); i++) {
          child = GetPage(table->pager, *InternalNodeChild(leftChild,i));
          *NodeParent(child) = leftChildPageNum;
        }
        child = GetPage(table->pager, *InternalNodeRightChild(leftChild));
        *NodeParent(child) = leftChildPageNum;
    }
    // 新的根节点是带有1个key和2个child的internal node
    InitializeInternalNode(root);
    SetNodeRoot(root, true);
    *InternalNodeKeyNums(root) = 1;
    *InternalNodeChild(root, 0) = leftChildPageNum;
    uint32_t leftChildMaxKey = GetNodeMaxKey(table->pager, leftChild);
    *InternalNodeKey(root, 0) = leftChildMaxKey;
    *InternalNodeRightChild(root) = rightChildPageNum;
    *NodeParent(leftChild) = table->rootPageNum;
    *NodeParent(rightChild) = table->rootPageNum;
}

void LeafNodeSplitAndInsert(Cursor *cursor, uint32_t key, Row *value) {
    // 1 创建一个新节点
    void *oldPage = GetPage(cursor->table->pager, cursor->pageNum);
    uint32_t oldMax = GetNodeMaxKey(cursor->table->pager, oldPage);
    uint32_t unusedPageNum = GetUnusedPageNum(cursor->table->pager);
    void *newPage = GetPage(cursor->table->pager, unusedPageNum);
    InitializeLeafNode(newPage);
    *NodeParent(newPage) = *NodeParent(oldPage);
    *LeafNodeNextLeaf(newPage) = *LeafNodeNextLeaf(oldPage);
    *LeafNodeNextLeaf(oldPage) = unusedPageNum;
    // 2 将原有已满节点的一半迁移过去，新加入的节点在过程中插入
    for (int32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--) {
        void *destPage;
        if (i >= LEAF_NODE_LEFT_SPLIT_COUNT) {
            destPage = newPage;
        } else {
            destPage = oldPage;
        }
        uint32_t cellDestIndex = i % LEAF_NODE_LEFT_SPLIT_COUNT;    // 当前节点在目标节点下标
        void *cell = LeafNodeCell(destPage, cellDestIndex);
        if (i > cursor->cellNum) {  // 插入节点后面的节点，拷贝过去即可
            memcpy(cell, LeafNodeCell(oldPage, i - 1), LEAF_NODE_CELL_SIZE);
        } else if (i == cursor->cellNum) {  // 刚好等于cursor光标，那就直接把value序列化到这里
            *(LeafNodeKey(destPage, cellDestIndex)) = key;
            SerializeRow(value, LeafNodeValue(destPage, cellDestIndex));
        } else if (i < cursor->cellNum) {   // 光标不用再取前一位了，因为已经被补充掉了
            memcpy(cell, LeafNodeCell(oldPage, i), LEAF_NODE_CELL_SIZE);
        }
    }
    *LeafNodeCellNums(oldPage) = LEAF_NODE_LEFT_SPLIT_COUNT;
    *LeafNodeCellNums(newPage) = LEAF_NODE_RIGHT_SPLIT_COUNT;
    
    if (IsNodeRoot(oldPage)) {  // 3 创建Root节点
        CreateNewRoot(cursor->table, unusedPageNum);
    } else {    // 更新父节点
        uint32_t parentPageNum = *NodeParent(oldPage);
        uint32_t newMax = GetNodeMaxKey(cursor->table->pager ,oldPage);
        void *parent = GetPage(cursor->table->pager, parentPageNum);
        UpdateInternalNodeKey(parent, oldMax, newMax);
        InternalNodeInsert(cursor->table, parentPageNum, unusedPageNum);
    }
}

void InternalNodeSplitAndInsert(Table *table, uint32_t parentPageNum, uint32_t childPageNum) {
    uint32_t oldPgaeNum = parentPageNum;
    void *oldNode = GetPage(table->pager, parentPageNum);
    uint32_t oldMax = GetNodeMaxKey(table->pager, oldNode);

    void *child = GetPage(table->pager, childPageNum);
    uint32_t childMax = GetNodeMaxKey(table->pager, child);

    uint32_t newPageNum = GetUnusedPageNum(table->pager);

    uint32_t splittingRoot = IsNodeRoot(oldNode);
    void *parent;
    void *newNode;
    if (splittingRoot) {
        CreateNewRoot(table, newPageNum);
        parent = GetPage(table->pager, table->rootPageNum);
        oldPgaeNum = *InternalNodeChild(parent, 0);
        oldNode = GetPage(table->pager, oldPgaeNum);
    } else {
        parent = GetPage(table->pager, *NodeParent(oldNode));
        newNode = GetPage(table->pager, newPageNum);
        InitializeInternalNode(newNode);
    }

    uint32_t *oldNumKeys = InternalNodeKeyNums(oldNode);
    uint32_t curPageNum = *InternalNodeRightChild(oldNode);
    void *cur = GetPage(table->pager, curPageNum);
    // 旧节点的right child迁移至new node，同时旧node的right child设置为invalid
    InternalNodeInsert(table, newPageNum, curPageNum);
    *(NodeParent(cur)) = newPageNum;
    *InternalNodeRightChild(oldNode) = INVALID_PAGE_NUM;
    // 移动旧node内容至新node
    for (uint32_t i = INTERNAL_NODE_MAX_CELLS - 1; i > (INTERNAL_NODE_MAX_CELLS >> 1); i--) {
        curPageNum = *InternalNodeChild(oldNode, i);
        cur = GetPage(table->pager, curPageNum);
        InternalNodeInsert(table, newPageNum, curPageNum);
        *NodeParent(cur) = newPageNum;
        (*oldNumKeys)--;
    }

    // 重新设置迁移后的oldNode的最右节点，并将key减小1
    *InternalNodeRightChild(oldNode) = *InternalNodeChild(oldNode,*oldNumKeys - 1);
    (*oldNumKeys)--;

    // 确定新key应该插入到分裂后的哪个node当中
    uint32_t maxAfterSplit = GetNodeMaxKey(table->pager, oldNode);
    uint32_t destinationPageNum = childMax < maxAfterSplit ? oldPgaeNum : newPageNum;
    InternalNodeInsert(table, destinationPageNum, childPageNum);
    *NodeParent(child) = destinationPageNum;
    UpdateInternalNodeKey(parent, oldMax, GetNodeMaxKey(table->pager, oldNode));

    if (!splittingRoot) {
        InternalNodeInsert(table, *NodeParent(oldNode), newPageNum);
        *NodeParent(newNode) = *NodeParent(oldNode);
    }
}

void UpdateInternalNodeKey(void *node, uint32_t oldKey, uint32_t newKey) {
    uint32_t oldChildIndex = InternalNodeFindChild(node, oldKey);
    *InternalNodeKey(node, oldChildIndex) = newKey;
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

uint32_t GetNodeMaxKey(Pager *pager, void *node) {
    if (GetNodeType(node) == NODE_LEAF) {
        return *LeafNodeKey(node, *LeafNodeCellNums(node) - 1);
    }
    void *rightChild = GetPage(pager, *InternalNodeRightChild(node));
    return GetNodeMaxKey(pager, rightChild);
}