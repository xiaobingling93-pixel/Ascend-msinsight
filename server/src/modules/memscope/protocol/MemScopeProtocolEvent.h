/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

#include "JsonUtil.h"
#include "ProtocolDefs.h"
#include "ProtocolMessage.h"

#ifndef PROFILER_SERVER_MEM_SCOPE_PROTOCOL_EVENT_H
#define PROFILER_SERVER_MEM_SCOPE_PROTOCOL_EVENT_H
namespace Dic::Protocol {
struct MemScopeParseSuccessEventBody {
    std::string fileId;
    std::unordered_map<std::string, std::vector<std::string>> deviceIds;
    std::vector<uint64_t> threadIds;
};

struct MemScopeParseSuccessEvent : public JsonEvent {
    MemScopeParseSuccessEvent() : JsonEvent(EVENT_PARSE_MEM_SCOPE_COMPLETED) {}
    MemScopeParseSuccessEventBody body;
    std::string errMsg;

    static bool BuildMemScopeParseSuccessEventDeviceIdsJson(const MemScopeParseSuccessEvent &event, json_t &deviceIds,
                                                            RAPIDJSON_DEFAULT_ALLOCATOR &allocator)
    {
        for (auto &devicePair: event.body.deviceIds) {
            json_t eventTypes(kArrayType);
            for (auto &eventType : devicePair.second) {
                eventTypes.PushBack(json_t().SetString(eventType.c_str(), allocator), allocator);
            }
            JsonUtil::AddMember(deviceIds, std::string_view(devicePair.first), eventTypes, allocator);
        }
        return true;
    }

    [[nodiscard]] std::optional<document_t> ToJson() const override
    {
        document_t json(kObjectType);
        auto &allocator = json.GetAllocator();
        ProtocolUtil::SetEventJsonBaseInfo(*this, json);
        json_t jsonBody(kObjectType);
        json_t deviceIds(kObjectType);
        json_t threadIds(kArrayType);
        json_t &moduleName = json["moduleName"];
        moduleName.SetString(Protocol::MODULE_MEM_SCOPE.c_str(), allocator);
        BuildMemScopeParseSuccessEventDeviceIdsJson(*this, deviceIds, allocator);
        JsonUtil::AddMember(jsonBody, "deviceIds", deviceIds, allocator);
        // 构建threadIds
        for (auto threadId : body.threadIds) {
            threadIds.PushBack(json_t().SetUint64(threadId), allocator);
        }
        JsonUtil::AddMember(jsonBody, "threadIds", threadIds, allocator);
        JsonUtil::AddMember(jsonBody, "dbPath", body.fileId, allocator);
        JsonUtil::AddMember(json, "body", jsonBody, allocator);
        JsonUtil::AddMember(json, "errMsg", errMsg, allocator);
        return std::optional<document_t>{std::move(json)};
    }
};
}

#endif  // PROFILER_SERVER_MEM_SCOPE_PROTOCOL_EVENT_H
