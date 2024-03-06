#ifndef DB_BTREE_H
#define DB_BTREE_H

#include "common.h"
#include "db.h"

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
static const uint32_t LEAF_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE + LEAF_NODE_CELL_NUMS_SIZE;

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
 * 初始化叶子node
*/
void InitializeLeafNode(void *node);

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
#endif