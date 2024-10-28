/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "SummaryStatisticsHandler.h"

namespace Dic {
namespace Module {
namespace Summary {
using namespace Dic;
using namespace Dic::Server;

bool SummaryStatisticsHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    Protocol::SummaryStatisticRequest &request =
            dynamic_cast<Protocol::SummaryStatisticRequest &>(*requestPtr.get());
    std::unique_ptr<Protocol::SummaryStatisticsResponse> responsePtr =
            std::make_unique<Protocol::SummaryStatisticsResponse>();
    SummaryStatisticsResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    // add response to response queue in session
    WsSession &session = *WsSessionManager::Instance().GetSession();
    if (request.params.rankId.empty()) {
        ServerLog::Info("rankId is empty,exit request handler");
        SetResponseResult(response, false, "rank Id is empty");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        database = Timeline::DataBaseManager::Instance().GetTraceDatabaseWithOutHost(request.params.rankId);
        if (database == nullptr) {
            ServerLog::Error("Failed to get connection for get summary statistics.");
            return false;
        }
    }
    if (!request.params.timeFlag.empty() && request.params.timeFlag.find("compute") != std::string::npos) {
        if (!database->QueryComputeStatisticsData(request.params, response.body)) {
            ServerLog::Warn("Query compute statistics data is failed");
            SetResponseResult(response, false, "Query compute statistics data failed");
            session.OnResponse(std::move(responsePtr));
            return false;
        }
    } else {
        if (!database->QueryCommunicationStatisticsData(request.params, response.body)) {
            ServerLog::Warn("Query communication statistics data is failed");
            SetResponseResult(response, false, "Query communication statistics data failed");
            session.OnResponse(std::move(responsePtr));
            return false;
        }
    }
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // Summary
} // Module
} // Dic