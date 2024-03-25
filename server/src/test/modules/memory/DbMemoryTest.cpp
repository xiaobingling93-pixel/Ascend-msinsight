/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "MemoryProtocolRequest.h"
#include "DataBaseManager.h"
#include "DbMemoryDataBase.h"
#include "ParamsParser.h"
#include "FileUtil.h"
#include "TraceTime.h"

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
        DataBaseManager::Instance().SetFileType(FileType::MS_PROF);
        auto memoryDatabase =
                dynamic_cast<DbMemoryDataBase *>(DataBaseManager::Instance().GetMemoryDatabase("0"));
        memoryDatabase->OpenDb(currPath + dbPath3 + "report_0.db", false);
        // minTime = 1710490360222620070, maxTime = 1710490361833931450
        TraceTime::Instance().UpdateTime(1710490360222620070, 1710490361833931450);
    }

    static void TearDownTestCase() {}
};

TEST_F(DbMemoryTest, FullDb_of_QueryMemoryOperatorData)
{
    DataBaseManager::Instance().SetDataType(DataType::FULL_DB);
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
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
    EXPECT_EQ(result, true);
    int expectSize = 10;
    int expectColumnSize = 14;
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryMemoryOperatorWithTime)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
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
    EXPECT_EQ(result, true);
    int expectSize = 100;
    int expectColumnSize = 14;
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryMemoryOperatorWithSize)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.currentPage = 0;
    requestParams.pageSize = 10; // page size = 10
    requestParams.startTime = -1;
    requestParams.endTime = -1;
    requestParams.minSize = 10; // min size = 10
    requestParams.maxSize = 65536; // min size = 65536
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::MemoryOperator> responseBody;
    auto result = database->QueryOperatorDetail(requestParams, columnAttr, responseBody);
    EXPECT_EQ(result, true);
    int expectSize = 10;
    int expectColumnSize = 14;
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryMemoryOperatorByStreamExceptZero)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
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
    EXPECT_EQ(result, true);
    int expectSize = 100;
    int expectColumnSize = 14;
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryMemoryOperatorByStreamExceptSeveral)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
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
    EXPECT_EQ(result, true);
    int expectSize = 100;
    int expectColumnSize = 14;
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryOperatorsTotalNum)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.searchName = "";
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    requestParams.startTime = -1;
    requestParams.endTime = -1;
    int64_t totalNum;
    auto result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    EXPECT_EQ(result, true);
    int expectSize = 247;
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryOperatorsTotalNumWithSize)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.searchName = "Notify_Record";
    requestParams.minSize = 65500; // min size = 65500
    requestParams.maxSize = 600000000; // max size = 600000000
    requestParams.startTime = -1;
    requestParams.endTime = -1;
    int64_t totalNum;
    auto result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    EXPECT_EQ(result, true);
    int expectSize = 126;
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryOperatorsTotalNumWithTime)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.searchName = "Notify_Record";
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    requestParams.startTime = 0;
    requestParams.endTime = 1695120000000; // end time = 1695120000000
    int64_t totalNum;
    auto result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    EXPECT_EQ(result, true);
    int expectSize = 126;
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryOperatorsTotalNumByStreamExpectZero)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "1";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    requestParams.startTime = -1;
    requestParams.endTime = -1; // end time = 1695120000000
    int64_t totalNum;
    auto result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    EXPECT_EQ(result, true);
    int expectSize = 0;
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryOperatorsTotalNumByStreamExpectSeveral)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    requestParams.startTime = -1;
    requestParams.endTime = -1; // end time = 1695120000000
    int64_t totalNum;
    auto result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    EXPECT_EQ(result, true);
    int expectSize = 247;
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryMemoryViewData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryComponentParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    Dic::Protocol::MemoryViewData responseBody;
    auto result = database->QueryMemoryView(requestParams, responseBody);
    EXPECT_EQ(result, true);
    int expectSize = 247;
    EXPECT_EQ(responseBody.lines.size(), expectSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryMemoryViewDataByStreamExpectZero)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryComponentParams requestParams;
    requestParams.rankId = "1";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    Dic::Protocol::MemoryViewData responseBody;
    auto result = database->QueryMemoryView(requestParams, responseBody);
    EXPECT_EQ(result, true);
    int expectSize = 0;
    EXPECT_EQ(responseBody.lines.size(), expectSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryMemoryViewDataByStreamExpectSeveral)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryComponentParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    Dic::Protocol::MemoryViewData responseBody;
    auto result = database->QueryMemoryView(requestParams, responseBody);
    EXPECT_EQ(result, true);
    int expectSize = 1;
    EXPECT_EQ(responseBody.lines.size(), expectSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryOperatorSizeData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    double min;
    double max;
    auto result = database->QueryOperatorSize(min, max, "0");
    EXPECT_EQ(result, true);
    EXPECT_EQ(min, 65536); // minSize = 65536
    EXPECT_EQ(max, 604045312); // maxSize = 604045312
}