/**
 * 用于操作B+树节点的类，包含了一些常用的B+树操作方法
*/

#ifndef DB_B_TREE_HELPTER_H
#define DB_B_TREE_HELPTER_H

#include "table.h"
#include "cursor.h"

/**
 * 插入一个叶子节点
*/
void LeafNodeInsert(Cursor *cursor, uint32_t key, Row *value);

/**
 * 向满leaf节点插入新child，导致internal节点分裂和插入
*/
void LeafNodeSplitAndInsert(Cursor *cursor, uint32_t key, Row *value);

/**
 * 向满parent节点插入新child，导致internal节点分裂和插入
*/
void InternalNodeSplitAndInsert(Table *table, uint32_t parentPageNum, uint32_t childPageNum);

/**
 * 创建一个新根节点，并将原根节点内容复制给child节点
*/
void CreateNewRoot(Table *table, uint32_t rightChildPageNum);

/**
 * 向parent节点中加入指定的child
*/
void InternalNodeInsert(Table *table, uint32_t parentPageNum, uint32_t childPageNum);

/**
 * InternalNode的查找包含指定key的child
*/
uint32_t InternalNodeFindChild(void *node, uint32_t key);

/**
 * 指定table的pageNum对应的node中寻找key对应的cursor
 * algorithm: Binary Search
*/
Cursor *LeafNodeFind(Table *table, uint32_t pageNum, uint32_t key);

/**
 * InternalNode的二分查找过程
 * 需要注意的是，internal node的每个cell中的key的值，都是cell中child节点的key最大值
*/
Cursor *InternalNodeFind(Table* table, uint32_t pageNum, uint32_t key);

/**
 * 递归获取node下最大key的方法
*/
uint32_t GetNodeMaxKey(Pager *pager, void *node);

#endif