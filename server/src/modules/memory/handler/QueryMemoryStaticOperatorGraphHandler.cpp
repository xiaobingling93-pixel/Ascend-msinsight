/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "DataBaseManager.h"
#include "BaselineManager.h"
#include "MemoryProtocolRequest.h"
#include "MemoryProtocolRespose.h"
#include "QueryMemoryStaticOperatorGraphHandler.h"

namespace Dic {
namespace Module {
namespace Memory {
using namespace Dic::Server;
bool QueryMemoryStaticOperatorGraphHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    MemoryStaticOperatorGraphRequest &request =
            dynamic_cast<MemoryStaticOperatorGraphRequest &>(*requestPtr.get());
    std::unique_ptr<MemoryStaticOperatorGraphResponse> responsePtr =
            std::make_unique<MemoryStaticOperatorGraphResponse>();
    MemoryStaticOperatorGraphResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    std::string errorMsg;
    if (!request.params.CommonCheck(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetMemoryDatabase(request.params.rankId);
    if (!request.params.isCompare) {
        if (!database->QueryStaticOperatorGraph(request.params, response.data)) {
            SendResponse(std::move(responsePtr), false, "Failed to query memory static operator graph data.");
            return false;
        }
    } else {
        StaticOperatorGraphItem compareData;
        StaticOperatorGraphItem baselineData;
        if (!GetRespectiveData(database, compareData, baselineData, request, errorMsg)) {
            SendResponse(std::move(responsePtr), false, errorMsg);
            return false;
        }
        ExecuteComparisonAlgorithm(compareData, baselineData, response);
    }
    SendResponse(std::move(responsePtr), true);
    return true;
}

bool QueryMemoryStaticOperatorGraphHandler::GetRespectiveData(std::shared_ptr<VirtualMemoryDataBase> database,
                                                              Dic::Protocol::StaticOperatorGraphItem &compareData,
                                                              Dic::Protocol::StaticOperatorGraphItem &baselineData,
                                                              Dic::Protocol::MemoryStaticOperatorGraphRequest &request,
                                                              std::string &errorMsg)
{
    std::string baselineId = Global::BaselineManager::Instance().GetBaselineId();
    if (baselineId == "") {
        errorMsg = "Failed to get baseline id.";
        return false;
    }
    auto databaseBaseline = Timeline::DataBaseManager::Instance().GetMemoryDatabase(baselineId);
    if (!databaseBaseline) {
        errorMsg = "Failed to connect to database of baseline.";
        return false;
    }
    if (!database->QueryStaticOperatorGraph(request.params, compareData)) {
        errorMsg = "Failed to query memory static operator graph compare data.";
        return false;
    }
    if (!databaseBaseline->QueryStaticOperatorGraph(request.params, baselineData)) {
        errorMsg = "Failed to query memory static operator graph baseline data.";
        return false;
    }
    return true;
}

void QueryMemoryStaticOperatorGraphHandler::ExecuteComparisonAlgorithm(
    const Protocol::StaticOperatorGraphItem &compareData, const Protocol::StaticOperatorGraphItem &baselineData,
    Protocol::MemoryStaticOperatorGraphResponse &response)
{
    GetCompareGraphLegends(compareData, baselineData, response.data);
    GetCompareGraphLines(compareData, baselineData, response.data);
}

void QueryMemoryStaticOperatorGraphHandler::GetCompareGraphLegends(const Protocol::StaticOperatorGraphItem &compareData,
    const Protocol::StaticOperatorGraphItem &baselineData, Protocol::StaticOperatorGraphItem &resultData)
{
    resultData.legends = compareData.legends;
    for (size_t i = 1; i < compareData.legends.size(); ++i) {
        resultData.legends[i] += " of Compare";
    }
    if (baselineData.legends.size() > 0) {
        resultData.legends.insert(resultData.legends.end(),
            baselineData.legends.begin() + 1, baselineData.legends.end());
    }
    for (size_t i = compareData.legends.size(); i < resultData.legends.size(); ++i) {
        resultData.legends[i] += " of Baseline";
    }
}

void QueryMemoryStaticOperatorGraphHandler::GetCompareGraphLines(const Protocol::StaticOperatorGraphItem &compareData,
                                                                 const Protocol::StaticOperatorGraphItem &baselineData,
                                                                 Protocol::StaticOperatorGraphItem &resultData)
{
    // compareData.lines和baselineData.lines都已经按照Node Index排好序，接下来将两个有序表进行归并。
    size_t indexCompare = 0;
    size_t indexBaseline = 0;
    while ((indexCompare < compareData.lines.size()) || (indexBaseline < baselineData.lines.size())) {
        // 如果baseline已经遍历完或者compare的Node Index小于baseline的Node Index，返回compare数据并补NULL。
        if (indexBaseline >= baselineData.lines.size() ||
            (indexCompare < compareData.lines.size() &&
            NumberUtil::TryParseUnsignedLongLong(compareData.lines[indexCompare][0]) <
            NumberUtil::TryParseUnsignedLongLong(baselineData.lines[indexBaseline][0]))) {
            std::vector<std::string> points = compareData.lines[indexCompare];
            if (baselineData.legends.size() > 0) {
                points.insert(points.end(), baselineData.legends.size() - 1, "NULL");
            }
            resultData.lines.emplace_back(points);
            ++indexCompare;
            continue;
        }
        // 如果compare已经遍历完或者compare的Node Index大于baseline的Node Index，返回baseline数据并补NULL。
        if (indexCompare >= compareData.lines.size() ||
            (indexBaseline < baselineData.lines.size() &&
            NumberUtil::TryParseUnsignedLongLong(compareData.lines[indexCompare][0]) >
            NumberUtil::TryParseUnsignedLongLong(baselineData.lines[indexBaseline][0]))) {
            std::vector<std::string> points = {};
            points.push_back(baselineData.lines[indexBaseline][0]);
            if (compareData.legends.size() > 0) {
                points.insert(points.end(), compareData.legends.size() - 1, "NULL");
            }
            if (baselineData.lines[indexBaseline].size() > 0) {
                points.insert(points.end(), baselineData.lines[indexBaseline].begin() + 1,
                    baselineData.lines[indexBaseline].end());
            }
            resultData.lines.emplace_back(points);
            ++indexBaseline;
            continue;
            }
        // 如果compare的Node Index等于baseline的Node Index，合并compare和baseline的数据。
        std::vector<std::string> points = compareData.lines[indexCompare];
        if (baselineData.lines[indexBaseline].size() > 0) {
            points.insert(points.end(), baselineData.lines[indexBaseline].begin() + 1,
                baselineData.lines[indexBaseline].end());
        }
        resultData.lines.emplace_back(points);
        ++indexCompare;
        ++indexBaseline;
    }
}
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic