/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/
#ifndef PROFILER_SERVER_MEMORY_DETAIL_PROTOCOL_RESPONSE_H
#define PROFILER_SERVER_MEMORY_DETAIL_PROTOCOL_RESPONSE_H
#include <vector>
#include "GlobalDefs.h"
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"
#include "MemoryDetailDefs.h"
#include "LeaksMemoryDetailTreeNode.h"
#include "LeaksMemoryPythonTrace.h"

namespace Dic::Protocol {
using namespace Dic::Module::MemoryDetail;
struct MemoryBlockItem : MemoryBlock {
    std::vector<std::pair<std::uint64_t, std::uint64_t>> path;
    MemoryBlockItem() = default;
    explicit MemoryBlockItem(const MemoryBlock& block) : MemoryBlock(block) {}

    void AddPathPoint(uint64_t timestamp, uint64_t size)
    {
        if (path.empty()) {
            path.emplace_back(timestamp, size);
            return;
        }
        std::pair<std::uint64_t, std::uint64_t> lastPoint = path.back();
        // 如果新加入的点x值与最后一个点x值相同, 或者x值在最后一个点的x之前 视为无效点
        if (timestamp <= lastPoint.first) {
            return;
        }
        if (path.size() == 1) {
            path.emplace_back(timestamp, size);
            return;
        }
        // 如果新加入的点, y值与最后一个点的y值相同
        std::pair<std::uint64_t, std::uint64_t> secondLastPoint = path[path.size() - 2];
        if (size == lastPoint.second && size == secondLastPoint.second) {
            // 如果倒数第二个点、最后一个点、和新加入的点y值都相同, 则中间点可省略
            path.back().first = timestamp;
            return;
        }
        path.emplace_back(timestamp, size);
    }

    std::optional<document_t> ToLeaksMemoryBlockJson(Document::AllocatorType& allocator) const
    {
        document_t json(kObjectType);
        JsonUtil::AddMember(json, "id", id, allocator);
        JsonUtil::AddMember(json, "addr", ptr, allocator);
        JsonUtil::AddMember(json, "size", size, allocator);
        JsonUtil::AddMember(json, "startTimestamp", startTimestamp, allocator);
        JsonUtil::AddMember(json, "endTimestamp", endTimestamp, allocator);
        JsonUtil::AddMember(json, "owner", owner, allocator);
        JsonUtil::AddMember(json, "attr", otherAttr, allocator);
        JsonUtil::AddMember(json, "processId", processId, allocator);
        JsonUtil::AddMember(json, "threadId", threadId, allocator);
        document_t pathJson(kArrayType);
        for (auto& point : path) {
            document_t pointJson(kArrayType);
            pointJson.PushBack(json_t().SetUint64(point.first), allocator);
            pointJson.PushBack(json_t().SetUint64(point.second), allocator);
            pathJson.PushBack(pointJson, allocator);
        }
        JsonUtil::AddMember(json, "path", pathJson, allocator);
        return std::optional<document_t>{std::move(json)};
    }
};
struct LeaksMemoryBlocksResponse : public JsonResponse {
    LeaksMemoryBlocksResponse()
        : JsonResponse(REQ_RES_LEAKS_MEMORY_BLOCKS),
          minTimestamp(0),
          maxTimestamp(0),
          minSize(0),
          maxSize(0),
          totalNum(0)
    {
    }

    std::vector<MemoryBlockItem> blocks;
    uint64_t minTimestamp;
    uint64_t maxTimestamp;
    uint64_t minSize;
    uint64_t maxSize;
    uint64_t totalNum;

    [[nodiscard]] std::optional<document_t> ToJson() const override
    {
        document_t json(kObjectType);
        auto& allocator = json.GetAllocator();
        ProtocolUtil::SetResponseJsonBaseInfo(*this, json);
        json_t body(kObjectType);
        json_t jsonBlocks(kArrayType);
        JsonUtil::AddMember(body, "minTimestamp", minTimestamp, allocator);
        JsonUtil::AddMember(body, "maxTimestamp", maxTimestamp, allocator);
        JsonUtil::AddMember(body, "minSize", minSize, allocator);
        JsonUtil::AddMember(body, "maxSize", maxSize, allocator);
        for (const auto& block : blocks) {
            auto blockJson = block.ToLeaksMemoryBlockJson(allocator);
            if (blockJson.has_value()) {
                jsonBlocks.PushBack(blockJson.value(), allocator);
            }
        }
        JsonUtil::AddMember(body, "blocks", jsonBlocks, allocator);
        JsonUtil::AddMember(json, "body", body, allocator);
        return std::optional<document_t>{std::move(json)};
    }
};

struct LeaksMemoryAllocationsResponse : public JsonResponse {
    LeaksMemoryAllocationsResponse() : JsonResponse(REQ_RES_LEAKS_MEMORY_ALLOCATIONS), minTimestamp(0), maxTimestamp(0)
    {
    }
    uint64_t minTimestamp;
    uint64_t maxTimestamp;
    std::vector<MemoryAllocation> allocations;

    [[nodiscard]] std::optional<document_t> ToJson() const override
    {
        document_t json(kObjectType);
        auto& allocator = json.GetAllocator();
        ProtocolUtil::SetResponseJsonBaseInfo(*this, json);
        json_t body(kObjectType);
        json_t allocationsJson(kArrayType);
        JsonUtil::AddMember(body, "minTimestamp", minTimestamp, allocator);
        JsonUtil::AddMember(body, "maxTimestamp", maxTimestamp, allocator);
        for (const auto& allocation : allocations) {
            auto allocationJson = ToLeaksMemoryAllocationJson(allocation, allocator);
            if (allocationJson.has_value()) {
                allocationsJson.PushBack(allocationJson.value(), allocator);
            }
        }
        JsonUtil::AddMember(body, "allocations", allocationsJson, allocator);
        JsonUtil::AddMember(json, "body", body, allocator);
        return std::optional<document_t>{std::move(json)};
    }

    static std::optional<document_t> ToLeaksMemoryAllocationJson(const MemoryAllocation& allocation,
                                                                 Document::AllocatorType& allocator)
    {
        document_t json(kObjectType);
        JsonUtil::AddMember(json, "id", allocation.id, allocator);
        JsonUtil::AddMember(json, "timestamp", allocation.timestamp, allocator);
        JsonUtil::AddMember(json, "totalSize", allocation.totalSize, allocator);
        return std::optional<document_t>{std::move(json)};
    }
};

struct LeaksMemoryDetailsResponse : public JsonResponse {
    LeaksMemoryDetailsResponse() : JsonResponse(REQ_RES_LEAKS_MEMORY_DETAILS), timestamp(0) {}
    uint64_t timestamp;
    LeaksMemoryDetailTreeNode detail;

    [[nodiscard]] std::optional<document_t> ToJson() const override
    {
        document_t json(kObjectType);
        auto& allocator = json.GetAllocator();
        ProtocolUtil::SetResponseJsonBaseInfo(*this, json);
        auto body_json = ToLeaksDetailTreeJson(detail, allocator);
        if (!body_json.has_value()) {
            body_json = document_t(kObjectType);
        }
        JsonUtil::AddMember(json, "body", body_json.value(), allocator);
        return std::optional<document_t>{std::move(json)};
    }

    // 此处转json存在递归，但treeNode在构造时确保了层数不超过8
    static std::optional<document_t> ToLeaksDetailTreeJson(const LeaksMemoryDetailTreeNode& treeNode,
                                                           Document::AllocatorType& allocator)
    {
        document_t json(kObjectType);
        JsonUtil::AddMember(json, "name", treeNode.name, allocator);
        JsonUtil::AddMember(json, "tag", treeNode.tag, allocator);
        JsonUtil::AddMember(json, "size", treeNode.size, allocator);
        json_t subNodes(kArrayType);
        for (auto& subNode : treeNode.subNodes) {
            auto subNodeJson = ToLeaksDetailTreeJson(subNode, allocator);
            if (subNodeJson.has_value()) {
                subNodes.PushBack(subNodeJson.value(), allocator);
            }
        }
        JsonUtil::AddMember(json, "subNodes", subNodes, allocator);
        return std::optional<document_t>{std::move(json)};
    }
};

struct LeaksMemoryTracesResponse : public JsonResponse {
    LeaksMemoryTracesResponse() : JsonResponse(REQ_RES_LEAKS_MEMORY_TRACES) {}
    LeaksMemoryPythonTrace trace;
    [[nodiscard]] std::optional<document_t> ToJson() const override
    {
        document_t json(kObjectType);
        auto& allocator = json.GetAllocator();
        ProtocolUtil::SetResponseJsonBaseInfo(*this, json);
        document_t body(kObjectType);
        auto tracesJson = ToLeaksTracesJson(trace.slices, allocator);
        if (!tracesJson.has_value()) {
            tracesJson = document_t(kObjectType);
        }
        JsonUtil::AddMember(body, "traces", tracesJson.value(), allocator);
        JsonUtil::AddMember(body, "minTimestamp", trace.minTimestamp, allocator);
        JsonUtil::AddMember(body, "maxTimestamp", trace.maxTimestamp, allocator);
        JsonUtil::AddMember(json, "threadId", trace.threadId, allocator);
        JsonUtil::AddMember(json, "body", body, allocator);
        return std::optional<document_t>{std::move(json)};
    }

    static std::optional<document_t> ToLeaksTracesJson(const std::vector<PythonTraceSlice>& slices,
                                                       Document::AllocatorType& allocator)
    {
        document_t json(kArrayType);
        for (auto& slice : slices) {
            document_t traceJson(kObjectType);
            JsonUtil::AddMember(traceJson, "func", slice.func, allocator);
            JsonUtil::AddMember(traceJson, "startTimestamp", slice.startTimestamp, allocator);
            JsonUtil::AddMember(traceJson, "endTimestamp", slice.endTimestamp, allocator);
            JsonUtil::AddMember(traceJson, "depth", slice.depth, allocator);
            json.PushBack(traceJson, allocator);
        }
        return std::optional<document_t>{std::move(json)};
    }
};
}  // end of namespace Dic::Protocol
#endif  // PROFILER_SERVER_MEMORY_DETAIL_PROTOCOL_RESPONSE_H
