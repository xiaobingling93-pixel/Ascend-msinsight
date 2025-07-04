/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "BandwidthContentionAnalyzer.h"
#include "DataBaseManager.h"
#include "ServerLog.h"

namespace Dic {
namespace Module {
namespace Communication {
bool BandwidthContentionAnalyzer::QueryAdvisorData(const std::string &clusterPath)
{
    std::vector<IterationsOrRanksObject> rankList;
    auto communicationDatabase = Timeline::DataBaseManager::Instance().GetClusterDatabase(clusterPath);
    if (!communicationDatabase || !communicationDatabase->QueryRanksHandler(rankList)) {
        Server::ServerLog::Error("Failed to get ranks data when query bandwidth contention data.");
        return false;
    }

    for (const auto &rank : rankList) {
        data.matMulData.insert({rank.iterationOrRankId, {}});
        data.SDMAData.insert({rank.iterationOrRankId, {}});
        auto summaryDatabase = Timeline::DataBaseManager::Instance().GetSummaryDatabaseByRankId(rank.iterationOrRankId);
        if (!summaryDatabase) {
            summaryDatabase = Timeline::DataBaseManager::Instance().GetSummaryDatabaseWithCluster(clusterPath,
                                                                                                  rank.iterationOrRankId);
        }
        if (!summaryDatabase) {
            Server::ServerLog::Error("Failed to get summary database connection.");
            continue;
        }
        summaryDatabase->QueryBandwidthContentionMatMulData(data.matMulData[rank.iterationOrRankId]);
        communicationDatabase->QueryBandwidthContentionAnalyzerData(data.SDMAData[rank.iterationOrRankId],
            rank.iterationOrRankId);
    }
    return true;
}

void BandwidthContentionAnalyzer::ComputeStatistics()
{
    for (const auto &item : data.matMulData) {
        size_t HCCLIndex = 0;
        size_t matMulIndex = 0;
        while (HCCLIndex < data.SDMAData[item.first].size() && matMulIndex < data.matMulData[item.first].size()) {
            if (data.SDMAData[item.first][HCCLIndex].startTime + data.SDMAData[item.first][HCCLIndex].duration <
                data.matMulData[item.first][matMulIndex].startTime) {
                ++HCCLIndex;
                continue;
            }
            if (data.matMulData[item.first][matMulIndex].startTime + data.matMulData[item.first][matMulIndex].duration
                < data.SDMAData[item.first][HCCLIndex].startTime) {
                ++matMulIndex;
                continue;
            }
            if (data.SDMAData[item.first][HCCLIndex].bandwidth < BANDWIDTH_CONTENTION_ANALYZER_THRESHOLD) {
                BandwidthContentionAnalyzerStatistics op;
                op.name = data.SDMAData[item.first][HCCLIndex].name;
                op.duration = data.SDMAData[item.first][HCCLIndex].duration;
                op.bandwidth = data.SDMAData[item.first][HCCLIndex].bandwidth;
                statistics.emplace_back(op);
            }
            ++HCCLIndex;
        }
    }
}

void BandwidthContentionAnalyzer::AssembleAdvisor(Dic::Protocol::CommunicationAdvisorInfo &info)
{
    info.name = BANDWIDTHCONTENTION_ANALYZER_TITLE;
    info.statistics.insert({"name", {}});
    info.statistics.insert({"duration(us)", {}});
    info.statistics.insert({"bandwidth(GB/s)", {}});
    for (const auto &item : statistics) {
        info.statistics["name"].emplace_back(item.name);
        info.statistics["duration(us)"].emplace_back(std::to_string(item.duration));
        info.statistics["bandwidth(GB/s)"].emplace_back(std::to_string(item.bandwidth));
    }
}
}
}
}
