/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include <gtest/gtest.h>
#include "MemoryProtocolRequest.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "../../../TestSuit.cpp"

using namespace Dic::Module::Timeline;

TEST_F(TestSuit, QueryMemoryComponentDataExpectSeveral)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryComponentParams requestParams;
    requestParams.rankId = "0";
    requestParams.currentPage = 1;
    requestParams.pageSize = 10; // page size = 10
    requestParams.order = "ascend";
    requestParams.orderBy = "component";
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Protocol::MemoryComponent> responseBody;
    bool result = database->QueryComponentDetail(requestParams, columnAttr, responseBody);
    int expectSize = 2;
    int expectColumnSize = 3;
    EXPECT_TRUE(result);
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(TestSuit, QueryMemoryComponentDataExpectZero)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryComponentParams requestParams;
    requestParams.rankId = "0";
    requestParams.currentPage = 2; // current page = 2
    requestParams.pageSize = 10; // page size = 10
    requestParams.order = "ascend";
    requestParams.orderBy = "component";
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Protocol::MemoryComponent> responseBody;
    bool result = database->QueryComponentDetail(requestParams, columnAttr, responseBody);
    int expectSize = 0;
    int expectColumnSize = 3;
    EXPECT_TRUE(result);
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(TestSuit, QueryMemoryEntireComponentTable)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    uint64_t offsetTime = Dic::Module::Timeline::TraceTime::Instance().GetOffsetByFileId("0");
    std::vector<Protocol::MemoryComponent> responseBody;
    bool result = database->QueryEntireComponentTable(responseBody, offsetTime);
    int expectSize = 2;
    EXPECT_TRUE(result);
    EXPECT_EQ(responseBody.size(), expectSize);
}

TEST_F(TestSuit, QueryMemoryOperatorData)
{
    DataBaseManager::Instance().SetDataType(DataType::TEXT);
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
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
    bool result = database->QueryOperatorDetail(requestParams, columnAttr, responseBody);
    int expectSize = 10;
    int expectColumnSize = 9;
    EXPECT_TRUE(result);
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(TestSuit, QueryMemoryOperatorWithTime)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
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
    bool result = database->QueryOperatorDetail(requestParams, columnAttr, responseBody);
    int expectSize = 28;
    int expectColumnSize = 9;
    EXPECT_TRUE(result);
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(TestSuit, QueryMemoryOperatorWithLimitedTime)
{
    uint64_t startTime = Dic::Module::Timeline::TraceTime::Instance().GetStartTime();
    uint64_t offsetTime = Dic::Module::Timeline::TraceTime::Instance().GetOffsetByFileId("0");
    const uint64_t timeStamp = 1695115378729750000;
    const double secondToMillisecond = 1000.0;
    const int precision = 3;
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.currentPage = 0;
    requestParams.pageSize = 100; // page size = 100
    requestParams.startTime = NumberUtil::DoubleReservedNDigits(
        (timeStamp - startTime - offsetTime) / (secondToMillisecond * secondToMillisecond), precision);
    requestParams.endTime = NumberUtil::DoubleReservedNDigits(
        (timeStamp - startTime - offsetTime) / (secondToMillisecond * secondToMillisecond), precision);
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::MemoryOperator> responseBody;
    bool result = database->QueryOperatorDetail(requestParams, columnAttr, responseBody);
    int expectSize = 1;
    int expectColumnSize = 9;
    EXPECT_TRUE(result);
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(TestSuit, QueryMemoryOperatorWithLimitedTimeOnlyShowWithin)
{
    uint64_t startTime = Dic::Module::Timeline::TraceTime::Instance().GetStartTime();
    uint64_t offsetTime = Dic::Module::Timeline::TraceTime::Instance().GetOffsetByFileId("0");
    const uint64_t timeStamp = 1695115378729750000;
    const double secondToMillisecond = 1000.0;
    const int precision = 3;
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.currentPage = 0;
    requestParams.pageSize = 100; // page size = 100
    requestParams.startTime = NumberUtil::DoubleReservedNDigits(
        (timeStamp - startTime - offsetTime) / (secondToMillisecond * secondToMillisecond), precision);
    requestParams.endTime = NumberUtil::DoubleReservedNDigits(
        (timeStamp - startTime - offsetTime) / (secondToMillisecond * secondToMillisecond), precision);
    requestParams.isOnlyShowAllocatedOrReleasedWithinInterval = true;
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::MemoryOperator> responseBody;
    bool result = database->QueryOperatorDetail(requestParams, columnAttr, responseBody);
    int expectSize = 0;
    int expectColumnSize = 9;
    EXPECT_TRUE(result);
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(TestSuit, QueryMemoryEntireOperatorTable)
{
    uint64_t offsetTime = Dic::Module::Timeline::TraceTime::Instance().GetOffsetByFileId("0");
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    std::vector<Protocol::MemoryOperator> opDetails;
    bool result = database->QueryEntireOperatorTable(opDetails, offsetTime);
    int expectSize = 28;
    EXPECT_TRUE(result);
    EXPECT_EQ(opDetails.size(), expectSize);
}

TEST_F(TestSuit, QueryMemoryTypeDynamic)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    std::string type = Module::Memory::MEMORY_TYPE_STATIC;
    std::vector<std::string> graphId;
    bool result = database->QueryMemoryType(type, graphId);
    int expectSize = 0;
    EXPECT_TRUE(result);
    EXPECT_EQ(type, Module::Memory::MEMORY_TYPE_DYNAMIC);
    EXPECT_EQ(graphId.size(), expectSize);
}

TEST_F(TestSuit, QueryMemoryTypeStatic)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    std::string type = Module::Memory::MEMORY_TYPE_DYNAMIC;
    std::vector<std::string> graphId;
    bool result = database->QueryMemoryType(type, graphId);
    int expectSize = 2;
    EXPECT_TRUE(result);
    EXPECT_EQ(type, Module::Memory::MEMORY_TYPE_STATIC);
    EXPECT_EQ(graphId.size(), expectSize);
}

TEST_F(TestSuit, QueryMemoryResourceTypePytorch)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    std::string type = Module::Memory::MEMORY_RESOURCE_TYPE_MIND_SPORE;
    bool result = database->QueryMemoryResourceType(type);
    EXPECT_TRUE(result);
    EXPECT_EQ(type, Module::Memory::MEMORY_RESOURCE_TYPE_PYTORCH);
}

