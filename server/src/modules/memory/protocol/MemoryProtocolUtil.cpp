/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "MemoryProtocolUtil.h"
#include "ProtocolUtil.h"
#include "JsonUtil.h"

namespace Dic {
namespace Protocol {
using namespace rapidjson;
#pragma region <<Response to json>>

template <> std::optional<document_t> ToResponseJson<MemoryOperatorResponse>(const MemoryOperatorResponse &response)
{
    document_t json(kObjectType);
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    auto &allocator = json.GetAllocator();
    json_t body(kObjectType);
    json_t columnAttr(kArrayType);
    bool hasStream;
    for (const auto &attr : response.columnAttr) {
        json_t attrJson = json_t(kObjectType);
        JsonUtil::AddMember(attrJson, "name", attr.name, allocator);
        JsonUtil::AddMember(attrJson, "type", attr.type, allocator);
        JsonUtil::AddMember(attrJson, "key", attr.key, allocator);
        columnAttr.PushBack(attrJson, allocator);
        if (attr.name == "Stream") {
            hasStream = true;
        }
    }
    json_t operatorDetail(kArrayType);
    for (const MemoryOperator& anOperator : response.operatorDetails) {
        json_t basicJson = json_t(kObjectType);
        if (anOperator.name.empty()) {
            JsonUtil::AddMember(basicJson, "name", "Unknown", allocator);
        } else {
            JsonUtil::AddMember(basicJson, "name", anOperator.name, allocator);
        }
        JsonUtil::AddMember(basicJson, "size", anOperator.size, allocator);
        JsonUtil::AddMember(basicJson, "allocationTime", anOperator.allocationTime, allocator);
        JsonUtil::AddMember(basicJson, "releaseTime", anOperator.releaseTime, allocator);
        JsonUtil::AddMember(basicJson, "duration", anOperator.duration, allocator);
        JsonUtil::AddMember(basicJson, "allocationAllocated", anOperator.allocationAllocated, allocator);
        JsonUtil::AddMember(basicJson, "allocationReserved", anOperator.allocationReserved, allocator);
        JsonUtil::AddMember(basicJson, "releaseAllocated", anOperator.releaseAllocated, allocator);
        JsonUtil::AddMember(basicJson, "releaseReserved", anOperator.releaseReserved, allocator);
        if (hasStream) {
            JsonUtil::AddMember(basicJson, "activeReleaseTime", anOperator.activeReleaseTime, allocator);
            JsonUtil::AddMember(basicJson, "activeDuration", anOperator.activeDuration, allocator);
            JsonUtil::AddMember(basicJson, "allocationActive", anOperator.allocationActive, allocator);
            JsonUtil::AddMember(basicJson, "releaseActive", anOperator.releaseActive, allocator);
            JsonUtil::AddMember(basicJson, "streamId", anOperator.streamId, allocator);
        }
        operatorDetail.PushBack(basicJson, allocator);
    }
    JsonUtil::AddMember(body, "totalNum", response.totalNum, allocator);
    JsonUtil::AddMember(body, "operatorDetail", operatorDetail, allocator);
    JsonUtil::AddMember(body, "columnAttr", columnAttr, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<MemoryViewResponse>(const MemoryViewResponse &response)
{
    document_t json(kObjectType);
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    auto &allocator = json.GetAllocator();
    json_t body(kObjectType);
    json_t legends(kArrayType);
    for (const auto& legend : response.data.legends) {
        legends.PushBack(json_t().SetString(legend.c_str(), allocator), allocator);
    }
    json_t linesList(kArrayType);
    for (const auto &lines: response.data.lines) {
        json_t lineArr(kArrayType);
        for (const auto &line: lines) {
            lineArr.PushBack(json_t().SetString(line.c_str(), allocator), allocator);
        }
        linesList.PushBack(lineArr, allocator);
    }
    JsonUtil::AddMember(body, "legends", legends, allocator);
    JsonUtil::AddMember(body, "lines", linesList, allocator);
    JsonUtil::AddMember(body, "title", response.data.title, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template<>
std::optional<document_t> ToResponseJson<MemoryOperatorSizeResponse>(const MemoryOperatorSizeResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "minSize", response.size.minSize, allocator);
    JsonUtil::AddMember(body, "maxSize", response.size.maxSize, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}


template<>
std::optional<document_t> ToResponseJson<MemoryTypeResponse>(const MemoryTypeResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t graphIdList(kArrayType);
    for (const auto &graphId: response.graphId) {
        graphIdList.PushBack(json_t().SetString(graphId.c_str(), allocator), allocator);
    }
    JsonUtil::AddMember(body, "type", response.type, allocator);
    JsonUtil::AddMember(body, "graphId", graphIdList, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template<>
std::optional<document_t> ToResponseJson<MemoryResourceTypeResponse>(const MemoryResourceTypeResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "type", response.type, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template<>
std::optional<document_t> ToResponseJson<MemoryStaticOperatorGraphResponse>
        (const MemoryStaticOperatorGraphResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t legends(kArrayType);
    for (const auto& legend : response.data.legends) {
        legends.PushBack(json_t().SetString(legend.c_str(), allocator), allocator);
    }
    json_t linesList(kArrayType);
    for (const auto &lines: response.data.lines) {
        json_t lineArr(kArrayType);
        for (const auto &line: lines) {
            lineArr.PushBack(json_t().SetString(line.c_str(), allocator), allocator);
        }
        linesList.PushBack(lineArr, allocator);
    }
    JsonUtil::AddMember(body, "legends", legends, allocator);
    JsonUtil::AddMember(body, "lines", linesList, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template<>
std::optional<document_t> ToResponseJson<MemoryStaticOperatorListResponse>
        (const MemoryStaticOperatorListResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t columnAttr(kArrayType);
    for (const auto &attr : response.columnAttr) {
        json_t attrJson = json_t(kObjectType);
        JsonUtil::AddMember(attrJson, "name", attr.name, allocator);
        JsonUtil::AddMember(attrJson, "type", attr.type, allocator);
        JsonUtil::AddMember(attrJson, "key", attr.key, allocator);
        columnAttr.PushBack(attrJson, allocator);
    }
    json_t operatorDetail(kArrayType);
    for (const StaticOperatorItem& anOperator : response.operatorDetails) {
        json_t basicJson = json_t(kObjectType);
        JsonUtil::AddMember(basicJson, "deviceId", anOperator.deviceId, allocator);
        JsonUtil::AddMember(basicJson, "opName", anOperator.opName, allocator);
        JsonUtil::AddMember(basicJson, "nodeIndexStart", anOperator.nodeIndexStart, allocator);
        JsonUtil::AddMember(basicJson, "nodeIndexEnd", anOperator.nodeIndexEnd, allocator);
        JsonUtil::AddMember(basicJson, "size", anOperator.size, allocator);
        operatorDetail.PushBack(basicJson, allocator);
    }
    JsonUtil::AddMember(body, "totalNum", response.totalNum, allocator);
    JsonUtil::AddMember(body, "staticOperatorListDetail", operatorDetail, allocator);
    JsonUtil::AddMember(body, "columnAttr", columnAttr, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}


#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic