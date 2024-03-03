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
public:
    // 提供由字符串初始化InputBuffer的基础设施方法
    static void InitInputBufferByStr(char *str, InputBuffer **inputBuffer) {
        size_t size = strlen(str);
        (*inputBuffer)->buffer = (char *)malloc(size);
        (*inputBuffer)->bufferSize = size;
        (*inputBuffer)->inputSize = size;
        memcpy((*inputBuffer)->buffer, str, size);
    }

    InputBuffer *inputBuffer;
    Table *table;

protected:
    void SetUp() override {
        inputBuffer = CreateInputBuffer();
        table = CreateTable();
    }
    void TearDown() override {
        DestroyInputBuffer(inputBuffer);
        DestroyTable(table);
    }
};

TEST_F(TestDbFixture, InsertTest) {
    char insert[] = "insert 1 test test@db.com";
    TestDbFixture::InitInputBufferByStr(insert, &(this->inputBuffer));

    Statement statement;
    PrepareResult prepareRet = PrepareStatement(inputBuffer, &statement);
    ASSERT_EQ(prepareRet, PREPARE_SUCCESS);

    ASSERT_NE(this->table, nullptr);
    ExecuteResult exeRet = ExecuteStatement(&statement, this->table);
    ASSERT_EQ(exeRet, EXECUTE_SUCCESS);

    EXPECT_EQ(table->rowNum, 1);
    void *data = RowSlot(this->table, 0);
    Row row;
    DeserializeRow(data, &row);
    EXPECT_EQ(row.id, 1);
    EXPECT_STREQ(row.username, "test");
    EXPECT_STREQ(row.email, "test@db.com");
}

TEST_F(TestDbFixture, InsertMaxLenTest) {
    
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
    for (auto i = 0; i < COLUMN_USERNAME_SIZE; i++) {
        memset(username + i, 'a', 1);
    }
    for (auto i = 0; i < COLUMN_EMAIL_SIZE; i++) {
        memset(email + i, 'a', 1);
    }
    username[COLUMN_USERNAME_SIZE] = 0;
    email[COLUMN_EMAIL_SIZE] = 0;

    Statement statement;
    statement.statementType = STATEMENT_INSERT;
    statement.rowToInsert.id = 0;
    memcpy(statement.rowToInsert.username, username, COLUMN_USERNAME_SIZE + 1);
    memcpy(statement.rowToInsert.email, email, COLUMN_EMAIL_SIZE + 1);

    ExecuteStatement(&statement, table);

    void *data = RowSlot(this->table, 0);
    Row row = {0};
    DeserializeRow(data, &row);
    EXPECT_STREQ(username, row.username);
    EXPECT_STREQ(email, row.email);
}

TEST_F(TestDbFixture, ExitTest) {
    char metaCmd[] = ".exit";
    InputBuffer *inputBuffer = CreateInputBuffer();
    TestDbFixture::InitInputBufferByStr(metaCmd, &inputBuffer);
    MetaCommandResult ret =  ExecuteMetaCommand(inputBuffer);
    EXPECT_EQ(ret, META_COMMAND_SUCCESS);
}