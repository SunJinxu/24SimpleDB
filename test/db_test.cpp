#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

extern "C" {
#include "time.h"
#include "stdlib.h"
#include "module/util.h"
#include "module/compiler.h"
#include "module/vm.h"
#include "module/table.h"
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

TEST_F(TestDbFixture, DbTestDemo) {
    Table *table = DbOpen("/Users/jessesun/Projects/24SimpleDB/test.db");
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
    for (auto i = 0; i < COLUMN_USERNAME_SIZE; i++) {
        memset(username + i, 'a', 1);
    }
    for (auto i = 0; i < COLUMN_EMAIL_SIZE >> 4; i++) {
        memset(email + i, 'a', 1);
    }
    username[COLUMN_USERNAME_SIZE] = 0;
    email[COLUMN_EMAIL_SIZE] = 0;

    Statement statement;
    statement.statementType = STATEMENT_INSERT;
    memcpy(statement.rowToInsert.username, username, COLUMN_USERNAME_SIZE + 1);
    memcpy(statement.rowToInsert.email, email, COLUMN_EMAIL_SIZE + 1);

    srand(time(NULL));

    for (uint32_t i = 1; i < 50; i++) {
        statement.rowToInsert.id = rand(); 
        ExecuteStatement(&statement, table);
    }

    PrintTree(table->pager, 0, 0);
}