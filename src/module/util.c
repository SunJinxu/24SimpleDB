#include "b_tree.h"
#include "db.h"
#include "common.h"
#include "util.h"

void PrintConstants() {
    printf("ROW_SIZE: %u\n", ROW_SIZE);
    printf("COMMON_NODE_HEADER_SIZE: %u\n", COMMON_NODE_HEADER_SIZE);
    printf("LEAF_NODE_HEADER_SIZE: %u\n", LEAF_NODE_HEADER_SIZE);
    printf("LEAF_NODE_CELL_SIZE: %u\n", LEAF_NODE_CELL_SIZE);
    printf("LEAF_NODE_SPACE_FOR_CELLS: %u\n", LEAF_NODE_SPACE_FOR_CELLS);
    printf("LEAF_NODE_MAX_CELLS: %u\n", LEAF_NODE_MAX_CELLS);
}

/**
 * Row的打印方法
*/
void PrintRow(Row *row) {
    printf("ID: %d, USERNAME: %s, EMAIL: %s\n", row->id, row->username, row->email);
}