TEST_F(TestSuit, QueryStaticOperatorListParamsException)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::StaticOperatorListParams requestParams;
    requestParams.rankId = "0";
    requestParams.graphId = "0";
    requestParams.currentPage = 0;
    requestParams.startNodeIndex = -1;
    requestParams.endNodeIndex = -1;
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::StaticOperatorItem> responseBody;
    bool result = database->QueryStaticOperatorList(requestParams, columnAttr, responseBody);
    int expectSize = 0;
    int expectColumnSize = 5; // 该场景下表头数据正常返回，表格数据无内容
    EXPECT_TRUE(result);
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(TestSuit, QueryStaticOperatorListParams)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    Dic::Protocol::StaticOperatorListParams requestParams;
    requestParams.rankId = "1";
    requestParams.graphId = "0";
    requestParams.currentPage = 3; // 选择最后一页 3/3
    requestParams.startNodeIndex = -1;
    requestParams.endNodeIndex = -1;
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::StaticOperatorItem> responseBody;
    bool result = database->QueryStaticOperatorList(requestParams, columnAttr, responseBody);
    int expectSize = 7;
    int expectColumnSize = 5;
    EXPECT_TRUE(result);
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(TestSuit, QueryStaticOperatorListTotal)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    Dic::Protocol::StaticOperatorListParams requestParams;
    requestParams.rankId = "1";
    requestParams.graphId = "0";
    requestParams.currentPage = 3; // 选择最后一页 3/3
    requestParams.startNodeIndex = -1;
    requestParams.endNodeIndex = -1;
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    int64_t number;
    bool result = database->QueryStaticOperatorsTotalNum(requestParams, number);
    int expectNumber = 27;
    EXPECT_TRUE(result);
    EXPECT_EQ(number, expectNumber);
}

