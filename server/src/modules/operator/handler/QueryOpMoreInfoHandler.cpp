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
        if (!CheckRequestParam(request.params)) {
            ServerLog::Error("[Operator]Failed to check request parameter in query op more info.");
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return false;
        }
        std::string rankId = Summary::VirtualSummaryDataBase::GetFileIdFromCombinationId(request.params.rankId);
        auto database = Timeline::DataBaseManager::Instance().GetSummaryDatabase(rankId);
        if (!database->QueryOperatorMoreInfo(request.params, response)) {
            ServerLog::Error("[Operator]Failed to query More Info by rankId.");
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return false;
        }
        SetResponseResult(response, true);
        session.OnResponse(std::move(responsePtr));
        return true;
    }

    bool QueryOpMoreInfoHandler::CheckRequestParam(OperatorMoreInfoReqParams& params)
    {
        std::string errMsg;
        if (!CheckStrParamValid(params.rankId, errMsg)) {
            ServerLog::Error(std::string("[Operator]Failed to check rankId in query op more info.") + errMsg);
            return false;
        }
        if (!CheckStrParamValid(params.opName, errMsg) && !CheckStrParamValid(params.opType, errMsg)) {
            ServerLog::Error(std::string("[Operator]Failed to check name and type in query op more info.") + errMsg);
            return false;
        }
        OperatorGroupConverter::OperatorGroup operatorGroup = Protocol::OperatorGroupConverter::ToEnum(params.group);
        if (operatorGroup != OperatorGroupConverter::OperatorGroup::OP_TYPE_GROUP &&
            operatorGroup != OperatorGroupConverter::OperatorGroup::HCCL_TYPE_GROUP &&
            operatorGroup != OperatorGroupConverter::OperatorGroup::OP_INPUT_SHAPE_GROUP) {
            ServerLog::Error("[Operator]Wrong group type in query op more info.");
            return false;
        }
        if (!params.orderBy.empty()) {
            if (OperatorProtocol::GetDetailColumName(params.orderBy).empty()) {
                ServerLog::Error("[Operator]Failed to check orderBy in query op more info.");
                return false;
            }
            params.orderBy = OperatorProtocol::GetDetailColumName(params.orderBy);
        }

        return true;
    }

}