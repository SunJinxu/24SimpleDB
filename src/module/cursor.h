#ifndef DB_CURSOR_H
#define DB_CURSOR_H

#include "common.h"
#include "db.h"

/**
 * Cursor结构
*/
typedef struct {
    Table *table;   // 指向的table
    uint32_t rowNum;
    bool endOfTable;    // 指向table末尾位置（新增row在此插入）
} Cursor;

/**
 * 创建位于table头尾的Cursor
*/
Cursor *TableStart(Table *table);
Cursor *TableEnd(Table *table);

/**
 * 移动cursor至下一个位置
*/
void CursorAdvance(Cursor *cursor);

/**
 * 根据Cursor获取数据在内存Page中位置的方法(早期版本为RowSlot(table, rowNum))
*/
void *CursorValue(Cursor *cursor);

#endif