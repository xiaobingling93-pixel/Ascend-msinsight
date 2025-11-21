/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/
#ifndef PROFILER_SERVER_MEMORY_DETAIL_PROTOCOL_REQUEST_H
#define PROFILER_SERVER_MEMORY_DETAIL_PROTOCOL_REQUEST_H

#include "pch.h"
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"
#include "CommonRequests.h"
#include "LeaksMemoryTableColumn.h"

namespace Dic::Protocol {
using namespace Dic::Module::MemoryDetail;
namespace BLOCK_TABLE = Dic::Module::MemoryDetail::MemoryBlockTableColumn;
namespace ALLOCATION_TABLE = Dic::Module::MemoryDetail::MemoryAllocationTableColumn;
namespace EVENT_TABLE = Dic::Module::MemoryDetail::MemoryEventTableColumn;
namespace TRACE_TABLE = Dic::Module::MemoryDetail::MemoryPythonTraceTableColumn;

const uint64_t MAX_LEAKS_MEMORY_BLOCK_SIZE = 1 * 1024 * 1024 * 1024;
const int64_t COMMON_RANGE_VALUE_MAX = 1000000000000;
const int64_t COMMON_RANGE_VALUE_MIN = 0;


struct Threshold {
public:
    uint32_t perT{0};
    uint64_t valueT{0};

    static const uint32_t MAX_PER = 100;
    static const uint32_t MIN_PER = 0;
    static const uint64_t MAX_VALUE = INT64_MAX;
    static const uint64_t MIN_VALUE = 0;

    bool CheckValid() const
    {
        return perT <= MAX_PER && valueT < MAX_VALUE;
    }

    std::string GetPerStr() const
    {
        return StringUtil::DoubleToStringWithTwoDecimalPlaces(perT/100.0f);
    }
};

struct LeaksMemoryBlockParams : PaginationParam, FiltersParam, OrderByParam, RangeFiltersParam {
    uint64_t startTimestamp;
    uint64_t endTimestamp;
    uint64_t minSize;
    uint64_t maxSize;
    std::string deviceId;
    std::string eventType;
    bool relativeTime;

    // 低效内存识别相关请求参数
    Threshold lazyUsedThreshold;
    Threshold delayedFreeThreshold;
    Threshold longIdleThreshold;
    // 仅展示低效内存
    bool onlyInefficient{false};

    LeaksMemoryBlockParams() : startTimestamp(0), endTimestamp(0), minSize(0), maxSize(0), relativeTime(false) {}

