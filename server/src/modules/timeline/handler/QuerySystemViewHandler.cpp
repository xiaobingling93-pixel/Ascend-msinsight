/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "QuerySystemViewHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;

bool QuerySystemViewHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    SystemViewRequest &request = dynamic_cast<SystemViewRequest &>(*requestPtr.get());

    std::unique_ptr<SystemViewResponse> responsePtr = std::make_unique<SystemViewResponse>();
    WsSession &session = *WsSessionManager::Instance().GetSession();
    SystemViewResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    std::string warnMsg;
    if (!request.params.CheckParams(warnMsg)) {
        ServerLog::Warn(warnMsg);
        SetResponseResult(response, false, warnMsg);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    SetResponseResult(response, true);
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Query system view failed to get connection.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    if (!database->QuerySystemViewData(request.params, response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get timeline table response data.");
    }
    if (request.params.layer == "Python" && std::empty(response.body.systemViewDetail)) {
        request.params.layer = "MindSpore";
        if (!database->QuerySystemViewData(request.params, response.body)) {
            SetResponseResult(response, false);
            ServerLog::Error("Failed to get timeline table response data.");
            session.OnResponse(std::move(responsePtr));
            return false;
        }
    }
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    return true;
}

} // Timeline
} // Module
} // Dic