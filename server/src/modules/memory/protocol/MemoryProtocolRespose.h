/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_MEMORYPROTOCLRESPOSE_H
#define PROFILER_SERVER_MEMORYPROTOCLRESPOSE_H

#include <vector>
#include "GlobalDefs.h"
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"

namespace Dic {
namespace Protocol {
struct MemoryOperator {
    std::string name;
    double size;
    double allocationTime;
    double releaseTime;
    double duration;
};

struct ComponentMemory {
    std::string title;
    // [timesTamp, memoryUsage]
    std::vector<std::vector<double>> ptaAllocatesLine;
    std::vector<std::vector<double>> ptaReservedLine;
    std::vector<std::vector<double>> geAllocatesLine;
    std::vector<std::vector<double>> geReservedLine;
    std::vector<std::vector<double>> appLine;
};

struct OperatorMemory {
    bool hasApp = false;
    std::string peakMemoryUsage;
    // [timesTamp, memoryUsage]
    std::vector<std::vector<std::string>> lines;
};

struct MemoryOperatorResponse : public Response {
    MemoryOperatorResponse() : Response(REQ_RES_MEMORY_OPERATOR) {}
    std::vector<MemoryOperator> operatorDetails;
    int64_t totalNum = 0;
};

struct MemoryViewResponse : public Response {
    MemoryViewResponse() : Response(REQ_RES_MEMORY_VIEW) {}
    OperatorMemory map;
};

struct OperatorSize {
    double minSize;
    double maxSize;
};

struct MemoryOperatorSizeResponse : public Response {
    MemoryOperatorSizeResponse() : Response(REQ_RES_MEMORY_OPERATOR_MIN_MAX) {}
    OperatorSize size;
};

struct ComponentDto {
    std::string component;
    double totalReserved;
    double totalAllocated;
    double timesTamp;
};

struct MemoryPeak {
    bool hasPtaGe = false;
    bool hasApp = false;
    double ptaGeAllocated = 0;
    double ptaGeReserved = 0;
    double appReserved = 0;
};
} // end of namespace Protocol
} // end of namespace Dic
#endif // PROFILER_SERVER_MEMORYPROTOCLRESPOSE_H
