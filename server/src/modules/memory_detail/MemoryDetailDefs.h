/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_MEMORY_DETAIL_DEFS_H
#define PROFILER_SERVER_MEMORY_DETAIL_DEFS_H

#include <string>
#include <set>
#include <unordered_map>
#include "FileDef.h"

namespace Dic::Module::MemoryDetail {
const char OWNER_STRING_DELIMITER = '@';
// 内存事件
struct LEAKS_DUMP_EVENT {
    inline const static std::string MALLOC = "MALLOC"; // 内存申请事件
    inline const static std::string FREE = "FREE"; // 内存释放事件
    inline const static std::string ACCESS = "ACCESS"; // 内存访问
    inline const static std::string OP_LAUNCH = "OP_LAUNCH"; // 算子launch
    inline const static std::string KERNEL_LAUNCH = "KERNEL_LAUNCH"; // kernel launch
    inline const static std::string SYSTEM = "SYSTEM"; // 系统事件
};
// 内存事件类型
struct LEAKS_DUMP_EVENT_TYPE {
    // 内存申请/释放事件 子类型
    inline const static std::string MALLOC_FREE_PTA = "PTA"; // PTA内存池分配(从PTA内存池申请)
    inline const static std::string MALLOC_FREE_MINDSPORE = "MINDSPORE"; // MINDSPORE内存池分配(从MINDSPORE内存池申请)
    inline const static std::string MALLOC_FREE_ATB = "ATB"; // ATB申请
    inline const static std::string MALLOC_FREE_HAL = "HAL"; // 从HAL申请
    inline const static std::string MALLOC_FREE_PTA_WORKSPACE = "PTA_WORKSPACE"; // 从workspace申请
    // 内存访问事件 子类型
    inline const static std::string ACCESS_READ = "READ"; // 内存读事件
    inline const static std::string ACCESS_WRITE = "WRITE"; // 内存写事件
    // 算子launch事件 子类型
    inline const static std::string OP_LAUNCH_ATEN_START = "ATEN_START"; // aten开始, name字段标识aten算子名
    inline const static std::string OP_LAUNCH_ATEN_END = "ATEN_END"; // aten结束
    // kernel launch事件 子类型
    inline const static std::string KERNEL_LAUNCH = "KERNEL_LAUNCH"; // kernel launch
    inline const static std::string KERNEL_LAUNCH_START = "KERNEL_EXECUTE_START"; // kernel launch 开始
    inline const static std::string KERNEL_LAUNCH_END = "KERNEL_EXECUTE_END"; // kernel launch 结束
    // 系统事件 子类型
    inline const static std::string SYSTEM_ACL_INIT = "ACL_INIT"; // ACL初始化
    inline const static std::string SYSTEM_ACL_FINI = "ACL_FINI"; // ACL结束
};
// 内存事件 - 类型映射表
const std::unordered_map<std::string, std::set<std::string>> EVENT_TYPE_MAP = {
    {LEAKS_DUMP_EVENT::MALLOC, {LEAKS_DUMP_EVENT_TYPE::MALLOC_FREE_PTA,
                                LEAKS_DUMP_EVENT_TYPE::MALLOC_FREE_MINDSPORE,
                                LEAKS_DUMP_EVENT_TYPE::MALLOC_FREE_ATB,
                                LEAKS_DUMP_EVENT_TYPE::MALLOC_FREE_HAL,
                                LEAKS_DUMP_EVENT_TYPE::MALLOC_FREE_PTA_WORKSPACE}},
    {LEAKS_DUMP_EVENT::FREE, {LEAKS_DUMP_EVENT_TYPE::MALLOC_FREE_PTA,
                              LEAKS_DUMP_EVENT_TYPE::MALLOC_FREE_MINDSPORE,
                              LEAKS_DUMP_EVENT_TYPE::MALLOC_FREE_ATB,
                              LEAKS_DUMP_EVENT_TYPE::MALLOC_FREE_HAL,
                              LEAKS_DUMP_EVENT_TYPE::MALLOC_FREE_PTA_WORKSPACE}},
    {LEAKS_DUMP_EVENT::ACCESS, {LEAKS_DUMP_EVENT_TYPE::ACCESS_READ,
                                LEAKS_DUMP_EVENT_TYPE::ACCESS_WRITE}},
    {LEAKS_DUMP_EVENT::OP_LAUNCH, {LEAKS_DUMP_EVENT_TYPE::OP_LAUNCH_ATEN_START,
                                   LEAKS_DUMP_EVENT_TYPE::OP_LAUNCH_ATEN_END}},
    {LEAKS_DUMP_EVENT::KERNEL_LAUNCH, {LEAKS_DUMP_EVENT_TYPE::KERNEL_LAUNCH,
                                       LEAKS_DUMP_EVENT_TYPE::KERNEL_LAUNCH_START,
                                       LEAKS_DUMP_EVENT_TYPE::KERNEL_LAUNCH_END}},
    {LEAKS_DUMP_EVENT::SYSTEM, {LEAKS_DUMP_EVENT_TYPE::SYSTEM_ACL_INIT,
                                LEAKS_DUMP_EVENT_TYPE::SYSTEM_ACL_FINI}}
};

struct MemoryEvent {
    uint64_t id{0};
    std::string event;
    std::string eventType;
    std::string name;
    uint64_t timestamp{0};
    uint64_t processId{0};
    uint64_t threadId{0};
    std::string deviceId;
    std::string ptr;
    std::string attr;
    int64_t size{0};
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
    uint64_t id{0};
    std::string ptr;
    std::string deviceId;
    uint64_t size{0};
    uint64_t startTimestamp{0};
    uint64_t endTimestamp{0};
    std::string owner;
    std::string eventType;
    std::string otherAttr;
    uint64_t processId{0};
    uint64_t threadId{0};

    MemoryBlock() = default;
    virtual ~MemoryBlock() = default;
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
}  // namespace Dic::Module::MemoryDetail
#endif  // PROFILER_SERVER_MEMORY_DETAIL_DEFS_H
