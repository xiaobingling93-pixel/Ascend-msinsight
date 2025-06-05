/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_MEMORYPROTOCOLREQUEST_H
#define PROFILER_SERVER_MEMORYPROTOCOLREQUEST_H

#include <string>
#include <optional>
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"

namespace Dic {
namespace Protocol {

const std::string MEMORY_OVERALL_GROUP = "Overall";
const std::string MEMORY_STREAM_GROUP = "Stream";
const std::string MEMORY_COMPONENT_GROUP = "Component";
const std::vector<std::string> operatorTableColumn = {
    "name", "size", "allocation_time", "release_time", "duration", "active_release_time", "active_duration",
    "allocation_allocated", "allocation_reserve", "allocation_active", "release_allocated", "release_reserve",
    "release_active", "stream", "device_type"
};
const std::vector<std::string> componentTableColumn = {
    "component", "timestamp", "totalReserved", "device"
};
const std::vector<std::string> staticOperatorTableColumn = {
    "device_id", "op_name", "node_index_start", "node_index_end", "size"
};
const uint64_t MAX_LEAKS_MEMORY_BLOCK_SIZE = 1 * 1024 * 1024 * 1024;

struct MemoryOperatorParams {
    std::string rankId;
    std::string deviceId;
    std::string type;
    std::string searchName;
    int64_t minSize;
    int64_t maxSize;
    double startTime;
    double endTime;
    int64_t currentPage = 0;
    int64_t pageSize = 0;
    std::string orderBy;
    std::string order;
    bool isCompare = false;
    bool isOnlyShowAllocatedOrReleasedWithinInterval = false;
    bool CommonCheck(std::string &errorMsg, uint64_t minTimeStamp)
    {
        if (!CheckStrParamValid(rankId, errorMsg)) {
            return false;
        }
        if (!CheckStrParamValidEmptyAllowed(deviceId, errorMsg)) {
            return false;
        }
        if (type != MEMORY_OVERALL_GROUP && type != MEMORY_STREAM_GROUP) {
            errorMsg = "Group By parameter should be Overall or Stream.";
            return false;
        }
        if (isCompare && type == MEMORY_STREAM_GROUP) {
            errorMsg = "Memory comparing does not support request type Stream.";
            return false;
        }
        if (!CheckStrParamValidEmptyAllowed(searchName, errorMsg)) {
            return false;
        }
        if (minSize > maxSize) {
            errorMsg = "Min Size should be smaller or equal to Max Size.";
            return false;
        }
        if (startTime > endTime) {
            errorMsg = "Start Time should be smaller or equal to End Time.";
            return false;
        }
        if (endTime > std::numeric_limits<uint64_t>::max() - minTimeStamp) {
            errorMsg = "End Time is invalid.";
            return false;
        }
        if (!CheckPageValid(pageSize, currentPage, errorMsg)) {
            return false;
        }
        if (!order.empty() && order != "ascend" && order != "descend") {
            errorMsg = "Order parameter is not legal";
            return false;
        }
        if (!orderBy.empty() && std::find(operatorTableColumn.begin(), operatorTableColumn.end(), orderBy) ==
            operatorTableColumn.end()) {
            errorMsg = "Order By parameter is not legal.";
            return false;
        }
        return true;
    }
};

struct MemoryOperatorSizeParams {
    std::string rankId;
    std::string deviceId;
    std::string type;
    bool isCompare = false;
    bool CommonCheck(std::string &errorMsg)
    {
        if (!CheckStrParamValid(rankId, errorMsg)) {
            return false;
        }
        if (!CheckStrParamValidEmptyAllowed(deviceId, errorMsg)) {
            return false;
        }
        if (type != MEMORY_OVERALL_GROUP && type != MEMORY_STREAM_GROUP) {
            errorMsg = "Group By parameter should be Overall or Stream.";
            return false;
        }
        if (isCompare && type == MEMORY_STREAM_GROUP) {
            errorMsg = "Memory comparing does not support request type Stream.";
            return false;
        }
        return true;
    }
};

struct StaticOperatorListParams {
    std::string rankId;
    std::string graphId;
    std::string searchName;
    int64_t minSize;
    int64_t maxSize;
    int64_t startNodeIndex;
    int64_t endNodeIndex;
    int64_t currentPage = 0;
    int64_t pageSize = 0;
    std::string orderBy;
    std::string order;
    bool isCompare = false;
    bool CommonCheck(std::string &errorMsg)
    {
        if (!CheckStrParamValid(rankId, errorMsg)) {
            return false;
        }
        if (!CheckStrParamValidEmptyAllowed(graphId, errorMsg)) {
            return false;
        }
        if (!CheckStrParamValidEmptyAllowed(searchName, errorMsg)) {
            return false;
        }
        if (minSize > maxSize) {
            errorMsg = "Min Size should be smaller or equal to Max Size.";
            return false;
        }
        if (startNodeIndex > endNodeIndex) {
            errorMsg = "Start Node Index should be smaller or equal to End Node Index.";
            return false;
        }
        if (!CheckPageValid(pageSize, currentPage, errorMsg)) {
            return false;
        }
        if (!order.empty() && order != "ascend" && order != "descend") {
            errorMsg = "Order parameter is not legal";
            return false;
        }
        if (!orderBy.empty() && std::find(staticOperatorTableColumn.begin(), staticOperatorTableColumn.end(),
            orderBy) == operatorTableColumn.end()) {
            errorMsg = "Order By parameter is not legal.";
            return false;
        }
        return true;
    }
};

struct StaticOperatorGraphParams {
    std::string rankId;
    std::string modelName;
    std::string graphId;
    bool isCompare = false;
    bool CommonCheck(std::string &errorMsg)
    {
        if (!CheckStrParamValid(rankId, errorMsg)) {
            return false;
        }
        if (!CheckStrParamValidEmptyAllowed(modelName, errorMsg)) {
            return false;
        }
        if (!CheckStrParamValidEmptyAllowed(graphId, errorMsg)) {
            return false;
        }
        return true;
    }
};

struct StaticOperatorSizeParams {
    std::string rankId;
    std::string graphId;
    bool isCompare = false;
    bool CommonCheck(std::string &errorMsg)
    {
        if (!CheckStrParamValid(rankId, errorMsg)) {
            return false;
        }
        if (!CheckStrParamValidEmptyAllowed(graphId, errorMsg)) {
            return false;
        }
        return true;
    }
};

struct MemoryTypeRequest : public Request {
    MemoryTypeRequest() : Request(REQ_RES_MEMORY_TYPE) {};
    std::string rankId;
};

struct MemoryResourceTypeRequest : public Request {
    MemoryResourceTypeRequest() : Request(REQ_RES_MEMORY_RESOURCE_TYPE) {};
    std::string rankId;
};

struct MemoryOperatorRequest : public Request {
    MemoryOperatorRequest() : Request(REQ_RES_MEMORY_OPERATOR) {};
    MemoryOperatorParams params;
};

struct MemoryOperatorSizeRequest : public Request {
    MemoryOperatorSizeRequest() : Request(REQ_RES_MEMORY_OPERATOR_MIN_MAX) {};
    MemoryOperatorSizeParams params;
};

struct MemoryComponentParams {
    std::string rankId;
    std::string deviceId;
    int64_t currentPage = 0;
    int64_t pageSize = 0;
    std::string orderBy;
    std::string order;
    bool isCompare = false;
    bool CommonCheck(std::string &errorMsg)
    {
        if (!CheckStrParamValid(rankId, errorMsg)) {
            return false;
        }
        if (!CheckStrParamValidEmptyAllowed(deviceId, errorMsg)) {
            return false;
        }
        if (!CheckPageValid(pageSize, currentPage, errorMsg)) {
            return false;
        }
        if (!order.empty() && order != "ascend" && order != "descend") {
            errorMsg = "Order parameter is not legal: Order parameter is " + order +
                ", it should be ascend or descend.";
            return false;
        }
        if (!orderBy.empty() && std::find(componentTableColumn.begin(), componentTableColumn.end(),
                                          orderBy) == componentTableColumn.end()) {
            errorMsg = "Order By parameter is not legal: Order By parameter is " + orderBy +
                ", it should be one of the column names";
            return false;
        }
        return true;
    }
};

struct MemoryComponentRequest : public Request {
    MemoryComponentRequest() : Request(REQ_RES_MEMORY_COMPONENT) {};
    MemoryComponentParams params;
};

struct MemoryViewParams {
    std::string rankId;
    std::string deviceId;
    std::string type; // Overall, Stream, Component
    bool isCompare = false;
    bool CommonCheck(std::string &errorMsg)
    {
        if (!CheckStrParamValid(rankId, errorMsg)) {
            return false;
        }
        if (!CheckStrParamValidEmptyAllowed(deviceId, errorMsg)) {
            return false;
        }
        if (type != MEMORY_OVERALL_GROUP && type != MEMORY_STREAM_GROUP && type != MEMORY_COMPONENT_GROUP) {
            errorMsg = "Group By parameter should be Overall or Stream or Component.";
            return false;
        }
        if (isCompare && type == MEMORY_STREAM_GROUP) {
            errorMsg = "Memory comparing does not support request type Stream.";
            return false;
        }
        return true;
    }
};

struct MemoryFindSliceParams {
    std::string fileId;
    std::string id;
    std::string name;
    bool CommonCheck(std::string &errorMsg)
    {
        if (!CheckStrParamValid(fileId, errorMsg)) {
            errorMsg = "The value of request param [fileId] is invalid, detail:" + errorMsg;
            return false;
        }
        if (!CheckStrParamValid(id, errorMsg)) {
            errorMsg = "The value of request param [id] is invalid, detail:" + errorMsg;
            return false;
        }
        return true;
    }
};

// leaks部分
struct LeaksMemoryBlockParams {
    uint64_t startTimestamp;
    uint64_t endTimestamp;
    uint64_t minSize;
    uint64_t maxSize;
    std::string deviceId;
    std::string eventType;
    bool relativeTime;

