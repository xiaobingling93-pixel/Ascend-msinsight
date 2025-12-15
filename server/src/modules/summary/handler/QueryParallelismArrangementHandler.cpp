/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "ParallelStrategyAlgorithmManager.h"
#include "TrackInfoManager.h"
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
    if (!request.params.CheckParams(err)) {
        SetSummaryError(ErrorCode::PARAMS_ERROR);
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(request.params.clusterPath);
    if (database == nullptr) {
        SetSummaryError(ErrorCode::CONNECT_DATABASE_FAILED);
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    if (!QueryArrangementByDimension(database->GetDbPath(), err, request, response)) {
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    for (const auto &item: FullDb::TrackInfoManager::Instance().GetRankIdToFileIdByClusterDb(
        request.params.clusterPath)) {
        response.arrangeData.rankDbPathList.push_back({item.first, item.second});
    }
    SendResponse(std::move(responsePtr), true);
    return true;
}

bool QueryParallelismArrangementHandler::QueryArrangementByDimension(const std::string& projectName, std::string& err,
    const QueryParallelismArrangementRequest& request, ParallelismArrangementResponse& response)
{
    if (!ParallelStrategyAlgorithmManager::Instance().AddOrUpdateAlgorithm(projectName, request.params.config, err)) {
        return false;
    }
    auto algPtr = ParallelStrategyAlgorithmManager::Instance().GetAlgorithmByProjectName(projectName);
    if (algPtr == nullptr) {
        return false;
    }
    BaseParallelStrategyAlgorithm &algorithm = *algPtr;
    if (!algorithm.UpdateParallelDimension(request.params.dimension, request.params.config, err)) {
        return false;
    }
    if (!algorithm.GenerateArrangementByDimension(err)) {
        return false;
    }
    response.arrangeData = algorithm.GetArrangementData();
    return true;
}
} // Dic::Module::Summary