/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

#ifndef PROFILER_SERVER_MEMORY_DETAIL_ENTITIES_H
#define PROFILER_SERVER_MEMORY_DETAIL_ENTITIES_H

#include "pch.h"
#include "MemoryDetailDefs.h"

namespace Dic::Module::MemoryDetail {
// [PYTHON_TRACE] 内存调用栈火焰图相关实体
struct PythonTraceSlice {
    int64_t id{0};
    std::string func{"UNKNOWN"};
    uint64_t startTimestamp{0};
    uint64_t endTimestamp{0};
    int depth{-1};
    uint64_t processId{0};
    uint64_t threadId{0};

    PythonTraceSlice() = default;
    PythonTraceSlice(std::string func, uint64_t startTimestamp, uint64_t endTimestamp, int depth)
        : func(std::move(func)),
          startTimestamp(startTimestamp),
          endTimestamp(endTimestamp),
          depth(depth) {}
};
/***
 * 调用栈消减策略:
 * FILTER_OUT_SMALL_FUNCTIONS: 过滤掉小函数(基于duration比例)
 * COMPRESS_SMALL_FUNCTIONS: 压缩同一层级的小函数(同层级小函数 且 间隔较短，则合并成同一个块)
 * COMPRESS_AND_FILTER: 两个策略同时应用
 */
enum class PythonTrimCompressStrategy {
    ONLY_FILTER_OUT_SMALL_FUNCTIONS,
    COMPRESS_SMALL_FUNCTIONS,
    COMPRESS_AND_FILTER_SMALL_FUNCTIONS
};

struct LeaksMemoryPythonTrace {
    uint64_t maxTimestamp{};
    uint64_t minTimestamp{INT64_MAX};
    uint64_t threadId{};
    std::vector<PythonTraceSlice> slices;
    int maxDepth{};

    [[nodiscard]] bool Empty() const;

    /***
     * 基于策略进行压缩
     * @param strategy 压缩策略
     */
    void Trim(const PythonTrimCompressStrategy &strategy);

private:
    [[nodiscard]] bool IsSmallSlice(const PythonTraceSlice &slice) const;
    void DoCompress(std::vector<PythonTraceSlice> &waitForCompressSlices, const bool filterOutSmallFunc);
    void DoCompressByDepth(std::vector<PythonTraceSlice> &depthSlices, const bool filterOutSmallFunc);
};
// [PYTHON_TRACE]

// [EVENT] 事件/内存块扩展属性
inline const std::string BLOCK_EVENT_ATTR_SIZE_FIELD = "size";
inline const std::string BLOCK_EVENT_ATTR_OWNER_FIELD = "owner";
inline const std::string BLOCK_EVENT_ATTR_TOTAL_FIELD = "total";
inline const std::string BLOCK_EVENT_ATTR_USED_FIELD = "used";
inline const std::string BLOCK_EVENT_ATTR_GROUP_ID_FIELD = "allocation_id";
inline const std::string BLOCK_ATTR_FIRST_ACCESS_FILED = "first_access_timestamp";
inline const std::string BLOCK_ATTR_LAST_ACCESS_FILED = "last_access_timestamp";
struct MemoryEventAttrs {
    int64_t size{0}; // 对应内存事件涉及的内存大小, 如申请、释放大小；访问时tensor大小
    std::string owner;
    uint64_t total{0};
    uint64_t used{0};
    uint64_t groupId{0};
};
struct MemoryBlockAttrs {
    int64_t size{0}; // 内存块大小
    uint64_t groupId{0}; // 组id
    uint64_t firstAccessTimestamp{0};
    uint64_t lastAccessTimestamp{0};
    std::map<std::string, std::string> extendAttrs;

    /***
     * 将内存块attr转化为json
     * @return
     */
    std::string ToJsonString();

    /***
     * 从json字符串构建BlockAttrs
     * @param jsonString
     * @return
     */
    static std::optional<MemoryBlockAttrs> FromJson(std::string jsonString);
};
/***
 * 内存事件组
 */
struct EventGroup {
    std::optional<MemoryEvent> mallocEvent{std::nullopt};
    std::optional<MemoryEvent> freeEvent{std::nullopt};
    std::vector<MemoryEvent> accessEvents;

    EventGroup() = default;
    /***
     * 通过events快速构建一个eventGroup, 注意需要events已根据timestamp升序排序
     * @param sortedEvents 事件数组，要求已经过排序
     */
    explicit EventGroup(std::vector<MemoryEvent> &sortedEvents);

    void AddEvent(const MemoryEvent &event);
};
// [EVENT]
} // Dic::Module::MemoryDetail

#endif  // PROFILER_SERVER_MEMORY_DETAIL_ENTITIES_H