TEST_F(TestSuit, QueryStaticOperatorListParamsWithNodeIndex)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    Dic::Protocol::StaticOperatorListParams requestParams;
    requestParams.rankId = "1";
    requestParams.graphId = "0";
    requestParams.currentPage = 0;
    requestParams.pageSize = 100; // page size = 100
    requestParams.startNodeIndex = 0;
    requestParams.endNodeIndex = 10; // 结束节点索引 = 10
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::StaticOperatorItem> responseBody;
    bool result = database->QueryStaticOperatorList(requestParams, columnAttr, responseBody);
    int expectSize = 16;
    int expectColumnSize = 5;
    EXPECT_TRUE(result);
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(TestSuit, QueryStaticOperatorListParamsWithSizeFileter)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    Dic::Protocol::StaticOperatorListParams requestParams;
    requestParams.rankId = "1";
    requestParams.graphId = "0";
    requestParams.currentPage = 0;
    requestParams.pageSize = 100; // page size = 100
    requestParams.startNodeIndex = -1;
    requestParams.endNodeIndex = -1;
    requestParams.minSize = 800; // 筛选 min Size = 800
    requestParams.maxSize = 3000; // 筛选 max Size = 3000
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::StaticOperatorItem> responseBody;
    bool result = database->QueryStaticOperatorList(requestParams, columnAttr, responseBody);
    int expectSize = 4;
    EXPECT_TRUE(result);
    EXPECT_EQ(responseBody.size(), expectSize);
}

TEST_F(TestSuit, QueryStaticOperatorListParamsWithAllFilter)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    Dic::Protocol::StaticOperatorListParams requestParams;
    requestParams.rankId = "1";
    requestParams.graphId = "1";
    requestParams.order = "ascend";
    requestParams.orderBy = "op_name";
    requestParams.searchName = "re";
    requestParams.currentPage = 0;
    requestParams.pageSize = 100; // page size = 100
    requestParams.startNodeIndex = -1;
    requestParams.endNodeIndex = -1;
    requestParams.minSize = 600; // 筛选 min Size = 600
    requestParams.maxSize = 3000; // 筛选 max Size = 3000
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::StaticOperatorItem> responseBody;
    bool result = database->QueryStaticOperatorList(requestParams, columnAttr, responseBody);
    int expectSize = 4;
    EXPECT_TRUE(result);
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(responseBody[0].opName, "reducemax_03");
}

TEST_F(TestSuit, QueryStaticOperatorListTotalWithAllFilter)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    Dic::Protocol::StaticOperatorListParams requestParams;
    requestParams.rankId = "1";
    requestParams.graphId = "1";
    requestParams.searchName = "re";
    requestParams.currentPage = 0;
    requestParams.pageSize = 100; // page size = 100
    requestParams.startNodeIndex = -1;
    requestParams.endNodeIndex = -1;
    requestParams.minSize = 600; // 筛选 min Size = 600
    requestParams.maxSize = 3000; // 筛选 max Size = 3000
    int64_t number;
    bool result = database->QueryStaticOperatorsTotalNum(requestParams, number);
    int expectNumber = 4;
    EXPECT_TRUE(result);
    EXPECT_EQ(number, expectNumber);
}

TEST_F(TestSuit, QueryStaticOperatorListTotalWithNodeIndex)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    Dic::Protocol::StaticOperatorListParams requestParams;
    requestParams.rankId = "1";
    requestParams.graphId = "1";
    requestParams.currentPage = 0;
    requestParams.pageSize = 100; // page size = 100
    requestParams.startNodeIndex = 0;
    requestParams.endNodeIndex = 10; // end Node Index = 10
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    int64_t number;
    bool result = database->QueryStaticOperatorsTotalNum(requestParams, number);
    int expectNumber = 16;
    EXPECT_TRUE(result);
    EXPECT_EQ(number, expectNumber);
}

TEST_F(TestSuit, QueryEntireStaticOperatorTable)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    Dic::Protocol::StaticOperatorListParams requestParams;
    requestParams.rankId = "1";
    requestParams.currentPage = 0;
    requestParams.pageSize = 0;
    requestParams.startNodeIndex = -1;
    requestParams.endNodeIndex = -1;
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    std::vector<Protocol::StaticOperatorItem> opDetails;
    bool result = database->QueryEntireStaticOperatorTable(requestParams, opDetails);
    int expectSize = 54;
    EXPECT_TRUE(result);
    EXPECT_EQ(opDetails.size(), expectSize);
}

