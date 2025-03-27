/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "SetCardAliasHandler.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"


namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
bool SetCardAliasHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    SetCardAliasRequest &request = dynamic_cast<SetCardAliasRequest &>(*requestPtr.get());
    std::unique_ptr<SetCardAliasResponse> responsePtr = std::make_unique<SetCardAliasResponse>();
    SetCardAliasResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    std::string errMsg;
    WsSession &session = *WsSessionManager::Instance().GetSession();
    if (!request.params.CheckParams(request.params.cardAlias, errMsg)) {
        SendResponse(std::move(responsePtr), false, errMsg);
        return false;
    }
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        SendResponse(std::move(responsePtr), false, "Set card alias failed to get connection.");
        return false;
    }

    if (!database->SetCardAlias(request.params, response.body)) {
        SendResponse(std::move(responsePtr), false, "Failed to set card alias.");
        return false;
    }
    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}

} // Timeline
} // Module
} // Dic