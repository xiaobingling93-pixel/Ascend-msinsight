/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "DataBaseManager.h"
#include "SummaryProtocolRequest.h"
#include "ParallelStrategyAlgorithmManager.h"
#include "SummarySlowRankAdvisorHandler.h"

namespace Dic::Module::Summary {
bool SummarySlowRankAdvisorHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<QueryParallelismArrangementRequest &>(*requestPtr);
    std::unique_ptr<SummarySlowRankAdvisorResponse> responsePtr = std::make_unique<SummarySlowRankAdvisorResponse>();
    SummarySlowRankAdvisorResponse &response = *responsePtr;
    SetBaseResponse(request, response);

    // check request parameter
    std::string err;
    if (!request.params.CheckParams(err)) {
        SetSummaryError(ErrorCode::PARAMS_ERROR);
        SendResponse(std::move(responsePtr), false, err);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(request.params.clusterPath);
    if (database == nullptr) {
        SetSummaryError(ErrorCode::CONNECT_DATABASE_FAILED);
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    auto algPtr = ParallelStrategyAlgorithmManager::Instance().GetAlgorithmByProjectName(database->GetDbPath());
    if (algPtr == nullptr) {
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    BaseParallelStrategyAlgorithm &algorithm = *algPtr;
    // Summary慢卡建议依赖于排布数据查询parallelism/arrangement/all和性能指标查询parallelism/performance/data的计算结果
    response.body.topNElements = algorithm.GetTopNAdviceInfo(response.body.matchSuccess);
    if (response.body.matchSuccess && response.body.topNElements.empty()) {
        response.body.hasSlowRank = false; // 当前维度无明显快慢卡问题
    }
    SendResponse(std::move(responsePtr), true);
    return true;
}
}
