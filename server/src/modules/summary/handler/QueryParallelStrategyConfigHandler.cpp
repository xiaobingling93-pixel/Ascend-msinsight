/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <memory>
#include "MegatronParallelStrategyAlgorithm.h"
#include "MindSpeedParallelStrategyAlgorithm.h"
#include "WsSessionManager.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "BaselineManager.h"
#include "QueryParallelStrategyConfigHandler.h"

namespace Dic::Module::Summary {
using namespace Dic::Server;
using namespace Dic::Module::Global;
bool QueryParallelStrategyConfigHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<QueryParallelStrategyRequest &>(*requestPtr);
    std::unique_ptr<QueryParallelStrategyResponse> responsePtr = std::make_unique<QueryParallelStrategyResponse>();
    std::string errMsg;
    if (!request.params.CheckParams(errMsg)) {
        SetSummaryError(ErrorCode::PARAMS_ERROR);
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    QueryParallelStrategyResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(request.params.clusterPath);
    if (database == nullptr || !database->QueryParallelStrategyConfig(response.config, response.level)) {
        SetSummaryError(ErrorCode::QUERY_PARALLEL_STATISTICS_FAILED);
        SetResponseResult(response, false);
        ServerLog::Error("Failed to query parallel strategy config.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    if (!response.IsValid()) {
        response.SetDefault();
    }
    if (!ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(
        database->GetDbPath(), response.config, errMsg)) {
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    // 如果存在baseline，则对baseline进行同样的设置
    auto baselineDatabase = Timeline::DataBaseManager::Instance().GetClusterDatabase(
        BaselineManager::Instance().GetBaseLineClusterPath());
    if (baselineDatabase != nullptr && !ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(
        baselineDatabase->GetDbPath(), response.config, errMsg)) {
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    session.OnResponse(std::move(responsePtr));
    return true;
}
}