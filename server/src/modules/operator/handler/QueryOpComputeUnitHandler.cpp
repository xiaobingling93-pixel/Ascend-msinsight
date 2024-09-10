/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "pch.h"
#include "DataBaseManager.h"
#include "OperatorProtocolRequest.h"
#include "OperatorProtocolResponse.h"
#include "WsSessionManager.h"
#include "QueryOpComputeUnitHandler.h"

namespace Dic::Module::Operator {
    using namespace Dic::Server;

    void QueryOpComputeUnitHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
    {
        OperatorComputeUnitInfoRequest &request = dynamic_cast<OperatorComputeUnitInfoRequest &>(*requestPtr);
        WsSession &session = *WsSessionManager::Instance().GetSession();
        auto responsePtr = std::make_unique<OperatorComputeUnitInfoResponse>();
        OperatorComputeUnitInfoResponse &response = *responsePtr;
        SetBaseResponse(request, response);
        if (!CheckRequestParam(request.params)) {
            ServerLog::Error("[Operator]Failed to check request parameter in Query Compute Unit Info.");
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return;
        }
        std::string rankId = Summary::VirtualSummaryDataBase::GetFileIdFromCombinationId(request.params.rankId);
        auto database = Timeline::DataBaseManager::Instance().GetSummaryDatabase(rankId);
        if (!database->QueryOperatorDurationInfo(request.params, QueryType::COMPUTE_UNIT, response.datas)) {
            ServerLog::Error("[Operator]Failed to query Compute Unit Info by rankId.");
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return;
        }
        SetResponseResult(response, true);
        session.OnResponse(std::move(responsePtr));
    }

    bool QueryOpComputeUnitHandler::CheckRequestParam(OperatorDurationReqParams params)
    {
        if (params.rankId.empty()) {
            ServerLog::Error("[Operator]Failed to check rankId in Query Compute Unit Info.");
            return false;
        }
        return true;
    }
}