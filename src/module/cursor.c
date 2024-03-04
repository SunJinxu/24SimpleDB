#include "db.h"
#include "cursor.h"

Cursor *TableStart(Table *table) {
    Cursor *cursor = (Cursor *)malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->rowNum = 0;
    cursor->endOfTable = (table->rowNum == 0);
    return cursor;
}

Cursor *TableEnd(Table *table) {
    Cursor *cursor = (Cursor *)malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->rowNum = table->rowNum;
    cursor->endOfTable = true;
    return cursor;
}

void CursorAdvance(Cursor *cursor) {
    cursor->rowNum++;
    if (cursor->rowNum >= cursor->table->rowNum) {
        cursor->endOfTable = true;
    }
}

void *CursorValue(Cursor *cursor) {
    uint32_t rowNum = cursor->rowNum;   // 想要获取的row即为cursor指向的row
    uint32_t pageNum = rowNum / ROWS_PER_PAGE;
    void *page = GetPage(cursor->table->pager, pageNum);
    uint32_t rowOffset = rowNum % ROWS_PER_PAGE;
    uint32_t byteOffset = rowOffset * ROW_SIZE;
    return page + byteOffset;
}
