#include "b_tree.h"
#include "table.h"
#include "common.h"
#include "util.h"

/**
 * 打印提示消息
*/
void PrintDbPrompt() {
    printf(DB_PROMPT);
}

void PrintConstants() {
    printf("ROW_SIZE: %u\n", ROW_SIZE);
    printf("COMMON_NODE_HEADER_SIZE: %u\n", COMMON_NODE_HEADER_SIZE);
    printf("LEAF_NODE_HEADER_SIZE: %u\n", LEAF_NODE_HEADER_SIZE);
    printf("LEAF_NODE_CELL_SIZE: %u\n", LEAF_NODE_CELL_SIZE);
    printf("LEAF_NODE_SPACE_FOR_CELLS: %u\n", LEAF_NODE_SPACE_FOR_CELLS);
    printf("LEAF_NODE_MAX_CELLS: %u\n", LEAF_NODE_MAX_CELLS);
}

/**
 * Row的打印方法
*/
void PrintRow(Row *row) {
    printf("ID: %d, USERNAME: %s, EMAIL: %s\n", row->id, row->username, row->email);
}

void Indent(uint32_t level) {
    for (uint32_t i = 0; i < level; i++) {
        printf(" ");
    }
}

void PrintTree(Pager *pager, uint32_t pageNum, uint32_t indentationLevel) {
    void *node = GetPage(pager, pageNum);
    uint32_t numKeys, child;
    switch(GetNodeType(node)) {
        case NODE_LEAF:
            numKeys = *LeafNodeCellNums(node);
            Indent(indentationLevel);
            printf(" - leaf (size %d)\n", numKeys);
            for (uint32_t i = 0; i < numKeys; i++) {
                Indent(indentationLevel + 1);
                printf("- %d\n", *LeafNodeKey(node, i));
            }
            break;
        case NODE_INTERNAL:
            numKeys = *InternalNodeKeyNums(node);
            Indent(indentationLevel);
            printf("- internal (size %d)\n", numKeys);
            if (numKeys > 0) {
                for (uint32_t i = 0; i < numKeys; i++) {
                    child = *InternalNodeChild(node, i);
                    PrintTree(pager, child, indentationLevel + 1);
                    Indent(indentationLevel + 1);
                    printf(" - key %d\n", *InternalNodeKey(node, i));
                }
                child = *InternalNodeRightChild(node);
                PrintTree(pager, child, indentationLevel + 1);
            }
            break;
    }
}