TEST_F(TestSuit, QueryStaticOperatorSizeDataWithoutGraphId)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    Dic::Protocol::StaticOperatorSizeParams requestParams;
    requestParams.graphId = "";
    double min;
    double max;
    bool result = database->QueryStaticOperatorSize(requestParams, min, max);
    double expectMin = 0.21191406300000001;
    double expectMax = 2343.78125;
    EXPECT_TRUE(result);
    EXPECT_EQ(min, expectMin);
    EXPECT_EQ(max, expectMax);
}

TEST_F(TestSuit, QueryStaticOperatorSizeDataWithGraphId)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    Dic::Protocol::StaticOperatorSizeParams requestParams;
    requestParams.graphId = "0";
    double min;
    double max;
    bool result = database->QueryStaticOperatorSize(requestParams, min, max);
    double expectMin = 0.21191406300000001;
    double expectMax = 2343.78125;
    EXPECT_TRUE(result);
    EXPECT_EQ(min, expectMin);
    EXPECT_EQ(max, expectMax);
}

TEST_F(TestSuit, QueryStaticOperatorGraph)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    Dic::Protocol::StaticOperatorGraphParams requestParams;
    requestParams.rankId = "1";
    requestParams.graphId = "0";
    StaticOperatorGraphItem data;
    bool result = database->QueryStaticOperatorGraph(requestParams, data);
    int expectSize = 3;
    int expectColumnSize = 24;
    int pickedIndex = 5;
    string exceptPickedData = "4.35";
    EXPECT_TRUE(result);
    EXPECT_EQ(data.legends.size(), expectSize);
    EXPECT_EQ(data.lines.size(), expectColumnSize);
    EXPECT_EQ(data.lines[pickedIndex][1], exceptPickedData);
}

TEST_F(TestSuit, QueryStaticOperatorGraphWithGraphId)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    Dic::Protocol::StaticOperatorGraphParams requestParams;
    requestParams.rankId = "1";
    requestParams.graphId = "1";
    StaticOperatorGraphItem data;
    bool result = database->QueryStaticOperatorGraph(requestParams, data);
    int expectColumnSize = 26;
    string exceptPickedData = "4.48";
    int pickedIndex = 19;
    EXPECT_TRUE(result);
    EXPECT_EQ(data.lines.size(), expectColumnSize);
    EXPECT_EQ(data.lines[pickedIndex][1], exceptPickedData);
}

