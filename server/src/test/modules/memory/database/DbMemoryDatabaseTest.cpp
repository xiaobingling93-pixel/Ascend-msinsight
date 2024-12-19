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
#include "NumberUtil.h"

using namespace Dic::Module::Timeline;
using namespace Dic::Module::FullDb;
using namespace Dic;

class DbMemoryDatabaseTest : public ::testing::Test {
public:
    static void SetUpTestSuite()
    {
        std::string currPath = Dic::FileUtil::GetCurrPath();
        int index = currPath.find_last_of("server");
        currPath = currPath.substr(0, index + 1);
        std::string dbPath3 = R"(/src/test/test_data/full_db/)";
        DataBaseManager::Instance().SetDataType(DataType::DB);
        DataBaseManager::Instance().SetFileType(FileType::PYTORCH);
        auto memoryDatabase =
            std::dynamic_pointer_cast<DbMemoryDataBase, Dic::Module::Memory::VirtualMemoryDataBase>(
                DataBaseManager::Instance().GetMemoryDatabase("0"));
        memoryDatabase->OpenDb(currPath + dbPath3 + "ascend_pytorch_profiler.db", false);
        // minTime = 1734230739709945000, maxTime = 1734230739709945000
        TraceTime::Instance().UpdateTime(1734230739709945000, 1734230739709945000);
    }

    static void TearDownTestSuite()
    {
        auto memoryDatabase = DataBaseManager::Instance().GetMemoryDatabase("0");
        memoryDatabase->CloseDb();
        DataBaseManager::Instance().Clear();
    }
};

TEST_F(DbMemoryDatabaseTest, FullDbQueryMemoryComponentData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryComponentParams requestParams;
    requestParams.rankId = "0";
    requestParams.currentPage = 0;
    requestParams.pageSize = 100; // page size = 100
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Protocol::MemoryComponent> responseBody;
    auto result = database->QueryComponentDetail(requestParams, columnAttr, responseBody);
    EXPECT_TRUE(result);
    int expectSize = 0;
    int expectColumnSize = 3;
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(DbMemoryDatabaseTest, FullDbQueryEntireComponentTable)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    uint64_t offsetTime = Dic::Module::Timeline::TraceTime::Instance().GetOffsetByFileId("0");
    std::vector<Dic::Protocol::MemoryComponent> responseBody;
    bool result = database->QueryEntireComponentTable(responseBody, offsetTime);
    int expectSize = 0;
    EXPECT_TRUE(result);
    EXPECT_EQ(responseBody.size(), expectSize);
}

TEST_F(DbMemoryDatabaseTest, FullDbQueryMemoryOperatorData)
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

TEST_F(DbMemoryDatabaseTest, FullDbQueryMemoryOperatorWithTime)
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

TEST_F(DbMemoryDatabaseTest, FullDbQueryMemoryOperatorWithLimitedTime)
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

TEST_F(DbMemoryDatabaseTest, FullDbQueryMemoryOperatorWithSize)
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

TEST_F(DbMemoryDatabaseTest, FullDbQueryMemoryOperatorByStreamExceptZero)
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

TEST_F(DbMemoryDatabaseTest, FullDbQueryMemoryOperatorByStreamExceptSeveral)
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

TEST_F(DbMemoryDatabaseTest, FullDbQueryComponentsTotalNum)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryComponentParams requestParams;
    requestParams.rankId = "0";
    int64_t totalNum;
    auto result = database->QueryComponentsTotalNum(requestParams, totalNum);
    EXPECT_TRUE(result);
    int expectSize = 0;
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(DbMemoryDatabaseTest, FullDbQueryOperatorsTotalNum)
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

TEST_F(DbMemoryDatabaseTest, FullDbQueryOperatorsTotalNumWithSize)
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

TEST_F(DbMemoryDatabaseTest, FullDbQueryOperatorsTotalNumWithTime)
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

TEST_F(DbMemoryDatabaseTest, FullDbQueryOperatorsTotalNumWithLimitedTime)
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

TEST_F(DbMemoryDatabaseTest, FullDbQueryOperatorsTotalNumByStreamExpectZero)
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

TEST_F(DbMemoryDatabaseTest, FullDbQueryOperatorsTotalNumByStreamExpectSeveral)
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

TEST_F(DbMemoryDatabaseTest, FullDbQueryEntireOperatorTable)
{
    DataBaseManager::Instance().SetDataType(DataType::DB);
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    uint64_t offsetTime = Dic::Module::Timeline::TraceTime::Instance().GetOffsetByFileId("0");
    std::vector<Dic::Protocol::MemoryOperator> opDetails;
    bool result = database->QueryEntireOperatorTable(opDetails, offsetTime);
    int expectSize = 359;
    EXPECT_TRUE(result);
    EXPECT_EQ(opDetails.size(), expectSize);
}

TEST_F(DbMemoryDatabaseTest, FullDbQueryMemoryResourceTypeData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    std::string type;
    bool result = database->QueryMemoryResourceType(type);
    std::string expectType = "Pytorch";
    EXPECT_TRUE(result);
    EXPECT_EQ(type, expectType);
}

