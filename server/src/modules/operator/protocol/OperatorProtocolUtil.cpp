/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "pch.h"
#include "ProtocolUtil.h"
#include "OperatorProtocolUtil.h"

namespace Dic::Protocol {
using namespace rapidjson;
template<>
std::optional<document_t> ToResponseJson<OperatorCategoryInfoResponse>(const OperatorCategoryInfoResponse &res)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(res, json);
    json_t body(kObjectType);
    json_t data(kArrayType);
    for (const OperatorDurationRes& ele : res.datas) {
        json_t dataJson(kObjectType);
        JsonUtil::AddMember(dataJson, "name", ele.name, allocator);
        JsonUtil::AddMember(dataJson, "duration", ele.duration, allocator);
        data.PushBack(dataJson, allocator);
    }
    JsonUtil::AddMember(body, "data", data, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template<>
std::optional<document_t> ToResponseJson<OperatorComputeUnitInfoResponse>(const OperatorComputeUnitInfoResponse &res)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(res, json);
    json_t body(kObjectType);
    json_t data(kArrayType);
    for (const OperatorDurationRes& ele : res.datas) {
        json_t dataJson(kObjectType);
        JsonUtil::AddMember(dataJson, "name", ele.name, allocator);
        JsonUtil::AddMember(dataJson, "duration", ele.duration, allocator);
        data.PushBack(dataJson, allocator);
    }
    JsonUtil::AddMember(body, "data", data, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template<>
std::optional<document_t> ToResponseJson<OperatorStatisticInfoResponse>(const OperatorStatisticInfoResponse &res)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(res, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "total", res.total, allocator);
    json_t data(kArrayType);
    for (const OperatorStatisticInfoRes& ele : res.datas) {
        json_t dataJson(kObjectType);
        JsonUtil::AddMember(dataJson, "opType", ele.opType, allocator);
        JsonUtil::AddMember(dataJson, "opName", ele.opName, allocator);
        JsonUtil::AddMember(dataJson, "accCore", ele.accCore, allocator);
        JsonUtil::AddMember(dataJson, "inputShape", ele.inputShape, allocator);
        JsonUtil::AddMember(dataJson, "totalTime", ele.totalTime, allocator);
        JsonUtil::AddMember(dataJson, "count", ele.count, allocator);
        JsonUtil::AddMember(dataJson, "avgTime", ele.avgTime, allocator);
        JsonUtil::AddMember(dataJson, "maxTime", ele.maxTime, allocator);
        JsonUtil::AddMember(dataJson, "minTime", ele.minTime, allocator);
        data.PushBack(dataJson, allocator);
    }
    JsonUtil::AddMember(body, "data", data, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

void AdDetaildMemberWithLabel(rapidjson::Value& parent, const char* label, const OperatorDetailInfoRes& ele,
                              rapidjson::Document::AllocatorType& allocator)
{
    rapidjson::Value dataJson(rapidjson::kObjectType);
    JsonUtil::AddMember(dataJson, "name", ele.name, allocator);
    JsonUtil::AddMember(dataJson, "type", ele.type, allocator);
    JsonUtil::AddMember(dataJson, "accCore", ele.accCore, allocator);
    JsonUtil::AddMember(dataJson, "startTime", ele.startTime, allocator);
    JsonUtil::AddMember(dataJson, "duration", ele.duration, allocator);
    JsonUtil::AddMember(dataJson, "waitTime", ele.waitTime, allocator);
    JsonUtil::AddMember(dataJson, "blockDim", ele.blockDim, allocator);
    JsonUtil::AddMember(dataJson, "inputShape", ele.inputShape, allocator);
    JsonUtil::AddMember(dataJson, "inputType", ele.inputType, allocator);
    JsonUtil::AddMember(dataJson, "inputFormat", ele.inputFormat, allocator);
    JsonUtil::AddMember(dataJson, "outputShape", ele.outputShape, allocator);
    JsonUtil::AddMember(dataJson, "outputType", ele.outputType, allocator);
    JsonUtil::AddMember(dataJson, "outputFormat", ele.outputFormat, allocator);
    parent.AddMember(rapidjson::Value(label, allocator).Move(), dataJson, allocator);
}
 
template<>
std::optional<document_t> ToResponseJson<OperatorDetailInfoResponse>(const OperatorDetailInfoResponse &res)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(res, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "total", res.total, allocator);
    JsonUtil::AddMember(body, "level", res.level, allocator);
    rapidjson::Document data;
    data.SetArray();
    for (const OperatorDetailCmpInfoRes& eles : res.datas) {
        rapidjson::Value cmpResJson(rapidjson::kObjectType);
        AdDetaildMemberWithLabel(cmpResJson, "diff", eles.diff, allocator);
        AdDetaildMemberWithLabel(cmpResJson, "baseline", eles.baseline, allocator);
        AdDetaildMemberWithLabel(cmpResJson, "compare", eles.compare, allocator);
        data.PushBack(cmpResJson, allocator);
    }
    JsonUtil::AddMember(body, "data", data, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template<>
std::optional<document_t> ToResponseJson<OperatorMoreInfoResponse>(const OperatorMoreInfoResponse &res)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(res, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "total", res.total, allocator);
    JsonUtil::AddMember(body, "level", res.level, allocator);
    json_t data(kArrayType);
    for (const OperatorDetailInfoRes& ele : res.datas) {
        json_t dataJson(kObjectType);
        JsonUtil::AddMember(dataJson, "name", ele.name, allocator);
        JsonUtil::AddMember(dataJson, "type", ele.type, allocator);
        JsonUtil::AddMember(dataJson, "accCore", ele.accCore, allocator);
        JsonUtil::AddMember(dataJson, "startTime", ele.startTime, allocator);
        JsonUtil::AddMember(dataJson, "duration", ele.duration, allocator);
        JsonUtil::AddMember(dataJson, "waitTime", ele.waitTime, allocator);
        JsonUtil::AddMember(dataJson, "blockDim", ele.blockDim, allocator);
        JsonUtil::AddMember(dataJson, "inputShape", ele.inputShape, allocator);
        JsonUtil::AddMember(dataJson, "inputType", ele.inputType, allocator);
        JsonUtil::AddMember(dataJson, "inputFormat", ele.inputFormat, allocator);
        JsonUtil::AddMember(dataJson, "outputShape", ele.outputShape, allocator);
        JsonUtil::AddMember(dataJson, "outputType", ele.outputType, allocator);
        JsonUtil::AddMember(dataJson, "outputFormat", ele.outputFormat, allocator);
        data.PushBack(dataJson, allocator);
    }
    JsonUtil::AddMember(body, "data", data, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template<>
std::optional<document_t> ToEventJson<OperatorParseStatusEvent>(const OperatorParseStatusEvent &event)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetEventJsonBaseInfo(event, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "rankId", event.data.rankId, allocator);
    JsonUtil::AddMember(body, "status", event.data.status, allocator);
    JsonUtil::AddMember(body, "error", event.data.error, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::move(json);
}

template<>
std::optional<document_t> ToEventJson<OperatorParseClearEvent>(const OperatorParseClearEvent &event)
{
    document_t json(kObjectType);
    ProtocolUtil::SetEventJsonBaseInfo(event, json);
    return std::move(json);
}
}
