/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <atomic>
#include "WsSessionManager.h"
#include "TraceTime.h"
#include "SystemViewOverallRepoInterface.h"
#include "SystemViewOverallRepoFactory.h"
#include "QuerySystemViewOverallHandler.h"

std::atomic<uint32_t> Dic::Protocol::SystemViewOverallRes::idCounter{0};
namespace Dic::Module::Timeline {
using namespace Dic::Server;
bool QuerySystemViewOverallHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<SystemViewOverallRequest &>(*requestPtr);
    Protocol::SystemViewOverallRes::idCounter = 0;
    std::unique_ptr<SystemViewOverallResponse> responsePtr = std::make_unique<SystemViewOverallResponse>();
    SystemViewOverallResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(request.params.rankId);
    if (database == nullptr) {
        SendResponse(std::move(responsePtr), false, "Failed to get connection for system view overall statistics.");
        return false;
    }
    std::string error;
    request.params.page.Check(error);
    if (!std::empty(error)) {
        SendResponse(std::move(responsePtr), false, error);
        return false;
    }
    SystemViewOverallHelper overallHelper;
    // 执行一层级Overlap查询，结果存放在overallHelper.overlapInfos, 并显式返回e2eTime
    overallHelper.e2eTime = GetOverlapAnalysisData(overallHelper, database, request, response.details);
    if (overallHelper.e2eTime == 0.0) {
        SendResponse(std::move(responsePtr), false,
                     "Failed to query system view overall. Overlap information query incomplete or missing.");
        return false;
    }
    auto repoPtr = SystemViewOverallRepoFactory::Instance()->GetSystemViewOverallRepo(
        DataBaseManager::Instance().GetDataType());
    if (repoPtr == nullptr) {
        // GetSystemViewOverallRepo中已有日志报错
        return false;
    }
    // Computing拆解
    if (!repoPtr->QueryDataForComputingOverallMetric(request.params, overallHelper, database)) {
        ServerLog::Error("Failed to get computing data for system view overall.");  // Computing部分失败，但是继续剩下的查询
    }
    if (!overallHelper.kernelEvents.empty()) {
        // 若kernel details含有效pmu数据，则进行Computing Overall统计，否则跳过Computing拆解
        overallHelper.CategorizeComputingEvents();
        overallHelper.AggregateComputingOverallMetrics(response.details);
    }
    // Communication拆解
    repoPtr->QueryCommunicationOverlapOverallInfos(request.params, overallHelper.e2eTime, response.details, database);
    response.pageParam.total = response.details.size();
    response.pageParam.current = request.params.page.current;
    response.pageParam.pageSize = request.params.page.pageSize;
    SendResponse(std::move(responsePtr), true);
    return true;
}

double QuerySystemViewOverallHandler::GetOverlapAnalysisData(SystemViewOverallHelper &overallHelper,
    const std::shared_ptr<VirtualTraceDatabase> &database, const SystemViewOverallRequest &request,
    std::vector<SystemViewOverallRes> &responseBody)
{
    double e2eTime{};
    auto repoPtr = SystemViewOverallRepoFactory::Instance()->GetSystemViewOverallRepo(
        DataBaseManager::Instance().GetDataType());
    if (repoPtr == nullptr) {
        // GetSystemViewOverallRepo中已有日志报错
        return e2eTime;
    }
    overallHelper.overlapInfos = repoPtr->QueryOverlapAnalysisDataForOverallMetric(request.params, database);
    // Overlap: CommunicationIndex = 0, ComputingIndex = 1, FreeIndex = 2按字典序排序
    uint32_t expectOverlapSize = 3;
    if (overallHelper.overlapInfos.size() < expectOverlapSize) {
        return e2eTime;
    }
    const size_t communicationIndex = 0;
    const size_t computingIndex = 1;
    const size_t freeIndex = 2;
    double communicationNotOverlapped = overallHelper.overlapInfos[communicationIndex].duration;
    double computingTime = overallHelper.overlapInfos[computingIndex].duration;
    double freeTime = overallHelper.overlapInfos[freeIndex].duration;
    e2eTime = communicationNotOverlapped + computingTime + freeTime;
    if (std::fabs(e2eTime - 0.0) < std::numeric_limits<double>::epsilon()) {
        return e2eTime;
    }
    double computingRatio = NumberUtil::DoubleReservedNDigits(PERCENTAGE_RATIO_SCALE * computingTime / e2eTime, TWO);
    double communicationRatio = NumberUtil::DoubleReservedNDigits(
        PERCENTAGE_RATIO_SCALE * communicationNotOverlapped / e2eTime, TWO);
    double freeRatio = NumberUtil::DoubleReservedNDigits(PERCENTAGE_RATIO_SCALE * freeTime / e2eTime, TWO);
    double e2eRatio = PERCENTAGE_RATIO_SCALE;
    // 组装一层级数据
    Protocol::SystemViewOverallRes tmpRes = { .totalTime = computingTime, .ratio = computingRatio, .nums = 0,
        .avg = 0, .max = 0, .min = UINT32_MAX, .name = COMPUTING_TIME, .children = {}, .level = 1,
        .id = std::to_string(Protocol::SystemViewOverallRes::idCounter.fetch_add(1))};
    responseBody.emplace_back(tmpRes);
    tmpRes = {.totalTime = communicationNotOverlapped, .ratio = communicationRatio, .nums = 0, .avg = 0, .max = 0,
        .min = 0, .name = COMMUNICATION_NOT_OVERLAP_TIME, .children = {}, .level = 1,
        .id = std::to_string(Protocol::SystemViewOverallRes::idCounter.fetch_add(1))};
    responseBody.emplace_back(tmpRes);
    tmpRes = {.totalTime = freeTime, .ratio = freeRatio, .nums = 0, .avg = 0, .max = 0,
        .min = 0, .name = FREE_TIME, .children = {}, .level = 1,
        .id = std::to_string(Protocol::SystemViewOverallRes::idCounter.fetch_add(1))};
    responseBody.emplace_back(tmpRes);
    tmpRes = {.totalTime = NumberUtil::DoubleReservedNDigits(e2eTime, TWO), .ratio = e2eRatio, .nums = 0, .avg = 0,
              .max = 0, .min = 0, .name = E2E_TIME, .children = {}, .level = 1,
        .id = std::to_string(Protocol::SystemViewOverallRes::idCounter.fetch_add(1))};
    return e2eTime;
}
} // namespace Dic::Module::Timeline