/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "DataBaseManager.h"
#include "WsSessionManager.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "SetParallelStrategyConfigHandler.h"

namespace Dic::Module::Summary {

bool SetParallelStrategyConfigHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<SetParallelStrategyRequest &>(*requestPtr);
    std::unique_ptr<SetParallelStrategyResponse> responsePtr = std::make_unique<SetParallelStrategyResponse>();
    SetParallelStrategyResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    auto database = Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
    std::string level = PARALLEL_CONFIG_LEVEL_CONFIGURED;
    if (!database->UpdateParallelStrategyConfig(request.config, level, response.msg)) {
        response.result = false;
        SetResponseResult(response, false);
        ServerLog::Error("Failed to update parallel strategy config.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }

    session.OnResponse(std::move(responsePtr));
    return true;
}
}
