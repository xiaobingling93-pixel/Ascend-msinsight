/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include <limits>
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "BaselineManager.h"
#include "MemoryProtocolRequest.h"
#include "MemoryProtocolRespose.h"
#include "NumberUtil.h"
#include "QueryMemoryOperatorHandler.h"

namespace Dic {
namespace Module {
namespace Memory {
using namespace Dic::Server;
void QueryMemoryOperatorHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    MemoryOperatorRequest &request = dynamic_cast<MemoryOperatorRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<MemoryOperatorComparisonResponse> responsePtr =
        std::make_unique<MemoryOperatorComparisonResponse>();
    MemoryOperatorComparisonResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    auto database = Timeline::DataBaseManager::Instance().GetMemoryDatabase(request.params.rankId);
    if (!request.params.isCompare) {
        std::vector<MemoryOperator> opDetails;
        if (!database->QueryOperatorDetail(request.params, response.columnAttr, opDetails) or
        !database->QueryOperatorsTotalNum(request.params, response.totalNum)) {
            SetResponseResult(response, false);
            ServerLog::Error("Failed to query memory operator data.");
            session.OnResponse(std::move(responsePtr));
            return;
        }
        for (const auto &item: opDetails) {
            MemoryOperatorComparison element = {item, {}, {}};
            response.operatorDiffDetails.emplace_back(element);
        }
    } else {
        if (request.params.type == Protocol::MEMORY_STREAM_GROUP) {
            SetResponseResult(response, false);
            ServerLog::Error("Memory comparing does not support request type Stream.");
            session.OnResponse(std::move(responsePtr));
            return;
        }
        std::string baselineId = Global::BaselineManager::Instance().GetBaselineId();
        auto databaseBaseline = DataBaseManager::Instance().GetMemoryDatabase(baselineId);
        if (!databaseBaseline) {
            SetResponseResult(response, false);
            ServerLog::Error("Failed to connect to database of baseline.");
            session.OnResponse(std::move(responsePtr));
            return;
        }
        if (!CompareOperator(database, databaseBaseline, request, responsePtr, session)) {
            return;
        }
    }
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

bool QueryMemoryOperatorHandler::CompareOperator(VirtualMemoryDataBase *database,
    VirtualMemoryDataBase *databaseBaseline,
    MemoryOperatorRequest &request, std::unique_ptr<MemoryOperatorComparisonResponse> &responsePtr, WsSession &session)
{
    MemoryOperatorComparisonResponse &response = *responsePtr.get();
    std::unique_ptr<MemoryOperatorResponse> responsePtrCompare = std::make_unique<MemoryOperatorResponse>();
    MemoryOperatorResponse &responseCompare = *responsePtrCompare.get();
    if (!database->QueryEntireOperatorTable(responseCompare.columnAttr, responseCompare.operatorDetails,
                                            request.params.rankId)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to query memory operator compare data.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    responseCompare.totalNum = responseCompare.operatorDetails.size();
    std::unique_ptr<MemoryOperatorResponse> responsePtrBaseline = std::make_unique<MemoryOperatorResponse>();
    MemoryOperatorResponse &responseBaseline = *responsePtrBaseline.get();
    if (!databaseBaseline->QueryEntireOperatorTable(responseBaseline.columnAttr, responseBaseline.operatorDetails,
                                                    request.params.rankId)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to query memory operator baseline data.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    responseBaseline.totalNum = responseBaseline.operatorDetails.size();
    std::unique_ptr<MemoryOperatorComparisonResponse> responsePtrDiffResult =
        std::make_unique<MemoryOperatorComparisonResponse>();
    MemoryOperatorComparisonResponse &responseDiffResult = *responsePtrDiffResult.get();
    GetOperatorDiff(responseCompare, responseBaseline, responseDiffResult);
    return SelectDiffResult(request, responsePtr, responseDiffResult, session);
}

void QueryMemoryOperatorHandler::GetOperatorDiff(const MemoryOperatorResponse &compareData,
                                                 const MemoryOperatorResponse &baselineData,
                                                 MemoryOperatorComparisonResponse &diffData)
{
    std::set<std::string> opName;
    std::map<std::string, std::vector<MemoryOperator>> compareList;
    std::map<std::string, std::vector<MemoryOperator>> baselineList;
    for (const auto &item : compareData.operatorDetails) {
        opName.insert(item.name);
        // 即使item.name这个key不存在，也会将item.name添加为新的key
        compareList[item.name].push_back(item);
    }
    for (const auto &item : baselineData.operatorDetails) {
        opName.insert(item.name);
        // 即使item.name这个key不存在，也会将item.name添加为新的key
        baselineList[item.name].push_back(item);
    }
    std::vector<MemoryOperator> emptyVec;
    for (const auto &name: opName) {
        if (compareList.find(name) == compareList.end()) {
            VectorMerge(emptyVec, baselineList[name], diffData);
            continue;
        }
        if (baselineList.find(name) == baselineList.end()) {
            VectorMerge(compareList[name], emptyVec, diffData);
            continue;
        }
        std::vector<MemoryOperator> &compareVec = compareList[name];
        std::vector<MemoryOperator> &baselineVec = baselineList[name];
        VectorMerge(compareVec, baselineVec, diffData);
    }
    diffData.totalNum = diffData.operatorDiffDetails.size();
}

void QueryMemoryOperatorHandler::VectorMerge(std::vector<MemoryOperator> &compareVec,
    std::vector<MemoryOperator> &baselineVec, MemoryOperatorComparisonResponse &diffData)
{
    const MemoryOperator emptyDiff = {"", std::numeric_limits<double>::lowest(), "", "",
        std::numeric_limits<double>::lowest(), "", std::numeric_limits<double>::lowest(),
        std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(),
        std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(),
        std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), "", ""};
    for (size_t i = 0; i < std::min(compareVec.size(), baselineVec.size()); ++i) {
        MemoryOperatorComparison element = {compareVec[i], baselineVec[i], {}};
        const int precision = 3;
        element.diff.name = compareVec[i].name;
        element.diff.size = NumberUtil::DoubleReservedNDigits(compareVec[i].size - baselineVec[i].size, precision);
        element.diff.allocationTime = NumberUtil::StringDoubleMinus(compareVec[i].allocationTime,
                                                                    baselineVec[i].allocationTime);
        element.diff.releaseTime = NumberUtil::StringDoubleMinus(compareVec[i].releaseTime, baselineVec[i].releaseTime);
        element.diff.duration = NumberUtil::DoubleReservedNDigits(compareVec[i].duration - baselineVec[i].duration,
                                                                  precision);
        element.diff.activeReleaseTime = NumberUtil::StringDoubleMinus(compareVec[i].activeReleaseTime,
                                                                       baselineVec[i].activeReleaseTime);
        element.diff.activeDuration = NumberUtil::DoubleReservedNDigits(
            compareVec[i].activeDuration - baselineVec[i].activeDuration, precision);
        element.diff.allocationAllocated = NumberUtil::DoubleReservedNDigits(
            compareVec[i].allocationAllocated - baselineVec[i].allocationAllocated, precision);
        element.diff.allocationReserved = NumberUtil::DoubleReservedNDigits(
            compareVec[i].allocationReserved - baselineVec[i].allocationReserved, precision);
        element.diff.allocationActive = NumberUtil::DoubleReservedNDigits(
            compareVec[i].allocationActive - baselineVec[i].allocationActive, precision);
        element.diff.releaseAllocated = NumberUtil::DoubleReservedNDigits(
            compareVec[i].releaseAllocated - baselineVec[i].releaseAllocated, precision);
        element.diff.releaseReserved = NumberUtil::DoubleReservedNDigits(
            compareVec[i].releaseReserved - baselineVec[i].releaseReserved, precision);
        element.diff.releaseActive = NumberUtil::DoubleReservedNDigits(
            compareVec[i].releaseActive - baselineVec[i].releaseActive, precision);
        element.diff.streamId = "";
        element.diff.deviceType = "";
        diffData.operatorDiffDetails.emplace_back(element);
    }
    for (size_t i = compareVec.size(); i < baselineVec.size(); ++i) {
        MemoryOperatorComparison element = {{}, baselineVec[i], emptyDiff};
        element.diff.name = baselineVec[i].name;
        diffData.operatorDiffDetails.emplace_back(element);
    }
    for (size_t i = baselineVec.size(); i < compareVec.size(); ++i) {
        MemoryOperatorComparison element = {compareVec[i], {}, emptyDiff};
        element.diff.name = compareVec[i].name;
        diffData.operatorDiffDetails.emplace_back(element);
    }
}

bool QueryMemoryOperatorHandler::SelectDiffResult(MemoryOperatorRequest &request,
    std::unique_ptr<MemoryOperatorComparisonResponse> &responsePtr, MemoryOperatorComparisonResponse &fullDiffResult,
    WsSession &session)
{
    MemoryOperatorComparisonResponse filteredDiffResult;
    for (const auto &item: fullDiffResult.operatorDiffDetails) {
        if (IsSelected(request, item)) {
            filteredDiffResult.operatorDiffDetails.push_back(item);
        }
    }
    SortResult(request, filteredDiffResult);
    const int defaultPageSize = 10;
    int64_t pageSize = request.params.pageSize == 0 ? defaultPageSize : request.params.pageSize;
    int64_t currentPage = request.params.currentPage - 1;
    currentPage = currentPage < 0 ? 0 : currentPage;
    int64_t offset = currentPage * pageSize;
    MemoryOperatorComparisonResponse &response = *responsePtr.get();
    if (offset >= filteredDiffResult.operatorDiffDetails.size()) {
        response.operatorDiffDetails.clear();
        response.totalNum = 0;
    } else {
        response.totalNum = filteredDiffResult.operatorDiffDetails.size();
        for (size_t i = offset; i < offset + pageSize && i < filteredDiffResult.operatorDiffDetails.size(); ++i) {
            response.operatorDiffDetails.push_back(filteredDiffResult.operatorDiffDetails[i]);
        }
    }
    for (const auto& column : tableColumnAttr) {
        response.columnAttr.emplace_back(column);
        if (column.name == "Name") {
            MemoryTableColumnAttr sourceItem = {"Source", "string", "source"};
            response.columnAttr.emplace_back(sourceItem);
        }
    }
    return true;
}

bool QueryMemoryOperatorHandler::IsSelected(MemoryOperatorRequest &request, const MemoryOperatorComparison &op)
{
    bool filter = true;
    filter = filter && (op.diff.name.find(request.params.searchName) != std::string::npos);
    if (request.params.minSize != -1) {
        filter = filter && (op.diff.size >= request.params.minSize);
    }
    if (request.params.maxSize != -1) {
        filter = filter && (op.diff.size <= request.params.maxSize);
    }
    if (op.compare.allocationTime == "" || op.compare.releaseTime == "" ||
        op.baseline.allocationTime == "" || op.baseline.releaseTime == "") {
        return filter;
    }
    if (request.params.startTime != -1 && request.params.endTime != -1) {
        // 要求compare对象的开始和结束时间有一个在startTime endTime内，且baseline对象的开始和结束时间也有一个在startTime endTime内。
        long double compareAlloTime = NumberUtil::StringToLongDouble(op.compare.allocationTime);
        long double compareReleTime = NumberUtil::StringToLongDouble(op.compare.releaseTime);
        long double baselineAlloTime = NumberUtil::StringToLongDouble(op.baseline.allocationTime);
        long double baselineReleTime = NumberUtil::StringToLongDouble(op.baseline.releaseTime);
        filter = filter &&
            ((compareAlloTime >= request.params.startTime && compareAlloTime <= request.params.endTime) ||
            (compareReleTime >= request.params.startTime && compareReleTime <= request.params.endTime)) &&
            ((baselineAlloTime >= request.params.startTime && baselineAlloTime <= request.params.endTime) ||
            (baselineReleTime >= request.params.startTime && baselineReleTime <= request.params.endTime));
    }
    return filter;
}

void QueryMemoryOperatorHandler::SortResult(MemoryOperatorRequest& request, MemoryOperatorComparisonResponse& result)
{
    if (request.params.orderBy.empty() || request.params.order.empty()) {
        return;
    }
    if (request.params.order == "ascend") {
        SortAscend(request, result);
    } else {
        SortDescend(request, result);
    }
}

void QueryMemoryOperatorHandler::SortAscend(MemoryOperatorRequest& request, MemoryOperatorComparisonResponse &result)
{
    std::map<std::string, bool (*)(MemoryOperatorComparison &, MemoryOperatorComparison &)> compFunc = {
        {"name", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.name < op2.diff.name;}},
        {"size", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.size < op2.diff.size;}},
        {"allocation_time", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.allocationTime < op2.diff.allocationTime;}},
        {"release_time", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.releaseTime < op2.diff.releaseTime;}},
        {"duration", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.duration < op2.diff.duration;}},
        {"active_release_time", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.activeReleaseTime < op2.diff.activeReleaseTime;}},
        {"active_duration", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.activeDuration < op2.diff.activeDuration;}},
        {"allocation_allocated", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.allocationAllocated < op2.diff.allocationAllocated;}},
        {"allocation_reserve", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.allocationReserved < op2.diff.allocationReserved;}},
        {"allocation_active", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.allocationActive < op2.diff.allocationActive;}},
        {"release_allocated", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.releaseAllocated < op2.diff.releaseAllocated;}},
        {"release_reserve", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.releaseReserved < op2.diff.releaseReserved;}},
        {"release_active", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.releaseActive < op2.diff.releaseActive;}},
        {"stream", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.streamId < op2.diff.streamId;}},
        {"device_type", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.deviceType < op2.diff.deviceType;}}};
    if (compFunc.find(request.params.orderBy) != compFunc.end()) {
        std::sort(result.operatorDiffDetails.begin(), result.operatorDiffDetails.end(),
            compFunc[request.params.orderBy]);
    }
}

