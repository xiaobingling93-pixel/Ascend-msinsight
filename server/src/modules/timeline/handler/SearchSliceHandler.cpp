/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "SearchSliceHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
void SearchSliceHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    SearchSliceRequest &request = dynamic_cast<SearchSliceRequest &>(*requestPtr.get());
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<SearchSliceResponse> responsePtr = std::make_unique<SearchSliceResponse>();
    SearchSliceResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);

    response.body.rankId = request.params.rankId;
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Can't find rankId. rankId:", request.params.rankId);
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    if (!database->SearchSliceName(request.params, request.params.index - 1,
                                   TraceTime::Instance().GetStartTime(), response.body)) {
        ServerLog::Error("Failed to search slice name. rankId:", request.params.rankId, ", searchContent:",
                         request.params.searchContent, ", index:", request.params.index);
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

} // Timeline
} // Module
} // Dic