    bool CommonCheck(std::string& errorMsg) const
    {
        if (minSize > maxSize) {
            errorMsg = "[minSize] must be less than [maxSize].";
            return false;
        }
        if (maxSize > MAX_LEAKS_MEMORY_BLOCK_SIZE) {
            errorMsg = "The maximum size (maxSize = " + std::to_string(maxSize) + ") exceeds " +
                       std::to_string(MAX_LEAKS_MEMORY_BLOCK_SIZE) + ".";
            return false;
        }
        if (startTimestamp > endTimestamp) {
            errorMsg = "The start timestamp (startTimestamp) should be less than the end timestamp (endTimestamp).";
            return false;
        }
        if (endTimestamp > INT64_MAX) {
            errorMsg = "Invalid timestamp, detail: exceeds the maximum limit of " + std::to_string(INT64_MAX);
            return false;
        }
        if (!CheckStrParamValid(deviceId, errorMsg)) {
            errorMsg = "Invalid deviceId, detail: " + errorMsg;
            return false;
        }
        if (!lazyUsedThreshold.CheckValid()) {
            errorMsg = "Invalid lazy_used threshold.";
            return false;
        }
        if (!delayedFreeThreshold.CheckValid()) {
            errorMsg = "Invalid delayed free threshold.";
            return false;
        }
        if (!longIdleThreshold.CheckValid()) {
            errorMsg = "Invalid long idle threshold";
            return false;
        }
        if (!CheckStrParamValid(eventType, errorMsg)) {
            errorMsg = "Invalid eventType, detail: " + errorMsg;
            return false;
        }
        return PaginationParam::Check(errorMsg);
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

    bool CommonCheck(std::string& errorMsg) const
    {
        if (startTimestamp > endTimestamp) {
            errorMsg = "The start timestamp (startTimestamp) should be less than the end timestamp (endTimestamp).";
            return false;
        }
        if (endTimestamp > INT64_MAX) {
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
    std::string eventType;

    LeaksMemoryDetailParams() : timestamp(0), relativeTime(false) {}

    bool CommonCheck(std::string& errorMsg) const
    {
        if (!CheckStrParamValid(deviceId, errorMsg)) {
            errorMsg = "Invalid deviceId, detail: " + errorMsg;
            return false;
        }
        if (!CheckStrParamValid(eventType, errorMsg)) {
            errorMsg = "Invalid eventType, detail: " + errorMsg;
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

    bool CommonCheck(std::string& errorMsg) const
    {
        if (!CheckStrParamValid(deviceId, errorMsg)) {
            errorMsg = "Invalid deviceId, detail: " + errorMsg;
            return false;
        }
        if (startTimestamp > endTimestamp) {
            errorMsg = "The start timestamp (startTimestamp) should be less than the end timestamp (endTimestamp).";
            return false;
        }
        if (endTimestamp > INT64_MAX) {
            errorMsg = "Invalid timestamp, detail: exceeds the maximum limit of " + std::to_string(INT64_MAX);
            return false;
        }
        if (threadId >= INT64_MAX) {
            errorMsg = "The threadId value is too large.";
            return false;
        }
        return true;
    }
};

struct LeaksMemoryEventParams : public PaginationParam, FiltersParam, OrderByParam, RangeFiltersParam {
    std::string deviceId;
    uint64_t startTimestamp{};
    uint64_t endTimestamp{};
    bool relativeTime{};

    LeaksMemoryEventParams() = default;

    bool CommonCheck(std::string& errorMsg) const
    {
        if (!CheckStrParamValid(deviceId, errorMsg)) {
            errorMsg = "Invalid deviceId, detail: " + errorMsg;
            return false;
        }
        if (startTimestamp > endTimestamp) {
            errorMsg = "The start timestamp (startTimestamp) should be less than the end timestamp (endTimestamp).";
            return false;
        }
        if (endTimestamp > INT64_MAX) {
            errorMsg = StringUtil::FormatString("Invalid timestamp, detail: exceeds the range of [{},{}]",
                                                "0", std::to_string(INT64_MAX));
            return false;
        }
        for (const auto&  [colName, rangePair] : rangeFilters) {
            (void)(colName);
            if (rangePair.first < COMMON_RANGE_VALUE_MIN || rangePair.second < COMMON_RANGE_VALUE_MIN) {
                errorMsg = StringUtil::FormatString("Invalid range value, detail: less than ",
                                                    std::to_string(COMMON_RANGE_VALUE_MIN));
                return false;
            }

            if (rangePair.first > COMMON_RANGE_VALUE_MAX || rangePair.second > COMMON_RANGE_VALUE_MAX) {
                errorMsg = StringUtil::FormatString("Invalid range value, detail: greater than ",
                                                    std::to_string(COMMON_RANGE_VALUE_MAX));
                return false;
            }
        }
        return PaginationParam::Check(errorMsg);
    }
};

struct LeaksMemoryBlockRequest : public Request {
    LeaksMemoryBlockRequest() : Request(REQ_RES_LEAKS_MEMORY_BLOCKS) {};
    LeaksMemoryBlockParams params;
    bool isTable{};

    void SetThresholdsFromJson(const json_t& json)
    {
        if (json.HasMember("lazyUsedThreshold") && json["lazyUsedThreshold"].IsObject()) {
            JsonUtil::SetByJsonKeyValue(params.lazyUsedThreshold.perT, json["lazyUsedThreshold"], "perT");
            JsonUtil::SetByJsonKeyValue(params.lazyUsedThreshold.valueT, json["lazyUsedThreshold"], "valueT");
        }
        if (json.HasMember("delayedFreeThreshold") && json["delayedFreeThreshold"].IsObject()) {
            JsonUtil::SetByJsonKeyValue(params.delayedFreeThreshold.perT, json["delayedFreeThreshold"], "perT");
            JsonUtil::SetByJsonKeyValue(params.delayedFreeThreshold.valueT, json["delayedFreeThreshold"], "valueT");
        }
        if (json.HasMember("longIdleThreshold") && json["longIdleThreshold"].IsObject()) {
            JsonUtil::SetByJsonKeyValue(params.longIdleThreshold.perT, json["longIdleThreshold"], "perT");
            JsonUtil::SetByJsonKeyValue(params.longIdleThreshold.valueT, json["longIdleThreshold"], "valueT");
        }
    }

    static std::unique_ptr<Request> FromJson(const json_t& json, std::string& error)
    {
        std::unique_ptr<LeaksMemoryBlockRequest> reqPtr = std::make_unique<LeaksMemoryBlockRequest>();
        if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
            error = "Failed to set request base info, command is: " + reqPtr->command;
            return nullptr;
        }
        if (!json.HasMember("params") || !json["params"].HasMember("deviceId") ||
            !json["params"].HasMember("eventType")) {
            error = "Request[requestId=" + std::to_string(reqPtr->id) +
                    "] json lacks member params or deviceId or eventType.";
            return nullptr;
        }
        const json_t& param_json = json["params"];
        JsonUtil::SetByJsonKeyValue(reqPtr->isTable, param_json, "isTable");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.onlyInefficient, param_json, "onlyInefficient");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.startTimestamp, param_json, "startTimestamp");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.endTimestamp, param_json, "endTimestamp");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.minSize, param_json, "minSize");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.maxSize, param_json, "maxSize");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.deviceId, param_json, "deviceId");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.relativeTime, param_json, "relativeTime");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.eventType, param_json, "eventType");
        if (reqPtr->isTable) {
            reqPtr->params.SetPaginationParamFromJson(param_json);
            if (!reqPtr->params.SetFiltersFromJson(param_json, BLOCK_TABLE::FIELD_FULL_COLUMNS, error)) {
                Server::ServerLog::Error("Failed set filters from json param: %", error);
                return nullptr;
            }
            if (!reqPtr->params.SetOrderFromJson(param_json, BLOCK_TABLE::FIELD_FULL_COLUMNS, error)) {
                Server::ServerLog::Error("Failed set order from json param: %", error);
                return nullptr;
            }
            if (!reqPtr->params.SetRangeFiltersFromJson(param_json, BLOCK_TABLE::FIELD_FULL_COLUMNS, error)) {
                Server::ServerLog::Error("Failed set range filters from json param: %", error);
                return nullptr;
            }
            reqPtr->SetThresholdsFromJson(param_json);
        } else {
            // 展示block图时，只可根据startTimestamp排序
            reqPtr->params.orderBy = std::string(BLOCK_TABLE::START_TIMESTAMP);
        }
        return reqPtr;
    }
};