    LeaksMemoryBlockParams()
        : startTimestamp(0), endTimestamp(0), minSize(0), maxSize(0), relativeTime(false) {}

    bool CommonCheck(std::string &errorMsg)
    {
        if (minSize > maxSize) {
            errorMsg = "[minSize] must be less than [maxSize].";
            return false;
        }
        if (maxSize > MAX_LEAKS_MEMORY_BLOCK_SIZE) {
            errorMsg = "The maximum size (maxSize = " + std::to_string(maxSize) + ") exceeds "
                    + std::to_string(MAX_LEAKS_MEMORY_BLOCK_SIZE) + ".";
            return false;
        }
        if (startTimestamp > endTimestamp) {
            errorMsg = "[startTimestamp] must be preceed [endTimestamp].";
            return false;
        }
        if (startTimestamp > INT64_MAX || endTimestamp > INT64_MAX) {
            errorMsg = "Invalid timestamp, detail: exceeds the maximum limit of " + std::to_string(INT64_MAX);
            return false;
        }
        if (!CheckStrParamValid(deviceId, errorMsg)) {
            errorMsg = "Invalid deviceId, detail: " + errorMsg;
            return false;
        }
        if (!CheckStrParamValid(eventType, errorMsg)) {
            errorMsg = "Invalid eventType, detail: " + errorMsg;
            return false;
        }
        return true;
    }
};

struct LeaksMemoryAllocationParams {
    uint64_t startTimestamp;
    uint64_t endTimestamp;
    std::string deviceId;
    std::string eventType;
    bool optimized;
    bool relativeTime;

