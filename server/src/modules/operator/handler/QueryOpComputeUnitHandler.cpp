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

    bool QueryOpComputeUnitHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
    {
        OperatorComputeUnitInfoRequest &request = dynamic_cast<OperatorComputeUnitInfoRequest &>(*requestPtr);
        WsSession &session = *WsSessionManager::Instance().GetSession();
        auto responsePtr = std::make_unique<OperatorComputeUnitInfoResponse>();
        OperatorComputeUnitInfoResponse &response = *responsePtr;
        SetBaseResponse(request, response);
        if (!CheckRequestParam(request.params)) {
            ServerLog::Warn("[Operator]Failed to check request parameter in Query Compute Unit Info.");
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return false;
        }
        std::string rankId = Summary::VirtualSummaryDataBase::GetFileIdFromCombinationId(request.params.rankId);
        auto database = Timeline::DataBaseManager::Instance().GetSummaryDatabaseByRankId(rankId);
        std::string deviceId = Timeline::DataBaseManager::Instance().GetDeviceIdFromRankId(rankId);
        if (deviceId.empty()) {
            ServerLog::Error("[Operator]Failed to query Compute Unit Info by empty deviceId.");
            SetOperatorError(ErrorCode::GET_DEVICE_ID_FAILED);
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return false;
        }
        request.params.deviceId = deviceId;
        if (!database
            || !database->QueryOperatorDurationInfo(request.params, QueryType::COMPUTE_UNIT, response.datas)) {
            ServerLog::Error("[Operator]Failed to query Compute Unit Info by rankId.");
            SetOperatorError(ErrorCode::QUERY_DURATION_FAILED);
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return false;
        }
        SetResponseResult(response, true);
        session.OnResponse(std::move(responsePtr));
        return true;
    }

    bool QueryOpComputeUnitHandler::CheckRequestParam(OperatorDurationReqParams params)
    {
        std::string errMsg;
        if (!params.CommonCheck(errMsg)) {
            ServerLog::Warn(errMsg);
            SetOperatorError(ErrorCode::PARAMS_ERROR);
            return false;
        }
        return true;
    }
}