/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "ParallelStrategyAlgorithmManager.h"
#include "TraceTime.h"
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
    if (!request.params.config.CheckParams(err)) {
        SendResponse(std::move(responsePtr), false, err);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    if (database == nullptr) {
        SendResponse(std::move(responsePtr), false, "Failed to get connection for query parallelism performance.");
        return false;
    }
    std::unordered_map<std::uint32_t, StepStatistic> stepStatisticData{};
    bool result = database->QueryAllPerformanceDataByStep(request.params.step, stepStatisticData);
    if (!result || stepStatisticData.empty()) {
        SendResponse(std::move(responsePtr), false, "Failed to query parallelism performance data.");
        return false;
    }
    auto algPtr = ParallelStrategyAlgorithmManager::Instance().GetAlgorithmByProjectName(database->GetDbPath(), err);
    if (algPtr == nullptr) {
        SendResponse(std::move(responsePtr), false,
            "Failed to get algorithm by project name for query parallelism performance.");
        return false;
    }
    result = algPtr->GetPerformanceIndicatorByDimension(request.params, stepStatisticData, response.indicatorData, err);
    if (!result) {
        SendResponse(std::move(responsePtr), false, err);
        return false;
    }
    SendResponse(std::move(responsePtr), true);
    return true;
}
}