TEST_F(DbMemoryDatabaseTest, FullDbQueryMemoryStaticOperatorGraph)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::StaticOperatorGraphParams requestParams;
    requestParams.rankId = "0";
    requestParams.graphId = "0";
    Dic::Protocol::StaticOperatorGraphItem graphItem;
    bool result = database->QueryStaticOperatorGraph(requestParams, graphItem);
    int expectLegendsSize = 0;
    int expectLinesSize = 0;
    EXPECT_FALSE(result);
    EXPECT_EQ(graphItem.legends.size(), expectLegendsSize);
    EXPECT_EQ(graphItem.lines.size(), expectLinesSize);
}

TEST_F(DbMemoryDatabaseTest, FullDbQueryMemoryStaticOperatorList)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::StaticOperatorListParams requestParams;
    requestParams.rankId = "0";
    requestParams.graphId = "0";
    requestParams.currentPage = 0;
    requestParams.pageSize = 0;
    requestParams.startNodeIndex = -1;
    requestParams.endNodeIndex = -1;
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Protocol::StaticOperatorItem> responseBody;
    bool result = database->QueryStaticOperatorList(requestParams, columnAttr, responseBody);
    int expectSize = 0;
    int expectColumnSize = 0;
    EXPECT_FALSE(result);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
    EXPECT_EQ(responseBody.size(), expectSize);
}

TEST_F(DbMemoryDatabaseTest, FullDbQueryMemoryStaticOperatorsTotalNum)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::StaticOperatorListParams requestParams;
    requestParams.rankId = "0";
    requestParams.graphId = "0";
    requestParams.currentPage = 0;
    requestParams.pageSize = 0;
    requestParams.startNodeIndex = -1;
    requestParams.endNodeIndex = -1;
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    int64_t totalNum = 0;
    bool result = database->QueryStaticOperatorsTotalNum(requestParams, totalNum);
    int expectNum = 0;
    EXPECT_FALSE(result);
    EXPECT_EQ(totalNum, expectNum);
}

TEST_F(DbMemoryDatabaseTest, FullDbQueryMemoryEntireStaticOperatorTable)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::StaticOperatorListParams requestParams;
    requestParams.rankId = "0";
    requestParams.graphId = "0";
    requestParams.currentPage = 0;
    requestParams.pageSize = 0;
    requestParams.startNodeIndex = -1;
    requestParams.endNodeIndex = -1;
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    std::vector<Protocol::StaticOperatorItem> responseBody;
    bool result = database->QueryEntireStaticOperatorTable(requestParams, responseBody);
    int expectSize = 0;
    EXPECT_FALSE(result);
    EXPECT_EQ(responseBody.size(), expectSize);
}

TEST_F(DbMemoryDatabaseTest, FullDbQueryMemoryTypeDataDynamic)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    std::string type;
    std::vector<std::string> graphId;
    bool result = database->QueryMemoryType(type, graphId);
    int expectSize = 0;
    EXPECT_TRUE(result);
    EXPECT_EQ(type, Module::Memory::MEMORY_TYPE_DYNAMIC);
    EXPECT_EQ(graphId.size(), expectSize);
}

TEST_F(DbMemoryDatabaseTest, FullDbQueryMemoryViewData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryViewParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    Dic::Protocol::MemoryViewData responseBody;
    uint64_t offsetTime = Dic::Module::Timeline::TraceTime::Instance().GetOffsetByFileId("0");
    auto result = database->QueryMemoryView(requestParams, responseBody, offsetTime);
    EXPECT_EQ(result, true);
    int expectSize = 8401;
    EXPECT_EQ(responseBody.lines.size(), expectSize);
}

TEST_F(DbMemoryDatabaseTest, FullDbQueryMemoryViewDataByStreamExpectSeveral)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryViewParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    Dic::Protocol::MemoryViewData responseBody;
    uint64_t offsetTime = Dic::Module::Timeline::TraceTime::Instance().GetOffsetByFileId("0");
    auto result = database->QueryMemoryView(requestParams, responseBody, offsetTime);
    EXPECT_EQ(result, true);
    int expectSize = 806;
    EXPECT_EQ(responseBody.lines.size(), expectSize);
}

TEST_F(DbMemoryDatabaseTest, FullDbQueryMemoryViewDataByComponentExpectSeveral)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryViewParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_COMPONENT_GROUP;
    Dic::Protocol::MemoryViewData responseBody;
    uint64_t offsetTime = Dic::Module::Timeline::TraceTime::Instance().GetOffsetByFileId("0");
    auto result = database->QueryMemoryView(requestParams, responseBody, offsetTime);
    EXPECT_EQ(result, true);
    int expectSize = 8401;
    EXPECT_EQ(responseBody.lines.size(), expectSize);
}

TEST_F(DbMemoryDatabaseTest, FullDbQueryOperatorSizeData)
{
    auto database = Dic::Module::Timeline::DataBaseManager::Instance().GetMemoryDatabase("0");
    double min;
    double max;
    auto result = database->QueryOperatorSize(min, max, "0");
    EXPECT_EQ(result, true);
    EXPECT_EQ(min, 0.5); // minSize = 0.5
    EXPECT_EQ(max, 32896); // maxSize = 32896
}