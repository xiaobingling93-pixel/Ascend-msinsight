/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "ProtocolMessage.h"
#include "JsonUtil.h"
#include "MemoryDef.h"
#include "MemoryProtocolUtil.h"

namespace Dic {
namespace Protocol {
using namespace rapidjson;
using namespace Dic::Module::Memory;
#pragma region <<Response to json>>
template <> std::optional<document_t> ToResponseJson<MemoryOperatorComparisonResponse>(
    const MemoryOperatorComparisonResponse &response)
{
    document_t json(kObjectType);
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    auto &allocator = json.GetAllocator();
    json_t body(kObjectType);
    json_t columnAttr(kArrayType);
    bool hasStream;
    for (const auto &attr : response.columnAttr) {
        json_t attrJson = json_t(kObjectType);
        JsonUtil::AddMember(attrJson, "name", attr.name, allocator);
        JsonUtil::AddMember(attrJson, "type", attr.type, allocator);
        JsonUtil::AddMember(attrJson, "key", attr.key, allocator);
        columnAttr.PushBack(attrJson, allocator);
        if (attr.name == "Stream") {
            hasStream = true;
        }
    }
    json_t operatorDiffDetail(kArrayType);
    for (const MemoryOperatorComparison& anOperator : response.operatorDiffDetails) {
        json_t basicJson = json_t(kObjectType);
        std::optional<document_t> jsonCompare = ToMemoryOperatorJson(anOperator.compare, hasStream, allocator);
        std::optional<document_t> jsonBaseline = ToMemoryOperatorJson(anOperator.baseline, hasStream, allocator);
        std::optional<document_t> jsonDiff = ToMemoryOperatorJson(anOperator.diff, hasStream, allocator);
        if (jsonCompare.has_value()) {
            JsonUtil::AddMember(basicJson, "compare", jsonCompare.value(), allocator);
        }
        if (jsonBaseline.has_value()) {
            JsonUtil::AddMember(basicJson, "baseline", jsonBaseline.value(), allocator);
        }
        if (jsonDiff.has_value()) {
            JsonUtil::AddMember(basicJson, "diff", jsonDiff.value(), allocator);
        }
        operatorDiffDetail.PushBack(basicJson, allocator);
    }
    JsonUtil::AddMember(body, "totalNum", response.totalNum, allocator);
    JsonUtil::AddMember(body, "operatorDetail", operatorDiffDetail, allocator);
    JsonUtil::AddMember(body, "columnAttr", columnAttr, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::optional<document_t>{std::move(json)};
}

std::optional<document_t> ToMemoryOperatorJson(const MemoryOperator &op, bool hasStream,
    Document::AllocatorType &allocator)
{
    document_t json(kObjectType);
    JsonUtil::AddMember(json, "id", op.id, allocator);
    if (op.name.empty()) {
        JsonUtil::AddMember(json, "name", "Unknown", allocator);
    } else {
        JsonUtil::AddMember(json, "name", op.name, allocator);
    }
    JsonUtil::AddMember(json, "size", op.size, allocator);
    JsonUtil::AddMember(json, "allocationTime", op.allocationTime, allocator);
    JsonUtil::AddMember(json, "releaseTime", op.releaseTime, allocator);
    JsonUtil::AddMember(json, "duration", op.duration, allocator);
    JsonUtil::AddMember(json, "allocationAllocated", op.allocationAllocated, allocator);
    JsonUtil::AddMember(json, "allocationReserved", op.allocationReserved, allocator);
    JsonUtil::AddMember(json, "releaseAllocated", op.releaseAllocated, allocator);
    JsonUtil::AddMember(json, "releaseReserved", op.releaseReserved, allocator);
    if (hasStream) {
        JsonUtil::AddMember(json, "activeReleaseTime", op.activeReleaseTime, allocator);
        JsonUtil::AddMember(json, "activeDuration", op.activeDuration, allocator);
        JsonUtil::AddMember(json, "allocationActive", op.allocationActive, allocator);
        JsonUtil::AddMember(json, "releaseActive", op.releaseActive, allocator);
        JsonUtil::AddMember(json, "streamId", op.streamId, allocator);
    }
    return std::optional<document_t>{std::move(json)};
}

template <> std::optional<document_t> ToResponseJson<MemoryComponentComparisonResponse>(
    const MemoryComponentComparisonResponse &response)
{
    document_t json(kObjectType);
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    auto &allocator = json.GetAllocator();
    json_t body(kObjectType);
    json_t columnAttr(kArrayType);
    for (const auto &attr : response.columnAttr) {
        json_t attrJson(kObjectType);
        JsonUtil::AddMember(attrJson, "name", attr.name, allocator);
        JsonUtil::AddMember(attrJson, "type", attr.type, allocator);
        JsonUtil::AddMember(attrJson, "key", attr.key, allocator);
        columnAttr.PushBack(attrJson, allocator);
    }
    json_t componentDiffDetail(kArrayType);
    for (const MemoryComponentComparison &aComponent : response.componentDiffDetails) {
        json_t basicJson(kObjectType);
        std::optional<document_t> jsonCompare = ToMemoryComponentJson(aComponent.compare, allocator);
        std::optional<document_t> jsonBaseline = ToMemoryComponentJson(aComponent.baseline, allocator);
        std::optional<document_t> jsonDiff = ToMemoryComponentJson(aComponent.diff, allocator);
        if (jsonCompare.has_value()) {
            JsonUtil::AddMember(basicJson, "compare", jsonCompare.value(), allocator);
        }
        if (jsonBaseline.has_value()) {
            JsonUtil::AddMember(basicJson, "baseline", jsonBaseline.value(), allocator);
        }
        if (jsonDiff.has_value()) {
            JsonUtil::AddMember(basicJson, "diff", jsonDiff.value(), allocator);
        }
        componentDiffDetail.PushBack(basicJson, allocator);
    }
    JsonUtil::AddMember(body, "totalNum", response.totalNum, allocator);
    JsonUtil::AddMember(body, "componentDetail", componentDiffDetail, allocator);
    JsonUtil::AddMember(body, "columnAttr", columnAttr, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::optional<document_t>{std::move(json)};
}

std::optional<document_t> ToMemoryComponentJson(const MemoryComponent &component, Document::AllocatorType &allocator)
{
    document_t json(kObjectType);
    if (component.component.empty()) {
        JsonUtil::AddMember(json, "component", "Unknown", allocator);
    } else {
        JsonUtil::AddMember(json, "component", component.component, allocator);
    }
    JsonUtil::AddMember(json, "timestamp", component.timestamp, allocator);
    JsonUtil::AddMember(json, "totalReserved", component.totalReserved, allocator);
    JsonUtil::AddMember(json, "device", component.device, allocator);
    return std::optional<document_t>{std::move(json)};
}

std::optional<document_t> ToLeaksMemoryAllocationJson(const MemoryAllocation &allocation,
                                                      Document::AllocatorType &allocator)
{
    document_t json(kObjectType);
    JsonUtil::AddMember(json, "id", allocation.id, allocator);
    JsonUtil::AddMember(json, "timestamp", allocation.timestamp, allocator);
    JsonUtil::AddMember(json, "totalSize", allocation.totalSize, allocator);
    return std::optional<document_t>{std::move(json)};
}

std::optional<document_t> ToLeaksMemoryBlockJson(const MemoryBlockItem &blockItem, Document::AllocatorType &allocator)
{
    document_t json(kObjectType);
    JsonUtil::AddMember(json, "id", blockItem.id, allocator);
    JsonUtil::AddMember(json, "addr", blockItem.ptr, allocator);
    JsonUtil::AddMember(json, "size", blockItem.size, allocator);
    JsonUtil::AddMember(json, "startTimestamp", blockItem.startTimestamp, allocator);
    JsonUtil::AddMember(json, "endTimestamp", blockItem.endTimestamp, allocator);
    JsonUtil::AddMember(json, "owner", blockItem.owner, allocator);
    JsonUtil::AddMember(json, "attr", blockItem.otherAttr, allocator);
    document_t path(kArrayType);
    for (auto &point : blockItem.path) {
        document_t pointJson(kArrayType);
        pointJson.PushBack(json_t().SetUint64(point.first), allocator);
        pointJson.PushBack(json_t().SetUint64(point.second), allocator);
        path.PushBack(pointJson, allocator);
    }
    JsonUtil::AddMember(json, "path", path, allocator);
    return std::optional<document_t>{std::move(json)};
}
// 此处转json存在递归，但treeNode在构造时确保了层数不超过8
std::optional<document_t> ToLeaksDetailTreeJson(const LeaksMemoryDetailTreeNode &treeNode,
                                                Document::AllocatorType &allocator)
{
    document_t json(kObjectType);
    JsonUtil::AddMember(json, "name", treeNode.name, allocator);
    JsonUtil::AddMember(json, "tag", treeNode.tag, allocator);
    JsonUtil::AddMember(json, "size", treeNode.size, allocator);
    json_t subNodes(kArrayType);
    for (auto &subNode : treeNode.subNodes) {
        auto subNodeJson = ToLeaksDetailTreeJson(subNode, allocator);
        if (subNodeJson.has_value()) {
            subNodes.PushBack(subNodeJson.value(), allocator);
        }
    }
    JsonUtil::AddMember(json, "subNodes", subNodes, allocator);
    return std::optional<document_t>{std::move(json)};
}

std::optional<document_t> ToLeaksTracesJson(const std::vector<PythonTraceSlice> &slices,
                                            Document::AllocatorType &allocator)
{
    document_t json(kArrayType);
    for (auto &slice : slices) {
        document_t traceJson(kObjectType);
        JsonUtil::AddMember(traceJson, "func", slice.func, allocator);
        JsonUtil::AddMember(traceJson, "startTimestamp", slice.startTimestamp, allocator);
        JsonUtil::AddMember(traceJson, "endTimestamp", slice.endTimestamp, allocator);
        JsonUtil::AddMember(traceJson, "depth", slice.depth, allocator);
        json.PushBack(traceJson, allocator);
    }
    return std::optional<document_t>{std::move(json)};
}

template <> std::optional<document_t> ToResponseJson<MemoryViewResponse>(const MemoryViewResponse &response)
{
    document_t json(kObjectType);
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    auto &allocator = json.GetAllocator();
    json_t body(kObjectType);
    json_t legends(kArrayType);
    for (const auto& legend : response.data.legends) {
        legends.PushBack(json_t().SetString(legend.c_str(), allocator), allocator);
    }
    json_t linesList(kArrayType);
    for (const auto &lines: response.data.lines) {
        json_t lineArr(kArrayType);
        for (const auto &line: lines) {
            lineArr.PushBack(json_t().SetString(line.c_str(), allocator), allocator);
        }
        linesList.PushBack(lineArr, allocator);
    }
    JsonUtil::AddMember(body, "legends", legends, allocator);
    JsonUtil::AddMember(body, "lines", linesList, allocator);
    JsonUtil::AddMember(body, "title", response.data.title, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::optional<document_t>{std::move(json)};
}

template<>
std::optional<document_t> ToResponseJson<MemoryOperatorSizeResponse>(const MemoryOperatorSizeResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "minSize", response.size.minSize, allocator);
    JsonUtil::AddMember(body, "maxSize", response.size.maxSize, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::optional<document_t>{std::move(json)};
}


template <> std::optional<document_t> ToResponseJson<MemoryFindSliceResponse>(const MemoryFindSliceResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "id", response.data.id, allocator);
    JsonUtil::AddMember(body, "rankId", response.data.rankId, allocator);
    JsonUtil::AddMember(body, "processId", response.data.processId, allocator);
    JsonUtil::AddMember(body, "threadId", response.data.threadId, allocator);
    JsonUtil::AddMember(body, "metaType", response.data.metaType, allocator);
    JsonUtil::AddMember(body, "depth", response.data.depth, allocator);
    JsonUtil::AddMember(body, "startTime", response.data.startTime, allocator);
    JsonUtil::AddMember(body, "duration", response.data.duration, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::optional<document_t>{std::move(json)};
}

template <> std::optional<document_t> ToResponseJson<MemoryTypeResponse>(const MemoryTypeResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t graphIdList(kArrayType);
    for (const auto &graphId: response.graphId) {
        graphIdList.PushBack(json_t().SetString(graphId.c_str(), allocator), allocator);
    }
    JsonUtil::AddMember(body, "type", response.type, allocator);
    JsonUtil::AddMember(body, "graphId", graphIdList, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::optional<document_t>{std::move(json)};
}

template<>
std::optional<document_t> ToResponseJson<MemoryResourceTypeResponse>(const MemoryResourceTypeResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "type", response.type, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::optional<document_t>{std::move(json)};
}

template<>
std::optional<document_t> ToResponseJson<MemoryStaticOperatorGraphResponse>
        (const MemoryStaticOperatorGraphResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t legends(kArrayType);
    for (const auto& legend : response.data.legends) {
        legends.PushBack(json_t().SetString(legend.c_str(), allocator), allocator);
    }
    json_t linesList(kArrayType);
    for (const auto &lines: response.data.lines) {
        json_t lineArr(kArrayType);
        for (const auto &line: lines) {
            lineArr.PushBack(json_t().SetString(line.c_str(), allocator), allocator);
        }
        linesList.PushBack(lineArr, allocator);
    }
    JsonUtil::AddMember(body, "legends", legends, allocator);
    JsonUtil::AddMember(body, "lines", linesList, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::optional<document_t>{std::move(json)};
}

template<>
std::optional<document_t> ToResponseJson<MemoryStaticOperatorListCompResponse>
        (const MemoryStaticOperatorListCompResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    json_t columnAttr(kArrayType);
    for (const auto &attr : response.columnAttr) {
        json_t attrJson = json_t(kObjectType);
        JsonUtil::AddMember(attrJson, "name", attr.name, allocator);
        JsonUtil::AddMember(attrJson, "type", attr.type, allocator);
        JsonUtil::AddMember(attrJson, "key", attr.key, allocator);
        columnAttr.PushBack(attrJson, allocator);
    }
    json_t operatorDiffDetail(kArrayType);
    for (const StaticOperatorCompItem& anOperator : response.operatorDiffDetails) {
        json_t basicJson = json_t(kObjectType);
        std::optional<document_t> jsonCompare = ToMemoryStaticOperatorJson(anOperator.compare, allocator);
        std::optional<document_t> jsonBaseline = ToMemoryStaticOperatorJson(anOperator.baseline, allocator);
        std::optional<document_t> jsonDiff = ToMemoryStaticOperatorJson(anOperator.diff, allocator);
        if (jsonCompare.has_value()) {
            JsonUtil::AddMember(basicJson, "compare", jsonCompare.value(), allocator);
        }
        if (jsonBaseline.has_value()) {
            JsonUtil::AddMember(basicJson, "baseline", jsonBaseline.value(), allocator);
        }
        if (jsonDiff.has_value()) {
            JsonUtil::AddMember(basicJson, "diff", jsonDiff.value(), allocator);
        }
        operatorDiffDetail.PushBack(basicJson, allocator);
    }
    JsonUtil::AddMember(body, "totalNum", response.totalNum, allocator);
    JsonUtil::AddMember(body, "operatorDetail", operatorDiffDetail, allocator);
    JsonUtil::AddMember(body, "columnAttr", columnAttr, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::optional<document_t>{std::move(json)};
}

std::optional<document_t> ToMemoryStaticOperatorJson(const StaticOperatorItem &op,
    Document::AllocatorType &allocator)
{
    document_t json(kObjectType);
    JsonUtil::AddMember(json, "deviceId", op.deviceId, allocator);
    JsonUtil::AddMember(json, "opName", op.opName, allocator);
    JsonUtil::AddMember(json, "nodeIndexStart", op.nodeIndexStart, allocator);
    JsonUtil::AddMember(json, "nodeIndexEnd", op.nodeIndexEnd, allocator);
    JsonUtil::AddMember(json, "size", op.size, allocator);
    return std::optional<document_t>{std::move(json)};
}

template<>
std::optional<document_t> ToResponseJson<MemoryStaticOperatorSizeResponse>
    (const MemoryStaticOperatorSizeResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t body(kObjectType);
    JsonUtil::AddMember(body, "minSize", response.size.minSize, allocator);
    JsonUtil::AddMember(body, "maxSize", response.size.maxSize, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::optional<document_t>{std::move(json)};
}

template<>
std::optional<document_t> ToResponseJson<LeaksMemoryAllocationsResponse>
    (const LeaksMemoryAllocationsResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t &moduleName = json["moduleName"];
    moduleName.SetString(Protocol::MODULE_LEAKS.c_str(), allocator);
    json_t body(kObjectType);
    json_t allocations(kArrayType);
    JsonUtil::AddMember(body, "minTimestamp", response.minTimestamp, allocator);
    JsonUtil::AddMember(body, "maxTimestamp", response.maxTimestamp, allocator);
    for (const auto& allocation : response.allocations) {
        auto allocationJson = ToLeaksMemoryAllocationJson(allocation, allocator);
        if (allocationJson.has_value()) {
            allocations.PushBack(allocationJson.value(), allocator);
        }
    }
    JsonUtil::AddMember(body, "allocations", allocations, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::optional<document_t>{std::move(json)};
}
template<>
std::optional<document_t> ToResponseJson<LeaksMemoryBlocksResponse>
                (const LeaksMemoryBlocksResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t &moduleName = json["moduleName"];
    moduleName.SetString(Protocol::MODULE_LEAKS.c_str(), allocator);
    json_t body(kObjectType);
    json_t blocks(kArrayType);
    JsonUtil::AddMember(body, "minTimestamp", response.minTimestamp, allocator);
    JsonUtil::AddMember(body, "maxTimestamp", response.maxTimestamp, allocator);
    JsonUtil::AddMember(body, "minSize", response.minSize, allocator);
    JsonUtil::AddMember(body, "maxSize", response.maxSize, allocator);
    for (const auto &block : response.blocks) {
        auto blockJson = ToLeaksMemoryBlockJson(block, allocator);
        if (blockJson.has_value()) {
            blocks.PushBack(blockJson.value(), allocator);
        }
    }
    JsonUtil::AddMember(body, "blocks", blocks, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::optional<document_t>{std::move(json)};
}

template<>
std::optional<document_t> ToResponseJson<LeaksMemoryDetailsResponse>
        (const LeaksMemoryDetailsResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t &moduleName = json["moduleName"];
    moduleName.SetString(Protocol::MODULE_LEAKS.c_str(), allocator);
    auto body_json = ToLeaksDetailTreeJson(response.detail, allocator);
    if (!body_json.has_value()) {
        body_json = document_t(kObjectType);
    }
    JsonUtil::AddMember(json, "body", body_json.value(), allocator);
    return std::optional<document_t>{std::move(json)};
}

template<>
std::optional<document_t> ToResponseJson<LeaksMemoryTracesResponse>
                (const LeaksMemoryTracesResponse &response)
{
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    ProtocolUtil::SetResponseJsonBaseInfo(response, json);
    json_t &moduleName = json["moduleName"];
    moduleName.SetString(Protocol::MODULE_LEAKS.c_str(), allocator);
    document_t body(kObjectType);
    auto tracesJson = ToLeaksTracesJson(response.trace.slices, allocator);
    if (!tracesJson.has_value()) {
        tracesJson = document_t(kObjectType);
    }
    JsonUtil::AddMember(body, "traces", tracesJson.value(), allocator);
    JsonUtil::AddMember(body, "minTimestamp", response.trace.minTimestamp, allocator);
    JsonUtil::AddMember(body, "maxTimestamp", response.trace.maxTimestamp, allocator);
    JsonUtil::AddMember(json, "threadId", response.trace.threadId, allocator);
    JsonUtil::AddMember(json, "body", body, allocator);
    return std::optional<document_t>{std::move(json)};
}
#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic