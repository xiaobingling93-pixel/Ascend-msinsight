/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "DataBaseManager.h"
#include "OperatorProtocolRequest.h"
#include "OperatorProtocolResponse.h"
#include "WsSessionManager.h"
#include "ServerLog.h"
#include "OperatorProtocol.h"
#include "QueryOpStatisticInfoHandler.h"

namespace Dic::Module::Operator {
    using namespace Dic::Server;

    void QueryOpStatisticInfoHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
    {
        OperatorStatisticInfoRequest &request = dynamic_cast<OperatorStatisticInfoRequest &>(*requestPtr);
        std::string token = request.token;
        if (!WsSessionManager::Instance().CheckSession(token)) {
            ServerLog::Error("[Operator]Failed to check session token of Statistic Info, command = ", command);
            return;
        }
        WsSession &session = *WsSessionManager::Instance().GetSession(token);
        std::unique_ptr<OperatorStatisticInfoResponse> responsePtr = std::make_unique<OperatorStatisticInfoResponse>();
        OperatorStatisticInfoResponse &response = *responsePtr;
        SetBaseResponse(request, response);
        if (!CheckRequestParam(request.params)) {
            ServerLog::Error("[Operator]Failed to check request parameter in QueryOpStatisticInfoHandler.");
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return;
        }
        std::string rankId = Summary::SummaryDataBase::GetFileIdFromCombinationId(request.params.rankId);
        auto database = Timeline::DataBaseManager::Instance().GetSummaryDatabase(rankId);
        if (!database->QueryOperatorStatisticInfo(request.params, response)) {
            ServerLog::Error("[Operator]Failed to query Statistic Info, RankId = ", rankId);
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return;
        }
        SetResponseResult(response, true);
        session.OnResponse(std::move(responsePtr));
    }

    bool QueryOpStatisticInfoHandler::CheckRequestParam(OperatorStatisticReqParams& params)
    {
        if (params.rankId.empty()) {
            ServerLog::Error("[Operator]Failed to check rankId in Query Op Statistic Info.");
            return false;
        }
        if (params.group != Protocol::OP_TYPE_GROUP && params.group != Protocol::INPUT_SHAPE_GROUP) {
            ServerLog::Error("[Operator]Wrong group type in Query Op Statistic Info.");
            return false;
        }
        if (!params.orderBy.empty()) {
            if (OperatorProtocol::GetStatisticColumName(params.orderBy).empty()) {
                ServerLog::Error("[Operator]Failed to check orderBy in Query Op Statistic Info.");
                return false;
            }
            params.orderBy = OperatorProtocol::GetStatisticColumName(params.orderBy);
        }

        return true;
    }

}