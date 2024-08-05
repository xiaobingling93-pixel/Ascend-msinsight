/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 */

#include "pch.h"
#include "SourceProtocol.h"
#include "SourceProtocolUtil.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Server;
using namespace rapidjson;
#pragma region <<Response to json>>

template <typename RESPONSE> std::optional<document_t> ToResponseJson(const RESPONSE &response)
{
    ServerLog::Warn("Function to response json is not implemented. command:", response.command);
    return std::nullopt;
}

template<> std::optional<document_t> ToResponseJson<SourceCodeFileResponse>(const SourceCodeFileResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "fileContent", response.body.fileContent, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template<> std::optional<document_t> ToResponseJson<SourceApiLineResponse>(const SourceApiLineResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);

    json_t lines(kArrayType);
    for (auto lineRes: response.body.lines) {
        json_t line(kObjectType);
        JsonUtil::AddMember(line, "Line", lineRes.line, allocator);
        JsonUtil::AddMember(line, "Instruction Executed", lineRes.instructionExecuted, allocator);
        JsonUtil::AddMember(line, "Cycle", lineRes.cycle, allocator);
        json_t ranges(kArrayType);
        for (auto pair: lineRes.addressRange) {
            json_t range(kArrayType);
            range.PushBack(json_t().SetString(pair.first.c_str(), allocator), allocator);
            range.PushBack(json_t().SetString(pair.second.c_str(), allocator), allocator);
            ranges.PushBack(range, allocator);
        }
        JsonUtil::AddMember(line, "Address Range", ranges, allocator);
        lines.PushBack(line, allocator);
    }
    JsonUtil::AddMember(body, "lines", lines, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template<> std::optional<document_t> ToResponseJson<SourceApiInstrResponse>(const SourceApiInstrResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "instructions", response.body.instructions, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template<> std::optional<document_t> ToResponseJson<DetailsBaseInfoResponse>(const DetailsBaseInfoResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "name", response.body.name, allocator);
    JsonUtil::AddMember(body, "soc", response.body.soc, allocator);
    JsonUtil::AddMember(body, "opType", response.body.opType, allocator);
    JsonUtil::AddMember(body, "blockDim", response.body.blockDim, allocator);
    JsonUtil::AddMember(body, "mixBlockDim", response.body.mixBlockDim, allocator);
    JsonUtil::AddMember(body, "duration", response.body.duration, allocator);
    JsonUtil::AddMember(body, "deviceId", response.body.deviceId, allocator);
    JsonUtil::AddMember(body, "pid", response.body.pid, allocator);

    json_t blockDetail(kObjectType);
    JsonUtil::AddMember(blockDetail, "headerName", response.body.blockDetail.headerName, allocator);
    JsonUtil::AddMember(blockDetail, "size", response.body.blockDetail.size, allocator);
    json_t rowJson(kArrayType);
    for (const auto &rowItem: response.body.blockDetail.row) {
        json_t oneRow(kObjectType);
        JsonUtil::AddMember(oneRow, "value", rowItem.value, allocator);
        rowJson.PushBack(oneRow, allocator);
    }
    JsonUtil::AddMember(blockDetail, "row", rowJson, allocator);
    JsonUtil::AddMember(body, "blockDetail", blockDetail, allocator);
    json_t advice(kArrayType);
    for (const auto &item: response.body.advice) {
        advice.PushBack(json_t().SetString(item.c_str(), allocator), allocator);
    }
    JsonUtil::AddMember(body, "advice", advice, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template<> std::optional<document_t> ToResponseJson<DetailsLoadInfoResponse>(const DetailsLoadInfoResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t blockIdList(kArrayType);
    for (const auto &item: response.body.blockIdList) {
        blockIdList.PushBack(json_t().SetString(item.c_str(), allocator), allocator);
    }
    JsonUtil::AddMember(body, "blockIdList", blockIdList, allocator);
    json_t chartData(kObjectType);
    json_t detailDataList(kArrayType);
    for (const auto &item: response.body.chartData.detailDataList) {
        json_t unitData(kObjectType);
        JsonUtil::AddMember(unitData, "blockId", item.blockId, allocator);
        JsonUtil::AddMember(unitData, "blockType", item.blockType, allocator);
        JsonUtil::AddMember(unitData, "name", item.name, allocator);
        JsonUtil::AddMember(unitData, "unit", item.unit, allocator);
        JsonUtil::AddMember(unitData, "value", item.value, allocator);
        JsonUtil::AddMember(unitData, "originValue", item.originValue, allocator);
        detailDataList.PushBack(unitData, allocator);
    }
    JsonUtil::AddMember(chartData, "detailDataList", detailDataList, allocator);
    json_t advice(kArrayType);
    for (const auto &item: response.body.chartData.advice) {
        advice.PushBack(json_t().SetString(item.c_str(), allocator), allocator);
    }
    JsonUtil::AddMember(chartData, "advice", advice, allocator);
    JsonUtil::AddMember(body, "chartData", chartData, allocator);
    json_t tableData(kObjectType);
    json_t tableDetailDataList(kArrayType);
    for (const auto &item: response.body.tableData.detailDataList) {
        json_t unitData(kObjectType);
        JsonUtil::AddMember(unitData, "blockId", item.blockId, allocator);
        JsonUtil::AddMember(unitData, "blockType", item.blockType, allocator);
        JsonUtil::AddMember(unitData, "name", item.name, allocator);
        JsonUtil::AddMember(unitData, "unit", item.unit, allocator);
        JsonUtil::AddMember(unitData, "value", item.value, allocator);
        tableDetailDataList.PushBack(unitData, allocator);
    }
    JsonUtil::AddMember(tableData, "detailDataList", tableDetailDataList, allocator);
    json_t tableAdvice(kArrayType);
    for (const auto &item: response.body.tableData.advice) {
        tableAdvice.PushBack(json_t().SetString(item.c_str(), allocator), allocator);
    }
    JsonUtil::AddMember(tableData, "advice", tableAdvice, allocator);
    JsonUtil::AddMember(body, "tableData", tableData, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template<>
std::optional<document_t> ToResponseJson<DetailsMemoryGraphResponse>(const DetailsMemoryGraphResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t coreMemory(kArrayType);
    for (const auto &item: response.body.coreMemory) {
        json_t singleCoreMemory(kObjectType);
        JsonUtil::AddMember(singleCoreMemory, "advice", item.advice, allocator);
        JsonUtil::AddMember(singleCoreMemory, "blockId", item.blockId, allocator);
        json_t l2CacheJson(kObjectType);
        JsonUtil::AddMember(l2CacheJson, "hitRatio", item.l2Cache.hitRatio, allocator);
        JsonUtil::AddMember(l2CacheJson, "hit", item.l2Cache.hit, allocator);
        JsonUtil::AddMember(l2CacheJson, "totalRequest", item.l2Cache.totalRequest, allocator);
        JsonUtil::AddMember(l2CacheJson, "miss", item.l2Cache.miss, allocator);
        JsonUtil::AddMember(singleCoreMemory, "l2Cache", l2CacheJson, allocator);
        JsonUtil::AddMember(singleCoreMemory, "blockType", item.blockType, allocator);
        JsonUtil::AddMember(singleCoreMemory, "chipType", item.chipType, allocator);
        json_t memoryUnitJson(kArrayType);
        for (const auto &unit: item.memoryUnit) {
            json_t unitJson(kObjectType);
            JsonUtil::AddMember(unitJson, "request", unit.request, allocator);
            JsonUtil::AddMember(unitJson, "display", unit.display, allocator);
            JsonUtil::AddMember(unitJson, "peakRatio", unit.peakRatio, allocator);
            JsonUtil::AddMember(unitJson, "bandwidth", unit.bandwidth, allocator);
            JsonUtil::AddMember(unitJson, "memoryPath", unit.memoryPath, allocator);
            memoryUnitJson.PushBack(unitJson, allocator);
        }
        JsonUtil::AddMember(singleCoreMemory, "memoryUnit", memoryUnitJson, allocator);
        json_t vector(kObjectType);
        JsonUtil::AddMember(vector, "cycle", item.vector.cycle, allocator);
        JsonUtil::AddMember(vector, "totalCycles", item.vector.totalCycles, allocator);
        JsonUtil::AddMember(vector, "ratio", item.vector.ratio, allocator);
        JsonUtil::AddMember(singleCoreMemory, "vector", vector, allocator);
        json_t vector1(kObjectType);
        JsonUtil::AddMember(vector1, "cycle", item.vector1.cycle, allocator);
        JsonUtil::AddMember(vector1, "totalCycles", item.vector1.totalCycles, allocator);
        JsonUtil::AddMember(vector1, "ratio", item.vector1.ratio, allocator);
        JsonUtil::AddMember(singleCoreMemory, "vector1", vector1, allocator);
        json_t cube(kObjectType);
        JsonUtil::AddMember(cube, "cycle", item.cube.cycle, allocator);
        JsonUtil::AddMember(cube, "totalCycles", item.cube.totalCycles, allocator);
        JsonUtil::AddMember(cube, "ratio", item.cube.ratio, allocator);
        JsonUtil::AddMember(singleCoreMemory, "cube", cube, allocator);
        coreMemory.PushBack(singleCoreMemory, allocator);
    }
    JsonUtil::AddMember(body, "coreMemory", coreMemory, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template<>
std::optional<document_t> ToResponseJson<DetailsMemoryTableResponse>(const DetailsMemoryTableResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t memoryTable(kArrayType);
    for (const auto &item: response.body.memoryTable) {
        json_t singleMemoryTable(kObjectType);
        JsonUtil::AddMember(singleMemoryTable, "advice", item.advice, allocator);
        JsonUtil::AddMember(singleMemoryTable, "blockId", item.blockId, allocator);
        JsonUtil::AddMember(singleMemoryTable, "tableOpType", item.tableOpType, allocator);
        json_t tableDetailListJson(kArrayType);
        for (const auto &tableDetailItem: item.tableDetail) {
            json_t tableDetailJson(kObjectType);
            JsonUtil::AddMember(tableDetailJson, "headerName", tableDetailItem.headerName, allocator);
            JsonUtil::AddMember(tableDetailJson, "tableName", tableDetailItem.tableName, allocator);
            JsonUtil::AddMember(tableDetailJson, "size", tableDetailItem.size, allocator);
            json_t rowJson(kArrayType);
            for (const auto &rowItem: tableDetailItem.row) {
                json_t oneRow(kObjectType);
                JsonUtil::AddMember(oneRow, "name", rowItem.name, allocator);
                JsonUtil::AddMember(oneRow, "value", rowItem.value, allocator);
                rowJson.PushBack(oneRow, allocator);
            }
            JsonUtil::AddMember(tableDetailJson, "row", rowJson, allocator);
            tableDetailListJson.PushBack(tableDetailJson, allocator);
        }
        JsonUtil::AddMember(singleMemoryTable, "tableDetail", tableDetailListJson, allocator);
        memoryTable.PushBack(singleMemoryTable, allocator);
    }
    JsonUtil::AddMember(body, "memoryTable", memoryTable, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic