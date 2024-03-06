#include "util.h"
#include "b_tree.h"

uint32_t *LeafNodeCellNums(void *node) {
    return node + LEAF_NODE_CELL_NUMS_OFFSET;
}

void *LeafNodeCell(void *node, uint32_t cellNum) {
    return node + LEAF_NODE_HEADER_SIZE + cellNum * LEAF_NODE_CELL_SIZE;
}

uint32_t *LeafNodeKey(void *node, uint32_t cellNum) {
    return LeafNodeCell(node, cellNum);
}

void *LeafNodeValue(void *node, uint32_t cellNum) {
    return LeafNodeCell(node, cellNum) + LEAF_NODE_KEY_SIZE;
}

void InitializeLeafNode(void *node) {
    SetNodeType(node, NODE_LEAF);
    *LeafNodeCellNums(node) = 0;
}

NodeType GetNodeType(void *node) {
    uint8_t type =  *(uint8_t *)(node + NODE_TYPE_OFFSET);
    return (NodeType)type;
}

void SetNodeType(void *node, NodeType type) {
    *((uint8_t *)(node + NODE_TYPE_OFFSET)) = (uint8_t)type;
}

void PrintLeafNode(void *node) {
    uint32_t cellNum = *LeafNodeCellNums(node);
    printf("Leaf (size: %u)\n", *(LeafNodeCellNums(node)));
    Row row;
    for (uint32_t i = 0; i < cellNum; i++) {
        void *data = LeafNodeValue(node, i);
        DeserializeRow(data, &row);
        printf("%u: ", i);
        PrintRow(&row);
    }
    printf("\n");
}