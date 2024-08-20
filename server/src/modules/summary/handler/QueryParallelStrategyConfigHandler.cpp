/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "DataBaseManager.h"
#include "WsSessionManager.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "QueryParallelStrategyConfigHandler.h"

namespace Dic::Module::Summary {
using namespace Dic::Server;
void QueryParallelStrategyConfigHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<QueryParallelStrategyRequest &>(*requestPtr.get());
    std::unique_ptr<QueryParallelStrategyResponse> responsePtr = std::make_unique<QueryParallelStrategyResponse>();
    QueryParallelStrategyResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    auto database = Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
    if (!database->QueryParallelStrategyConfig(response.config, response.level)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to query parallel strategy config.");
    }
    session.OnResponse(std::move(responsePtr));
}
}