/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "DataBaseManager.h"
#include "BaselineManager.h"
#include "TraceTime.h"
#include "QueryMemoryViewHandler.h"

namespace Dic {
namespace Module {
namespace Memory {
using namespace Dic::Server;
bool QueryMemoryViewHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    MemoryViewRequest &request = dynamic_cast<MemoryViewRequest &>(*requestPtr.get());
    std::unique_ptr<MemoryViewResponse> responsePtr = std::make_unique<MemoryViewResponse>();
    MemoryViewResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    std::string errorMsg;
    if (!request.params.CommonCheck(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetMemoryDatabase(request.params.rankId);
    if (!request.params.isCompare) {
        uint64_t offsetTime = Timeline::TraceTime::Instance().GetOffsetByFileId(request.params.rankId);
        if (!database->QueryMemoryView(request.params, response.data, offsetTime)) {
            SendResponse(std::move(responsePtr), false, "Failed to query memory view data.");
            return false;
        }
    } else {
        std::string baselineId = Global::BaselineManager::Instance().GetBaselineId();
        if (baselineId == "") {
            SendResponse(std::move(responsePtr), false, "Failed to get baseline id.");
            return false;
        }
        auto databaseBaseline = Timeline::DataBaseManager::Instance().GetMemoryDatabase(baselineId);
        if (!databaseBaseline) {
            SendResponse(std::move(responsePtr), false, "Failed to connect to database of baseline.");
            return false;
        }
        if (!GetCompareGraph(database, databaseBaseline, request, *responsePtr.get(), errorMsg)) {
            SendResponse(std::move(responsePtr), false, errorMsg);
            return false;
        }
    }
    SendResponse(std::move(responsePtr), true);
    return true;
}

bool QueryMemoryViewHandler::GetCompareGraph(std::shared_ptr<VirtualMemoryDataBase> database,
    std::shared_ptr<VirtualMemoryDataBase> databaseBaseline, MemoryViewRequest &request,
    MemoryViewResponse &response, std::string &errorMsg)
{
    std::unique_ptr<MemoryViewResponse> responsePtrCompare = std::make_unique<MemoryViewResponse>();
    MemoryViewResponse &responseCompare = *responsePtrCompare.get();
    uint64_t offsetTimeCompare = Timeline::TraceTime::Instance().GetOffsetByFileId(request.params.rankId);
    if (!database->QueryMemoryView(request.params, responseCompare.data, offsetTimeCompare)) {
        errorMsg = "Failed to query memory view compare data.";
        return false;
    }
    std::unique_ptr<MemoryViewResponse> responsePtrBaseline = std::make_unique<MemoryViewResponse>();
    MemoryViewResponse &responseBaseline = *responsePtrBaseline.get();
    std::string baselineId = Global::BaselineManager::Instance().GetBaselineId();
    if (baselineId == "") {
        errorMsg = "Failed to get baseline id.";
        return false;
    }
    uint64_t offsetTimeBaseline = Timeline::TraceTime::Instance().GetOffsetByFileId(baselineId);
    if (!databaseBaseline->QueryMemoryView(request.params, responseBaseline.data, offsetTimeBaseline)) {
        errorMsg = "Failed to query memory view baseline data.";
        return false;
    }
    GetCompareGraphLegends(responseCompare.data, responseBaseline.data, response.data);
    GetCompareGraphLines(responseCompare.data, responseBaseline.data, response.data);
    return true;
}

void QueryMemoryViewHandler::GetCompareGraphLegends(const Protocol::MemoryViewData &compareData,
                                                    const Protocol::MemoryViewData &baselineData,
                                                    Protocol::MemoryViewData &resultData)
{
    resultData.title = "";
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

void QueryMemoryViewHandler::GetCompareGraphLines(const Protocol::MemoryViewData &compareData,
                                                  const Protocol::MemoryViewData &baselineData,
                                                  Protocol::MemoryViewData &resultData)
{
    // compareData.lines和baselineData.lines都已经按照时间排好序，接下来将两个有序表进行归并。
    size_t indexCompare = 0;
    size_t indexBaseline = 0;
    while ((indexCompare < compareData.lines.size()) || (indexBaseline < baselineData.lines.size())) {
        // 如果baseline已经遍历完或者compare的时间戳小于baseline的时间戳，返回compare数据并补NULL。
        if (indexBaseline >= baselineData.lines.size() ||
            (indexCompare < compareData.lines.size() &&
            NumberUtil::StringToLongDouble(compareData.lines[indexCompare][0]) <
            NumberUtil::StringToLongDouble(baselineData.lines[indexBaseline][0]))) {
            std::vector<std::string> points = compareData.lines[indexCompare];
            if (baselineData.legends.size() > 0) {
                points.insert(points.end(), baselineData.legends.size() - 1, "NULL");
            }
            resultData.lines.emplace_back(points);
            ++indexCompare;
            continue;
        }
        // 如果compare已经遍历完或者compare的时间戳大于baseline的时间戳，返回baseline数据并补NULL。
        if (indexCompare >= compareData.lines.size() ||
            (indexBaseline < baselineData.lines.size() &&
            NumberUtil::StringToLongDouble(compareData.lines[indexCompare][0]) >
            NumberUtil::StringToLongDouble(baselineData.lines[indexBaseline][0]))) {
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
        // 如果compare的时间戳等于baseline的时间戳，合并compare和baseline的数据。
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