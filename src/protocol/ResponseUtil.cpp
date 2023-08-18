/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#include "ServerLog.h"
#include "JsonUtil.h"
#include "ProtocolEnumUtil.h"
#include "ProtocolUtil.h"
#include "ResponseUtil.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Server;
#pragma region << template_functions>>
template <typename RESPONSE> std::optional<json_t> ToResponseJson(const RESPONSE &response)
{
    ServerLog::Warn("ToResponseJson is not implemented. command:", response.command);
    return std::nullopt;
}

template <> std::optional<json_t> ToResponseJson<TokenCreateResponse>(const TokenCreateResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["createTime"] = response.body.createTime;
    if (response.body.parentToken.has_value() && !response.body.parentToken.value().empty()) {
        json["body"]["parentToken"] = response.body.parentToken.value();
    }
    return json;
}

template <> std::optional<json_t> ToResponseJson<TokenDestroyResponse>(const TokenDestroyResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["destroyTime"] = response.body.destroyTime;
    json["body"]["destroyToken"] = response.body.destroyToken;
    return json;
}

template <> std::optional<json_t> ToResponseJson<TokenCheckResponse>(const TokenCheckResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["checkedToken"] = response.body.checkedToken;
    json["body"]["deadTime"] = response.body.deadTime;
    json["body"]["createTime"] = response.body.createTime;
    return json;
}

template <> std::optional<json_t> ToResponseJson<ConfigGetResponse>(const ConfigGetResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    if (response.body.globalConfig.has_value()) {
        json["body"]["config"]["global"] = json_t::object();
        ProtocolUtil::SetGlobalConfigJson(response.body.globalConfig.value(), json["body"]["config"]["global"]);
    }
    if (response.body.harmonyConfig.has_value()) {
        json["body"]["config"]["harmony"] = json_t::object();
        ProtocolUtil::SetHarmonyConfigJson(response.body.harmonyConfig.value(), json["body"]["config"]["harmony"]);
    }
    return json;
}

template <> std::optional<json_t> ToResponseJson<ConfigSetResponse>(const ConfigSetResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["configSetTime"] = response.body.configSetTime;
    json["body"]["isAlertMsg"] = response.body.isAlertMsg;
    return json;
}

template <> std::optional<json_t> ToResponseJson<ImportActionResponse>(const ImportActionResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["result"] = json_t::array();
    for (const Action& action : response.body.result) {
        json_t actionJson = json_t::object();
        actionJson["cardName"] = action.cardName;
        actionJson["rankId"] = action.rankId;
        actionJson["result"] = action.result;
        json["body"]["result"].emplace_back(actionJson);
    }
    return json;
}

template <> std::optional<json_t> ToResponseJson<UnitThreadTracesResponse>(const UnitThreadTracesResponse &response) {
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["data"] = json_t::array();
    for (const std::vector<ThreadTraces>& array : response.body.data) {
        json_t threadTracesArray = json_t::array();
        for (const ThreadTraces& threadTraces : array) {
            json_t threadJson = json_t::object();
            threadJson["name"] = threadTraces.name;
            threadJson["duration"] = threadTraces.duration;
            threadJson["startTime"] = threadTraces.startTime;
            threadJson["endTime"] = threadTraces.endTime;
            threadJson["depth"] = threadTraces.depth;
            threadJson["threadId"] = threadTraces.threadId;
            threadTracesArray.emplace_back(threadJson);
        }
        json["body"]["data"].emplace_back(threadTracesArray);
    }
    return json;
}

template <> std::optional<json_t> ToResponseJson<UnitThreadsResponse>(const UnitThreadsResponse &response) {
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["emptyFlag"] = response.body.emptyFlag;
    json["body"]["data"] = json_t::array();
    for (const Threads& threads : response.body.data) {
        json_t threadsJson;
        threadsJson["title"] = threads.title;
        threadsJson["wallDuration"] = threads.wallDuration;
        threadsJson["occurrences"] = threads.occurrences;
        threadsJson["avgWallDuration"] = threads.avgWallDuration;
        threadsJson["selfTime"] = threads.selfTime;
        json["body"]["data"].emplace_back(threadsJson);
    }
    return json;
}

template <> std::optional<json_t> ToResponseJson<UnitThreadDetailResponse>(const UnitThreadDetailResponse &response) {
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["emptyFlag"] = response.body.emptyFlag;
    json["body"]["data"]["selfTime"] = response.body.data.selfTime;
    json["body"]["data"]["args"] = response.body.data.args;
    json["body"]["data"]["title"] = response.body.data.title;
    json["body"]["data"]["duration"] = response.body.data.duration;
    json["body"]["data"]["cat"] = response.body.data.cat;
    return json;
}

template <> std::optional<json_t> ToResponseJson<UnitFlowNameResponse>(const UnitFlowNameResponse &response) {
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["flowDetail"] = json_t::array();
    for (const FlowName& flowName : response.body.flowDetail) {
        json_t flowJson = json_t::object();
        flowJson["title"] = flowName.title;
        flowJson["tid"] = flowName.tid;
        flowJson["pid"] = flowName.pid;
        flowJson["timestamp"] = flowName.timestamp;
        flowJson["depth"] = flowName.depth;
        flowJson["flowId"] = flowName.flowId;
        json["body"]["data"].emplace_back(flowJson);
    }
    return json;
}

template <> std::optional<json_t> ToResponseJson<UnitFlowResponse>(const UnitFlowResponse &response) {
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["title"] = response.body.title;
    json["body"]["cat"] = response.body.cat;
    json["body"]["id"] = response.body.id;
    json_t fromJson = json_t::object();
    fromJson["pid"] = response.body.from.pid;
    fromJson["tid"] = response.body.from.tid;
    fromJson["timestamp"] = response.body.from.timestamp;
    fromJson["depth"] = response.body.from.depth;
    json["body"]["from"] = fromJson;
    json_t toJson = json_t::object();
    toJson["pid"] = response.body.from.pid;
    toJson["tid"] = response.body.from.tid;
    toJson["timestamp"] = response.body.from.timestamp;
    toJson["depth"] = response.body.from.depth;
    json["body"]["to"] = toJson;
    return json;
}

template <> std::optional<json_t> ToResponseJson<ResetWindowResponse>(const ResetWindowResponse &response) {
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    return json;
}

template <> std::optional<json_t> ToResponseJson<UnitChartResponse>(const UnitChartResponse &response) {
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["data"] = json_t::array();
    for (const Chart& chart: response.body.data) {
        json_t chartJson = json_t::object();
        chartJson["ts"] = chart.ts;
        chartJson["value"] = chart.value;
        json["body"]["data"].emplace_back(chartJson);
    }
    return json;
}
#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic