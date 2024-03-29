#ifndef DB_PAGER_H
#define DB_PAGER_H

#include "common.h"

typedef struct {
    int fileDescriptor;
    uint32_t fileLength;    // 标识记录信息的总长度
    uint32_t pageNums;  // 标记pager中含有多少页page
    void *pages[TABLE_MAX_PAGES];
} Pager;

/**
 * 文件名加载Pager（文件不存在，则新建）
*/
Pager *PagerOpen(const char *fileName);

/**
 * 关闭pager，并刷新至磁盘
*/
void PagerFlush(Pager *pager, uint32_t pageNum);

/**
 * 根据给定pageNum，pager从内存中获取或者从磁盘加载对应的page，并返回page的指针
*/
void *GetPage(Pager *pager, uint32_t pageNum);

/**
 * 获取当前未使用过的page号
*/
uint32_t GetUnusedPageNum(Pager *pager);

#endif