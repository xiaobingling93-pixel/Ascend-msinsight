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
    // check request parameters
    std::string errorMsg;
    if (!request.params.CheckParams(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetTraceDatabaseByRankId(request.params.rankId);
    if (database == nullptr) {
        database = Timeline::DataBaseManager::Instance().GetTraceDatabaseWithOutHost(request.params.rankId);
        if (database == nullptr) {
            SendResponse(std::move(responsePtr), false, "Failed to get connection for get summary statistics.");
            return false;
        }
    }
    if (!request.params.timeFlag.empty() && request.params.timeFlag.find("compute") != std::string::npos) {
        if (!database->QueryComputeStatisticsData(request.params, response.body)) {
            SendResponse(std::move(responsePtr), false, "Query compute statistics data is failed.");
            return false;
        }
    } else {
        if (!database->QueryCommunicationStatisticsData(request.params, response.body)) {
            SendResponse(std::move(responsePtr), false, "Query communication statistics data is failed.");
            return false;
        }
    }
    SendResponse(std::move(responsePtr), true);
    return true;
}
} // Summary
} // Module
} // Dic