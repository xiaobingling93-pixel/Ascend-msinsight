/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ServerLog.h"
#include "JsonUtil.h"
#include "SummaryProtocolUtil.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "SummaryProtocol.h"

namespace Dic {
namespace Protocol {
void SummaryProtocol::RegisterJsonToRequestFuncs()
{
    jsonToReqFactory.emplace(REQ_RES_SUMMARY_QUERY_TOP_DATA, ToTopNRequest);
    jsonToReqFactory.emplace(REQ_RES_SUMMARY_STATISTIC, ToStatisticsRequest);
}

void SummaryProtocol::RegisterResponseToJsonFuncs()
{
    resToJsonFactory.emplace(REQ_RES_SUMMARY_QUERY_TOP_DATA, ToTopNResponse);
    resToJsonFactory.emplace(REQ_RES_SUMMARY_STATISTIC, ToStatisticsResponse);
}

void SummaryProtocol::RegisterEventToJsonFuncs()
{
}

#pragma region <<Json To Request>>

std::unique_ptr<Request> SummaryProtocol::ToTopNRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<SummaryTopRankRequest> reqPtr = std::make_unique<SummaryTopRankRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyArrayValue(reqPtr->params.stepIdList, json["params"], "stepIdList");
    JsonUtil::SetByJsonKeyArrayValue(reqPtr->params.rankIdList, json["params"], "rankIdList");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.orderBy, json["params"], "orderBy");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.limit, json["params"], "limit");
    return reqPtr;
}

std::unique_ptr<Request> SummaryProtocol::ToStatisticsRequest(const json_t &json, std::string &error)
{
    std::unique_ptr<SummaryStatisticRequest> reqPtr = std::make_unique<SummaryStatisticRequest>();
    if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
        error = "Failed to set request base info, command is: " + reqPtr->command;
        return nullptr;
    }
    JsonUtil::SetByJsonKeyValue(reqPtr->params.rankId, json["params"], "rankId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.stepId, json["params"], "stepId");
    JsonUtil::SetByJsonKeyValue(reqPtr->params.timeFlag, json["params"], "timeFlag");
    return reqPtr;
}

#pragma endregion

#pragma region <<Json To Request>>

std::optional<json_t> SummaryProtocol::ToTopNResponse(const Response &response)
{
    return ToResponseJson<SummaryTopRankResponse>(dynamic_cast<const SummaryTopRankResponse &>(response));
}

std::optional<json_t> SummaryProtocol::ToStatisticsResponse(const Response &response)
{
    return ToResponseJson<SummaryStatisticsResponse>(dynamic_cast<const SummaryStatisticsResponse &>(response));
}

#pragma endregion
} // namespace Protocol
} // namespace Dic