void QueryMemoryOperatorHandler::SortDescend(MemoryOperatorRequest& request, MemoryOperatorComparisonResponse &result)
{
    std::map<std::string, bool (*)(MemoryOperatorComparison &, MemoryOperatorComparison &)> compFunc = {
        {"name", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.name > op2.diff.name;}},
        {"size", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.size > op2.diff.size;}},
        {"allocation_time", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.allocationTime > op2.diff.allocationTime;}},
        {"release_time", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.releaseTime > op2.diff.releaseTime;}},
        {"duration", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.duration > op2.diff.duration;}},
        {"active_release_time", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.activeReleaseTime > op2.diff.activeReleaseTime;}},
        {"active_duration", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.activeDuration > op2.diff.activeDuration;}},
        {"allocation_allocated", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.allocationAllocated > op2.diff.allocationAllocated;}},
        {"allocation_reserve", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.allocationReserved > op2.diff.allocationReserved;}},
        {"allocation_active", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.allocationActive > op2.diff.allocationActive;}},
        {"release_allocated", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.releaseAllocated > op2.diff.releaseAllocated;}},
        {"release_reserve", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.releaseReserved > op2.diff.releaseReserved;}},
        {"release_active", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.releaseActive > op2.diff.releaseActive;}},
        {"stream", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.streamId > op2.diff.streamId;}},
        {"device_type", [](MemoryOperatorComparison &op1, MemoryOperatorComparison &op2) {
            return op1.diff.deviceType > op2.diff.deviceType;}}};
    if (compFunc.find(request.params.orderBy) != compFunc.end()) {
        std::sort(result.operatorDiffDetails.begin(), result.operatorDiffDetails.end(),
            compFunc[request.params.orderBy]);
    }
}

void QueryMemoryOperatorHandler::SendFailureMessage(MemoryOperatorResponse &response,
                                                    std::unique_ptr<MemoryOperatorResponse> &responsePtr,
                                                    WsSession &session, const std::string &&message)
{
    SetResponseResult(response, false);
    ServerLog::Error(message);
    session.OnResponse(std::move(responsePtr));
}
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic
