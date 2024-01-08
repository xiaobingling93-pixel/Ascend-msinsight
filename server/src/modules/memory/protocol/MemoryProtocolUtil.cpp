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
        operatorDetail.PushBack(basicJson, allocator);
    }
    JsonUtil::AddMember(body, "totalNum", response.totalNum, allocator);
    JsonUtil::AddMember(body, "operatorDetail", operatorDetail, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<MemoryViewResponse>(const MemoryViewResponse &response)
{
    document_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    auto &allocator = json.GetAllocator();
    json_t body(kObjectType);
    json_t linesList(kArrayType);
    for (const auto &lines: response.map.lines) {
        json_t lineArr(kArrayType);
        for (const auto &line: lines) {
            lineArr.PushBack(json_t().SetString(line.c_str(), allocator), allocator);
        }
        linesList.PushBack(lineArr, allocator);
    }
    JsonUtil::AddMember(body, "linesList", linesList, allocator);
    JsonUtil::AddMember(body, "hasApp", response.map.hasApp, allocator);
    JsonUtil::AddMember(body, "peakMemoryUsage", response.map.peakMemoryUsage, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template<>std::optional<document_t> ToResponseJson<MemoryOperatorSizeResponse>(const MemoryOperatorSizeResponse &response)
{
    document_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    body.AddMember("minSize", response.size.minSize, json.GetAllocator());
    body.AddMember("maxSize", response.size.maxSize, json.GetAllocator());
    json.AddMember("body", body, json.GetAllocator());
    return std::move(json);
}
#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic