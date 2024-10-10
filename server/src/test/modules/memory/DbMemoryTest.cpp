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
    static void SetUpTestSuite()
    {
        std::string currPath = Dic::FileUtil::GetCurrPath();
        const ParamsOption &option = ParamsParser::Instance().GetOption();
        ServerLog::Initialize(option.logPath, option.logSize, option.logLevel, to_string(option.wsPort));
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        std::string dbPath3 = R"(/src/test/test_data/full_db/)";
        DataBaseManager::Instance().SetDataType(DataType::DB);
        DataBaseManager::Instance().SetFileType(FileType::PYTORCH);
        auto memoryDatabase =
                dynamic_cast<DbMemoryDataBase *>(DataBaseManager::Instance().GetMemoryDatabase("0"));
        memoryDatabase->OpenDb(currPath + dbPath3 + "ascend_pytorch_profiler.db", false);
        // minTime = 1734230739709945000, maxTime = 1734230739709945000
        TraceTime::Instance().UpdateTime(1734230739709945000, 1734230739709945000);
    }

    static void TearDownTestSuite() {}
};

TEST_F(DbMemoryTest, FullDb_of_QueryMemoryOperatorData)
{
    DataBaseManager::Instance().SetDataType(DataType::DB);
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.currentPage = 0;
    requestParams.pageSize = 10; // page size = 10
    requestParams.startTime = -1;
    requestParams.endTime = -1;
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
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
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::MemoryOperator> responseBody;
    auto result = database->QueryOperatorDetail(requestParams, columnAttr, responseBody);
    EXPECT_EQ(result, true);
    int expectSize = 100;
    int expectColumnSize = 14;
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryMemoryOperatorWithLimitedTime)
{
    uint64_t startTime = Dic::Module::Timeline::TraceTime::Instance().GetStartTime();
    uint64_t offsetTime = Dic::Module::Timeline::TraceTime::Instance().GetOffsetByFileId("0");
    const uint64_t timeStamp = 1734230739778225840;
    const double secondToMillisecond = 1000.0;
    const int precision = 3;
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.currentPage = 0;
    requestParams.pageSize = 100; // page size = 100
    requestParams.startTime = NumberUtil::DoubleReservedNDigits(
        (timeStamp - startTime - offsetTime)/ (secondToMillisecond * secondToMillisecond), precision);
    requestParams.endTime = NumberUtil::DoubleReservedNDigits(
        (timeStamp - startTime - offsetTime) / (secondToMillisecond * secondToMillisecond), precision);
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::MemoryOperator> responseBody;
    auto result = database->QueryOperatorDetail(requestParams, columnAttr, responseBody);
    EXPECT_EQ(result, true);
    int expectSize = 6;
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
    requestParams.maxSize = 64; // min size = 64
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
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
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
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
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
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    requestParams.startTime = -1;
    requestParams.endTime = -1;
    int64_t totalNum;
    auto result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    EXPECT_EQ(result, true);
    int expectSize = 359;
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryOperatorsTotalNumWithSize)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.searchName = "aten::empty_strided";
    requestParams.minSize = 0; // min size = 0
    requestParams.maxSize = 600000000; // max size = 600000000
    requestParams.startTime = -1;
    requestParams.endTime = -1;
    int64_t totalNum;
    auto result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    EXPECT_EQ(result, true);
    int expectSize = 18;
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryOperatorsTotalNumWithTime)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.searchName = "aten::empty_strided";
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    requestParams.startTime = 0;
    requestParams.endTime = 1695120000000; // end time = 1695120000000
    int64_t totalNum;
    auto result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    EXPECT_EQ(result, true);
    int expectSize = 18;
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryOperatorsTotalNumWithLimitedTime)
{
    uint64_t startTime = Dic::Module::Timeline::TraceTime::Instance().GetStartTime();
    uint64_t offsetTime = Dic::Module::Timeline::TraceTime::Instance().GetOffsetByFileId("0");
    const uint64_t timeStamp = 1734230739778225840;
    const double secondToMillisecond = 1000.0;
    const int precision = 3;
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    requestParams.startTime = NumberUtil::DoubleReservedNDigits(
        (timeStamp - startTime - offsetTime)/ (secondToMillisecond * secondToMillisecond), precision);
    requestParams.endTime = NumberUtil::DoubleReservedNDigits(
        (timeStamp - startTime - offsetTime) / (secondToMillisecond * secondToMillisecond), precision);
    int64_t totalNum;
    auto result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    EXPECT_EQ(result, true);
    int expectSize = 6;
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryOperatorsTotalNumByStreamExpectZero)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.searchName = "aten::empty_stridedss";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
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
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    requestParams.startTime = -1;
    requestParams.endTime = -1; // end time = 1695120000000
    int64_t totalNum;
    auto result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    EXPECT_EQ(result, true);
    int expectSize = 277;
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryMemoryViewData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryComponentParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    Dic::Protocol::MemoryViewData responseBody;
    uint64_t offsetTime = 0;
    auto result = database->QueryMemoryView(requestParams, responseBody, offsetTime);
    EXPECT_EQ(result, true);
    int expectSize = 8401;
    EXPECT_EQ(responseBody.lines.size(), expectSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryMemoryViewDataByStreamExpectSeveral)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryComponentParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    Dic::Protocol::MemoryViewData responseBody;
    uint64_t offsetTime = 0;
    auto result = database->QueryMemoryView(requestParams, responseBody, offsetTime);
    EXPECT_EQ(result, true);
    int expectSize = 806;
    EXPECT_EQ(responseBody.lines.size(), expectSize);
}

TEST_F(DbMemoryTest, FullDb_of_QueryOperatorSizeData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    double min;
    double max;
    auto result = database->QueryOperatorSize(min, max, "0");
    EXPECT_EQ(result, true);
    EXPECT_EQ(min, 0.5); // minSize = 0.5
    EXPECT_EQ(max, 32896); // maxSize = 32896
}