#ifndef DB_UTIL_H
#define DB_UTIL_H

#include "table.h"

/**
 * 打印BTree的常数信息
*/
void PrintConstants();

/**
 * Row的打印方法
*/
void PrintRow(Row *row);

void Indent(uint32_t level);

void PrintTree(Pager *pager, uint32_t pageNum, uint32_t indentationLevel);

#endif