/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include <gtest/gtest.h>
#include "MemoryProtocolRequest.h"
#include "DataBaseManager.h"
#include "../../TestSuit.cpp"

using namespace Dic::Module::Timeline;
class MemoryTest : TestSuit {
};

TEST_F(TestSuit, QueryMemoryOperatorData)
{
    DataBaseManager::Instance().SetDataType(DataType::JSON);
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
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
    database->QueryOperatorDetail(requestParams, columnAttr, responseBody);
    int expectSize = 10;
    int expectColumnSize = 9;
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
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::MemoryOperator> responseBody;
    database->QueryOperatorDetail(requestParams, columnAttr, responseBody);
    int expectSize = 28;
    int expectColumnSize = 9;
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(TestSuit, QueryMemoryTypeDynamic)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    std::string type = Module::Memory::MEMORY_TYPE_STATIC;
    std::vector<std::string> graphId;
    database->QueryMemoryType(type, graphId);
    int expectSize = 0;
    EXPECT_EQ(type, Module::Memory::MEMORY_TYPE_DYNAMIC);
    EXPECT_EQ(graphId.size(), expectSize);
}

TEST_F(TestSuit, QueryMemoryTypeStatic)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    std::string type = Module::Memory::MEMORY_TYPE_DYNAMIC;
    std::vector<std::string> graphId;
    database->QueryMemoryType(type, graphId);
    int expectSize = 2;
    EXPECT_EQ(type, Module::Memory::MEMORY_TYPE_STATIC);
    EXPECT_EQ(graphId.size(), expectSize);
}

TEST_F(TestSuit, QueryMemoryResourceTypePytorch)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    std::string type = Module::Memory::MEMORY_RESOURCE_TYPE_MIND_SPORE;
    database->QueryMemoryResourceType(type);
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
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::StaticOperatorItem> responseBody;
    database->QueryStaticOperatorList(requestParams, columnAttr, responseBody);
    int expectSize = 0;
    int expectColumnSize = 5; // 该场景下表头数据正常返回，表格数据无内容
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
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::StaticOperatorItem> responseBody;
    database->QueryStaticOperatorList(requestParams, columnAttr, responseBody);
    int expectSize = 7;
    int expectColumnSize = 5;
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
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    int64_t number;
    database->QueryStaticOperatorsTotalNum(requestParams, number);
    int expectNumber = 27;
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
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::StaticOperatorItem> responseBody;
    database->QueryStaticOperatorList(requestParams, columnAttr, responseBody);
    int expectSize = 16;
    int expectColumnSize = 5;
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
    database->QueryStaticOperatorList(requestParams, columnAttr, responseBody);
    int expectSize = 4;
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
    database->QueryStaticOperatorList(requestParams, columnAttr, responseBody);
    int expectSize = 4;
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
    database->QueryStaticOperatorsTotalNum(requestParams, number);
    int expectNumber = 4;
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
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    int64_t number;
    database->QueryStaticOperatorsTotalNum(requestParams, number);
    int expectNumber = 16;
    EXPECT_EQ(number, expectNumber);
}

TEST_F(TestSuit, QueryStaticOperatorGraph)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    Dic::Protocol::StaticOperatorGraphParams requestParams;
    requestParams.rankId = "1";
    requestParams.graphId = "0";
    StaticOperatorGraphItem data;
    database->QueryStaticOperatorGraph(requestParams, data);
    int expectSize = 3;
    int expectColumnSize = 24;
    int pickedIndex = 5;
    string exceptPickedData = "4.35";
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
    database->QueryStaticOperatorGraph(requestParams, data);
    int expectColumnSize = 26;
    string exceptPickedData = "4.48";
    int pickedIndex = 19;
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
    database->QueryOperatorDetail(requestParams, columnAttr, responseBody);
    int expectSize = 10;
    int expectColumnSize = 9;
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
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::MemoryOperator> responseBody;
    database->QueryOperatorDetail(requestParams, columnAttr, responseBody);
    int expectSize = 0;
    int expectColumnSize = 9;
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
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    std::vector<Protocol::MemoryTableColumnAttr> columnAttr;
    std::vector<Dic::Protocol::MemoryOperator> responseBody;
    database->QueryOperatorDetail(requestParams, columnAttr, responseBody);
    int expectSize = 21;
    int expectColumnSize = 14;
    EXPECT_EQ(responseBody.size(), expectSize);
    EXPECT_EQ(columnAttr.size(), expectColumnSize);
}

TEST_F(TestSuit, QueryOperatorsTotalNum)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.searchName = "cann::Graph_";
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    requestParams.startTime = -1;
    requestParams.endTime = -1;
    int64_t totalNum;
    database->QueryOperatorsTotalNum(requestParams, totalNum);
    int expectSize = 28;
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
    database->QueryOperatorsTotalNum(requestParams, totalNum);
    int expectSize = 15;
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(TestSuit, QueryOperatorsTotalNumWithTime)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    requestParams.searchName = "cann::Graph_";
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    requestParams.startTime = 0;
    requestParams.endTime = 1695120000000; // end time = 1695120000000
    int64_t totalNum;
    database->QueryOperatorsTotalNum(requestParams, totalNum);
    int expectSize = 28;
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(TestSuit, QueryOperatorsTotalNumByStreamExpectZero)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    requestParams.startTime = -1;
    requestParams.endTime = -1; // end time = 1695120000000
    int64_t totalNum;
    database->QueryOperatorsTotalNum(requestParams, totalNum);
    int expectSize = 0;
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(TestSuit, QueryOperatorsTotalNumByStreamExpectSeveral)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    Dic::Protocol::MemoryOperatorParams requestParams;
    requestParams.rankId = "1";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    requestParams.minSize = -1;
    requestParams.maxSize = -1;
    requestParams.startTime = -1;
    requestParams.endTime = -1; // end time = 1695120000000
    int64_t totalNum;
    database->QueryOperatorsTotalNum(requestParams, totalNum);
    int expectSize = 21;
    EXPECT_EQ(totalNum, expectSize);
}

TEST_F(TestSuit, QueryMemoryViewData)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryComponentParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_OVERALL_GROUP;
    Dic::Protocol::MemoryViewData responseBody;
    database->QueryMemoryView(requestParams, responseBody);
    int expectSize = 5;
    EXPECT_EQ(responseBody.lines.size(), expectSize);
}

TEST_F(TestSuit, QueryMemoryViewDataByStreamExpectZero)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    Dic::Protocol::MemoryComponentParams requestParams;
    requestParams.rankId = "0";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    Dic::Protocol::MemoryViewData responseBody;
    database->QueryMemoryView(requestParams, responseBody);
    int expectSize = 0;
    EXPECT_EQ(responseBody.lines.size(), expectSize);
}

TEST_F(TestSuit, QueryMemoryViewDataByStreamExpectSeveral)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("1");
    Dic::Protocol::MemoryComponentParams requestParams;
    requestParams.rankId = "1";
    requestParams.type = Protocol::MEMORY_STREAM_GROUP;
    Dic::Protocol::MemoryViewData responseBody;
    database->QueryMemoryView(requestParams, responseBody);
    int expectSize = 1;
    EXPECT_EQ(responseBody.lines.size(), expectSize);
}

TEST_F(TestSuit, QueryOperatorSizeData)
{
    auto database = DataBaseManager::Instance().GetMemoryDatabase("0");
    double min;
    double max;
    database->QueryOperatorSize(min, max, "");
    int expectMin = 64;
    int expectMax = 81984;
    EXPECT_EQ(min, expectMin);
    EXPECT_EQ(max, expectMax);
}