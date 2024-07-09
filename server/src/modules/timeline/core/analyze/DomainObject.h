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
    int32_t depth = 0;
    bool operator < (const SliceDomain &right) const
    {
        if (depth < right.depth) {
            return true;
        }
        return depth == right.depth && timestamp < right.timestamp;
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
}
#endif // PROFILER_SERVER_DOMAINOBJECT_H
