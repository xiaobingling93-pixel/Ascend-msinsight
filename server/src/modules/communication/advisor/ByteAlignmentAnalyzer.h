/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_COMMUNICATION_BYTEALIGNMENT_ANALYZER_H
#define PROFILER_SERVER_COMMUNICATION_BYTEALIGNMENT_ANALYZER_H

#include "CommunicationProtocolResponse.h"
#include "ClusterDef.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Protocol;
using namespace Dic::Module;

const unsigned int BYTE_ALIGNMENT_SMALL_SIZE = 512;
const unsigned int BYTE_ALIGNMENT_BASE_SIZE = 512;

struct ByteAlignmentAnalyzerStatistics {
    const unsigned int smallSize = BYTE_ALIGNMENT_SMALL_SIZE;
    uint64_t abnormalOperatorCount = 0;
};

class ByteAlignmentAnalyzer {
public:
    ByteAlignmentAnalyzer() = default;
    ~ByteAlignmentAnalyzer() = default;
    bool GenerateAdvisor(CommunicationAdvisorInfo &info);
    bool QueryAdvisorData();
    void ComputeStatistics();
    void AssembleAdvisor(CommunicationAdvisorInfo &info);
private:
    std::vector<CommunicationLargeOperatorInfo> data;
    ByteAlignmentAnalyzerStatistics statistics;
};
} // end of namespace Communication
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_COMMUNICATION_BYTEALIGNMENT_ANALYZER_H
