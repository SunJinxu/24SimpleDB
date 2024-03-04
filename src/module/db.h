#ifndef DB_DB_H
#define DB_DB_H

#include "common.h"
#include "pager.h"

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

/**
 * 表的行
*/
typedef struct {
    int id;
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
} Row;

/**
 * Table结构体
*/
typedef struct {
    uint32_t rowNum;
    Pager *pager;   // Table使用pager访问页面
} Table;

// 定义Row的一些属性
static const uint32_t ID_SIZE  = size_of_attribute(Row, id);
static const uint32_t USERNAME_SIZE =  size_of_attribute(Row, username);
static const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
static const uint32_t ID_OFFSET = 0;
static const uint32_t USERNAME_OFFSET = (ID_OFFSET + ID_SIZE);
static const uint32_t EMAIL_OFFSET =  (USERNAME_OFFSET + USERNAME_SIZE);
static const uint32_t ROW_SIZE  = (ID_SIZE + USERNAME_SIZE + EMAIL_SIZE);

// 定义Table一些属性
static const uint32_t ROWS_PER_PAGE = (PAGE_SIZE / ROW_SIZE);
static const uint32_t TABLE_MAX_ROWS = (ROWS_PER_PAGE * TABLE_MAX_PAGES);

/**
 * 根据指定的rowNum返回对应的rowSlot(槽)指针
*/
void *RowSlot(Table *table, uint32_t rowNum);

/**
 * Row与CompactedRow之间的互相转换方法
*/
void SerializeRow(Row *row, void *compacted);
void DeserializeRow(void *compacted, Row *row);

/**
 * 文件名打开DB
*/
Table *DbOpen(const char *fileName);

/**
 * 关闭DB，刷新pager所有更新内容至磁盘
*/
void DbClose(Table *table);

#endif