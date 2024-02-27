/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "MemoryProtocolRequest.h"
#include "DataBaseManager.h"
#include "DbMemoryDataBase.h"
#include "ParamsParser.h"
#include "FileUtil.h"

using namespace Dic::Module::Timeline;
using namespace Dic::Module::FullDb;

class DbMemoryTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        std::string currPath = Dic::FileUtil::GetCurrPath();
        const ParamsOption &option = ParamsParser::Instance().GetOption();
        ServerLog::Initialize(option.logPath, option.logSize, option.logLevel, to_string(option.wsPort));
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        std::string dbPath3 = R"(/src/test/test_data/full_db/)";
        DataBaseManager::Instance().SetDataType(DataType::FULL_DB);
        auto memoryDatabase =
                dynamic_cast<DbMemoryDataBase *>(DataBaseManager::Instance().GetMemoryDatabase("2"));
        memoryDatabase->OpenDb(currPath + dbPath3 + "report_0.db", false);
    }

    static void TearDownTestCase() {}
};

TEST_F(DbMemoryTest, FullDb_of_QueryMemoryOperatorData)
{
    DataBaseManager::Instance().SetDataType(DataType::FULL_DB);
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("2");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.currentPage = 0;
    requestParams.pageSize = 10; // page size = 10
    requestParams.startTime = -1;
    requestParams.endTime = -1;
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::MemoryOperator> responseBody;
    auto result = database->QueryOperatorDetail(requestParams, columnAttr, responseBody);
    EXPECT_EQ(result, false);
}

TEST_F(DbMemoryTest, FullDb_of_QueryMemoryOperatorWithTime)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("2");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.currentPage = 0;
    requestParams.pageSize = 100; // page size = 100
    requestParams.startTime = 0;
    requestParams.endTime = 1695120000000; // end time = 1695120000000
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::MemoryOperator> responseBody;
    auto result = database->QueryOperatorDetail(requestParams, columnAttr, responseBody);
    EXPECT_EQ(result, false);
}

TEST_F(DbMemoryTest, FullDb_of_QueryMemoryOperatorWithSize)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("2");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.currentPage = 0;
    requestParams.pageSize = 10; // page size = 10
    requestParams.startTime = -1;
    requestParams.endTime = -1;
    requestParams.minSize = 10; // min size = 10
    requestParams.maxSize = 100; // min size = 100
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::MemoryOperator> responseBody;
    auto result = database->QueryOperatorDetail(requestParams, columnAttr, responseBody);
    EXPECT_EQ(result, false);
}

TEST_F(DbMemoryTest, FullDb_of_QueryMemoryOperatorByStreamExceptZero)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("2");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    requestParams.currentPage = 0;
    requestParams.pageSize = 100; // page size = 100
    requestParams.startTime = 0;
    requestParams.endTime = -1;
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::MemoryOperator> responseBody;
    auto result = database->QueryOperatorDetail(requestParams, columnAttr, responseBody);
    EXPECT_EQ(result, false);
}

TEST_F(DbMemoryTest, FullDb_of_QueryMemoryOperatorByStreamExceptSeveral)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("2");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "1";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    requestParams.currentPage = 0;
    requestParams.pageSize = 100; // page size = 100
    requestParams.startTime = -1;
    requestParams.endTime = -1;
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::MemoryOperator> responseBody;
    auto result = database->QueryOperatorDetail(requestParams, columnAttr, responseBody);
    EXPECT_EQ(result, false);
}

TEST_F(DbMemoryTest, FullDb_of_QueryOperatorsTotalNum)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("2");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.searchName = "cann::Graph_";
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    requestParams.startTime = -1;
    requestParams.endTime = -1;
    int64_t totalNum;
    auto result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    EXPECT_EQ(result, false);
}

TEST_F(DbMemoryTest, FullDb_of_QueryOperatorsTotalNumWithSize)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("2");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.searchName = "cann::Graph_";
    requestParams.minSize = 10; // min size = 10
    requestParams.maxSize = 1000; // max size = 1000
    requestParams.startTime = -1;
    requestParams.endTime = -1;
    int64_t totalNum;
    auto result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    EXPECT_EQ(result, false);
}

TEST_F(DbMemoryTest, FullDb_of_QueryOperatorsTotalNumWithTime)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("2");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.searchName = "cann::Graph_";
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    requestParams.startTime = 0;
    requestParams.endTime = 1695120000000; // end time = 1695120000000
    int64_t totalNum;
    auto result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    EXPECT_EQ(result, false);
}

TEST_F(DbMemoryTest, FullDb_of_QueryOperatorsTotalNumByStreamExpectZero)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("2");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    requestParams.startTime = -1;
    requestParams.endTime = -1; // end time = 1695120000000
    int64_t totalNum;
    auto result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    EXPECT_EQ(result, false);
}

TEST_F(DbMemoryTest, FullDb_of_QueryOperatorsTotalNumByStreamExpectSeveral)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("2");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "1";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    requestParams.startTime = -1;
    requestParams.endTime = -1; // end time = 1695120000000
    int64_t totalNum;
    auto result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    EXPECT_EQ(result, false);
}

TEST_F(DbMemoryTest, FullDb_of_QueryMemoryViewData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("2");
    Dic::Protocol::MemoryComponentParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    Dic::Protocol::MemoryViewData responseBody;
    auto result = database->QueryMemoryView(requestParams, responseBody);
    EXPECT_EQ(result, false);
}

TEST_F(DbMemoryTest, FullDb_of_QueryMemoryViewDataByStreamExpectZero)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("2");
    Dic::Protocol::MemoryComponentParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    Dic::Protocol::MemoryViewData responseBody;
    auto result = database->QueryMemoryView(requestParams, responseBody);
    EXPECT_EQ(result, false);
}

TEST_F(DbMemoryTest, FullDb_of_QueryMemoryViewDataByStreamExpectSeveral)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("2");
    Dic::Protocol::MemoryComponentParams requestParams;
    requestParams.rankId = "1";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    Dic::Protocol::MemoryViewData responseBody;
    auto result = database->QueryMemoryView(requestParams, responseBody);
    EXPECT_EQ(result, false);
}

TEST_F(DbMemoryTest, FullDb_of_QueryOperatorSizeData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("2");
    double min;
    double max;
    auto result = database->QueryOperatorSize(min, max);
    EXPECT_EQ(result, false);
}