struct LeaksMemoryAllocationRequest : public Request {
    LeaksMemoryAllocationRequest() : Request(REQ_RES_LEAKS_MEMORY_ALLOCATIONS) {};
    LeaksMemoryAllocationParams params;

    static std::unique_ptr<Request> FromJson(const json_t& json, std::string& error)
    {
        std::unique_ptr<LeaksMemoryAllocationRequest> reqPtr = std::make_unique<LeaksMemoryAllocationRequest>();
        if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
            error = "Failed to set request base info, command is: " + reqPtr->command;
            return nullptr;
        }
        if (!json.HasMember("params") || !json["params"].HasMember("deviceId") ||
            !json["params"].HasMember("eventType")) {
            error = "Request[requestId=" + std::to_string(reqPtr->id) +
                    "] json lacks member params or deviceId or eventType.";
            return nullptr;
        }
        const json_t& param_json = json["params"];
        JsonUtil::SetByJsonKeyValue(reqPtr->params.startTimestamp, param_json, "startTimestamp");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.endTimestamp, param_json, "endTimestamp");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.deviceId, param_json, "deviceId");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.optimized, param_json, "optimized");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.relativeTime, param_json, "relativeTime");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.eventType, param_json, "eventType");
        return reqPtr;
    }
};

struct LeaksMemoryDetailRequest : public Request {
    LeaksMemoryDetailRequest() : Request(REQ_RES_LEAKS_MEMORY_DETAILS) {};
    LeaksMemoryDetailParams params;

