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
    SetNodeRoot(node, false);
    *LeafNodeCellNums(node) = 0;
    *LeafNodeNextLeaf(node) = 0;    // sibling设置为0，表示不存在
}

void InitializeInternalNode(void *node) {
    SetNodeType(node, NODE_INTERNAL);
    SetNodeRoot(node, false);
    *InternalNodeKeyNums(node) = 0;
    *InternalNodeRightChild(node) = INVALID_PAGE_NUM;
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
}

uint32_t *InternalNodeKeyNums(void *node) {
    return node + INTERNAL_NODE_NUM_KEYS_OFFSET;
}

uint32_t *InternalNodeRightChild(void *node) {
    return node + INTERNAL_NODE_RIGHT_CHILD_OFFSET;
}

uint32_t *InternalNodeCell(void *node, uint32_t cellNum) {
    return node + INTERNAL_NODE_HEADER_SIZE + cellNum * INTERNAL_NODE_CELL_SIZE;
}

uint32_t *InternalNodeChild(void *node, uint32_t childNum) {
    uint32_t numKeys = *InternalNodeKeyNums(node);
    if (childNum > numKeys) {
        printf("try to access child num(%u) > num keys(%u)", childNum, numKeys);
        exit(EXIT_FAILURE);
    } else if (childNum == numKeys) {
        uint32_t *rightChild = InternalNodeRightChild(node);
        if (*rightChild == INVALID_PAGE_NUM) {
            printf("try to access the right child, but no valid page\n");
            exit(EXIT_FAILURE);
        }
        return rightChild;
    } else {
        uint32_t *child = InternalNodeCell(node, childNum);
        if (*child == INVALID_PAGE_NUM) {
            printf("try to access the child, but no valid page\n");
            exit(EXIT_FAILURE);
        }
        return child;
    }
}

uint32_t *InternalNodeKey(void *node, uint32_t keyNum) {
    return (void *)InternalNodeCell(node, keyNum) + INTERNAL_NODE_CHILD_SIZE;
}

uint32_t GetNodeMaxKey(Pager *pager, void *node) {
    if (GetNodeType(node) == NODE_LEAF) {
        return *LeafNodeKey(node, *LeafNodeCellNums(node) - 1);
    }
    void *rightChild = GetPage(pager, *InternalNodeRightChild(node));
    return GetNodeMaxKey(pager, rightChild);
}

bool IsNodeRoot(void *node) {
    uint8_t value = *((uint8_t *)(node + IS_ROOT_OFFSET));
    return (bool)value;
}

void SetNodeRoot(void *node, bool isRoot) {
    uint8_t value = isRoot;
    *((uint8_t *)(node + IS_ROOT_OFFSET)) = value;
}

uint32_t *LeafNodeNextLeaf(void *node) {
    return node + LEAF_NODE_NEXT_LEAF_OFFSET;
}