/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "DataBaseManager.h"
#include "OperatorProtocolRequest.h"
#include "OperatorProtocolResponse.h"
#include "WsSessionManager.h"
#include "ServerLog.h"
#include "OperatorProtocol.h"
#include "QueryOpDetailInfoHandler.h"

namespace Dic::Module::Operator {
    using namespace Dic::Server;

    void QueryOpDetailInfoHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
    {
        OperatorDetailInfoRequest &request = dynamic_cast<OperatorDetailInfoRequest &>(*requestPtr);
        std::string token = request.token;
        if (!WsSessionManager::Instance().CheckSession(token)) {
            ServerLog::Error("[Operator]Failed to check session token of Detail Info, command = ", command);
            return;
        }
        WsSession &session = *WsSessionManager::Instance().GetSession(token);
        std::unique_ptr<OperatorDetailInfoResponse> responsePtr = std::make_unique<OperatorDetailInfoResponse>();
        OperatorDetailInfoResponse &response = *responsePtr;
        SetBaseResponse(request, response);
        std::string errorMsg;
        if (!request.params.CommonCheck(errorMsg)) {
            ServerLog::Error(errorMsg);
            ServerLog::Error("[Operator]Failed to check request parameter in query op detail info.");
            SetResponseResult(response, false, errorMsg);
            session.OnResponse(std::move(responsePtr));
            return;
        }
        std::string rankId = Summary::VirtualSummaryDataBase::GetFileIdFromCombinationId(request.params.rankId);
        auto database = Timeline::DataBaseManager::Instance().GetSummaryDatabase(rankId);
        if (!database->QueryOperatorDetailInfo(request.params, response)) {
            ServerLog::Error("[Operator]Failed to query Detail Info, RankId = ", rankId);
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return;
        }
        SetResponseResult(response, true);
        session.OnResponse(std::move(responsePtr));
    }
}