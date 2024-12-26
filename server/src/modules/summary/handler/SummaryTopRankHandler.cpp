/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "SummaryTopRankHandler.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "TraceTime.h"
#include "DataBaseManager.h"
#include "WsSessionManager.h"
#include "SummaryService.h"

namespace Dic {
namespace Module {
namespace Summary {
using namespace Dic;
using namespace Dic::Server;

bool SummaryTopRankHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    SummaryTopRankRequest &request = dynamic_cast<SummaryTopRankRequest &>(*requestPtr.get());
    std::unique_ptr<SummaryTopRankResponse> responsePtr = std::make_unique<SummaryTopRankResponse>();
    SummaryTopRankResponse &response = *responsePtr.get();
    std::vector<std::string> timeFlagVector = {"computingTime", "communicationNotOverLappedTime",
                                               "communicationOverLappedTime", "freeTime", "rankId"};
    if (request.params.orderBy.empty() ||
        std::find(timeFlagVector.begin(), timeFlagVector.end(), request.params.orderBy) ==
        timeFlagVector.end()) {
        request.params.orderBy = "computingTime";
    }
    WsSession &session = *WsSessionManager::Instance().GetSession();
    // check request parameters
    std::string errorMsg;
    if (!request.params.CheckParams(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }

    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    if (database == nullptr || !database->QuerySummaryData(request.params, response.body)) {
        ServerLog::Warn("Query summary data or query base info is failed");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    SummaryService::QueryCompareSummaryBaseInfo(request, response);
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // Timeline
} // Module
} // Dic