    static std::unique_ptr<Request> FromJson(const json_t& json, std::string& error)
    {
        std::unique_ptr<LeaksMemoryDetailRequest> reqPtr = std::make_unique<LeaksMemoryDetailRequest>();
        if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
            error = "Failed to set request base info, command is: " + reqPtr->command;
            return nullptr;
        }
        if (!json.HasMember("params") || !json["params"].HasMember("deviceId") ||
            !json["params"].HasMember("timestamp") || !json["params"].HasMember("eventType")) {
            error = "Request[requestId=" + std::to_string(reqPtr->id) +
                    "] json lacks member params or deviceId or timestamp or eventType.";
            return nullptr;
        }
        const json_t& param_json = json["params"];
        JsonUtil::SetByJsonKeyValue(reqPtr->params.timestamp, param_json, "timestamp");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.deviceId, param_json, "deviceId");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.relativeTime, param_json, "relativeTime");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.eventType, param_json, "eventType");
        return reqPtr;
    }
};

struct LeaksMemoryTraceRequest : public Request {
    LeaksMemoryTraceRequest() : Request(REQ_RES_LEAKS_MEMORY_TRACES) {};
    LeaksMemoryThreadPythonTraceParams params;
    bool allowTrim{false};

    static std::unique_ptr<Request> FromJson(const json_t& json, std::string& error)
    {
        std::unique_ptr<LeaksMemoryTraceRequest> reqPtr = std::make_unique<LeaksMemoryTraceRequest>();
        if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
            error = "Failed to set request base info, command is: " + reqPtr->command;
            return nullptr;
        }
        if (!json.HasMember("params") || !json["params"].HasMember("deviceId") ||
            !json["params"].HasMember("threadId")) {
            error = "Request[requestId=" + std::to_string(reqPtr->id) +
                    "] json lacks member params or deviceId or threadId.";
            return nullptr;
        }
        const json_t& param_json = json["params"];
        JsonUtil::SetByJsonKeyValue(reqPtr->params.startTimestamp, param_json, "startTimestamp");
        JsonUtil::SetByJsonKeyValue(reqPtr->allowTrim, param_json, "allowTrim");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.endTimestamp, param_json, "endTimestamp");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.relativeTime, param_json, "relativeTime");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.threadId, param_json, "threadId");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.deviceId, param_json, "deviceId");
        return reqPtr;
    }
};

struct LeaksMemoryEventRequest : public Request {
    LeaksMemoryEventRequest() : Request(REQ_RES_LEAKS_MEMORY_EVENTS) {}
    LeaksMemoryEventParams params;

    static std::unique_ptr<Request> FromJson(const json_t& json, std::string& error)
    {
        std::unique_ptr<LeaksMemoryEventRequest> reqPtr = std::make_unique<LeaksMemoryEventRequest>();
        if (!ProtocolUtil::SetRequestBaseInfo(*reqPtr, json)) {
            error = "Failed to set request base info, command is: " + reqPtr->command;
            return nullptr;
        }
        const json_t& param_json = json["params"];
        if (param_json.IsNull() || !param_json.HasMember("deviceId")) {
            error = "Request[requestId=" + std::to_string(reqPtr->id) +
                    "] json lacks member params or deviceId.";
            return nullptr;
        }
        JsonUtil::SetByJsonKeyValue(reqPtr->params.deviceId, param_json, "deviceId");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.startTimestamp, param_json, "startTimestamp");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.endTimestamp, param_json, "endTimestamp");
        JsonUtil::SetByJsonKeyValue(reqPtr->params.relativeTime, param_json, "relativeTime");
        reqPtr->params.SetPaginationParamFromJson(param_json);
        if (!reqPtr->params.SetFiltersFromJson(param_json, EVENT_TABLE::FIELD_FULL_COLUMNS, error)) {
            Server::ServerLog::Error("Failed set filters from json param: %", error);
            return nullptr;
        }
        if (!reqPtr->params.SetOrderFromJson(param_json, EVENT_TABLE::FIELD_FULL_COLUMNS, error)) {
            Server::ServerLog::Error("Failed set order from json param: %", error);
            return nullptr;
        }
        if (!reqPtr->params.SetRangeFiltersFromJson(param_json, EVENT_TABLE::FIELD_FULL_COLUMNS, error)) {
            Server::ServerLog::Error("Failed set range filters from json param: %", error);
            return nullptr;
        }
        return reqPtr;
    }
};
}  // end of namespace Dic::Protocol
#endif  // PROFILER_SERVER_MEMORY_DETAIL_PROTOCOL_REQUEST_H
