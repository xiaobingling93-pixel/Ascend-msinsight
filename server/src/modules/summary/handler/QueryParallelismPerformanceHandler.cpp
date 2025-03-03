/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "SummaryService.h"
#include "QueryParallelismPerformanceHandler.h"

namespace Dic::Module::Summary {
bool QueryParallelismPerformanceHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<QueryParallelismPerformanceRequest &>(*requestPtr);
    std::unique_ptr<ParallelismPerformanceResponse> responsePtr = std::make_unique<ParallelismPerformanceResponse>();
    ParallelismPerformanceResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    // check request parameter
    std::string err;
    if (!request.params.config.CheckParams(err) ||
        !SummaryService::CheckParamForMindSpeed(request.params.config, err)) {
        SendResponse(std::move(responsePtr), false, err);
        return false;
    }
    bool res = SummaryService::QueryParallelismPerformanceInfo(request.params, response.indicatorData);
    SendResponse(std::move(responsePtr), res);
    return true;
}
}
