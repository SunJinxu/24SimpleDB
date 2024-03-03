#ifndef DB_COMPLIER_H
#define DB_COMPLIER_H

#include "common.h"
#include "vm.h"

/**
 * 创建并返回一个信息缓存指针
*/
InputBuffer *CreateInputBuffer();

/**
 * 销毁信息缓存
*/
void DestroyInputBuffer(InputBuffer *inputBuffer);

void ReadCliInput(InputBuffer *inputBuffer);

/**
 * Statement处理方法
*/
PrepareResult PrepareStatement(InputBuffer *inputBuffer, Statement *statement);

#endif