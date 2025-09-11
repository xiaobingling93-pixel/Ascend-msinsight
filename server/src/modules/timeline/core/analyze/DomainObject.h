/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_DOMAINOBJECT_H
#define PROFILER_SERVER_DOMAINOBJECT_H
#include <string>
#include <vector>
#include <cstdint>
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

struct SliceShape {
    std::string inputShapes;
    std::string inputDataTypes;
    std::string inputFormats;
    std::string outputShapes;
    std::string outputDataTypes;
    std::string outputFormats;
    std::string attrInfo;
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
    std::string tid;
    std::string pid;
    std::string metaType;
    std::string flagId;
    SliceShape sliceShape;
    uint64_t dataSize {};
    std::string memcpyDirection;
    std::string bandwidth;
    std::string args;
    std::string cardId;
    bool operator < (const CompeteSliceDomain &right) const
    {
        if (depth < right.depth) {
            return true;
        }
        return depth == right.depth && timestamp < right.timestamp;
    }
};

struct Thread {
    uint64_t trackId = 0;
    std::string tid;
    std::string pid;
    std::string threadName;
    uint32_t sortIndex = 0;
};

struct Process {
    std::string pid;
    std::string name;
    std::string label;
    uint32_t sortIndex = 0;
    std::string parentPid = "0";
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
    std::string pid;
    std::string rankId;
    bool operator < (const FlowPoint &right) const
    {
        if (trackId < right.trackId) {
            return true;
        }
        if (trackId > right.trackId) {
            return false;
        }
        if (trackId == right.trackId && timestamp < right.timestamp) {
            return true;
        }
        if (trackId == right.trackId && timestamp > right.timestamp) {
            return false;
        }
        return timestamp == right.timestamp && id < right.id;
    }

    static bool CompareTimestampASC(const FlowPoint &first, const FlowPoint &second)
    {
        if (first.timestamp == second.timestamp) {
            return first.trackId < second.trackId;
        }
        return first.timestamp < second.timestamp;
    }
};

// PROCESS_TYPE 仅仅给 DB 场景使用，Text 场景最好不要使用此枚举值
enum class PROCESS_TYPE {
    ASCEND_HARDWARE,
    HCCL,
    OVERLAP_ANALYSIS,
    CANN_API,
    API,
    HBM,
    LLC,
    DDR,
    ACC_PMU,
    STARS_SOC,
    NPU_MEM,
    HCCS,
    QOS,
    PCIE,
    NIC,
    SAMPLE_PMU,
    AI_CORE,
    CPU_USAGE,
    HOST_DISK_USAGE,
    HOST_MEM_USAGE,
    HOST_NETWORK_USAGE,
    OSRT_API,
    MS_TX,
    TEXT,
    NONE,
    DB,
    PYTHON_GC,
    PROCESS,
};

struct ParallelGroupInfo {
    std::string group;
    std::string groupName;
    std::vector<std::string> globalRanks;
};
}
#endif // PROFILER_SERVER_DOMAINOBJECT_H
