/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "ParallelStrategyAlgorithmManager.h"
#include "QueryParallelismArrangementHandler.h"
namespace Dic::Module::Summary {
bool QueryParallelismArrangementHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<QueryParallelismArrangementRequest &>(*requestPtr);
    std::unique_ptr<ParallelismArrangementResponse> responsePtr = std::make_unique<ParallelismArrangementResponse>();
    ParallelismArrangementResponse &response = *responsePtr;
    SetBaseResponse(request, response);

    // check request parameter
    std::string err;
    if (!request.params.config.CheckParams(err)) {
        SendResponse(std::move(responsePtr), false, err);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    if (database == nullptr) {
        SendResponse(std::move(responsePtr), false, "Failed to get connection for query parallelism arrangement.");
        return false;
    }
    if (!QueryArrangementByDimension(database->GetDbPath(), err, request, response)) {
        SendResponse(std::move(responsePtr), false, err);
        return false;
    }
    SendResponse(std::move(responsePtr), true);
    return true;
}

bool QueryParallelismArrangementHandler::QueryArrangementByDimension(const std::string& projectName, std::string& err,
    const QueryParallelismArrangementRequest& request, ParallelismArrangementResponse& response)
{
    auto algPtr = ParallelStrategyAlgorithmManager::Instance().GetAlgorithmByProjectName(projectName, err);
    if (algPtr == nullptr) {
        err = "Failed to get algorithm by project name for query  parallelism arrangement.";
        return false;
    }
    BaseParallelStrategyAlgorithm &algorithm = *algPtr;
    algorithm.UpdateParallelDimension(request.params.dimension, request.params.config, err);
    algorithm.GenerateArrangementByDimension();
    response.arrangeData = algorithm.GetArrangementData();
    return true;
}
} // Dic::Module::Summary