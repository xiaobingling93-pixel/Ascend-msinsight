/*
 * ------------------------------------------------------------------------- * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * ------------------------------------------------------------------------- */

#include <gtest/gtest.h>
#include "DataBaseManager.h"
#include "MemSnapshotDatabase.h"
#include "MemSnapshotDefs.h"
#include "MemSnapshotTableColumn.h"
#include "MemSnapshotParser.h"
#include "FileUtil.h"
#include "StringUtil.h"
#include "../TestSuit.h"

using namespace Dic::Module::Timeline;
using namespace Dic::Module::FullDb;
using namespace Dic::Module::MemSnapshot;
using namespace Dic;

class MemSnapshotDatabaseTest : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        // 准备测试数据
        testDbPath = TestSuit::GetSrcTestPath() + R"(test_data/snapshot/snapshot_expandable.pkl.db)";

        // 获取数据库实例
        snapshotDb = DataBaseManager::Instance().GetMemSnapshotDatabase(testDbPath);
        ASSERT_TRUE(snapshotDb != nullptr);

        // 打开数据库
        ASSERT_TRUE(snapshotDb->OpenDbReadOnly(testDbPath));
    }

    static void TearDownTestSuite()
    {
        if (snapshotDb != nullptr) {
            snapshotDb->CloseDb();
        }
        DataBaseManager::Instance().Clear(DatabaseType::MEM_SNAPSHOT);
    }

protected:
    static std::string testDbPath;
    static std::shared_ptr<MemSnapshotDatabase> snapshotDb;
};

std::string MemSnapshotDatabaseTest::testDbPath;
std::shared_ptr<MemSnapshotDatabase> MemSnapshotDatabaseTest::snapshotDb = nullptr;

// 测试数据库打开和关闭
TEST_F(MemSnapshotDatabaseTest, OpenAndCloseDb)
{
    EXPECT_TRUE(snapshotDb->IsOpen());

    // 测试重复打开
    EXPECT_TRUE(snapshotDb->OpenDbReadOnly(testDbPath));

    // 关闭数据库
    snapshotDb->CloseDb();
    EXPECT_FALSE(snapshotDb->IsOpen());

    // 重新打开
    EXPECT_TRUE(snapshotDb->OpenDbReadOnly(testDbPath));
    EXPECT_TRUE(snapshotDb->IsOpen());
}

// 测试表存在性检查
TEST_F(MemSnapshotDatabaseTest, CheckAllTableExist)
{
    EXPECT_TRUE(snapshotDb->CheckAllTableExist());
}

// 测试查询所有内存块
TEST_F(MemSnapshotDatabaseTest, QueryAllBlocks)
{
    std::vector<Block> blocks;
    bool result = snapshotDb->QueryAllBlocks(blocks);
    EXPECT_TRUE(result);

    EXPECT_EQ(blocks.size(), 3219);
}

// 测试根据ID查询内存块
TEST_F(MemSnapshotDatabaseTest, QueryBlockById)
{
    // 先查询所有块获取一个有效的ID
    const auto expectBlockId = 1;
    const auto block = snapshotDb->QueryBlockById(expectBlockId);

    EXPECT_TRUE(block.has_value());
    EXPECT_EQ(block->id, expectBlockId);
    EXPECT_EQ(block->state, BLOCK_STATE_ACTIVE_ALLOC);

    // 测试查询不存在的ID
    auto nonExistentBlock = snapshotDb->QueryBlockById(-1000);
    EXPECT_FALSE(nonExistentBlock.has_value());
}

// 测试查询最大事件ID
TEST_F(MemSnapshotDatabaseTest, QueryMaxEntryId)
{
    int64_t maxId = snapshotDb->QueryMaxEntryId();
    EXPECT_EQ(maxId, 8091);
}

// 测试字典映射功能
TEST_F(MemSnapshotDatabaseTest, GetRealValueInTableDictionaryMap)
{
    // 测试存在的映射
    std::string realValue = snapshotDb->GetRealValueInTableDictionaryMap("block", "state", 1);

    // 测试不存在的表
    std::string nonExistentTableValue = snapshotDb->GetRealValueInTableDictionaryMap("non_existent_table", "state", 1);
    EXPECT_EQ(nonExistentTableValue, "1");

    // 测试不存在的列
    std::string nonExistentColumnValue = snapshotDb->
        GetRealValueInTableDictionaryMap("block", "non_existent_column", 1);
    EXPECT_EQ(nonExistentColumnValue, "1");

    // 测试不存在的键
    std::string nonExistentKeyValue = snapshotDb->GetRealValueInTableDictionaryMap("block", "state", 9999);
    EXPECT_EQ(nonExistentKeyValue, "9999");
}

// 测试数据库重置功能
TEST_F(MemSnapshotDatabaseTest, Reset)
{
    // 调用重置方法
    MemSnapshotDatabase::Reset();

    // 验证数据库已关闭
    EXPECT_FALSE(snapshotDb->IsOpen());

    // 重新获取并打开数据库
    snapshotDb = DataBaseManager::Instance().GetMemSnapshotDatabase(testDbPath);
    ASSERT_TRUE(snapshotDb != nullptr);
    EXPECT_TRUE(snapshotDb->OpenDbReadOnly(testDbPath));
}
