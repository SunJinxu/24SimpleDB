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

/**
 * 创建一个新节点，以右侧节点为参数，分配一个新page存放左侧节点
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
    memcpy(leftChild, root, PAGE_SIZE);
    SetNodeRoot(leftChild, false);
    // 新的根节点是带有1个key和2个child的internal node
    InitializeInternalNode(root);
    SetNodeRoot(root, true);
    *InternalNodeKeyNums(root) = 1;
    *InternalNodeChild(root, 0) = leftChildPageNum;
    uint32_t leftChildMaxKey = GetNodeMaxKey(leftChild);
    *InternalNodeKey(root, 0) = leftChildMaxKey;
    *InternalNodeRightChild(root) = rightChildPageNum;
}

void LeafNodeSplitAndInsert(Cursor *cursor, uint32_t key, Row *value) {
    // 1 创建一个新节点
    void *oldPage = GetPage(cursor->table->pager, cursor->pageNum);
    uint32_t unusedPageNum = GetUnusedPageNum(cursor->table->pager);
    void *newPage = GetPage(cursor->table->pager, unusedPageNum);
    InitializeLeafNode(newPage);
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
    // 3 更新父节点或者创建父节点
    if (IsNodeRoot(oldPage)) {
        return CreateNewRoot(cursor->table, unusedPageNum);
    } else {
        printf("need to implement updating parent node after spliting");
        exit(EXIT_FAILURE);
    }
}