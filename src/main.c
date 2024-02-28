#include "stdio.h"
#include "stdlib.h"
#include <string.h>

#define DB_PROMPT "24SimpleDb > "
#define true 1
#define false 0
#define DB_EXIT_SIGN ".exit"

void printPrompt() {
    printf(DB_PROMPT);
}

/**
 * 信息缓存结构体
*/
typedef struct InputBuffer {
    char *buffer;   // 内存
    size_t bufferSize;  // 内存长度（用于getline函数出参）
    ssize_t inputSize;  // 有效长度
} InputBuffer;

/**
 * 创建并返回一个信息缓存指针
*/
InputBuffer *createInputBuffer() {
    InputBuffer *ptr = (InputBuffer *)malloc(sizeof(InputBuffer));
    ptr->buffer = NULL;
    ptr->bufferSize = 0;
    ptr->inputSize = 0;
    return ptr;
}

/**
 * 销毁信息缓存
*/
void destroyInputBuffer(InputBuffer *inputBuffer) {
    free(inputBuffer->buffer);
    free(inputBuffer);
}

/**
 * 读取命令行输入至缓存中
*/
void readCliInput(InputBuffer *inputBuffer) {
    ssize_t readCliLen = getline(&(inputBuffer->buffer), &(inputBuffer->bufferSize), stdin);
    if (readCliLen <= 0) {
        printf("invalide cli read!");
        exit(EXIT_FAILURE);
    }
    // 忽视最后的换行符
    inputBuffer->inputSize = readCliLen - 1;
    inputBuffer->buffer[readCliLen - 1] = '\0';
}

int main() {
    InputBuffer *userInput = createInputBuffer();
    printf("welcome to use 24SimpleDb!\n");

    while (true) {
        printPrompt();
        readCliInput(userInput);

        if (strcmp(userInput->buffer, DB_EXIT_SIGN) == 0) {
            printf("24SimpleDb stopped!");
            exit(EXIT_SUCCESS);
        } else {
            printf("unrecognized command '%s'. \n", userInput->buffer);
        }
    }

    return 0;
}