    LeaksMemoryAllocationParams() : startTimestamp(0), endTimestamp(0), optimized(false), relativeTime(false) {}

    bool CommonCheck(std::string &errorMsg)
    {
        if (startTimestamp > endTimestamp) {
            errorMsg = "The start timestamp (startTimestamp) should be less than the end timestamp (endTimestamp).";
            return false;
        }
        if (startTimestamp > INT64_MAX || endTimestamp > INT64_MAX) {
            errorMsg = "Invalid timestamp, detail: exceeds the maximum limit of " + std::to_string(INT64_MAX);
            return false;
        }
        if (!CheckStrParamValid(deviceId, errorMsg)) {
            errorMsg = "Invalid deviceId, detail: " + errorMsg;
            return false;
        }
        if (!CheckStrParamValid(eventType, errorMsg)) {
            errorMsg = "Invalid eventType, detail: " + errorMsg;
            return false;
        }
        return true;
    }
};

struct LeaksMemoryDetailParams {
    uint64_t timestamp;
    bool relativeTime;
    std::string deviceId;

    bool CommonCheck(std::string &errorMsg)
    {
        if (!CheckStrParamValid(deviceId, errorMsg)) {
            errorMsg = "Invalid deviceId, detail: " + errorMsg;
            return false;
        }
        if (deviceId.empty()) {
            errorMsg = "Invalid deviceId, detail: deviceId is empty";
            return false;
        }
        if (timestamp > INT64_MAX) {
            errorMsg = "Invalid timestamp, detail: exceeds the maximum limit of " + std::to_string(INT64_MAX);
            return false;
        }
        return true;
    }
};

struct LeaksMemoryThreadPythonTraceParams {
    uint64_t startTimestamp;
    uint64_t endTimestamp;
    bool relativeTime;
    std::string deviceId;
    uint64_t threadId;

