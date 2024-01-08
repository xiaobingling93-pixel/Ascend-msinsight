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
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "totalNum", response.totalNum, allocator);
    json_t details(kArrayType);
    for (const MemoryOperator& item : response.operatorDetails) {
        json_t basicJson(kObjectType);
        JsonUtil::AddMember(basicJson, "name", item.name.empty() ? "Unknown" : item.name, allocator);
        JsonUtil::AddMember(basicJson, "size", item.size, allocator);
        JsonUtil::AddMember(basicJson, "allocationTime", item.allocationTime, allocator);
        JsonUtil::AddMember(basicJson, "releaseTime", item.releaseTime, allocator);
        JsonUtil::AddMember(basicJson, "duration", item.duration, allocator);
        details.PushBack(basicJson, allocator);
    }
    JsonUtil::AddMember(body, "operatorDetail", details, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<MemoryViewResponse>(const MemoryViewResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t lines(kArrayType);
//    for (const auto& item : response.map.lines) {
//        json_t lineJson(kObjectType);
//        lineJson.SetString(item.c)
//    }
//    JsonUtil::AddMember(body, "lines", response.map.lines, allocator);
    JsonUtil::AddMember(body, "hasApp", response.map.hasApp, allocator);
    JsonUtil::AddMember(body, "peakMemoryUsage", response.map.peakMemoryUsage, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template<>std::optional<document_t> ToResponseJson<MemoryOperatorSizeResponse>(const MemoryOperatorSizeResponse &response)
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
#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic