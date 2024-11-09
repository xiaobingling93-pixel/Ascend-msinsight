/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <gtest/gtest.h>
#include <string>
#include "EnumHcclDataTypeTable.h"
#include "TestCaseDatabaseUtil.h"
using namespace Dic::Protocol;
using namespace Dic::TimeLine::TestCaseUtil;
class EnumHcclDataTypeTableTest : public ::testing::Test {};

/**
 * 测试EnumHcclDataTypeTable字段映射
 */
TEST_F(EnumHcclDataTypeTableTest, TestEnumHcclDataTypeTableTestColumnMaping)
{
    sqlite3 *db = nullptr;
    std::string sql = "CREATE TABLE ENUM_HCCL_DATA_TYPE (id INTEGER PRIMARY KEY,name TEXT);";
    TestCaseDatabaseUtil::CreateDatabse(db, sql);
    std::string sqlInsert = "INSERT INTO \"main\".\"ENUM_HCCL_DATA_TYPE\" (\"id\", \"name\") VALUES (1, 'INT16');";
    TestCaseDatabaseUtil::InsertData(db, sqlInsert);
    std::vector<EnumHcclDataTypePO> enumHcclDataTypePOS;
    Dic::Module::Timeline::EnumHcclDataTypeTable enumHcclDataTypeTable;
    const uint64_t expectSize = 1;
    const uint64_t expectId = 1;
    const uint64_t index = 0;
    enumHcclDataTypeTable.Select(EnumHcclDataTypeClumn::ID, EnumHcclDataTypeClumn::NAME)
        .ExcuteQuery(db, enumHcclDataTypePOS);
    EXPECT_EQ(enumHcclDataTypePOS.size(), expectSize);
    EXPECT_EQ(enumHcclDataTypePOS[index].name, "INT16");
    EXPECT_EQ(enumHcclDataTypePOS[index].id, expectId);
}
