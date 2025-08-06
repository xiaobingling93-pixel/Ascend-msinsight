/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/
#include "MemoryDetailEntities.h"
namespace Dic::Module::MemoryDetail {
// [PYTHON_TRACE]
bool LeaksMemoryPythonTrace::Empty() const
{
    return slices.empty();
}
// [PYTHON_TRACE]

// [EVENT]
EventGroup::EventGroup(std::vector<MemoryEvent>& sortedEvents)
{
    for (auto &event : sortedEvents) {
            AddEvent(event);
    }
}

void EventGroup::AddEvent(const MemoryEvent& event)
{
    if (event.event == LEAKS_DUMP_EVENT::MALLOC) {
        if (mallocEvent.has_value()) {
            Server::ServerLog::Warn("Duplicated malloc events in the same group.");
            return;
        }
        mallocEvent = std::make_optional(event);
        return;
    }
    if (event.event == LEAKS_DUMP_EVENT::FREE) {
        if (freeEvent.has_value()) {
            Server::ServerLog::Warn("Duplicated free events in the same group.");
            return;
        }
        freeEvent = std::make_optional(event);
        return;
    }
    if (event.event == LEAKS_DUMP_EVENT::ACCESS) {
        accessEvents.push_back(event);
    }
}
// [EVENT]

// [BLOCK]
std::string MemoryBlockAttrs::ToJsonString()
{
    document_t blockAttrJson(kObjectType);
    auto& allocator = blockAttrJson.GetAllocator();
    JsonUtil::AddMember(blockAttrJson, BLOCK_EVENT_ATTR_SIZE_FIELD, size, allocator);
    JsonUtil::AddMember(blockAttrJson, BLOCK_EVENT_ATTR_GROUP_ID_FIELD, groupId, allocator);
    JsonUtil::AddMember(blockAttrJson, BLOCK_ATTR_FIRST_ACCESS_FILED, firstAccessTimestamp, allocator);
    JsonUtil::AddMember(blockAttrJson, BLOCK_ATTR_LAST_ACCESS_FILED, lastAccessTimestamp, allocator);
    for (auto &attrPair : extendAttrs) {
        JsonUtil::AddMember(blockAttrJson, attrPair.first, attrPair.second, allocator);
    }
    return JsonUtil::JsonDump(blockAttrJson);
}
std::optional<MemoryBlockAttrs> MemoryBlockAttrs::FromJson(std::string jsonString)
{
    MemoryBlockAttrs attrs;
    std::string error;
    auto attrsJsonDoc = JsonUtil::TryParse(jsonString, error);
    if (!error.empty() || !attrsJsonDoc.has_value()) {
        Server::ServerLog::Warn("Failed to parse block attrs: ", error);
        return std::nullopt;
    }
    auto &attrsJson = attrsJsonDoc.value();
    JsonUtil::SetByJsonKeyValue(attrs.size, attrsJson, "size");
    JsonUtil::SetByJsonKeyValue(attrs.groupId, attrsJson, "allocation_id");
    JsonUtil::SetByJsonKeyValue(attrs.firstAccessTimestamp, attrsJson, "first_access_timestamp");
    JsonUtil::SetByJsonKeyValue(attrs.lastAccessTimestamp, attrsJson, "last_access_timestamp");
    return attrs;
}
// [BLOCK]
} // Dic::Module::MemoryDetail