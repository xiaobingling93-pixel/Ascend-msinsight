/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "DataBaseManager.h"
#include "OperatorProtocolRequest.h"
#include "OperatorProtocolResponse.h"
#include "WsSessionManager.h"
#include "ServerLog.h"
#include "QueryOpMoreInfoHandler.h"

namespace Dic::Module::Operator {
    using namespace Dic::Server;

    void QueryOpMoreInfoHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
    {
        OperatorMoreInfoRequest &request = dynamic_cast<OperatorMoreInfoRequest &>(*requestPtr);
        std::string token = request.token;
        if (!WsSessionManager::Instance().CheckSession(token)) {
            ServerLog::Error("[Operator]Failed to check session token of More Info, command = ", command);
            return;
        }
        WsSession &session = *WsSessionManager::Instance().GetSession(token);
        std::unique_ptr<OperatorMoreInfoResponse> responsePtr = std::make_unique<OperatorMoreInfoResponse>();
        OperatorMoreInfoResponse &response = *responsePtr;
        SetBaseResponse(request, response);
        std::string rankId = request.params.rankId;
        auto database = Timeline::DataBaseManager::Instance().GetSummaryDatabase(rankId);
        if (!database->QueryOperatorMoreInfo(request.params, response)) {
            ServerLog::Error("[Operator]Failed to query More Info, RankId = ", rankId);
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return;
        }
        SetResponseResult(response, true);
        session.OnResponse(std::move(responsePtr));
    }

}