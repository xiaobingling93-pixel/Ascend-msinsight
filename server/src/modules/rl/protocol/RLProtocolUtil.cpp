/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "pch.h"
#include "RLProtocol.h"
#include "RLProtocolUtil.h"

namespace Dic::Protocol {
using namespace Dic::Server;
using namespace rapidjson;
#pragma region <<Response to json>>
template <typename RESPONSE> std::optional<document_t> ToResponseJson(const RESPONSE &response)
{
    ServerLog::Warn("Function to response json is not implemented. command:", response.command);
    return std::nullopt;
}

template<> std::optional<document_t> ToResponseJson<RLPipelineResponse>(const RLPipelineResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "minTime", response.body.minTime, allocator);
    JsonUtil::AddMember(body, "maxTime", response.body.maxTime, allocator);
    json_t data(kArrayType);
    for (const auto &item : response.body.data) {
        json_t pipelineItem(kObjectType);
        JsonUtil::AddMember(pipelineItem, "dbPath", item.dbPath, allocator);
        JsonUtil::AddMember(pipelineItem, "rankId", item.rankId, allocator);
        json_t nodeList(kArrayType);
        for (const auto &node: item.lists) {
            json_t nodeJson(kObjectType);
            JsonUtil::AddMember(nodeJson, "nodeType", node.nodeType, allocator);
            JsonUtil::AddMember(nodeJson, "startTime", node.startTime, allocator);
            JsonUtil::AddMember(nodeJson, "duration", node.duration, allocator);
            JsonUtil::AddMember(nodeJson, "name", node.name, allocator);
            JsonUtil::AddMember(nodeJson, "stageType", node.stageType, allocator);
            nodeList.PushBack(nodeJson, allocator);
        }
        JsonUtil::AddMember(pipelineItem, "lists", nodeList, allocator);
        data.PushBack(pipelineItem, allocator);
    }
    JsonUtil::AddMember(body, "data", data, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::optional<document_t>{std::move(json)};
}
}