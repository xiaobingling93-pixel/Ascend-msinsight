/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_MEMORYPROTOCLRESPOSE_H
#define PROFILER_SERVER_MEMORYPROTOCLRESPOSE_H

#include <vector>
#include "GlobalDefs.h"
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"
#include "MemoryDef.h"
#include "LeaksMemoryDetailTreeNode.h"
#include "LeaksMemoryPythonTrace.h"

namespace Dic {
namespace Protocol {
using namespace Dic::Module::Memory;
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
    std::string id;
};

struct MemoryOperatorComparison {
    MemoryOperator compare;
    MemoryOperator baseline;
    MemoryOperator diff;
};

struct MemoryComponent {
    std::string component;
    std::string timestamp;
    double totalReserved;
    std::string device;
};

struct MemoryComponentComparison {
    MemoryComponent compare;
    MemoryComponent baseline;
    MemoryComponent diff;
};

struct StaticOperatorItem {
    std::string deviceId;
    std::string opName;
    int64_t nodeIndexStart;
    int64_t nodeIndexEnd;
    double size;
};

struct StaticOperatorCompItem {
    StaticOperatorItem compare;
    StaticOperatorItem baseline;
    StaticOperatorItem diff;
};

struct StaticOperatorGraphItem {
    std::vector<std::string> legends;
    std::vector<std::vector<std::string>> lines;
};

struct MemoryViewData {
    std::string title;
    std::vector<std::string> legends;
    std::vector<std::vector<std::string>> lines;
};

struct SliceInfo {
    std::string id;
    std::string rankId;
    std::string processId;
    std::string threadId;
    std::string metaType;
    uint32_t depth = 0;
    uint64_t startTime = 0;
    uint64_t duration = 0;
};

struct MemoryTableColumnAttr {
    std::string name;
    std::string type;
    std::string key;
};

struct MemoryTypeResponse : public Response {
    MemoryTypeResponse() : Response(REQ_RES_MEMORY_TYPE) {}
    std::string type = Module::Memory::MEMORY_TYPE_DYNAMIC;
    std::vector<std::string> graphId;
};

struct MemoryResourceTypeResponse : public Response {
    MemoryResourceTypeResponse() : Response(REQ_RES_MEMORY_RESOURCE_TYPE) {}
    std::string type = Module::Memory::MEMORY_RESOURCE_TYPE_PYTORCH;
};

struct MemoryOperatorComparisonResponse : public Response {
    MemoryOperatorComparisonResponse() : Response(REQ_RES_MEMORY_OPERATOR) {}
    std::vector<MemoryTableColumnAttr> columnAttr;
    std::vector<MemoryOperatorComparison> operatorDiffDetails;
    int64_t totalNum = 0;
};

struct MemoryComponentComparisonResponse : public Response {
    MemoryComponentComparisonResponse() : Response(REQ_RES_MEMORY_COMPONENT) {}
    std::vector<MemoryTableColumnAttr> columnAttr;
    std::vector<MemoryComponentComparison> componentDiffDetails;
    int64_t totalNum = 0;
};

struct MemoryFindSliceResponse : public Response {
    MemoryFindSliceResponse() : Response(REQ_RES_MEMORY_FIND_SLICE) {}
    SliceInfo data;
};

struct MemoryViewResponse : public Response {
    MemoryViewResponse() : Response(REQ_RES_MEMORY_VIEW) {}
    MemoryViewData data;
};

struct OperatorSize {
    double minSize = 0.0;
    double maxSize = 0.0;
};

struct StaticOperatorSize {
    double minSize = 0.0;
    double maxSize = 0.0;
};

struct MemoryOperatorSizeResponse : public Response {
    MemoryOperatorSizeResponse() : Response(REQ_RES_MEMORY_OPERATOR_MIN_MAX) {}
    OperatorSize size;
};

struct MemoryStaticOperatorGraphResponse : public Response {
    MemoryStaticOperatorGraphResponse() : Response(REQ_RES_MEMORY_STATIC_OP_MEMORY_GRAPH) {}
    StaticOperatorGraphItem data;
};

struct MemoryStaticOperatorListCompResponse : public Response {
    MemoryStaticOperatorListCompResponse() : Response(REQ_RES_MEMORY_STATIC_OP_MEMORY_LIST) {}
    std::vector<MemoryTableColumnAttr> columnAttr;
    std::vector<StaticOperatorCompItem> operatorDiffDetails;
    int64_t totalNum = 0;
};

struct MemoryStaticOperatorSizeResponse : public Response {
    MemoryStaticOperatorSizeResponse() : Response(REQ_RES_MEMORY_STATIC_OP_MEMORY_MIN_MAX) {}
    StaticOperatorSize size;
};
struct MemoryBlockItem : Dic::Module::Memory::MemoryBlock {
    std::vector<std::pair<std::uint64_t, std::uint64_t>> path;
    MemoryBlockItem() = default;
    explicit MemoryBlockItem(const MemoryBlock &block)
        : MemoryBlock(block) {}

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
        std::pair<std::uint64_t, std::uint64_t> secondLastPoint = path[path.size()-2];
        if (size == lastPoint.second && size == secondLastPoint.second) {
            // 如果倒数第二个点、最后一个点、和新加入的点y值都相同, 则中间点可省略
            path.back().first = timestamp;
            return;
        }
        path.emplace_back(timestamp, size);
    }
};
struct LeaksMemoryBlocksResponse : public Response {
    LeaksMemoryBlocksResponse()
        : Response (REQ_RES_LEAKS_MEMORY_BLOCKS),
          minTimestamp(0),
          maxTimestamp(0),
          minSize(0),
          maxSize(0),
          totalNum(0) {}

    std::vector<MemoryBlockItem> blocks;
    uint64_t minTimestamp;
    uint64_t maxTimestamp;
    uint64_t minSize;
    uint64_t maxSize;
    uint64_t totalNum;
};

struct LeaksMemoryAllocationsResponse : public Response {
    LeaksMemoryAllocationsResponse()
        : Response(REQ_RES_LEAKS_MEMORY_ALLOCATIONS),
          minTimestamp(0),
          maxTimestamp(0) {}
    uint64_t minTimestamp;
    uint64_t maxTimestamp;
    std::vector<Dic::Module::Memory::MemoryAllocation> allocations;
};

struct LeaksMemoryDetailsResponse : public Response {
    LeaksMemoryDetailsResponse()
        : Response(REQ_RES_LEAKS_MEMORY_DETAILS),
          timestamp(0) {}
    uint64_t timestamp;
    LeaksMemoryDetailTreeNode detail;
};

struct LeaksMemoryTracesResponse : public Response {
    LeaksMemoryTracesResponse()
        : Response(REQ_RES_LEAKS_MEMORY_TRACES) {}
    LeaksMemoryPythonTrace trace;
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
    bool hasPta = false;
    bool hasGe = false;
    bool hasApp = false;
    bool hasWorkspace = false;
    double ptaGeAllocated = 0;
    double ptaGeReserved = 0;
    double ptaGeActivated = 0;
    double ptaAllocated = 0;
    double ptaReserved = 0;
    double ptaActivated = 0;
    double geAllocated = 0;
    double geReserved = 0;
    double geActivated = 0;
    double appReserved = 0;
};
} // end of namespace Protocol
} // end of namespace Dic
#endif // PROFILER_SERVER_MEMORYPROTOCLRESPOSE_H
