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
    std::string name;
    std::string cat;
    std::string flowId;
    std::string pid;
    int32_t tid = 0;
    int32_t depth = 0;
    uint64_t timestamp = 0;
    uint64_t duration = 0;
    std::string type;
    std::string sliceName;
};

struct FlowCategoryEventsDto {
    std::string type;
    std::string flowId;
    std::string pid;
    int32_t tid = 0;
    int32_t depth = 0;
    uint64_t timestamp = 0;
};

struct MetaDataDto {
    std::string pid;
    std::string processName;
    std::string label;
    int64_t threadId = 0;
    std::string threadName;
    int maxDepth = 0;
    std::string name; // ph = C, name
    std::string args; // ph = C, args
};

struct SliceTimeData {
    int64_t id;
    uint64_t time;
    uint64_t dur;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_TRACE_DATABASE_DEF_H