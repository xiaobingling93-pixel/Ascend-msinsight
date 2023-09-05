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
    double size = 0;
    double allocationTime = 0;
    double releaseTime = 0;
    double duration = 0;
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
    std::string peakMemoryUsage;
    // [timesTamp, memoryUsage]
    std::vector<std::vector<double>> allocatesLine;
    std::vector<std::vector<double>> reservedLine;
    std::vector<std::vector<double>> appLine;
};

struct MemoryOperatorResponse : public Response {
    MemoryOperatorResponse() : Response(REQ_RES_MEMORY_OPERATOR) {}
    std::vector<MemoryOperator> operatorDetails;
};

struct MemoryViewResponse : public Response {
    MemoryViewResponse() : Response(REQ_RES_MEMORY_VIEW) {}
    OperatorMemory map;
};

struct ComponentDto {
    std::string component;
    double totalReserved;
    double totalAllocated;
    double timesTamp;
};
} // end of namespace Protocol
} // end of namespace Dic
#endif // PROFILER_SERVER_MEMORYPROTOCLRESPOSE_H
