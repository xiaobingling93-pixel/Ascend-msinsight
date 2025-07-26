/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_MEMORY_DETAIL_DEFS_H
#define PROFILER_SERVER_MEMORY_DETAIL_DEFS_H

#include <string>
#include <set>
#include "FileDef.h"

namespace Dic::Module::MemoryDetail {
struct MemoryEvent {
    uint64_t id;
    std::string event;
    std::string eventType;
    std::string name;
    uint64_t timestamp;
    uint64_t processId;
    uint64_t threadId;
    std::string deviceId;
    std::string ptr;
    std::string attr;
    int64_t size;
};

struct MemoryAllocation {
    uint64_t id{};
    uint64_t timestamp{};
    uint64_t totalSize{};
    std::string deviceId;
    std::string eventType;
    bool optimized{};

    MemoryAllocation() = default;
    MemoryAllocation(uint64_t timestamp, uint64_t totalSize, std::string deviceId, std::string eventType,
                     bool optimized)
        : id(0),
          timestamp(timestamp),
          totalSize(totalSize),
          deviceId(std::move(deviceId)),
          eventType(std::move(eventType)),
          optimized(optimized) {}
};

struct MemoryBlock {
    uint64_t id;
    std::string ptr;
    std::string deviceId;
    uint64_t size;
    uint64_t startTimestamp;
    uint64_t endTimestamp;
    std::string owner;
    std::string eventType;
    std::string otherAttr;
    uint64_t processId;
    uint64_t threadId;

    MemoryBlock() = default;
    MemoryBlock(std::string ptr, std::string deviceId, uint64_t size, uint64_t startTs, uint64_t endTs,
                std::string owner, std::string eventType, std::string otherAttr, uint64_t pid, uint64_t tid)
        : id(0),
          ptr(std::move(ptr)),
          deviceId(std::move(deviceId)),
          size(size),
          startTimestamp(startTs),
          endTimestamp(endTs),
          owner(std::move(owner)),
          eventType(std::move(eventType)),
          otherAttr(std::move(otherAttr)),
          processId(pid),
          threadId(tid) {}
};

struct MemoryDetailTree {
    uint64_t size;
    std::string name;
    std::vector<MemoryDetailTree> subNodes;
};

}  // namespace Dic::Module::MemoryDetail
#endif  // PROFILER_SERVER_MEMORY_DETAIL_DEFS_H
