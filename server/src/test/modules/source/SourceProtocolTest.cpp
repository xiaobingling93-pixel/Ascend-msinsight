/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <gtest/gtest.h>
#include "ProtocolManager.h"
#include "SourceProtocolResponse.h"
#include "SourceProtocolTest.h"

using namespace Dic::Protocol;

class SourceProtocolTest : public ::testing::Test {
protected:
    ProtocolManager *manager;
    std::string error;

protected:
    void SetUp() override
    {
        manager = &ProtocolManager::Instance();
    }
};

TEST_F(SourceProtocolTest, ToCodeFileRequest)
{
    const std::unique_ptr<Request> &ptr = manager->FromJson(TO_CODE_FILE_REQ_JSON, error);
    EXPECT_EQ(ptr->moduleName, ModuleType::SOURCE);
}

TEST_F(SourceProtocolTest, ToApiLineRequest)
{
    const std::unique_ptr<Request> &ptr = manager->FromJson(TO_API_LINE_REQ_JSON, error);
    EXPECT_EQ(ptr->moduleName, ModuleType::SOURCE);
}

TEST_F(SourceProtocolTest, ToApiInstrRequest)
{
    const std::unique_ptr<Request> &ptr = manager->FromJson(TO_API_INSTR_REQ_JSON, error);
    EXPECT_EQ(ptr->moduleName, ModuleType::SOURCE);
}

TEST_F(SourceProtocolTest, ToDetailsBaseInfoRequest)
{
    const std::unique_ptr<Request> &ptr = manager->FromJson(TO_BASE_INFO_REQ_JSON, error);
    EXPECT_EQ(ptr->moduleName, ModuleType::SOURCE);
}

TEST_F(SourceProtocolTest, ToDetailsLoadInfoRequest)
{
    const std::unique_ptr<Request> &ptr = manager->FromJson(TO_LOAD_INFO_REQ_JSON, error);
    EXPECT_EQ(ptr->moduleName, ModuleType::SOURCE);
}

TEST_F(SourceProtocolTest, ToDetailsMemoryGraphRequest)
{
    const std::unique_ptr<Request> &ptr = manager->FromJson(TO_MEMORY_GRAPH_REQ_JSON, error);
    EXPECT_EQ(ptr->moduleName, ModuleType::SOURCE);
}

TEST_F(SourceProtocolTest, ToDetailsMemoryTableRequest)
{
    const std::unique_ptr<Request> &ptr = manager->FromJson(TO_MEMORY_TABLE_REQ_JSON, error);
    EXPECT_EQ(ptr->moduleName, ModuleType::SOURCE);
}

TEST_F(SourceProtocolTest, ToDetailsInterCoreLoadGraphRequest)
{
    const std::unique_ptr<Request> &ptr = manager->FromJson(TO_INTER_CORE_LOAD_GRAPH_JSON, error);
    EXPECT_EQ(ptr->moduleName, ModuleType::SOURCE);
}

TEST_F(SourceProtocolTest, ToCodeFileResponse)
{
    SourceCodeFileResponse response;
    response.moduleName = ModuleType::SOURCE;
    manager->ToJson(response, error);
}

TEST_F(SourceProtocolTest, ToApiLineResponse)
{
    SourceApiLineResponse response;
    SourceFileLineRes lineRes;
    std::pair<std::string, std::string> pair = {"1", "10"};
    lineRes.addressRange.emplace_back(pair);
    response.body.lines.emplace_back(lineRes);
    response.moduleName = ModuleType::SOURCE;
    manager->ToJson(response, error);
}

TEST_F(SourceProtocolTest, ToApiInstrResponse)
{
    SourceApiInstrResponse response;
    response.moduleName = ModuleType::SOURCE;
    manager->ToJson(response, error);
}

TEST_F(SourceProtocolTest, ToDetailsBaseInfoResponse)
{
    DetailsBaseInfoResponse response;
    response.moduleName = ModuleType::SOURCE;
    manager->ToJson(response, error);
}

TEST_F(SourceProtocolTest, ToDetailsLoadInfoResponse)
{
    DetailsLoadInfoResponse response;
    SubBlockUnitData subBlockUnitData;
    CompareData<SubBlockUnitData> blockCompareData;
    blockCompareData.compare = subBlockUnitData;
    response.body.chartData.detailDataList.emplace_back(blockCompareData);
    response.body.tableData.detailDataList.emplace_back(blockCompareData);
    response.moduleName = ModuleType::SOURCE;
    manager->ToJson(response, error);
}

TEST_F(SourceProtocolTest, ToDetailsMemoryGraphResponse)
{
    DetailsMemoryGraphResponse response;
    MemoryGraph memoryGraph;
    MemoryUnit memoryUnit;
    CompareData<MemoryUnit> memoryCompareData;
    memoryCompareData.compare = memoryUnit;
    memoryGraph.memoryUnit.emplace_back(memoryCompareData);
    response.body.coreMemory.emplace_back(memoryGraph);
    response.moduleName = ModuleType::SOURCE;
    manager->ToJson(response, error);
}

TEST_F(SourceProtocolTest, ToDetailsMemoryTableResponse)
{
    DetailsMemoryTableResponse response;
    MemoryTable memoryTable;
    TableDetail<CompareData<TableRow>> tableDetail;
    CompareData<TableRow> tableRowCompare;
    TableRow tableRow;
    tableRowCompare.compare = tableRow;
    tableDetail.row.emplace_back(tableRowCompare);
    memoryTable.tableDetail.emplace_back(tableDetail);
    response.body.memoryTable.emplace_back(memoryTable);
    response.moduleName = ModuleType::SOURCE;
    manager->ToJson(response, error);
}

TEST_F(SourceProtocolTest, ToDetailsInterCoreLoadGraphResponse)
{
    DetailsInterCoreLoadGraphResponse response;
    DetailsInterCoreLoadOpDetail opDetail;
    DetailsInterCoreLoadSubCoreDetail subCoreDetail;
    float curRate = 11.0f;
    float maxRate = 23.0f;
    subCoreDetail.SetCacheHitRateDimension(curRate, maxRate);
    uint64_t curCycles = 100;
    uint64_t minCycles = 10;
    subCoreDetail.SetCyclesDimension(curCycles, minCycles);
    uint64_t curPut = 200;
    uint64_t minPut = 110;
    subCoreDetail.SetThroughputDimension(curPut, minPut);
    uint8_t subCoreIndex = 0;
    subCoreDetail.SetSubCoreName("cube", subCoreIndex);
    opDetail.AddSubCoreDetail(std::move(subCoreDetail));
    response.body.opDetails.emplace_back(opDetail);
    response.moduleName = ModuleType::SOURCE;
    manager->ToJson(response, error);
}