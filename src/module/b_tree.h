#ifndef DB_BTREE_H
#define DB_BTREE_H

#include "common.h"
#include "table.h"

/**
 * B-Tree节点类型
*/
typedef enum {
    NODE_INTERNAL,
    NODE_LEAF
} NodeType;

/**
 * 通用node节点头信息布局
 * 含有nodeType，isRoot，parentPointer
*/
static const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
static const uint32_t NODE_TYPE_OFFSET = 0;
static const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);
static const uint32_t IS_ROOT_OFFSET = NODE_TYPE_OFFSET + NODE_TYPE_SIZE;
static const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
static const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_OFFSET + IS_ROOT_SIZE;
static const uint32_t COMMON_NODE_HEADER_SIZE = NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;

/**
 * 叶子node节点头信息布局
 * 含有cellNums
*/
static const uint32_t LEAF_NODE_CELL_NUMS_SIZE = sizeof(uint32_t);
static const uint32_t LEAF_NODE_CELL_NUMS_OFFSET = COMMON_NODE_HEADER_SIZE;
static const uint32_t LEAF_NODE_NEXT_LEAF_SIZE = sizeof(uint32_t);  // 指向相邻叶子节点指针
static const uint32_t LEAF_NODE_NEXT_LEAF_OFFSET = COMMON_NODE_HEADER_SIZE + LEAF_NODE_CELL_NUMS_SIZE;
static const uint32_t LEAF_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE + LEAF_NODE_CELL_NUMS_SIZE + LEAF_NODE_NEXT_LEAF_SIZE;

/**
 * 叶子节点布局
*/
static const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
static const uint32_t LEAF_NODE_KEY_OFFSET = 0;
static const uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;
static const uint32_t LEAF_NODE_VALUE_OFFSET = LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;
static const uint32_t LEAF_NODE_CELL_SIZE = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
static const uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
static const uint32_t LEAF_NODE_MAX_CELLS = LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;

/**
 * 用于分割叶子节点的参数
*/
static const uint32_t LEAF_NODE_RIGHT_SPLIT_COUNT = (LEAF_NODE_MAX_CELLS + 1) >> 1;
static const uint32_t LEAF_NODE_LEFT_SPLIT_COUNT = (LEAF_NODE_MAX_CELLS + 1) - LEAF_NODE_RIGHT_SPLIT_COUNT;

/**
 * 内部节点头布局
 * 内部节点含有含有common header、key的数量、最右侧child的page number
 * 内部节点的key总比它的child指针少一个
*/
static const uint32_t INTERNAL_NODE_NUM_KEYS_SIZE = sizeof(uint32_t);
static const uint32_t INTERNAL_NODE_NUM_KEYS_OFFSET = COMMON_NODE_HEADER_SIZE;
static const uint32_t INTERNAL_NODE_RIGHT_CHILD_SIZE = sizeof(uint32_t);
static const uint32_t INTERNAL_NODE_RIGHT_CHILD_OFFSET = INTERNAL_NODE_NUM_KEYS_OFFSET + INTERNAL_NODE_NUM_KEYS_SIZE;
static const uint32_t INTERNAL_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE + INTERNAL_NODE_NUM_KEYS_SIZE + INTERNAL_NODE_RIGHT_CHILD_SIZE;
/**
 * 内部节点的body包含一个cell array，每个cell包含一个key和一个child指针
 * 每个key都是左侧节点中包含的最大key
*/
static const uint32_t INTERNAL_NODE_CHILD_SIZE = sizeof(uint32_t);
static const uint32_t INTERNAL_NODE_KEY_SIZE = sizeof(uint32_t);
static const uint32_t INTERNAL_NODE_CELL_SIZE = INTERNAL_NODE_CHILD_SIZE + INTERNAL_NODE_KEY_SIZE;

// 内部节点最多只能有3个key
static const uint32_t INTERNAL_NODE_MAX_CELLS = 3;


/**
 * 获取当前node上总共保存了多少row
*/
uint32_t *LeafNodeCellNums(void *node);

/**
 * 获取当前node中cellNum对应的cell指针
*/
void *LeafNodeCell(void *node, uint32_t cellNum);

/**
 * 获取当前node中cellNum对应的key指针
*/
uint32_t *LeafNodeKey(void *node, uint32_t cellNum);

/**
 * 获取当前node中cellNum对应的value指针
*/
void *LeafNodeValue(void *node, uint32_t cellNum);

/**
 * 初始化node
*/
void InitializeLeafNode(void *node);
void InitializeInternalNode(void *node);

/**
 * 获取node类型
*/
NodeType GetNodeType(void *node);

/**
 * 设置node类型
*/
void SetNodeType(void *node, NodeType type);

/**
 * 树的可视化
*/
void PrintLeafNode(void *node);

/**
 * Internal key的read和write方法
*/
uint32_t *InternalNodeKeyNums(void *node);
uint32_t *InternalNodeRightChild(void *node);
uint32_t *InternalNodeCell(void *node, uint32_t cellNum);
uint32_t *InternalNodeChild(void *node, uint32_t childNum); // childNum==keyNum，返回最右侧rigthChild
uint32_t *InternalNodeKey(void *node, uint32_t keyNum);

/**
 * 获取当前node最大的key
*/
uint32_t GetNodeMaxKey(Pager *pager, void *node);

/**
 * set get root key
*/
bool IsNodeRoot(void *node);
void SetNodeRoot(void *node, bool isRoot);

/**
 * 将internal node中的old key更新为new key
*/
void UpdateInternalNodeKey(void *node, uint32_t oldKey, uint32_t newKey);

/**
 * 获取当前叶子节点的sibling
*/
uint32_t *LeafNodeNextLeaf(void *node);

uint32_t *NodeParent(void *node);

#endif