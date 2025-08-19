/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/
#ifndef PROFILER_SERVER_MEMORY_DETAIL_PROTOCOL_REQUEST_H
#define PROFILER_SERVER_MEMORY_DETAIL_PROTOCOL_REQUEST_H

#include "pch.h"
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"
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
class PaginationParam {
public:
    int64_t currentPage{};
    int64_t pageSize{};

    bool Check(std::string& errorMsg) const
    {
        if (pageSize==0 && currentPage==0) {
            return true;
        }
        if (!CheckPageValid(pageSize, currentPage, errorMsg)) {
            errorMsg = "Invalid pagination params, detail: " + errorMsg;
            return false;
        }
        return true;
    }

    void SetPaginationParamFromJson(const json_t &json)
    {
        JsonUtil::SetByJsonKeyValue(currentPage, json, "currentPage");
        JsonUtil::SetByJsonKeyValue(pageSize, json, "pageSize");
    }
};

class FiltersParam {
public:
    std::unordered_map<std::string, std::string> filters;

    bool SetFiltersFromJson(const json_t &json,
                            const std::vector<SqliteDbTableColumn> &columns,
                            std::string &errorMsg)
    {
        if (!json.IsObject()) {
            errorMsg = "Failed to set filters params from param json object: json is null or type invalid";
            return false;
        }
        if (!json.HasMember("filters")) {
            return true;
        }
        const json_t& filtersJson = json["filters"];
        if (!filtersJson.IsObject()) {
            errorMsg = "Failed to set filters params from param json object: filter json is null or type invalid";
            return false;
        }
        for (auto member = filtersJson.MemberBegin(); member != filtersJson.MemberEnd(); member++) {
            auto &key  = member->name;
            auto &value = member->value;
            if (!key.IsString() || !value.IsString()) {
                errorMsg = "Failed to set filters params from param json object: invalid type";
                return false;
            }
            std::string strKey = key.GetString();
            std::string strValue = value.GetString();
            auto const columnIt = FindColumnByKey(strKey, columns);
            if (columnIt == columns.end()) {
                errorMsg = StringUtil::FormatString("Invalid filter, detail: Non-exist column '{}'", strKey);
                return false;
            }
            if (!columnIt->searchable) {
                errorMsg = StringUtil::FormatString("Invalid filter, detail: column '{}' is not searchable", strKey);
                return false;
            }
            filters.emplace(StringUtil::FormatString("_{}", columnIt->key), strValue);
        }
        return true;
    }
};

class OrderByParam {
public:
    std::string orderBy;
    bool desc{};

    bool SetOrderFromJson(const json_t &json,
                          const std::vector<SqliteDbTableColumn> &columns,
                          std::string &errorMsg)
    {
        if (!json.IsObject()) {
            errorMsg = "Failed to set orderBy param from param json object: json is null or type invalid";
            return false;
        }
        if (!json.HasMember("orderBy")) {
            return true;
        }
        const json_t &orderByJson = json["orderBy"];
        if (orderByJson.IsNull() || !orderByJson.IsString()) {
            errorMsg = "Failed to set orderBy param from param json object: orderBy is null or type invalid";
            return false;
        }
        std::string orderByStr = orderByJson.GetString();
        if (orderByStr.empty()) {
            return true;
        }
        auto const columnIt = FindColumnByKey(orderByStr, columns);
        if (columnIt == columns.end()) {
            errorMsg = "Failed to set orderBy param from param json object: Non-exist column " + orderByStr;
            return false;
        }
        if (!columnIt->sortable) {
            errorMsg = StringUtil::FormatString("Invalid order, detail: column '{}' is not sortable",
                                                columnIt->name);
            return false;
        }
        orderBy = StringUtil::FormatString("_{}", columnIt->key);
        JsonUtil::SetByJsonKeyValue(desc, json, "desc");
        return true;
    }
};

class RangeFiltersParam {
public:
    std::unordered_map<std::string, std::pair<double, double>> rangeFilters;

    bool SetRangeFiltersFromJson(const json_t &json,
                                 const std::vector<SqliteDbTableColumn> &columns,
                                 std::string &errorMsg)
    {
        if (!json.IsObject()) {
            errorMsg = "Failed to set range filters params from param json object: json is null or type invalid";
            return false;
        }
        // 没有rangeFilters字段或rangeFilters为空代表不做范围过滤，合法
        if (!json.HasMember("rangeFilters") || json["rangeFilters"].MemberCount() == 0) {
            return true;
        }
        const json_t& rangeFiltersJson = json["rangeFilters"];
        if (!rangeFiltersJson.IsObject()) {
            errorMsg = "Failed to set range filters params from param json object: filter json is null or type invalid";
            return false;
        }
        for (auto member = rangeFiltersJson.MemberBegin(); member != rangeFiltersJson.MemberEnd(); member++) {
            auto &key  = member->name;
            auto &value = member->value;
            if (!key.IsString() || !value.IsArray()) {
                errorMsg = "Failed to set range filters params from param json object: format error.";
                return false;
            }
            std::string strKey = key.GetString();
            auto rangeJson = value.GetArray();
            if (rangeJson.Size() != 2 || !rangeJson[0].IsNumber() || !rangeJson[1].IsNumber()) {
                errorMsg = StringUtil::FormatString("Invalid range array size or range array type.");
                return false;
            }
            std::pair<double, double> rangePair;
            rangePair.first = rangeJson[0].GetDouble();
            rangePair.second = rangeJson[1].GetDouble();
            auto const columnIt = FindColumnByKey(strKey, columns);
            if (columnIt == columns.end()) {
                errorMsg = StringUtil::FormatString("Invalid filter, detail: Non-exist column '{}'", strKey);
                return false;
            }
            if (!columnIt->rangeFilterable) {
                errorMsg = StringUtil::FormatString("Invalid range filter, "
                                                    "detail: column '{}' is not range filterable", strKey);
                return false;
            }
            rangeFilters.emplace(StringUtil::FormatString("_{}", columnIt->key), rangePair);
        }
        return true;
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
        for (auto [colName, rangePair] : rangeFilters) {
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
