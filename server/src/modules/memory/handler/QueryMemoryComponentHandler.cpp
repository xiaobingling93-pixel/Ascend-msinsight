/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "TraceTime.h"
#include "DataBaseManager.h"
#include "BaselineManager.h"
#include "MemoryProtocolRequest.h"
#include "MemoryProtocolRespose.h"
#include "QueryMemoryComponentHandler.h"

namespace Dic {
namespace Module {
namespace Memory {
bool QueryMemoryComponentHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    MemoryComponentRequest &request = dynamic_cast<MemoryComponentRequest &>(*requestPtr.get());
    std::unique_ptr<MemoryComponentComparisonResponse> responsePtr =
        std::make_unique<MemoryComponentComparisonResponse>();
    MemoryComponentComparisonResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    std::string errorMsg;
    if (!request.params.CommonCheck(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetMemoryDatabaseByRankId(request.params.rankId);
    if (!database) {
        SendResponse(std::move(responsePtr), false, "Failed to connect to database.");
        return false;
    }

    std::string deviceId = Timeline::DataBaseManager::Instance().GetDeviceIdFromRankId(request.params.rankId, "memory");
    if (deviceId.empty()) {
        SendResponse(std::move(responsePtr), false, "Failed to query memory component data.");
        return false;
    }
    request.params.deviceId = deviceId;
    if (!request.params.isCompare) {
        std::vector<MemoryComponent> componentDetails;
        if (!database->QueryComponentDetail(request.params, response.columnAttr, componentDetails) ||
        !database->QueryComponentsTotalNum(request.params, response.totalNum)) {
            SendResponse(std::move(responsePtr), false, "Failed to query memory component data.");
            return false;
        }
        for (const auto &item : componentDetails) {
            MemoryComponentComparison element = {item, {}, {}};
            response.componentDiffDetails.emplace_back(element);
        }
    } else {
        std::vector<MemoryComponent> compareData;
        std::vector<MemoryComponent> baselineData;
        if (!GetRespectiveData(database, compareData, baselineData, request, errorMsg)) {
            SendResponse(std::move(responsePtr), false, errorMsg);
            return false;
        }
        ExecuteComparisonAlgorithm(compareData, baselineData, request, response);
    }
    SendResponse(std::move(responsePtr), true);
    return true;
}

bool QueryMemoryComponentHandler::GetRespectiveData(std::shared_ptr<VirtualMemoryDataBase> database,
                                                    std::vector<MemoryComponent> &compareData,
                                                    std::vector<MemoryComponent> &baselineData,
                                                    Dic::Protocol::MemoryComponentRequest &request,
                                                    std::string &errorMsg)
{
    std::string baselineId = Global::BaselineManager::Instance().GetBaselineId();
    if (baselineId == "") {
        errorMsg = "Failed to get baseline id.";
        return false;
    }
    auto databaseBaseline = Timeline::DataBaseManager::Instance().GetMemoryDatabaseByRankId(baselineId);
    if (!databaseBaseline) {
        errorMsg = "Failed to connect to database of baseline.";
        return false;
    }
    uint64_t offsetTimeCompare = Timeline::TraceTime::Instance().GetOffsetByFileIdUsingMinTimestamp(request.params.rankId);
    if (!database->QueryEntireComponentTable(request.params, compareData, offsetTimeCompare)) {
        errorMsg = "Failed to query memory component compare data.";
        return false;
    }
    request.params.deviceId = FullDb::DataBaseManager::Instance().GetDeviceIdFromRankId(baselineId, "memory");
    uint64_t offsetTimeBaseline = Timeline::TraceTime::Instance().GetOffsetByFileIdUsingMinTimestamp(baselineId);
    if (!databaseBaseline->QueryEntireComponentTable(request.params, baselineData, offsetTimeBaseline)) {
        errorMsg = "Failed to query memory component baseline data.";
        return false;
    }
    return true;
}

void QueryMemoryComponentHandler::ExecuteComparisonAlgorithm(const std::vector<MemoryComponent> &compareData,
                                                             const std::vector<MemoryComponent> &baselineData,
                                                             Dic::Protocol::MemoryComponentRequest &request,
                                                             Dic::Protocol::MemoryComponentComparisonResponse &response)
{
    std::vector<MemoryComponentComparison> diffData;
    GetComponentDiff(compareData, baselineData, diffData);
    SelectResult(request, response, diffData);
}

void QueryMemoryComponentHandler::GetComponentDiff(const std::vector<MemoryComponent> &compareData,
                                                   const std::vector<MemoryComponent> &baselineData,
                                                   std::vector<MemoryComponentComparison> &diffData)
{
    // 与算子比对不同，因为数据库查询时同名组件只保留一条，不需要用vector保存同一名称的组件
    std::set<std::string> componentSet;
    std::map<std::string, MemoryComponent> compareMap;
    std::map<std::string, MemoryComponent> baselineMap;
    for (const auto &item : compareData) {
        componentSet.insert(item.component);
        compareMap[item.component] = item;
    }
    for (const auto &item : baselineData) {
        componentSet.insert(item.component);
        baselineMap[item.component] = item;
    }
    MemoryComponent empty = {"", "NA", 0.0, ""};
    MemoryComponentComparison mergeResult;
    for (const auto &component: componentSet) {
        if (compareMap.find(component) == compareMap.end()) {
            Merge(empty, baselineMap[component], mergeResult);
            diffData.emplace_back(mergeResult);
            continue;
        }
        if (baselineMap.find(component) == baselineMap.end()) {
            Merge(compareMap[component], empty, mergeResult);
            diffData.emplace_back(mergeResult);
            continue;
        }
        Merge(compareMap[component], baselineMap[component], mergeResult);
        diffData.emplace_back(mergeResult);
    }
}

void QueryMemoryComponentHandler::Merge(Dic::Protocol::MemoryComponent &componentCompare,
                                        Dic::Protocol::MemoryComponent &componentBaseline,
                                        Dic::Protocol::MemoryComponentComparison &mergeResult)
{
    mergeResult.compare = componentCompare;
    mergeResult.baseline = componentBaseline;
    const int precision = 3;
    if (!componentCompare.component.empty()) {
        mergeResult.diff.component = componentCompare.component;
    } else {
        mergeResult.diff.component = componentBaseline.component;
    }
    mergeResult.diff.timestamp = NumberUtil::StringDoubleMinus(componentCompare.timestamp, componentBaseline.timestamp);
    mergeResult.diff.totalReserved = NumberUtil::DoubleReservedNDigits(
        componentCompare.totalReserved - componentBaseline.totalReserved, precision);
}

void QueryMemoryComponentHandler::SelectResult(Dic::Protocol::MemoryComponentRequest &request,
                                               MemoryComponentComparisonResponse &response,
                                               std::vector<MemoryComponentComparison> &fullDiffResult)
{
    SortResult(request, fullDiffResult);
    uint64_t pageSize = request.params.pageSize <= 0 ? DEFAULT_PAGE_SIZE : static_cast<uint64_t>(request.params.pageSize);
    uint64_t currentPage = request.params.currentPage < 1 ? 0 : static_cast<uint64_t>(request.params.currentPage - 1);
    uint64_t offset = currentPage * pageSize;
    if (offset != 0 && offset >= fullDiffResult.size()) {
        response.componentDiffDetails.clear();
    } else {
        for (size_t i = offset; i < offset + pageSize && i < fullDiffResult.size(); ++i) {
            response.componentDiffDetails.push_back(fullDiffResult[i]);
        }
    }
    response.totalNum = std::min(static_cast<int64_t>(fullDiffResult.size()), std::numeric_limits<int64_t>::max());
    for (const auto& column : tableColumnAttr) {
        response.columnAttr.emplace_back(column);
        if (column.name == "Component") {
            MemoryTableColumnAttr sourceItem = {"Source", "string", "source"};
            response.columnAttr.emplace_back(sourceItem);
        }
    }
}

void QueryMemoryComponentHandler::SortResult(Dic::Protocol::MemoryComponentRequest &request,
                                             std::vector<MemoryComponentComparison> &result)
{
    if (request.params.order.empty() || request.params.orderBy.empty()) {
        return;
    }
    if (request.params.order == "ascend") {
        SortAscend(request, result);
    } else {
        SortDescend(request, result);
    }
}

void QueryMemoryComponentHandler::SortAscend(Dic::Protocol::MemoryComponentRequest &request,
                                             std::vector<MemoryComponentComparison> &result)
{
    std::map<std::string, bool (*)(MemoryComponentComparison &, MemoryComponentComparison &)> compFunc = {
        {"component", [](MemoryComponentComparison &comp1, MemoryComponentComparison &comp2) {
            return comp1.diff.component < comp2.diff.component;}},
        {"timestamp", [](MemoryComponentComparison &comp1, MemoryComponentComparison &comp2) {
            return NumberUtil::StringToDouble(comp1.diff.timestamp) <
            NumberUtil::StringToDouble(comp2.diff.timestamp);}},
        {"totalReserved", [](MemoryComponentComparison &comp1, MemoryComponentComparison &comp2) {
            return comp1.diff.totalReserved < comp2.diff.totalReserved;}}
    };
    if (compFunc.find(request.params.orderBy) != compFunc.end()) {
        std::sort(result.begin(), result.end(), compFunc[request.params.orderBy]);
    }
}

void QueryMemoryComponentHandler::SortDescend(Dic::Protocol::MemoryComponentRequest &request,
                                              std::vector<MemoryComponentComparison> &result)
{
    std::map<std::string, bool (*)(MemoryComponentComparison &, MemoryComponentComparison &)> compFunc = {
        {"component", [](MemoryComponentComparison &comp1, MemoryComponentComparison &comp2) {
            return comp1.diff.component > comp2.diff.component;}},
        {"timestamp", [](MemoryComponentComparison &comp1, MemoryComponentComparison &comp2) {
            return NumberUtil::StringToDouble(comp1.diff.timestamp) >
            NumberUtil::StringToDouble(comp2.diff.timestamp);}},
        {"totalReserved", [](MemoryComponentComparison &comp1, MemoryComponentComparison &comp2) {
            return comp1.diff.totalReserved > comp2.diff.totalReserved;}}
    };
    if (compFunc.find(request.params.orderBy) != compFunc.end()) {
        std::sort(result.begin(), result.end(), compFunc[request.params.orderBy]);
    }
}
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic
