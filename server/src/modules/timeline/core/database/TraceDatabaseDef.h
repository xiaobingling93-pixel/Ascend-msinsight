/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_TRACE_DATABASE_DEF_H
#define PROFILER_SERVER_TRACE_DATABASE_DEF_H

#include <string>

namespace Dic {
namespace Module {
namespace Timeline {
struct SliceDto {
    uint64_t id = 0;
    uint64_t timestamp = 0;
    uint64_t duration = 0;
    int32_t depth = 0;
    int64_t trackId = 0;
    std::string name;
    std::string args;
    std::string cat;
};

struct FlowDetailDto {
    std::string id;
    std::string name;
    std::string cat;
    std::string flowId;
    std::string pid;
    std::string tid;
    int32_t depth = 0;
    uint64_t timestamp = 0;
    uint64_t flowTimestamp = 0;
    uint64_t duration = 0;
    std::string type;
    int64_t trackId = 0;
    std::string sliceName;
};

struct FlowCategoryEventsDto {
    uint64_t id;
    uint64_t trackId;
    std::string type;
    std::string flowId;
    std::string pid;
    std::string tid;
    uint32_t depth = 0;
    uint64_t timestamp = 0;
};

struct MetaDataDto {
    std::string pid;
    std::string processName;
    std::string metaType;
    std::string label;
    std::string threadId;
    std::string threadName;
    int32_t maxDepth = 0;
    std::string name; // ph = C, name
    std::string args; // ph = C, args
};

struct OneKernelData {
    std::string threadId;
    std::string pid;
};

struct LayerStatData {
    uint64_t total = 0;
    double allOperatorTime = 0.1;
};

struct KernelShapesDataDto {
    std::string inputShapes;
    std::string inputDataTypes;
    std::string inputFormats;
    std::string outputShapes;
    std::string outputDataTypes;
    std::string outputFormats;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_TRACE_DATABASE_DEF_H