TEST_F(TestSuit, QueryMemoryOperatorWithSize)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
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
    bool result = database->QueryOperatorDetail(requestParams, columnAttr, responseBody);
    int expectSize = 10;
    int expectColumnSize = 9;
    EXPECT_TRUE(result);
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(TestSuit, QueryMemoryOperatorByStreamExceptZero)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
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
    bool result = database->QueryOperatorDetail(requestParams, columnAttr, responseBody);
    int expectSize = 0;
    int expectColumnSize = 9;
    EXPECT_TRUE(result);
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(TestSuit, QueryMemoryOperatorByStreamExceptSeveral)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "1";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    requestParams.currentPage = 0;
    requestParams.pageSize = 100; // page size = 100
    requestParams.startTime = -1;
    requestParams.endTime = -1;
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::MemoryOperator> responseBody;
    bool result = database->QueryOperatorDetail(requestParams, columnAttr, responseBody);
    int expectSize = 21;
    int expectColumnSize = 14;
    EXPECT_TRUE(result);
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(TestSuit, QueryComponentsTotalNum)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryComponentParams requestParams;
    requestParams.rankId = "0";
    requestParams.currentPage = 1;
    requestParams.pageSize = 10; // page size = 10
    requestParams.order = "descend";
    requestParams.orderBy = "timestamp";
    int64_t totalNum;
    bool result = database->QueryComponentsTotalNum(requestParams, totalNum);
    int expectSize = 2;
    EXPECT_TRUE(result);
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(TestSuit, QueryOperatorsTotalNum)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.searchName = "cann::Graph_";
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    requestParams.startTime = -1;
    requestParams.endTime = -1;
    int64_t totalNum;
    bool result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    int expectSize = 28;
    EXPECT_TRUE(result);
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(TestSuit, QueryOperatorsTotalNumWithSize)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.searchName = "cann::Graph_";
    requestParams.minSize = 10; // min size = 10
    requestParams.maxSize = 1000; // max size = 1000
    requestParams.startTime = -1;
    requestParams.endTime = -1;
    int64_t totalNum;
    bool result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    int expectSize = 15;
    EXPECT_TRUE(result);
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(TestSuit, QueryOperatorsTotalNumWithTime)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.searchName = "cann::Graph_";
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    requestParams.startTime = 0;
    requestParams.endTime = 1695120000000; // end time = 1695120000000
    int64_t totalNum;
    bool result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    int expectSize = 28;
    EXPECT_TRUE(result);
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(TestSuit, QueryOperatorsTotalNumWithLimitedTime)
{
    uint64_t startTime = Dic::Module::Timeline::TraceTime::Instance().GetStartTime();
    uint64_t offsetTime = Dic::Module::Timeline::TraceTime::Instance().GetOffsetByFileId("0");
    const uint64_t timeStamp = 1695115378729750000;
    const double secondToMillisecond = 1000.0;
    const int precision = 3;
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.searchName = "cann::Graph_";
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    requestParams.startTime = NumberUtil::DoubleReservedNDigits(
        (timeStamp - startTime - offsetTime) / (secondToMillisecond * secondToMillisecond), precision);
    requestParams.endTime = NumberUtil::DoubleReservedNDigits(
        (timeStamp - startTime - offsetTime) / (secondToMillisecond * secondToMillisecond), precision);
    int64_t totalNum;
    bool result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    int expectSize = 1;
    EXPECT_TRUE(result);
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(TestSuit, QueryOperatorsTotalNumByStreamExpectZero)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    requestParams.startTime = -1;
    requestParams.endTime = -1; // end time = 1695120000000
    int64_t totalNum;
    bool result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    int expectSize = 0;
    EXPECT_TRUE(result);
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(TestSuit, QueryOperatorsTotalNumByStreamExpectSeveral)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "1";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    requestParams.minSize = std::numeric_limits<int64_t>::min();
    requestParams.maxSize = std::numeric_limits<int64_t>::max();
    requestParams.startTime = -1;
    requestParams.endTime = -1; // end time = 1695120000000
    int64_t totalNum;
    bool result = database->QueryOperatorsTotalNum(requestParams, totalNum);
    int expectSize = 21;
    EXPECT_TRUE(result);
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(TestSuit, QueryMemoryViewData)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryViewParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    Dic::Protocol::MemoryViewData responseBody;
    uint64_t offsetTime = 0;
    bool result = database->QueryMemoryView(requestParams, responseBody, offsetTime);
    int expectSize = 5;
    EXPECT_TRUE(result);
    EXPECT_EQ(responseBody.lines.size(), expectSize);
}

TEST_F(TestSuit, QueryMemoryViewDataByStreamExpectZero)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryViewParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    Dic::Protocol::MemoryViewData responseBody;
    uint64_t offsetTime = 0;
    bool result = database->QueryMemoryView(requestParams, responseBody, offsetTime);
    int expectSize = 0;
    EXPECT_TRUE(result);
    EXPECT_EQ(responseBody.lines.size(), expectSize);
}

TEST_F(TestSuit, QueryMemoryViewDataByStreamExpectSeveral)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    Dic::Protocol::MemoryViewParams requestParams;
    requestParams.rankId = "1";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    Dic::Protocol::MemoryViewData responseBody;
    uint64_t offsetTime = 0;
    bool result = database->QueryMemoryView(requestParams, responseBody, offsetTime);
    int expectSize = 1;
    EXPECT_TRUE(result);
    EXPECT_EQ(responseBody.lines.size(), expectSize);
}

TEST_F(TestSuit, QueryMemoryViewDataByComponentExpectSeveral)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    Dic::Protocol::MemoryViewParams requestParams;
    requestParams.rankId = "1";
    requestParams.type = Protocol::MEMORY_COMPONENT_GROUP;
    Dic::Protocol::MemoryViewData responseBody;
    uint64_t offsetTime = 0;
    bool result = database->QueryMemoryView(requestParams, responseBody, offsetTime);
    int expectSize = 29;
    EXPECT_TRUE(result);
    EXPECT_EQ(responseBody.lines.size(), expectSize);
}

TEST_F(TestSuit, QueryOperatorSizeData)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    double min;
    double max;
    bool result = database->QueryOperatorSize(min, max);
    double expectMin = 64.0;
    double expectMax = 81984.0;
    EXPECT_TRUE(result);
    EXPECT_EQ(min, expectMin);
    EXPECT_EQ(max, expectMax);
}