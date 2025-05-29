/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <algorithm>
#include <stack>
#include "pch.h"
#include "DataBaseManager.h"
#include "LeaksMemoryService.h"

#include <utility>

namespace Dic {
namespace Module {
namespace Memory {
void LeaksMemoryService::ParserEnd(const std::string &rankId, bool result)
{
    if (!result) {
        return;
    }
    Server::ServerLog::Info("[Memory]Leaks Dumps Parser ends, filepath: ", rankId);
}

void LeaksMemoryService::ParseCallBack(const std::string &fileId, bool result, const std::string &msg)
{
    if (fileId.empty()) {
        auto event = std::make_unique<Protocol::LeaksParseSuccessEvent>();
        event->moduleName = Protocol::MODULE_MEMORY;
        event->result = true;
        event->reset = true;
        event->body.fileId = fileId;
        SendEvent(std::move(event));
    } else {
        auto event = std::make_unique<Protocol::LeaksParseSuccessEvent>();
        event->moduleName = Protocol::MODULE_MEMORY;
        event->result = result;
        Protocol::LeaksParseSuccessEventBody body;
        if (event->result) {
            auto memoryDatabase = Timeline::DataBaseManager::Instance().GetLeaksMemoryDatabase("");
            memoryDatabase->QueryDeviceIds(body.deviceIds);
        } else {
            body.errMsg = msg;
        }
        event->body = body;
        event->body.fileId = fileId;
        SendEvent(std::move(event));
    }
}
bool LeaksMemoryService::ParseMemoryLeaksDumpEvents(const std::string &fileId)
{
    auto database = Timeline::DataBaseManager::Instance().GetLeaksMemoryDatabase(fileId);
    if (database == nullptr) {
        Server::ServerLog::Error("Cannot get leaks db connections from database manager");
        return false;
    }
    if (database->HasFinishedParseLastTime() && database->CheckTablesExist()) {
        Timeline::ParserStatusManager::Instance().SetFinishStatus(MEMORY_PREFIX + fileId);
        return true;
    }

    if (!database->CreateMemoryAllocationAndBlockTable() || !database->InitStmt() ||
        !database->UpdateParseStatus(NOT_FINISH_STATUS)) {
        Server::ServerLog::Error("Cannot create memory allocation and memory block table.");
        return false;
    }

    std::vector<Memory::MemoryEvent> events;
    database->QueryEntireEventsTable(events);
    if (events.empty()) {
        Server::ServerLog::Warn("No memory events could be found in leaks_db.");
        return false;
    }
    Memory::LeaksMemoryService::ParseEventsToBlockAndAllocations(events, database);
    return true;
}
void LeaksMemoryService::ParseEventsToBlockAndAllocations(const std::vector<MemoryEvent> &events,
                                                          const std::shared_ptr<FullDb::LeaksMemoryDatabase> &db)
{
    if (db == nullptr) {
        Server::ServerLog::Error("Cannot parse event to blocks and allocations: invalid db connections");
        return;
    }
    std::unordered_map<std::string, std::map<std::string, const MemoryEvent *>> deviceMallocMap;
    std::unordered_map<std::string, uint64_t> deviceTotalSize;
    std::unordered_map<std::string, uint64_t> deviceMaxTimestamp;
    for (auto &event : events) {
        BlockEventAttr eventExtendAttr;
        BuildBlockEventAttrFromEvent(event, eventExtendAttr);
        deviceMaxTimestamp[event.deviceId] = std::max(deviceMaxTimestamp[event.deviceId], event.timestamp);
        auto &allocMap = deviceMallocMap[event.deviceId];
        if (!SingleDeviceEventParse(db, event, allocMap, eventExtendAttr)) {
            continue;
        }
        deviceTotalSize[event.deviceId] += eventExtendAttr.size;
        // 构造allocation折线图元素
        MemoryAllocation allocation(event.timestamp, deviceTotalSize[event.deviceId], event.deviceId, false);
        db->InsertMemoryAllocation(allocation);
    }

    for (auto &devicePair : deviceMallocMap) {
        std::string deviceId = devicePair.first;
        std::map<std::string, const MemoryEvent *> allocMap = devicePair.second;
        const std::uint64_t maxTimestamp = deviceMaxTimestamp[deviceId];
        for (auto &allocPair : allocMap) {
            auto &event = allocPair.second;
            BlockEventAttr eventExtendAttr;
            BuildBlockEventAttrFromEvent(*event, eventExtendAttr);
            if (eventExtendAttr.size <= 0) {
                Server::ServerLog::Warn("An invalid memory allocation event was detected: cannot get the valid 'size' "
                                        "attribute from the 'attr' field");
                continue;
            }
            // 构造block
            MemoryBlock block(event->ptr, event->deviceId, eventExtendAttr.size, event->timestamp, maxTimestamp,
                              eventExtendAttr.owner, "");
            db->InsertMemoryBlock(block);
        }
    }
    db->FlushMemoryBlocksCache();
    db->FlushMemoryAllocationsCache();
    db->UpdateParseStatus(FINISH_STATUS);
}
bool LeaksMemoryService::SingleDeviceEventParse(const std::shared_ptr<FullDb::LeaksMemoryDatabase> &db,
                                                const MemoryEvent &event,
                                                std::map<std::string, const MemoryEvent *> &allocMap,
                                                const BlockEventAttr &eventExtendAttr)
{
    if (event.event != "MALLOC" && event.event != "FREE") {
        return false;
    }
    if (eventExtendAttr.size == 0) {
        Server::ServerLog::Warn("An invalid memory allocation/free event[" + event.ptr +
                                "] was detected: cannot get the 'size' "
                                "attribute from the 'attr' field.");
        return false;
    }
    // 如果是分配事件
    if (event.event == "MALLOC") {
        // 已存在则忽略, 可能为重复申请
        if (allocMap.find(event.ptr) != allocMap.end()) {
            Server::ServerLog::Warn("An invalid memory allocation event[" + event.ptr +
                                    "] was detected: the address was already "
                                    "allocated and not released.");
            return false;
        }
        // 加入申请表
        allocMap[event.ptr] = &event;
    }
    // 如果是释放事件
    if (event.event == "FREE") {
        // 内存分配表中未找到匹配的分配事件，则忽略
        if (allocMap.find(event.ptr) == allocMap.end()) {
            Server::ServerLog::Warn("An invalid memory free event[" + event.ptr +
                                    "] was detected: no corresponding allocation was "
                                    "recorded for the same address.");
            return false;
        }

        auto &allocEvent = allocMap[event.ptr];
        // 构造block
        MemoryBlock block(event.ptr, event.deviceId, std::abs(eventExtendAttr.size),
                          allocEvent->timestamp, event.timestamp, eventExtendAttr.owner, "");
        db->InsertMemoryBlock(block);
        // 从申请表中去除内存申请事件
        allocMap.erase(event.ptr);
    }
    return true;
}
void LeaksMemoryService::BuildBlockEventAttrFromEvent(const MemoryEvent &event, BlockEventAttr &eventAttr)
{
    std::string attrStr = event.attr;
    if (attrStr.empty()) {
        return;
    }
    std::string parseError;
    auto jsonDoc = JsonUtil::TryParse(attrStr, parseError);
    if (!parseError.empty()) {
        Server::ServerLog::Warn("The leaks dump 'attr' field is invalid json string.");
        return;
    }
    if (!jsonDoc.has_value()) {
        return;
    }
    auto &json = jsonDoc.value();
    if (!JsonUtil::IsJsonKeyValid(json, BLOCK_EVENT_ATTR_SIZE_FIELD) ||
    !JsonUtil::IsJsonKeyValid(json, BLOCK_EVENT_ATTR_OWNER_FIELD)) {
        Server::ServerLog::Warn("The 'attr' field does not contain the required fields % and %.",
                                BLOCK_EVENT_ATTR_SIZE_FIELD, BLOCK_EVENT_ATTR_OWNER_FIELD);
        return;
    }
    std::string size_str = JsonUtil::GetString(json, BLOCK_EVENT_ATTR_SIZE_FIELD);
    eventAttr.size = NumberUtil::StringToLongLong(size_str);
    JsonUtil::SetByJsonKeyValue(eventAttr.owner, json, BLOCK_EVENT_ATTR_OWNER_FIELD);

    if (JsonUtil::IsJsonKeyValid(json, BLOCK_EVENT_ATTR_ADDR_FIELD)) {
        JsonUtil::SetByJsonKeyValue(eventAttr.addr, json, BLOCK_EVENT_ATTR_ADDR_FIELD);
    } else {
        eventAttr.addr = event.ptr;
    }
}

}  // Memory
}  // Module
}  // Dic