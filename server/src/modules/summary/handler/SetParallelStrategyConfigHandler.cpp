/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <memory>
#include "MegatronParallelStrategyAlgorithm.h"
#include "MindSpeedParallelStrategyAlgorithm.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "SummaryService.h"
#include "SetParallelStrategyConfigHandler.h"

namespace Dic::Module::Summary {
using namespace Dic::Server;
bool SetParallelStrategyConfigHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<SetParallelStrategyRequest &>(*requestPtr);
    std::unique_ptr<SetParallelStrategyResponse> responsePtr = std::make_unique<SetParallelStrategyResponse>();
    SetParallelStrategyResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    // check request parameters
    std::string errorMsg;
    if (!request.config.CheckParams(errorMsg) || !SummaryService::CheckParamForMindSpeed(request.config, errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    std::string level = PARALLEL_CONFIG_LEVEL_CONFIGURED;
    if (database == nullptr || !database->UpdateParallelStrategyConfig(request.config, level, response.msg)) {
        SendResponse(std::move(responsePtr), false, "Failed to update parallel strategy config.");
        return false;
    }
    std::string errMsg;
    if (!ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(
        database->GetDbPath(), request.config, errMsg)) {
        SendResponse(std::move(responsePtr), false, errMsg);
        return false;
    }
    // 如果存在baseline，则对baseline进行同样的设置
    auto baselineDatabase = Timeline::DataBaseManager::Instance().GetClusterDatabase(BASELINE);
    if (baselineDatabase != nullptr && !ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(
        baselineDatabase->GetDbPath(), request.config, errMsg)) {
        SendResponse(std::move(responsePtr), false, errMsg);
        return false;
    }
    session.OnResponse(std::move(responsePtr));
    return true;
}
}
