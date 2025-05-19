/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "PacketAnalyzer.h"
#include "DataBaseManager.h"
#include "ServerLog.h"

namespace Dic {
namespace Module {
namespace Communication {

bool PacketAnalyzer::GenerateAdvisor(Dic::Protocol::CommunicationAdvisorInfo &info)
{
    if (!QueryAdvisorData()) {
        return false;
    }
    ComputeStatistics();
    AssembleAdvisor(info);
    return true;
}

bool PacketAnalyzer::QueryAdvisorData()
{
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    if (!database || !database->QueryPacketAnalyzerData(data)) {
        Server::ServerLog::Error("Failed to query packet analyzer data.");
        return false;
    }
    return true;
}

void PacketAnalyzer::ComputeStatistics()
{
    for (const auto &singleOperator : data) {
        if (singleOperator.type == "SDMA") {
            ++statistics.sdmaCount;
            if (singleOperator.transitSize < statistics.smallSdmaSizeStandard) {
                ++statistics.smallSdmaCount;
                statistics.smallSdmaDuration += singleOperator.transitTime;
            }
        } else if (singleOperator.type == "RDMA") {
            ++statistics.rdmaCount;
            if (singleOperator.transitSize < statistics.smallRdmaSizeStandard) {
                ++statistics.smallRdmaCount;
                statistics.smallRdmaDuration += singleOperator.transitTime;
            }
        }
    }
    if (statistics.sdmaCount > 0) {
        statistics.smallSdmaProportion = static_cast<double>(statistics.smallSdmaCount) / statistics.sdmaCount;
    } else {
        statistics.smallSdmaProportion = 0.0;
    }
    if (statistics.smallSdmaProportion > statistics.smallSdmaProportionStandard) {
        statistics.sdmaIssue = true;
    }
    if (statistics.rdmaCount > 0) {
        statistics.smallRdmaProportion = static_cast<double>(statistics.smallRdmaCount) / statistics.rdmaCount;
    } else {
        statistics.smallRdmaProportion = 0.0;
    }
    if (statistics.smallRdmaProportion > statistics.smallRdmaProportionStandard) {
        statistics.rdmaIssue = true;
    }
}

void PacketAnalyzer::AssembleAdvisor(Dic::Protocol::CommunicationAdvisorInfo &info)
{
    const int hundred = 100;
    info.name = PACKET_ANALYZER_TITLE;
    info.statistics.insert({"Category", {"SDMA", "RDMA"}});
    info.statistics.insert({"Small Size Standard(MB)",
        {std::to_string(statistics.smallSdmaSizeStandard), std::to_string(statistics.smallRdmaSizeStandard)}});
    info.statistics.insert({"Small Size Proportion Standard(%)",
        {std::to_string(statistics.smallSdmaProportionStandard * hundred),
        std::to_string(statistics.smallRdmaProportionStandard * hundred)}});
    info.statistics.insert({"Small Size Proportion(%)",
        {std::to_string(statistics.smallSdmaProportion * hundred),
         std::to_string(statistics.smallRdmaProportion * hundred)}});
    info.statistics.insert({"Small Size Duration(ms)",
        {std::to_string(statistics.smallSdmaDuration), std::to_string(statistics.smallRdmaDuration)}});
    info.statistics.insert({"Issue",
        {statistics.sdmaIssue ? "Yes" : "No", statistics.rdmaIssue ? "Yes" : "No"}});
}
}
}
}