/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "SummaryStatisticsHandler.h"
#include "ServerLog.h"
#include "TraceFileParser.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Module {
namespace Summary {
using namespace Dic;
using namespace Dic::Server;

void SummaryStatisticsHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    ServerLog::Info("request to summary summaryStatisticHandler");
    Protocol::SummaryStatisticRequest &request =
            dynamic_cast<Protocol::SummaryStatisticRequest &>(*requestPtr.get());
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Error("Failed to check session token  , command = ", command);
        return;
    }
    std::unique_ptr<Protocol::SummaryStatisticsResponse> responsePtr =
            std::make_unique<Protocol::SummaryStatisticsResponse>();
    SummaryStatisticsResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    // add response to response queue in session
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    auto database = Timeline::DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (!request.params.timeFlag.empty() && request.params.timeFlag.find_last_of("compute") > 0) {
        if (!database->QueryComputeStatisticsData(request.params, response.body)) {
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return;
        }
    } else {
        if (!database->QueryCommunicationStatisticsData(request.params, response.body)) {
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return;
        }
    }
    session.OnResponse(std::move(responsePtr));
}
} // Summary
} // Module
} // Dic