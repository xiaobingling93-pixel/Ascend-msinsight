/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_DOMAINOBJECT_H
#define PROFILER_SERVER_DOMAINOBJECT_H
#include <string>
namespace Dic::Module::Timeline {
struct SliceDomain {
    uint64_t id = 0;
    uint64_t timestamp = 0;
    uint64_t endTime = 0;
    uint32_t depth = 0;
    bool operator < (const SliceDomain &right) const
    {
        if (depth < right.depth) {
            return true;
        }
        return depth == right.depth && timestamp < right.timestamp;
    }

    static bool CompareTimestampASC(const SliceDomain &first, const SliceDomain &second)
    {
        if (first.timestamp < second.timestamp) {
            return true;
        }
        if (first.timestamp == second.timestamp && first.id < second.id) {
            return true;
        }
        return false;
    }
};

struct CompeteSliceDomain {
    uint64_t id = 0;
    uint64_t timestamp = 0;
    uint64_t duration = 0;
    uint64_t endTime = 0;
    uint32_t depth = 0;
    std::string name;
    uint64_t trackId = 0;
    std::string cname;
    bool operator < (const CompeteSliceDomain &right) const
    {
        if (depth < right.depth) {
            return true;
        }
        return depth == right.depth && timestamp < right.timestamp;
    }
};

struct FlowPoint {
    uint64_t id = 0;
    std::string flowId;
    std::string name;
    std::string cat;
    uint64_t trackId = 0;
    uint64_t timestamp = 0;
    std::string type;
    uint32_t depth = 0;
    std::string tid;
    std::string  pid;
};
}
#endif // PROFILER_SERVER_DOMAINOBJECT_H
