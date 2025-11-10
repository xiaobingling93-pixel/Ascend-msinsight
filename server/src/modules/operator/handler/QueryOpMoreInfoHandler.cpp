/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "pch.h"
#include "DataBaseManager.h"
#include "OperatorProtocolRequest.h"
#include "OperatorProtocolResponse.h"
#include "OperatorGroupConverter.h"
#include "WsSessionManager.h"
#include "OperatorProtocol.h"
#include "QueryOpMoreInfoHandler.h"

namespace Dic::Module::Operator {
    using namespace Dic::Server;

    bool QueryOpMoreInfoHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
    {
        OperatorMoreInfoRequest &request = dynamic_cast<OperatorMoreInfoRequest &>(*requestPtr);
        WsSession &session = *WsSessionManager::Instance().GetSession();
        std::unique_ptr<OperatorMoreInfoResponse> responsePtr = std::make_unique<OperatorMoreInfoResponse>();
        OperatorMoreInfoResponse &response = *responsePtr;
        SetBaseResponse(request, response);
        std::string errMsg;
        if (!request.params.CommonCheck(errMsg)) {
            ServerLog::Error("[Operator]Failed to check request parameter in query op more info.%", errMsg);
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return false;
        }
        std::string rankId = Summary::VirtualSummaryDataBase::GetFileIdFromCombinationId(request.params.rankId);
        auto database = Timeline::DataBaseManager::Instance().GetSummaryDatabaseByRankId(rankId);
        std::string deviceId = Timeline::DataBaseManager::Instance().GetDeviceIdFromRankId(rankId);
        if (deviceId.empty()) {
            ServerLog::Error("[Operator]Failed to query More Info by empty deviceId.%");
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return false;
        }
        request.params.deviceId = deviceId;
        if (!database || !database->QueryOperatorMoreInfo(request.params, response)) {
            ServerLog::Error("[Operator]Failed to query More Info by rankId.");
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return false;
        }
        SetResponseResult(response, true);
        session.OnResponse(std::move(responsePtr));
        return true;
    }
}