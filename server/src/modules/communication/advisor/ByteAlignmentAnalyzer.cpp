/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "ByteAlignmentAnalyzer.h"
#include "DataBaseManager.h"
#include "ServerLog.h"

namespace Dic {
namespace Module {
namespace Communication {

bool ByteAlignmentAnalyzer::GenerateAdvisor(Dic::Protocol::CommunicationAdvisorInfo &info)
{
    if (!QueryAdvisorData()) {
        return false;
    }
    ComputeStatistics();
    AssembleAdvisor(info);
    return true;
}

bool ByteAlignmentAnalyzer::QueryAdvisorData()
{
    std::vector<IterationsOrRanksObject> rankList;
    auto communicationDatabase = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    if (!communicationDatabase || !communicationDatabase->QueryRanksHandler(rankList)) {
        Server::ServerLog::Error("Failed to get ranks data when query byte alignment data.");
        return false;
    }
    for (const auto rank : rankList) {
        auto timelineDatabase = Timeline::DataBaseManager::Instance().GetTraceDatabase(rank.iterationOrRankId);
        if (!timelineDatabase) {
            timelineDatabase =
                Timeline::DataBaseManager::Instance().GetTraceDatabaseWithOutHost(rank.iterationOrRankId);
            if (!timelineDatabase) {
                Server::ServerLog::Error("Failed to get connection to trace database when query byte alignment data.");
                continue;
            }
        }
        timelineDatabase->QueryByteAlignmentAnalyzerData(data);
    }
    return true;
}

void ByteAlignmentAnalyzer::ComputeStatistics()
{
    statistics.abnormalOperatorCount = 0;
    for (const auto &item : data) {
        bool flag = false;
        for (const auto &memcpyItem : item.memcpyTasks) {
            if (memcpyItem.transportType != "SDMA" || memcpyItem.linkType == "ON CHIP" ||
                memcpyItem.size <= BYTE_ALIGNMENT_SMALL_SIZE) {
                continue;
            }
            if (memcpyItem.size % BYTE_ALIGNMENT_BASE_SIZE > 0) {
                flag = true;
            }
        }
        for (const auto &reduceItem : item.reduceInlineTasks) {
            if (reduceItem.transportType != "SDMA" || reduceItem.linkType == "ON CHIP" ||
                reduceItem.size <= BYTE_ALIGNMENT_SMALL_SIZE) {
                continue;
            }
            if (reduceItem.size % BYTE_ALIGNMENT_BASE_SIZE > 0) {
                flag = true;
            }
        }
        if (flag) {
            ++statistics.abnormalOperatorCount;
        }
    }
}

void ByteAlignmentAnalyzer::AssembleAdvisor(Dic::Protocol::CommunicationAdvisorInfo &info)
{
    info.name = BYTEALIGNMENT_ANALYZER_TITLE;
    info.statistics.insert({"Small Size(Byte)", {std::to_string(statistics.smallSize)}});
    info.statistics.insert({"Abnormal Operator Count", {std::to_string(statistics.abnormalOperatorCount)}});
}
}
}
}