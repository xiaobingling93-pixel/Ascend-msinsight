/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "ProtocolEnumUtil.h"
#include "ProtocolUtil.h"
#include "JsonUtil.h"
#include "AdvisorProtocolToResponseJson.h"

namespace Dic::Protocol {
using namespace Dic::Server;
template <typename RESPONSE> std::optional<document_t> ToResponseJson(const RESPONSE &response)
{
    ServerLog::Warn("Invalid response parameters, type error. command:", response.command);
    return std::nullopt;
}

template
<> std::optional<document_t> ToResponseJson<AffinityOptimizerResponse>(const AffinityOptimizerResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "count", response.body.size, allocator);
    json_t dataList(kArrayType);
    for (auto item : response.body.datas) {
        json_t dataJson(kObjectType);
        JsonUtil::AddMember(dataJson, "rankId", item.baseInfo.rankId, allocator);
        JsonUtil::AddMember(dataJson, "startTime", item.baseInfo.startTime, allocator);
        JsonUtil::AddMember(dataJson, "duration", item.baseInfo.duration, allocator);
        JsonUtil::AddMember(dataJson, "pid", item.baseInfo.pid, allocator);
        JsonUtil::AddMember(dataJson, "tid", item.baseInfo.tid, allocator);
        JsonUtil::AddMember(dataJson, "depth", item.baseInfo.depth, allocator);
        JsonUtil::AddMember(dataJson, "originOptimizer", item.originOptimizer, allocator);
        JsonUtil::AddMember(dataJson, "replaceOptimizer", item.replaceOptimizer, allocator);
        dataList.PushBack(dataJson, allocator);
    }
    JsonUtil::AddMember(body, "data", dataList, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<AffinityAPIResponse>(const AffinityAPIResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "count", response.body.size, allocator);
    json_t dataList(kArrayType);
    for (auto item : response.body.datas) {
        json_t dataJson(kObjectType);
        JsonUtil::AddMember(dataJson, "rankId", item.baseInfo.rankId, allocator);
        JsonUtil::AddMember(dataJson, "startTime", item.baseInfo.startTime, allocator);
        JsonUtil::AddMember(dataJson, "duration", item.baseInfo.duration, allocator);
        JsonUtil::AddMember(dataJson, "pid", item.baseInfo.pid, allocator);
        JsonUtil::AddMember(dataJson, "tid", item.baseInfo.tid, allocator);
        JsonUtil::AddMember(dataJson, "depth", item.baseInfo.depth, allocator);
        JsonUtil::AddMember(dataJson, "name", item.name, allocator);
        JsonUtil::AddMember(dataJson, "originAPI", item.originAPI, allocator);
        JsonUtil::AddMember(dataJson, "replaceAPI", item.replaceAPI, allocator);
        JsonUtil::AddMember(dataJson, "note", item.note, allocator);
        dataList.PushBack(dataJson, allocator);
    }
    JsonUtil::AddMember(body, "data", dataList, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<OperatorFusionResponse>(const OperatorFusionResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "count", response.body.size, allocator);
    json_t dataList(kArrayType);
    for (auto item : response.body.datas) {
        json_t dataJson(kObjectType);
        JsonUtil::AddMember(dataJson, "rankId", item.baseInfo.rankId, allocator);
        JsonUtil::AddMember(dataJson, "startTime", item.baseInfo.startTime, allocator);
        JsonUtil::AddMember(dataJson, "duration", item.baseInfo.duration, allocator);
        JsonUtil::AddMember(dataJson, "pid", item.baseInfo.pid, allocator);
        JsonUtil::AddMember(dataJson, "tid", item.baseInfo.tid, allocator);
        JsonUtil::AddMember(dataJson, "depth", item.baseInfo.depth, allocator);
        JsonUtil::AddMember(dataJson, "name", item.name, allocator);
        JsonUtil::AddMember(dataJson, "originOpList", item.originOpList, allocator);
        JsonUtil::AddMember(dataJson, "fusedOp", item.fusedOp, allocator);
        JsonUtil::AddMember(dataJson, "note", item.note, allocator);
        dataList.PushBack(dataJson, allocator);
    }
    JsonUtil::AddMember(body, "data", dataList, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<AICpuOperatorResponse>(const AICpuOperatorResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "count", response.body.size, allocator);
    json_t dataList(kArrayType);
    for (auto item : response.body.datas) {
        json_t dataJson(kObjectType);
        JsonUtil::AddMember(dataJson, "rankId", item.baseInfo.rankId, allocator);
        JsonUtil::AddMember(dataJson, "startTime", item.baseInfo.startTime, allocator);
        JsonUtil::AddMember(dataJson, "duration", item.baseInfo.duration, allocator);
        JsonUtil::AddMember(dataJson, "pid", item.baseInfo.pid, allocator);
        JsonUtil::AddMember(dataJson, "tid", item.baseInfo.tid, allocator);
        JsonUtil::AddMember(dataJson, "depth", item.baseInfo.depth, allocator);
        JsonUtil::AddMember(dataJson, "name", item.opName, allocator);
        JsonUtil::AddMember(dataJson, "note", item.note, allocator);
        dataList.PushBack(dataJson, allocator);
    }
    JsonUtil::AddMember(body, "data", dataList, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template <> std::optional<document_t> ToResponseJson<AclnnOperatorResponse>(const AclnnOperatorResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "count", response.body.size, allocator);
    json_t dataList(kArrayType);
    for (auto item : response.body.datas) {
        json_t dataJson(kObjectType);
        JsonUtil::AddMember(dataJson, "rankId", item.baseInfo.rankId, allocator);
        JsonUtil::AddMember(dataJson, "startTime", item.baseInfo.startTime, allocator);
        JsonUtil::AddMember(dataJson, "duration", item.baseInfo.duration, allocator);
        JsonUtil::AddMember(dataJson, "pid", item.baseInfo.pid, allocator);
        JsonUtil::AddMember(dataJson, "tid", item.baseInfo.tid, allocator);
        JsonUtil::AddMember(dataJson, "depth", item.baseInfo.depth, allocator);
        JsonUtil::AddMember(dataJson, "name", item.opName, allocator);
        JsonUtil::AddMember(dataJson, "note", item.note, allocator);
        dataList.PushBack(dataJson, allocator);
    }
    JsonUtil::AddMember(body, "data", dataList, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

std::optional<document_t> AdvisorProtocolToResponseJson::ToAffinityOptimizerResponse(const Response &response)
{
    return ToResponseJson<AffinityOptimizerResponse>(dynamic_cast<const AffinityOptimizerResponse &>(response));
}

std::optional<document_t> AdvisorProtocolToResponseJson::ToAffinityAPIResponse(const Response &response)
{
    return ToResponseJson<AffinityAPIResponse>(dynamic_cast<const AffinityAPIResponse &>(response));
}

std::optional<document_t> AdvisorProtocolToResponseJson::ToOperatorFusionResponse(const Response &response)
{
    return ToResponseJson<OperatorFusionResponse>(dynamic_cast<const OperatorFusionResponse &>(response));
}

std::optional<document_t> AdvisorProtocolToResponseJson::ToAICpuOperatorResponse(const Response &response)
{
    return ToResponseJson<AICpuOperatorResponse>(dynamic_cast<const AICpuOperatorResponse &>(response));
}

std::optional<document_t> AdvisorProtocolToResponseJson::ToAclnnOperatorResponse(const Response &response)
{
    return ToResponseJson<AclnnOperatorResponse>(dynamic_cast<const AclnnOperatorResponse &>(response));
}
}
