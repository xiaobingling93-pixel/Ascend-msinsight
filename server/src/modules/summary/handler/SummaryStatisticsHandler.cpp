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
        SetSummaryError(ErrorCode::PARAMS_ERROR);
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetTraceDatabaseByFileId(request.fileId);
    if (database == nullptr) {
        database = Timeline::DataBaseManager::Instance().GetTraceDatabaseInCluster(request.params.clusterPath, request.params.rankId);
        if (database == nullptr) {
            SetSummaryError(ErrorCode::PARAMS_ERROR);
            SendResponse(std::move(responsePtr), false);
            return false;
        }
    }
    if (!request.params.timeFlag.empty() && request.params.timeFlag.find("compute") != std::string::npos) {
        if (!database->QueryComputeStatisticsData(request.params, response.body)) {
            SetSummaryError(ErrorCode::QUERY_COMPUTE_STATISTICS_FAILED);
            SendResponse(std::move(responsePtr), false);
            return false;
        }
    } else {
        if (!database->QueryCommunicationStatisticsData(request.params, response.body)) {
            SetSummaryError(ErrorCode::QUERY_COMMUNICATION_STATISTICS_FAILED);
            SendResponse(std::move(responsePtr), false);
            return false;
        }
    }
    SendResponse(std::move(responsePtr), true);
    return true;
}
} // Summary
} // Module
} // Dic