/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_COMMUNICATION_PACKET_ANALYZER_H
#define PROFILER_SERVER_COMMUNICATION_PACKET_ANALYZER_H

#include "CommunicationProtocolResponse.h"
#include "ClusterDef.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Protocol;
using namespace Dic::Module;

struct PacketAnalyzerStatistics {
    const double smallSdmaSizeStandard{16.0};
    const double smallSdmaProportionStandard{0.2};
    uint64_t sdmaCount{0};
    uint64_t smallSdmaCount{0};
    double smallSdmaProportion{0.0};
    double smallSdmaDuration{0.0};
    bool sdmaIssue{false};

    const double smallRdmaSizeStandard{1.0};
    const double smallRdmaProportionStandard{0.2};
    uint64_t rdmaCount{0};
    uint64_t smallRdmaCount{0};
    double smallRdmaProportion{0.0};
    double smallRdmaDuration{0.0};
    bool rdmaIssue{false};
};

class PacketAnalyzer {
public:
    PacketAnalyzer() = default;
    ~PacketAnalyzer() = default;
    bool GenerateAdvisor(CommunicationAdvisorInfo &info);
    bool QueryAdvisorData();
    void ComputeStatistics();
    void AssembleAdvisor(CommunicationAdvisorInfo &info);
private:
    std::vector<PacketAnalyzerData> data;
    PacketAnalyzerStatistics statistics;
};
} // end of namespace Communication
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_COMMUNICATION_PACKET_ANALYZER_H
