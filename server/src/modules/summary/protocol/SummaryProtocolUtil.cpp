/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "ServerLog.h"
#include "JsonUtil.h"
#include "ProtocolEnumUtil.h"
#include "SummaryProtocol.h"
#include "SummaryProtocolUtil.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Server;
#pragma region <<Response to json>>
template <typename RESPONSE> std::optional<json_t> ToResponseJson(const RESPONSE &response)
{
    ServerLog::Warn("ToResponseJson is not implemented. command:", response.command);
    return std::nullopt;
}

template <> std::optional<json_t> ToResponseJson<SummaryTopRankResponse>(const SummaryTopRankResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["stepList"] = json_t::array();
    for (const auto &item: response.body.stepList) {
        json["body"]["stepList"].emplace_back(item);
    }
    json["body"]["rankList"] = json_t::array();
    for (const auto &item: response.body.rankList) {
        json["body"]["rankList"].emplace_back(item);
    }
    json["body"]["rankCount"] = response.body.rankCount;
    json["body"]["dataSize"] = response.body.dataSize;
    json["body"]["collectStartTime"] = response.body.collectStartTime;
    json["body"]["filePath"] = response.body.filePath;
    json["body"]["collectDuration"] = response.body.collectDuration;
    json["body"]["stepNum"] = response.body.stepNum;
    json["body"]["summaryList"] = json_t::array();
    for (const SummaryDto& summaryDto : response.body.summaryList) {
        json_t summaryDtoJson = json_t::object();
        summaryDtoJson["rankId"] = summaryDto.rankId;
        summaryDtoJson["totalTime"] = summaryDto.totalTime;
        summaryDtoJson["computingTime"] = summaryDto.computingTime;
        summaryDtoJson["communicationOverLappedTime"] = summaryDto.communicationOverLappedTime;
        summaryDtoJson["communicationNotOverLappedTime"] = summaryDto.communicationNotOverLappedTime;
        summaryDtoJson["freeTime"] = summaryDto.freeTime;
        json["body"]["summaryList"].emplace_back(summaryDtoJson);
    }
    return json;
}

template <> std::optional<json_t> ToResponseJson<SummaryStatisticsResponse>(const SummaryStatisticsResponse &response)
{
    json_t json;
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json["body"]["summaryStatisticsItemList"] = json_t::array();
    for (const SummaryStatisticsItem &action : response.body.summaryStatisticsItemList) {
        json_t itemJson = json_t::object();
        itemJson["acceleratorCore"] = action.acceleratorCore;
        itemJson["overlapType"] = action.overlapType;
        itemJson["duration"] = action.duration;
        itemJson["utilization"] = action.utilization;
        json["body"]["summaryStatisticsItemList"].emplace_back(itemJson);
    }
    return json;
}

#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic