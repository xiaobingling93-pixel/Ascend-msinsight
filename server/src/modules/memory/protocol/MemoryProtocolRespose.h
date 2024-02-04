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
// 待与MemoryDef.h中的Operator合并
struct MemoryOperator {
    std::string name;
    double size;
    std::string allocationTime;
    std::string releaseTime;
    double duration;
    std::string activeReleaseTime;
    double activeDuration;
    double allocationAllocated;
    double allocationReserved;
    double allocationActive;
    double releaseAllocated;
    double releaseReserved;
    double releaseActive;
    std::string streamId;
    std::string deviceType;
};

struct MemoryViewData {
    std::string title;
    std::vector<std::string> legends;
    std::vector<std::vector<std::string>> lines;
};

struct MemoryTableColumnAttr {
    std::string name;
    std::string type;
    std::string key;
};

struct MemoryOperatorResponse : public Response {
    MemoryOperatorResponse() : Response(REQ_RES_MEMORY_OPERATOR) {}
    std::vector<MemoryTableColumnAttr> columnAttr;
    std::vector<MemoryOperator> operatorDetails;
    int64_t totalNum = 0;
};

struct MemoryViewResponse : public Response {
    MemoryViewResponse() : Response(REQ_RES_MEMORY_VIEW) {}
    MemoryViewData data;
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
    double totalActivated;
    std::string streamId;
    double timesTamp;
};

struct MemoryPeak {
    bool hasPtaGe = false;
    bool hasApp = false;
    double ptaGeAllocated = 0;
    double ptaGeReserved = 0;
    double ptaGeActivated = 0;
    double appReserved = 0;
};
} // end of namespace Protocol
} // end of namespace Dic
#endif // PROFILER_SERVER_MEMORYPROTOCLRESPOSE_H
