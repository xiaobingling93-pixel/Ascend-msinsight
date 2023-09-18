/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "ServerLog.h"
#include "JsonUtil.h"
#include "ProtocolEnumUtil.h"
#include "TimelineProtocol.h"
#include "TimelineProtocolUtil.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Server;
#pragma region <<Response to json>>
template <typename RESPONSE> std::optional<json_t> ToResponseJson(const RESPONSE &response)
{
    ServerLog::Warn("ToResponseJson is not implemented. command:", response.command);
    return std::nullopt;
}

template <> std::optional<json_t> ToResponseJson<ImportActionResponse>(const ImportActionResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["isCluster"] = response.body.isCluster;
    json["body"]["reset"] = response.body.reset;
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

template <> std::optional<json_t> ToResponseJson<UnitThreadTracesResponse>(const UnitThreadTracesResponse &response)
{
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

template <> std::optional<json_t> ToResponseJson<UnitThreadsResponse>(const UnitThreadsResponse &response)
{
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

template <> std::optional<json_t> ToResponseJson<UnitThreadDetailResponse>(const UnitThreadDetailResponse &response)
{
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

template <> std::optional<json_t> ToResponseJson<UnitFlowNameResponse>(const UnitFlowNameResponse &response)
{
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
        json["body"]["flowDetail"].emplace_back(flowJson);
    }
    return json;
}

template <> std::optional<json_t> ToResponseJson<UnitFlowResponse>(const UnitFlowResponse &response)
{
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
    toJson["pid"] = response.body.to.pid;
    toJson["tid"] = response.body.to.tid;
    toJson["timestamp"] = response.body.to.timestamp;
    toJson["depth"] = response.body.to.depth;
    json["body"]["to"] = toJson;
    return json;
}

template <> std::optional<json_t> ToResponseJson<ResetWindowResponse>(const ResetWindowResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    return json;
}

template <> std::optional<json_t> ToResponseJson<UnitChartResponse>(const UnitChartResponse &response)
{
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

template <> std::optional<json_t> ToResponseJson<SearchCountResponse>(const SearchCountResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["totalCount"] = response.body.totalCount;
    for (const auto &data : response.body.countList) {
        json_t tmp;
        tmp["rankId"] = data.rankId;
        tmp["count"] = data.count;
        json["body"]["countList"].emplace_back(tmp);
    }
    return json;
}

template <> std::optional<json_t> ToResponseJson<SearchSliceResponse>(const SearchSliceResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["rankId"] = response.body.rankId;
    json["body"]["pid"] = response.body.pid;
    json["body"]["tid"] = response.body.tid;
    json["body"]["startTime"] = response.body.startTime;
    json["body"]["duration"] = response.body.duration;
    json["body"]["depth"] = response.body.depth;
    return json;
}
#pragma endregion

#pragma region <<Event to json>>
template <typename EVENT> std::optional<json_t> ToEventJson(const EVENT &event)
{
    return std::nullopt;
}

json_t UnitTrackToJson(const UnitTrack &unitTrack)
{
    json_t json;
    json["type"] = unitTrack.type;
    json["metadata"]["cardId"] = unitTrack.metaData.cardId;
    json["metadata"]["processId"] = unitTrack.metaData.processId;
    json["metadata"]["processName"] = unitTrack.metaData.processName;
    json["metadata"]["label"] = unitTrack.metaData.label;
    json["metadata"]["threadId"] = unitTrack.metaData.threadId;
    json["metadata"]["threadName"] = unitTrack.metaData.threadName;
    json["metadata"]["maxDepth"] = unitTrack.metaData.maxDepth;
    for (const auto &track : unitTrack.children) {
        json["children"].emplace_back(UnitTrackToJson(*track));
    }
    return json;
}

template <> std::optional<json_t> ToEventJson<ParseSuccessEvent>(const ParseSuccessEvent &event)
{
    json_t json;
    ProtocolUtil::SetEventJsonBaseInfo(event, json);
    json["body"]["maxTimeStamp"] = event.body.maxTimeStamp;
    json["body"]["startTimeUpdated"] = event.body.startTimeUpdated;
    json["body"]["unit"]["type"] = event.body.unit.type;
    json["body"]["unit"]["metadata"]["cardId"] = event.body.unit.metadata.cardId;
    for (const auto &track : event.body.unit.children) {
        json["body"]["unit"]["children"].emplace_back(UnitTrackToJson(*track));
    }
    return json;
}

#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic