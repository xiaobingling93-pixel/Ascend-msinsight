/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <vector>
#include <set>
#include <map>
#include "DataBaseManager.h"
#include "ServerLog.h"
#include "CollectionUtil.h"
#include "ClusterCovert.h"
#include "NumberUtil.h"
#include "ClusterService.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Dic::Server;
void ClusterService::QueryIterations(const Protocol::IterationsRequest &request,
                                     Protocol::IterationsOrRanksResponse &response)
{
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    if (database == nullptr || !database->QueryIterations(response.body.compare)) {
        ServerLog::Warn("Fail to query compare iterations info.");
    }

    if (!request.params.isCompare) {
        return;
    }

    auto baselineDatabase = Timeline::DataBaseManager::Instance().GetClusterDatabase(BASELINE);
    if (baselineDatabase == nullptr || !baselineDatabase->QueryIterations(response.body.baseline)) {
        ServerLog::Warn("Fail to query baseline iterations info.");
    }
}

void ClusterService::QueryGroupInfo(const Protocol::MatrixGroupRequest &request,
                                    Protocol::MatrixGroupResponse &response)
{
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    std::vector<std::string> compareGroupList;
    if (database == nullptr || !database->GetGroups(request.params.iterationId, compareGroupList)) {
        ServerLog::Warn("Fail to query compare group info.");
    }

    auto baselineDatabase = Timeline::DataBaseManager::Instance().GetClusterDatabase(BASELINE);
    std::vector<std::string> baselineGroupList;
    if (request.params.isCompare) {
        if (baselineDatabase == nullptr ||
            !baselineDatabase->GetGroups(request.params.baselineIterationId, baselineGroupList)) {
            ServerLog::Warn("Fail to query baseline group info.");
        }
    }
    // 合并通信域列表
    response.body.groupList = MergeGroupInfo(compareGroupList, baselineGroupList);
    // todo-yqs 并行策略待另一个需求整改后补充
}

std::vector<Protocol::GroupInfo> ClusterService::MergeGroupInfo(std::vector<std::string> &compareGroupList,
                                                                std::vector<std::string> &baselineGroupList)
{
    std::vector<Protocol::GroupInfo> res;
    std::vector<std::string> intersection = CollectionUtil::CalIntersection(compareGroupList, baselineGroupList);
    for (const auto &item: intersection) {
        Protocol::GroupInfo groupInfo = {item, "", "common"};
        res.push_back(groupInfo);
    }

    std::vector<std::string> compareDiff = CollectionUtil::CalDifferenceVector(compareGroupList, intersection);
    for (const auto &item: compareDiff) {
        Protocol::GroupInfo groupInfo = {item, "", "compare"};
        res.push_back(groupInfo);
    }

    std::vector<std::string> baselineDiff = CollectionUtil::CalDifferenceVector(baselineGroupList, intersection);
    for (const auto &item: baselineDiff) {
        Protocol::GroupInfo groupInfo = {item, "", "baseline"};
        res.push_back(groupInfo);
    }
    return res;
}

void ClusterService::MergeMatrixInfo(Protocol::MatrixListResponseBody &body, const std::vector<MatrixInfoDo> &compare,
                                     const std::vector<MatrixInfoDo> &baseline)
{
    std::set<std::string> keySet;
    std::map<std::string, MatrixInfoDo> compareMap;
    for (const auto &item: compare) {
        std::string key = std::to_string(item.srcRank) + underline + std::to_string(item.dstRank);
        keySet.insert(key);
        compareMap[key] = item;
    }
    std::map<std::string, MatrixInfoDo> baselineMap;
    for (const auto &item: baseline) {
        std::string key = std::to_string(item.srcRank) + underline + std::to_string(item.dstRank);
        keySet.insert(key);
        baselineMap[key] = item;
    }

    for (const auto &key: keySet) {
        Protocol::MatrixList matrix;
        std::vector<std::string> srcAndDst = StringUtil::Split(key, underline);
        if (srcAndDst.size() != matrixPointNumber) {
            continue;
        }
        matrix.srcRank = StringUtil::StringToInt(srcAndDst[0]);
        matrix.dstRank = StringUtil::StringToInt(srcAndDst[1]);
        if (compareMap.count(key) != 0) {
            matrix.matrixData.compare = ClusterCovert::CovertMatrixDoToInfo(compareMap[key]);
        }

        if (baselineMap.count(key) != 0) {
            matrix.matrixData.baseline = ClusterCovert::CovertMatrixDoToInfo(baselineMap[key]);
        }

        body.matrixList.push_back(matrix);
    }
}

void ClusterService::QueryMatrixInfo(Protocol::MatrixBandwidthParam &params, Protocol::MatrixListResponseBody &body)
{
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    std::vector<MatrixInfoDo> compareMatrixList;
    std::vector<MatrixInfoDo> baselineMatrixList;
    if (database == nullptr || !database->QueryMatrixList(params, compareMatrixList)) {
        ServerLog::Error("Failed to get compare matrix response data.");
    }

    if (params.isCompare) {
        auto baselineDatabase = Timeline::DataBaseManager::Instance().GetClusterDatabase(BASELINE);
        if (baselineDatabase == nullptr || !baselineDatabase->QueryMatrixList(params, baselineMatrixList)) {
            ServerLog::Error("Failed to get baseline matrix response data.");
        }
    }

    MergeMatrixInfo(body, compareMatrixList, baselineMatrixList);
}

void ClusterService::MergeOperatorList(Protocol::OperatorListsResponseBody &body,
    const std::vector<OperatorTimeDo> &compare, const std::vector<OperatorTimeDo> &baseline)
{
    auto numericStringCompare = [](const std::string &num1, const std::string &num2) {
        return StringUtil::StringToInt(num1) < StringUtil::StringToInt(num2);
    };
    std::set<std::string, decltype(numericStringCompare)> rankList(numericStringCompare);
    std::map<std::string, std::vector<Protocol::OperatorTimeItem>> compareRankToOperator;
    for (const auto &item: compare) {
        Protocol::OperatorTimeItem operatorTime = ClusterCovert::CovertDoToOperatorTime(item);
        compareRankToOperator[item.rankId].push_back(operatorTime);
        body.minTime = std::min(body.minTime, operatorTime.startTime);
        body.maxTime = std::max(body.maxTime, operatorTime.startTime + operatorTime.elapseTime);
        rankList.insert(item.rankId);
    }
    std::map<std::string, std::vector<Protocol::OperatorTimeItem>> baselineRankToOperator;
    for (const auto &item: baseline) {
        Protocol::OperatorTimeItem operatorTime = ClusterCovert::CovertDoToOperatorTime(item);
        baselineRankToOperator[item.rankId].push_back(operatorTime);
        body.minTime = std::min(body.minTime, operatorTime.startTime);
        body.maxTime = std::max(body.maxTime, operatorTime.startTime + operatorTime.elapseTime);
        rankList.insert(item.rankId);
    }

    for (const auto &item: rankList) {
        body.rankLists.push_back(item);
        Dic::Protocol::CompareData<std::vector<Protocol::OperatorTimeItem>> data;
        if (compareRankToOperator.count(item) != 0) {
            data.compare = compareRankToOperator[item];
        }
        if (baselineRankToOperator.count(item) != 0) {
            data.baseline = baselineRankToOperator[item];
        }
        body.opLists.push_back(data);
    }
}

void ClusterService::QueryOperatorList(Protocol::DurationListParams &params, Protocol::OperatorListsResponseBody &body)
{
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    std::vector<OperatorTimeDo> compareOperatorTimeList;
    std::vector<OperatorTimeDo> baselineOperatorTimeList;
    if (database == nullptr || !database->QueryOperatorList(params, compareOperatorTimeList)) {
        ServerLog::Error("Failed to get compare operator list response data.");
    }

    if (params.isCompare) {
        auto baselineDatabase = Timeline::DataBaseManager::Instance().GetClusterDatabase(BASELINE);
        if (baselineDatabase == nullptr || !baselineDatabase->QueryOperatorList(params, baselineOperatorTimeList)) {
            ServerLog::Error("Failed to get baseline operator response data.");
        }
    }

    MergeOperatorList(body, compareOperatorTimeList, baselineOperatorTimeList);
}

void ClusterService::MergeDurationData(Protocol::DurationListsResponseBody &body, std::vector<DurationDo> &compare,
                                       std::vector<DurationDo> &baseline)
{
    std::set<std::string> rankIdSet;
    std::map<std::string, Protocol::DurationData> compareMap;
    for (const auto &item: compare) {
        compareMap[item.rankId] = ClusterCovert::CovertDoToDuration(item);
        rankIdSet.insert(item.rankId);
    }
    std::map<std::string, Protocol::DurationData> baselineMap;
    for (const auto &item: baseline) {
        baselineMap[item.rankId] = ClusterCovert::CovertDoToDuration(item);
        rankIdSet.insert(item.rankId);
    }

    for (const auto &item: rankIdSet) {
        Protocol::Duration duration;
        duration.rankId = item;
        if (compareMap.count(item) != 0) {
            duration.durationData.compare = compareMap[item];
        }
        if (baselineMap.count(item) != 0) {
            duration.durationData.baseline = baselineMap[item];
        }
        duration.durationData.diff = duration.durationData.compare - duration.durationData.baseline;
        body.durationList.push_back(duration);
    }
}

void ClusterService::StatisticBandwidthData(const DurationDo &item, std::vector<Protocol::BandwidthStatistic> &bwStat)
{
    for (auto &one : bwStat) {
        if (one.type == "SDMA") {
            one.maxBw = std::max(one.maxBw, item.sdmaBw);
            one.minBw = std::min(one.minBw, item.sdmaBw);
            one.avgBw += item.sdmaBw;
            one.allTime += item.sdmaTime;
        } else {
            one.maxBw = std::max(one.maxBw, item.rdmaBw);
            one.minBw = std::min(one.minBw, item.rdmaBw);
            one.avgBw += item.rdmaBw;
            one.allTime += item.rdmaTime;
        }
    }
}

void ClusterService::GetBandwidthStatisticResult(std::vector<Protocol::BandwidthStatistic> &bwStat,
                                                 Protocol::DurationListsResponseBody &responseBody)
{
    if (responseBody.durationList.empty()) {
        return;
    }
    int digit = 4;
    for (auto &item : bwStat) {
        if (item.avgBw == 0) {
            continue;
        }
        item.avgBw = NumberUtil::DoubleReservedNDigits(item.avgBw / responseBody.durationList.size(), digit);
        if (item.minBw != DBL_MAX) {
            item.diffBw = NumberUtil::DoubleReservedNDigits(item.maxBw - item.minBw, digit);
        }
        item.maxBw = NumberUtil::DoubleReservedNDigits(item.maxBw, digit);
        item.minBw = NumberUtil::DoubleReservedNDigits(item.minBw, digit);
        item.allTime = NumberUtil::DoubleReservedNDigits(item.allTime, digit);
        responseBody.bwStatistics.emplace_back(item);
    }
}

void ClusterService::CalBandwidthData(Protocol::DurationListsResponseBody &body,
                                      const std::vector<DurationDo> &durationDoList)
{
    std::vector<Protocol::BandwidthStatistic> bwStat = {{"SDMA", 0, 0, DBL_MAX, 0, 0}, {"RDMA", 0, 0, DBL_MAX, 0, 0}};
    for (const auto &item: durationDoList) {
        StatisticBandwidthData(item, bwStat);
    }
    GetBandwidthStatisticResult(bwStat, body);
}

void ClusterService::QueryDurationList(Protocol::DurationListParams &params, Protocol::DurationListsResponseBody &body)
{
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    std::vector<DurationDo> compareDurationDoList;
    std::vector<DurationDo> baselineDurationDoList;
    if (database == nullptr || !database->QueryDurationList(params, compareDurationDoList)) {
        ServerLog::Error("Failed to get compare during list response data.");
    }

    if (params.isCompare) {
        auto baselineDatabase = Timeline::DataBaseManager::Instance().GetClusterDatabase(BASELINE);
        if (baselineDatabase == nullptr || !baselineDatabase->QueryDurationList(params, baselineDurationDoList)) {
            ServerLog::Error("Failed to get baseline during response data.");
        }
    }

    MergeDurationData(body, compareDurationDoList, baselineDurationDoList);
    CalBandwidthData(body, compareDurationDoList);
}
}
}
}