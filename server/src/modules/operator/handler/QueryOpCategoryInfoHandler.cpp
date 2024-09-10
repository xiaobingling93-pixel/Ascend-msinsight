/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "pch.h"
#include "DataBaseManager.h"
#include "OperatorProtocolRequest.h"
#include "OperatorProtocolResponse.h"
#include "WsSessionManager.h"
#include "QueryOpCategoryInfoHandler.h"

namespace Dic::Module::Operator {
    using namespace Dic::Server;

    void QueryOpCategoryInfoHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
    {
        OperatorCategoryInfoRequest &request = dynamic_cast<OperatorCategoryInfoRequest &>(*requestPtr);
        WsSession &session = *WsSessionManager::Instance().GetSession();
        std::unique_ptr<OperatorCategoryInfoResponse> responsePtr = std::make_unique<OperatorCategoryInfoResponse>();
        OperatorCategoryInfoResponse &response = *responsePtr;
        SetBaseResponse(request, response);
        if (!CheckRequestParam(request.params)) {
            ServerLog::Error("[Operator]Failed to check request parameter in Query Category Info.");
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return;
        }
        std::string rankId = Summary::VirtualSummaryDataBase::GetFileIdFromCombinationId(request.params.rankId);
        auto database = Timeline::DataBaseManager::Instance().GetSummaryDatabase(rankId);
        if (!database->QueryOperatorDurationInfo(request.params, QueryType::CATEGORY, response.datas)) {
            ServerLog::Error("[Operator]Failed to query Category Info by rankId.");
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return;
        }
        SetResponseResult(response, true);
        session.OnResponse(std::move(responsePtr));
    }

    bool QueryOpCategoryInfoHandler::CheckRequestParam(OperatorDurationReqParams params)
    {
        if (params.rankId.empty()) {
            ServerLog::Error("[Operator]Failed to check rankId in Query Category Info.");
            return false;
        }
        return true;
    }
}