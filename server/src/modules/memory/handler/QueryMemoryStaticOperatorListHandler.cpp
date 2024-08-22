/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "BaselineManager.h"
#include "MemoryProtocolRequest.h"
#include "MemoryProtocolRespose.h"
#include "QueryMemoryStaticOperatorListHandler.h"

namespace Dic {
namespace Module {
namespace Memory {
using namespace Dic::Server;
void QueryMemoryStaticOperatorListHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    MemoryStaticOperatorListRequest &request =
            dynamic_cast<MemoryStaticOperatorListRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<MemoryStaticOperatorListCompResponse> responsePtr =
            std::make_unique<MemoryStaticOperatorListCompResponse>();
    MemoryStaticOperatorListCompResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    auto database = Timeline::DataBaseManager::Instance().GetMemoryDatabase(request.params.rankId);
    if (!request.params.isCompare) {
        std::vector<StaticOperatorItem> opDetails;
        if (!database->QueryStaticOperatorList(request.params, response.columnAttr, opDetails) or
            !database->QueryStaticOperatorsTotalNum(request.params, response.totalNum)) {
            SetResponseResult(response, false);
            ServerLog::Error("Failed to query memory static operator data.");
            session.OnResponse(std::move(responsePtr));
            return;
        }
        for (const auto &item: opDetails) {
            StaticOperatorCompItem element = {item, {}, {}};
            response.operatorDiffDetails.emplace_back(element);
        }
    } else {
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

bool QueryMemoryStaticOperatorListHandler::CompareOperator(VirtualMemoryDataBase *database,
    VirtualMemoryDataBase *databaseBaseline, MemoryStaticOperatorListRequest &request,
    std::unique_ptr<MemoryStaticOperatorListCompResponse> &responsePtr, Server::WsSession &session)
{
    MemoryStaticOperatorListCompResponse &response = *responsePtr.get();
    std::unique_ptr<MemoryStaticOperatorListResponse> responsePtrCompare =
        std::make_unique<MemoryStaticOperatorListResponse>();
    MemoryStaticOperatorListResponse &responseCompare = *responsePtrCompare.get();
    if (!database->QueryEntireStaticOperatorTable(request.params, responseCompare.columnAttr,
        responseCompare.operatorDetails)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to query memory static operator compare data.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    responseCompare.totalNum = responseCompare.operatorDetails.size();
    std::unique_ptr<MemoryStaticOperatorListResponse> responsePtrBaseline =
        std::make_unique<MemoryStaticOperatorListResponse>();
    MemoryStaticOperatorListResponse &responseBaseline = *responsePtrBaseline.get();
    if (!databaseBaseline->QueryEntireStaticOperatorTable(request.params, responseBaseline.columnAttr,
        responseBaseline.operatorDetails)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to query memory static operator baseline data.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    responseBaseline.totalNum = responseBaseline.operatorDetails.size();
    std::unique_ptr<MemoryStaticOperatorListCompResponse> responsePtrDiffResult =
        std::make_unique<MemoryStaticOperatorListCompResponse>();
    MemoryStaticOperatorListCompResponse &responseDiffResult = *responsePtrDiffResult.get();
    GetOperatorDiff(responseCompare, responseBaseline, responseDiffResult);
    return SelectDiffResult(request, responsePtr, responseDiffResult, session);
}

void QueryMemoryStaticOperatorListHandler::GetOperatorDiff(const MemoryStaticOperatorListResponse &compareData,
    const MemoryStaticOperatorListResponse &baselineData,
    MemoryStaticOperatorListCompResponse &diffData)
{
    std::set<std::string> opNameSet;
    std::map<std::string, std::vector<StaticOperatorItem>> compareList;
    std::map<std::string, std::vector<StaticOperatorItem>> baselineList;
    for (const auto &item : compareData.operatorDetails) {
        opNameSet.insert(item.opName);
        // 即使item.name这个key不存在，也会将item.name添加为新的key
        compareList[item.opName].push_back(item);
    }
    for (const auto &item : baselineData.operatorDetails) {
        opNameSet.insert(item.opName);
        // 即使item.name这个key不存在，也会将item.name添加为新的key
        baselineList[item.opName].push_back(item);
    }
    std::vector<StaticOperatorItem> emptyVec;
    for (const auto &name: opNameSet) {
        if (compareList.find(name) == compareList.end()) {
            VectorMerge(emptyVec, baselineList[name], diffData);
            continue;
        }
        if (baselineList.find(name) == baselineList.end()) {
            VectorMerge(compareList[name], emptyVec, diffData);
            continue;
        }
        std::vector<StaticOperatorItem> &compareVec = compareList[name];
        std::vector<StaticOperatorItem> &baselineVec = baselineList[name];
        VectorMerge(compareVec, baselineVec, diffData);
    }
    diffData.totalNum = diffData.operatorDiffDetails.size();
}

void QueryMemoryStaticOperatorListHandler::VectorMerge(std::vector<StaticOperatorItem> &compareVec,
    std::vector<StaticOperatorItem> &baselineVec, MemoryStaticOperatorListCompResponse &diffData)
{
    const StaticOperatorItem emptyDiff = {"", "", std::numeric_limits<long long>::lowest(),
        std::numeric_limits<long long>::lowest(), std::numeric_limits<double>::lowest()};
    for (size_t i = 0; i < std::min(compareVec.size(), baselineVec.size()); ++i) {
        StaticOperatorCompItem element = {compareVec[i], baselineVec[i], {}};
        const int precision = 3;
        element.diff.deviceId = "host";
        element.diff.opName = compareVec[i].opName;
        element.diff.nodeIndexStart = compareVec[i].nodeIndexStart - baselineVec[i].nodeIndexStart;
        element.diff.nodeIndexEnd = compareVec[i].nodeIndexEnd - baselineVec[i].nodeIndexEnd;
        element.diff.size = NumberUtil::DoubleReservedNDigits(compareVec[i].size - baselineVec[i].size, precision);
        diffData.operatorDiffDetails.emplace_back(element);
    }
    for (size_t i = compareVec.size(); i < baselineVec.size(); ++i) {
        StaticOperatorCompItem element = {{}, baselineVec[i], emptyDiff};
        element.diff.opName = baselineVec[i].opName;
        diffData.operatorDiffDetails.emplace_back(element);
    }
    for (size_t i = baselineVec.size(); i < compareVec.size(); ++i) {
        StaticOperatorCompItem element = {compareVec[i], {}, emptyDiff};
        element.diff.opName = compareVec[i].opName;
        diffData.operatorDiffDetails.emplace_back(element);
    }
}

bool QueryMemoryStaticOperatorListHandler::SelectDiffResult(MemoryStaticOperatorListRequest &request,
    std::unique_ptr<MemoryStaticOperatorListCompResponse> &responsePtr,
    MemoryStaticOperatorListCompResponse &fullDiffResult, Server::WsSession &session)
{
    MemoryStaticOperatorListCompResponse filteredDiffResult;
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
    MemoryStaticOperatorListCompResponse &response = *responsePtr.get();
    if (offset >= filteredDiffResult.operatorDiffDetails.size()) {
        response.operatorDiffDetails.clear();
        response.totalNum = 0;
    } else {
        response.totalNum = filteredDiffResult.operatorDiffDetails.size();
        for (size_t i = offset; i < offset + pageSize && i < filteredDiffResult.operatorDiffDetails.size(); ++i) {
            response.operatorDiffDetails.push_back(filteredDiffResult.operatorDiffDetails[i]);
        }
    }
    for (const auto& column : staticOpTableColumnAttr) {
        response.columnAttr.emplace_back(column);
        if (column.name == "Name") {
            MemoryTableColumnAttr sourceItem = {"Source", "string", "source"};
            response.columnAttr.emplace_back(sourceItem);
        }
    }
    return true;
}

bool QueryMemoryStaticOperatorListHandler::IsSelected(MemoryStaticOperatorListRequest& request,
                                                      const StaticOperatorCompItem& op)
{
    bool filter = true;
    filter = filter && (op.diff.opName.find(request.params.searchName) != std::string::npos);
    if (request.params.minSize != -1) {
        filter = filter && (op.diff.size >= request.params.minSize);
    }
    if (request.params.maxSize != -1) {
        filter = filter && (op.diff.size <= request.params.maxSize);
    }
    if (request.params.startNodeIndex != -1 && request.params.endNodeIndex != -1) {
        // 要求compare对象的开始和结束时间有一个在startTime endTime内，且baseline对象的开始和结束时间也有一个在startTime endTime内。
        filter = filter &&
            ((op.compare.nodeIndexStart >= request.params.startNodeIndex && op.compare.nodeIndexStart <= request.params.
                    endNodeIndex) ||
                (op.compare.nodeIndexEnd >= request.params.startNodeIndex && op.compare.nodeIndexEnd <= request.params.
                    endNodeIndex)) &&
            ((op.baseline.nodeIndexStart >= request.params.startNodeIndex && op.baseline.nodeIndexStart <= request.
                    params.endNodeIndex) ||
                (op.baseline.nodeIndexEnd >= request.params.startNodeIndex && op.baseline.nodeIndexEnd <= request.params
                    .endNodeIndex));
    }
    return filter;
}

void QueryMemoryStaticOperatorListHandler::SortResult(MemoryStaticOperatorListRequest& request,
    MemoryStaticOperatorListCompResponse& result)
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

void QueryMemoryStaticOperatorListHandler::SortAscend(MemoryStaticOperatorListRequest& request,
    MemoryStaticOperatorListCompResponse &result)
{
    std::map<std::string, bool (*)(StaticOperatorCompItem &, StaticOperatorCompItem &)> compFunc = {
        {"device_id", [](StaticOperatorCompItem &op1, StaticOperatorCompItem &op2) {
            return op1.diff.deviceId < op2.diff.deviceId;}},
        {"op_name", [](StaticOperatorCompItem &op1, StaticOperatorCompItem &op2) {
            return op1.diff.opName < op2.diff.opName;}},
        {"node_index_start", [](StaticOperatorCompItem &op1, StaticOperatorCompItem &op2) {
            return op1.diff.nodeIndexStart < op2.diff.nodeIndexStart;}},
        {"node_index_end", [](StaticOperatorCompItem &op1, StaticOperatorCompItem &op2) {
            return op1.diff.nodeIndexEnd < op2.diff.nodeIndexEnd;}},
        {"size", [](StaticOperatorCompItem &op1, StaticOperatorCompItem &op2) {
            return op1.diff.size < op2.diff.size;}}};
    if (compFunc.find(request.params.orderBy) != compFunc.end()) {
        std::sort(result.operatorDiffDetails.begin(), result.operatorDiffDetails.end(),
            compFunc[request.params.orderBy]);
    }
}

void QueryMemoryStaticOperatorListHandler::SortDescend(MemoryStaticOperatorListRequest& request,
    MemoryStaticOperatorListCompResponse &result)
{
    std::map<std::string, bool (*)(StaticOperatorCompItem &, StaticOperatorCompItem &)> compFunc = {
        {"device_id", [](StaticOperatorCompItem &op1, StaticOperatorCompItem &op2) {
            return op1.diff.deviceId > op2.diff.deviceId;}},
        {"op_name", [](StaticOperatorCompItem &op1, StaticOperatorCompItem &op2) {
            return op1.diff.opName > op2.diff.opName;}},
        {"node_index_start", [](StaticOperatorCompItem &op1, StaticOperatorCompItem &op2) {
            return op1.diff.nodeIndexStart > op2.diff.nodeIndexStart;}},
        {"node_index_end", [](StaticOperatorCompItem &op1, StaticOperatorCompItem &op2) {
            return op1.diff.nodeIndexEnd > op2.diff.nodeIndexEnd;}},
        {"size", [](StaticOperatorCompItem &op1, StaticOperatorCompItem &op2) {
            return op1.diff.size > op2.diff.size;}}};
    if (compFunc.find(request.params.orderBy) != compFunc.end()) {
        std::sort(result.operatorDiffDetails.begin(), result.operatorDiffDetails.end(),
            compFunc[request.params.orderBy]);
    }
}
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic