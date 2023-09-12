/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "SummaryTopRankHandler.h"
#include "ServerLog.h"
#include "TraceFileParser.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "WsSessionManager.h"
#include "TraceTime.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Module {
namespace Summary {
using namespace Dic;
using namespace Dic::Server;

void SummaryTopRankHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    ServerLog::Info("request to summaryHandler");
    SummaryTopRankRequest &request = dynamic_cast<SummaryTopRankRequest &>(*requestPtr.get());
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Error("Failed to check session token  , command = ", command);
        return;
    }
    std::unique_ptr<SummaryTopRankResponse> responsePtr = std::make_unique<SummaryTopRankResponse>();
    SummaryTopRankResponse &response = *responsePtr.get();
    std::vector<std::string> timeFlagVector = {"computingTime", "communicationNotOverLappedTime",
                                               "communicationOverLappedTime", "freeTime"};
    if (request.params.orderBy.empty() ||
        std::find(timeFlagVector.begin(), timeFlagVector.end(), request.params.orderBy) ==
        timeFlagVector.end()) {
        request.params.orderBy = "computingTime";
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    response.body.collectStartTime =
            Timeline::TraceTime::Instance().GetStartTime() / (numberThousands * numberThousands);
    response.body.collectDuration = Timeline::TraceTime::Instance().GetDuration() / numberThousands;
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase();
    if (!database->QuerySummaryData(request.params, response.body) ||
        !database->QueryBaseInfo(response.body)) {
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
} // Timeline
} // Module
} // Dic