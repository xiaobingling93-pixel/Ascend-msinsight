/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <memory>
#include "MegatronParallelStrategyAlgorithm.h"
#include "WsSessionManager.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "QueryParallelStrategyConfigHandler.h"

namespace Dic::Module::Summary {
using namespace Dic::Server;
bool QueryParallelStrategyConfigHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<QueryParallelStrategyRequest &>(*requestPtr.get());
    std::unique_ptr<QueryParallelStrategyResponse> responsePtr = std::make_unique<QueryParallelStrategyResponse>();
    QueryParallelStrategyResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    if (database == nullptr || !database->QueryParallelStrategyConfig(response.config, response.level)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to query parallel strategy config.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    if (!response.IsValid()) {
        response.SetDefault();
    }
    if (!AddAlgorithmToManager(database, response.config)) {
        SendResponse(std::move(responsePtr), false,
            "Failed to add algorithm to manager when query parallel config. Unexpected algorithm.");
        return false;
    }
    session.OnResponse(std::move(responsePtr));
    return true;
}
bool QueryParallelStrategyConfigHandler::AddAlgorithmToManager(const std::shared_ptr<VirtualClusterDatabase> &database,
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