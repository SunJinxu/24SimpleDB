#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

extern "C" {
#include "util.h"
#include "compiler.h"
#include "vm.h"
#include "table.h"
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

    for (uint32_t i = 1; i < 16; i++) {
        statement.rowToInsert.id = i; 
        ExecuteStatement(&statement, table);
    }

    // statement.rowToInsert.id = 20; 
    // ExecuteStatement(&statement, table);

    // statement.rowToInsert.id = 18; 
    // ExecuteStatement(&statement, table);
    Statement selectState = {
        .statementType = STATEMENT_SELECT
    };
    ExecuteStatement(&selectState,table);

    // PrintTree(table->pager, 0, 0);
}