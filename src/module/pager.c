#include <unistd.h>
#include "fcntl.h"
#include "pager.h"

Pager *PagerOpen(const char *fileName) {
    int fd = open(fileName, // 文件名
                    O_RDWR |    // 读写模式
                    O_CREAT ,   // 如果不存在，创建该文件
                    S_IWUSR |   // 用户写权限
                    S_IRUSR);   // 用户读权限
    
    if (fd == -1) {
        printf("unable to open file, exit!\n");
        exit(EXIT_FAILURE);
    }

    off_t fileLength = lseek(fd, 0, SEEK_END);
    
    Pager *pager = (Pager *)malloc(sizeof(Pager));
    pager->fileDescriptor = fd;
    pager->fileLength = fileLength;
    pager->pageNums = fileLength / PAGE_SIZE;

    if (fileLength % PAGE_SIZE) {
        printf("Db file is not a whole number of pages, exit!");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        pager->pages[i] = NULL;
    }

    return pager;
}

void *GetPage(Pager *pager, uint32_t pageNum) {
    if (pageNum >= TABLE_MAX_PAGES) {
        printf("fetch num is larger than max page limit\n");
        exit(EXIT_FAILURE);
    }

    if (pager->pages[pageNum] == NULL) {    // 未命中缓存
        // 分配缓存并加载
        void *page = malloc(PAGE_SIZE);
        
        // 获取Pager总页数，(如果不是恰好为整数页，需要向上取1页)
        uint32_t pagerCurNum = pager->fileLength / PAGE_SIZE;
        if (pager->fileLength % PAGE_SIZE) {
            pagerCurNum++;
        }

        if (pagerCurNum < pageNum) {
            exit(EXIT_FAILURE);
        }

        // 加载缓存至新页面
        lseek(pager->fileDescriptor, pageNum * PAGE_SIZE, SEEK_SET);    // 移动pager描述符至寻找页面位置
        ssize_t readBytes = read(pager->fileDescriptor, page, PAGE_SIZE);  // 读取pager描述符页面内容至page
        if (readBytes == -1) {
            printf("try to get page failed, exit!\n");
            exit(EXIT_FAILURE);
        }
        pager->pages[pageNum] = page;
        // 由于PagerFlush的方法根据pager->pageNums作为尾部哨兵，因此当发现修改脏页>=时，需要后移哨兵
        if (pageNum >= pager->pageNums) {
            pager->pageNums++;
        }
    }
    return pager->pages[pageNum];
}

void PagerFlush(Pager *pager, uint32_t pageNum) {
    if (pager->pages[pageNum] == NULL) {
        printf("try to flush null page,exit!\n");
        exit(EXIT_FAILURE);
    }

    off_t offset = lseek(pager->fileDescriptor, pageNum * PAGE_SIZE, SEEK_SET);
    if (offset == -1) {
        printf("error to seek page, exit!\n");
        exit(EXIT_FAILURE);
    }

    ssize_t writeBytes = write(pager->fileDescriptor, pager->pages[pageNum], PAGE_SIZE);
    if (writeBytes == -1) {
        printf("try to flush page failed, exit!\n");
        exit(EXIT_FAILURE);
    }
}