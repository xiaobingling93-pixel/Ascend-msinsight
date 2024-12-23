/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <memory>
#include "MegatronParallelStrategyAlgorithm.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
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
    if (!request.config.CheckParams(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    std::string level = PARALLEL_CONFIG_LEVEL_CONFIGURED;
    if (database == nullptr || !database->UpdateParallelStrategyConfig(request.config, level, response.msg)) {
        SendResponse(std::move(responsePtr), false, "Failed to update parallel strategy config.");
        return false;
    }
    if (!AddAlgorithmToManager(database, request.config)) {
        SendResponse(std::move(responsePtr), false,
            "Failed to add algorithm to manager when set parallel config. Unexpected algorithm.");
        return false;
    }
    session.OnResponse(std::move(responsePtr));
    return true;
}
bool SetParallelStrategyConfigHandler::AddAlgorithmToManager(const std::shared_ptr<VirtualClusterDatabase> &database,
                                                             const ParallelStrategyConfig &config)
{
    if (StringUtil::Contains(StringUtil::ToLower(config.algorithm), MEGATRON_ALG)) {
        ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(database->GetDbPath(),
            std::make_shared<MegatronParallelStrategyAlgorithm>(), config);
        return true;
    }
    return false;
}
}
