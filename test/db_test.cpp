#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

extern "C" {
#include "compiler.h"
#include "vm.h"
#include "db.h"
}

/**
 * 测试的基础设施，继承Test类
*/
class TestDbFixture : public testing::Test {
protected:
    void SetUp() override {
    }
    void TearDown() override {
    }
};


TEST_F(TestDbFixture, InsertTest) {

    char insert[] = "insert 1 test test@db.com";
    auto insertSize = sizeof(insert) - 1;
    InputBuffer *inputBuffer = CreateInputBuffer();
    inputBuffer->buffer = (char *)malloc(insertSize);
    inputBuffer->bufferSize = insertSize;
    inputBuffer->inputSize = insertSize;
    memcpy(inputBuffer->buffer, insert, insertSize);

    Statement statement;
    PrepareResult prepareRet = PrepareStatement(inputBuffer, &statement);
    ASSERT_EQ(prepareRet, PREPARE_SUCCESS);

    Table *table = CreateTable();
    ASSERT_NE(table, nullptr);
    ExecuteResult exeRet = ExecuteStatement(&statement, table);
    ASSERT_EQ(exeRet, EXECUTE_SUCCESS);

    EXPECT_EQ(table->rowNum, 1);
    void *data = RowSlot(table, 0);
    Row row;
    DeserializeRow(data, &row);
    EXPECT_EQ(row.id, 1);
    EXPECT_STREQ(row.username, "test");
    EXPECT_STREQ(row.email, "test@db.com");
}