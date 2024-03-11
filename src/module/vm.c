#include <string.h>
#include "util.h"
#include "table.h"
#include "cursor.h"
#include "b_tree.h"
#include "vm.h"

/**
 * 处理meta-commands
*/
MetaCommandResult ExecuteMetaCommand(InputBuffer *InputBuffer, Table *table) {
    if (strcmp(InputBuffer->buffer, ".exit") == 0) {
        DbClose(table);
        exit(EXIT_SUCCESS);
    } else if (strcmp(InputBuffer->buffer, ".constants") == 0) {
        printf("Constants: \n");
        PrintConstants();
        return META_COMMAND_SUCCESS;
    } else if(strcmp(InputBuffer->buffer, ".btree") == 0) {
        printf("Tree:\n");
        PrintTree(table->pager, 0, 0);
        return META_COMMAND_SUCCESS;
    }
    return META_COMMAND_UNRECOGNIZED_COMMAND;
}

/**
 * insert执行函数（插入至table最末尾）
*/
ExecuteResult ExecuteInsert(Statement *statement, Table *table) {
    void *node = GetPage(table->pager, table->rootPageNum);
    uint32_t cellNums = *LeafNodeCellNums(node);

    Row* rowToInsert = &(statement->rowToInsert);
    uint32_t keyToInsert = rowToInsert->id;
    Cursor* cursor = TableFind(table, keyToInsert);

    if (cursor->cellNum < cellNums) {
        uint32_t ketAtIndex = *LeafNodeKey(node, cursor->cellNum);
        if (ketAtIndex == keyToInsert) {
            return EXECUTE_DUPLICATE_KEY;
        }
    }

    LeafNodeInsert(cursor, rowToInsert->id, rowToInsert);
    return EXECUTE_SUCCESS;
}

/**
 * select执行函数（打印table中所有row）
*/
ExecuteResult ExecuteSelect(Statement *statement, Table *table) {
    Cursor *cursor = TableStart(table);
    Row row;
    while (!(cursor->endOfTable)) {
        void *slot = CursorValue(cursor);
        DeserializeRow(slot, &row);
        PrintRow(&row);
        CursorAdvance(cursor);
    }
    free(cursor);
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

uint32_t *NodeParent(void *node) {
    return node + PARENT_POINTER_OFFSET;
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

/**
 * 插入一个叶子节点
*/
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

void UpdateInternalNodeKey(void *node, uint32_t oldKey, uint32_t newKey) {
    uint32_t oldChildIndex = InternalNodeFindChild(node, oldKey);
    *InternalNodeKey(node, oldChildIndex) = newKey;
}

/**
 * 创建一个新节点，以右侧节点为参数，分配一个新page存放左侧节点，过程中rootpage始终是0号页面
*/
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