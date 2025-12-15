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
#include "BaselineManager.h"
#include "SetParallelStrategyConfigHandler.h"

namespace Dic::Module::Summary {
using namespace Dic::Server;
using namespace Dic::Module::Global;
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
    if (!request.params.CheckParams(errorMsg)) {
        SetSummaryError(ErrorCode::PARAMS_ERROR);
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(request.params.clusterPath);
    std::string level = PARALLEL_CONFIG_LEVEL_CONFIGURED;
    if (database == nullptr || !database->UpdateParallelStrategyConfig(request.params.config, level, response.msg)) {
        SetSummaryError(ErrorCode::UPDATE_PARALLEL_STRATEGY_FAILED);
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    std::string errMsg;
    if (!ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(
        database->GetDbPath(), request.params.config, errMsg)) {
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    // 如果存在baseline，则对baseline进行同样的设置
    auto baselineDatabase = Timeline::DataBaseManager::Instance().GetClusterDatabase(
        BaselineManager::Instance().GetBaseLineClusterPath());
    if (baselineDatabase != nullptr && !ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(
        baselineDatabase->GetDbPath(), request.params.config, errMsg)) {
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    session.OnResponse(std::move(responsePtr));
    return true;
}
}
