/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ServerLog.h"
#include "WsSessionManager.h"
#include "TraceTime.h"
#include "DataBaseManager.h"
#include "QueryFlowCategoryEventsHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
void QueryFlowCategoryEventsHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    FlowCategoryEventsRequest &request = dynamic_cast<FlowCategoryEventsRequest &>(*requestPtr.get());
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<FlowCategoryEventsResponse> responsePtr = std::make_unique<FlowCategoryEventsResponse>();
    FlowCategoryEventsResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    bool result = false;
    std::vector<std::string> fileIdList;
    if (request.params.rankId.empty() || !DataBaseManager::Instance().GetDbPathByHost(request.params.rankId).empty()) {
        fileIdList = DataBaseManager::Instance().GetDbPathByHost(request.params.rankId);
    } else {
        fileIdList.emplace_back(request.params.rankId);
    }
    for (const auto &fileId : fileIdList) {
        auto database = DataBaseManager::Instance().GetTraceDatabase(fileId);
        if (database != nullptr) {
            result = database->QueryFlowCategoryEvents(request.params, TraceTime::Instance().GetStartTime(),
                                                       response.body.flowDetailList);
        }
        if (!result) {
            break;
        }
    }
    SetResponseResult(response, result);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
} // Timeline
} // Module
} // Dic