    LeaksMemoryThreadPythonTraceParams() : startTimestamp(0), endTimestamp(0), relativeTime(false), threadId(0) {}

    bool CommonCheck(std::string &errorMsg)
    {
        if (!CheckStrParamValid(deviceId, errorMsg)) {
            errorMsg = "Invalid deviceId, detail: " + errorMsg;
            return false;
        }
        if (startTimestamp > endTimestamp) {
            errorMsg = "The start timestamp (startTimestamp) should be less than the end timestamp (endTimestamp).";
            return false;
        }
        if (startTimestamp > INT64_MAX || endTimestamp > INT64_MAX) {
            errorMsg = "The timestamp value is too large.";
            return false;
        }
        if (threadId >= INT64_MAX) {
            errorMsg = "The threadId value is too large.";
            return false;
        }
        return true;
    }
};

struct LeaksMemoryBlockRequest : public Request {
    LeaksMemoryBlockRequest() : Request(REQ_RES_LEAKS_MEMORY_BLOCKS){};
    LeaksMemoryBlockParams params;
};

struct LeaksMemoryAllocationRequest : public Request {
    LeaksMemoryAllocationRequest() : Request(REQ_RES_LEAKS_MEMORY_ALLOCATIONS){};
    LeaksMemoryAllocationParams params;
};

struct LeaksMemoryDetailRequest : public Request {
    LeaksMemoryDetailRequest() : Request(REQ_RES_LEAKS_MEMORY_DETAILS){};
    LeaksMemoryDetailParams params;
};

struct LeaksMemoryTraceRequest : public Request {
    LeaksMemoryTraceRequest() : Request(REQ_RES_LEAKS_MEMORY_TRACES){};
    LeaksMemoryThreadPythonTraceParams params;
};

struct MemoryFindSliceRequest : public Request {
    MemoryFindSliceRequest() : Request(REQ_RES_MEMORY_FIND_SLICE){};
    MemoryFindSliceParams params;
};

struct MemoryViewRequest : public Request {
    MemoryViewRequest() : Request(REQ_RES_MEMORY_VIEW) {};
    MemoryViewParams params;
};

struct MemoryStaticOperatorGraphRequest : public Request {
    MemoryStaticOperatorGraphRequest() : Request(REQ_RES_MEMORY_STATIC_OP_MEMORY_GRAPH) {};
    StaticOperatorGraphParams params;
};

struct MemoryStaticOperatorListRequest : public Request {
    MemoryStaticOperatorListRequest() : Request(REQ_RES_MEMORY_STATIC_OP_MEMORY_LIST) {};
    StaticOperatorListParams params;
};

struct MemoryStaticOperatorSizeRequest : public Request {
    MemoryStaticOperatorSizeRequest() : Request(REQ_RES_MEMORY_STATIC_OP_MEMORY_MIN_MAX) {};
    StaticOperatorSizeParams params;
};

} // end of namespace Protocol
} // end of namespace Dic

#endif // PROFILER_SERVER_MEMORYPROTOCOLREQUEST_H
