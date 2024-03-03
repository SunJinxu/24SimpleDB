#ifndef DB_DB_H
#define DB_DB_H

#include <stdio.h>
#include <stdlib.h>
#include "common.h"

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100

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
    void *pages[TABLE_MAX_PAGES];
} Table;

// 定义Row的一些属性
#define ID_SIZE size_of_attribute(Row, id)
#define USERNAME_SIZE size_of_attribute(Row, username)
#define EMAIL_SIZE size_of_attribute(Row, email)
#define ID_OFFSET 0
#define USERNAME_OFFSET ID_OFFSET + ID_SIZE
#define EMAIL_OFFSET USERNAME_OFFSET + USERNAME_SIZE
#define ROW_SIZE ID_SIZE + USERNAME_SIZE + EMAIL_SIZE

// 定义Table一些属性
#define PAGE_SIZE  4096    // 与大部分操作系统的内存页大小相同
#define ROWS_PER_PAGE  PAGE_SIZE / ROW_SIZE
#define TABLE_MAX_ROWS  ROWS_PER_PAGE * TABLE_MAX_PAGES

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
 * Row打印方法
*/
void PrintRow(Row *row);

/**
 * Table创建与销毁方法
*/
Table *CreateTable();
void DestroyTable(Table *table);

#endif