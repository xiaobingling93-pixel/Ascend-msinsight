/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <algorithm>
#include <stack>
#include "DataBaseManager.h"
#include "LeaksMemoryService.h"

namespace Dic {
namespace Module {
namespace MemoryDetail {
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
        event->moduleName = Protocol::MODULE_LEAKS;
        event->result = true;
        SendEvent(std::move(event));
    } else {
        auto event = std::make_unique<Protocol::LeaksParseSuccessEvent>();
        event->moduleName = Protocol::MODULE_LEAKS;
        event->result = result;
        Protocol::LeaksParseSuccessEventBody body;
        if (event->result) {
            auto memoryDatabase = Timeline::DataBaseManager::Instance().GetLeaksMemoryDatabase("");
            if (memoryDatabase == nullptr) {
                Server::ServerLog::Error("Cannot get leaks db connections from database manager");
                event->errMsg = "Failed parse leaks dump data.";
                event->result = false;
                SendEvent(std::move(event));
                return;
            }
            memoryDatabase->QueryMallocOrFreeEventTypeWithDeviceId(body.deviceIds);
            memoryDatabase->QueryThreadIds(body.threadIds);
            memoryDatabase->SetDataBaseVersion();
        } else {
            event->errMsg = msg;
        }
        body.fileId = fileId;
        event->body = body;
        SendEvent(std::move(event));
    }
}
bool LeaksMemoryService::ParseMemoryLeaksDumpEvents(const std::string &fileId)
{
    auto database = Timeline::DataBaseManager::Instance().GetLeaksMemoryDatabase("");
    if (database == nullptr) {
        Server::ServerLog::Error("Cannot get leaks db connections from database manager");
        return false;
    }
    if (database->CheckTablesExist() && database->HasFinishedParseLastTime()) {
        Timeline::ParserStatusManager::Instance().SetFinishStatus(MEMORY_PREFIX + fileId);
        return true;
    }

    if (!database->CreateMemoryAllocationAndBlockTable() || !database->InitStmt() ||
        !database->UpdateParseStatus(NOT_FINISH_STATUS)) {
        Server::ServerLog::Error("Cannot create memory allocation and memory block table.");
        return false;
    }

    std::vector<MemoryEvent> events;
    database->QueryEntireEventsTable(events);
    if (events.empty()) {
        Server::ServerLog::Warn("No memory events could be found in leaks_db.");
        return false;
    }
    LeaksMemoryService::ParseEventsToBlockAndAllocations(events, database);
    return true;
}

uint64_t SafeCalculateAllocationSize(uint64_t currentSize, int64_t eventSize)
{
    uint64_t tmpSize;
    if (eventSize >= 0) {
        tmpSize = static_cast<uint64_t>(eventSize);
        if (currentSize > UINT64_MAX - tmpSize) {
            Server::ServerLog::Warn("Allocation total size is too large.");
            return UINT64_MAX;
        }
        return currentSize + tmpSize;
    }
    tmpSize = static_cast<uint64_t>(-eventSize);
    if (currentSize > tmpSize) {
        return currentSize - tmpSize;
    }
    return  0;
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
        deviceTotalSize[event.deviceId + event.eventType] =
                SafeCalculateAllocationSize(deviceTotalSize[event.deviceId + event.eventType],
                                            eventExtendAttr.size);
        // 构造allocation折线图元素
        MemoryAllocation allocation(event.timestamp, deviceTotalSize[event.deviceId + event.eventType], event.deviceId,
                                    event.eventType, false);
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
                              eventExtendAttr.owner, event->eventType, "", event->processId, event->threadId);
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
    if (event.event != LEAKS_DUMP_EVENT::MALLOC && event.event != LEAKS_DUMP_EVENT::FREE) {
        return false;
    }
    if (eventExtendAttr.size == 0) {
        Server::ServerLog::Warn("An invalid memory allocation/free event[" + event.ptr +
                                "] was detected: cannot get the 'size' "
                                "attribute from the 'attr' field.");
        return false;
    }
    // 如果是分配事件
    if (event.event == LEAKS_DUMP_EVENT::MALLOC) {
        // 已存在则忽略, 可能为重复申请
        if (allocMap.find(event.ptr + event.eventType) != allocMap.end()) {
            Server::ServerLog::Warn("An invalid memory allocation event[" + event.ptr +
                                    "] was detected: the address was already "
                                    "allocated and not released.");
            return false;
        }
        // 加入申请表
        allocMap[event.ptr + event.eventType] = &event;
    }
    // 如果是释放事件
    if (event.event == LEAKS_DUMP_EVENT::FREE) {
        // 内存分配表中未找到匹配的分配事件，则忽略
        if (allocMap.find(event.ptr + event.eventType) == allocMap.end()) {
            Server::ServerLog::Warn("An invalid memory free event[" + event.ptr +
                                    "] was detected: no corresponding allocation was "
                                    "recorded for the same address.");
            return false;
        }

        auto &allocEvent = allocMap[event.ptr + event.eventType];
        BlockEventAttr allocEventAttr;
        BuildBlockEventAttrFromEvent(*allocEvent, allocEventAttr);
        // 构造block
        MemoryBlock block(event.ptr, event.deviceId, std::abs(eventExtendAttr.size), allocEvent->timestamp,
                          event.timestamp, allocEventAttr.owner, event.eventType, "", event.processId, event.threadId);
        db->InsertMemoryBlock(block);
        // 从申请表中去除内存申请事件
        allocMap.erase(event.ptr + event.eventType);
    }
    return true;
}
void LeaksMemoryService::GetEventAttrWithDefaultValueByJson(json_t &json, BlockEventAttr &eventAttr)
{
    JsonUtil::SetByJsonKeyValue(eventAttr.addr, json, BLOCK_EVENT_ATTR_ADDR_FIELD);
    JsonUtil::SetByJsonKeyValue(eventAttr.owner, json, BLOCK_EVENT_ATTR_OWNER_FIELD);
    std::string tmp_str = JsonUtil::GetString(json, BLOCK_EVENT_ATTR_SIZE_FIELD);
    eventAttr.size = tmp_str.empty() ? 0 : NumberUtil::StringToLongLong(tmp_str);
    tmp_str = JsonUtil::GetDumpString(json, BLOCK_EVENT_ATTR_TOTAL_FIELD);
    eventAttr.total = tmp_str.empty() ? 0 : NumberUtil::StringToUnsignedLongLong(tmp_str);
    tmp_str = JsonUtil::GetDumpString(json, BLOCK_EVENT_ATTR_USED_FIELD);
    eventAttr.used = tmp_str.empty() ? 0 : NumberUtil::StringToUnsignedLongLong(tmp_str);
    tmp_str = JsonUtil::GetDumpString(json, BLOCK_EVENT_ATTR_MID_FIELD);
    eventAttr.mid = tmp_str.empty() ? 0 : NumberUtil::StringToLongLong(tmp_str);
    JsonUtil::SetByJsonKeyValue(eventAttr.owner, json, BLOCK_EVENT_ATTR_OWNER_FIELD);
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
    GetEventAttrWithDefaultValueByJson(json, eventAttr);
    if (event.event == LEAKS_DUMP_EVENT::FREE) {
        eventAttr.size = -std::abs(eventAttr.size);
    }
    if (event.event == LEAKS_DUMP_EVENT::MALLOC) {
        eventAttr.size = std::abs(eventAttr.size);
    }
}
// 该方法用于寻找当前owner set中两两字符串的最长相同前缀，并添加到owners中, 暴力枚举,时间复杂度O(n^2), 后续考虑优化为Trie树
static void HandleOwnerSet(std::set<std::string> &owners)
{
    std::set<std::string> newPrefixes;
    for (auto it1 = owners.begin(); it1 != owners.end(); ++it1) {
        for (auto it2 = std::next(it1); it2 != owners.end(); ++it2) {
            std::string lcp = StringUtil::FindLCP(*it1, *it2);
            if (!lcp.empty() && !StringUtil::EndWith(lcp, std::string(1, OWNER_STRING_DELIMITER))) {
                owners.insert(lcp);
            }
        }
    }
}

static bool CheckIfSubTagIsPrefixOfOwner(const std::vector<std::string> &subNodeTags,
                                         const std::string &owner)
{
    for (auto &subNodeTag : subNodeTags) {
        if (owner.find(subNodeTag) != std::string::npos) {
            return true;
        }
    }
    return false;
}

static std::vector<std::string> FindSubNodeTags(const std::string &tag, const std::set<std::string> &owners)
{
    std::vector<std::string> subNodeTags;
    if (!LeaksMemoryDetailTreeNode::IsValidOwnerTag(tag)) {
        Server::ServerLog::Warn("[LeaksDetail]The tag of current node is invalid.");
        return subNodeTags;
    }
    if (tag == LEAKS_MEMORY_ALLOC_OWNER_HAL_CANN) {
        subNodeTags = std::vector<std::string>(LEAKS_MEMORY_ALLOC_OWNER_CANN_BASE_TAGS.begin(),
                                               LEAKS_MEMORY_ALLOC_OWNER_CANN_BASE_TAGS.end());
        return subNodeTags;
    }
    if (tag == LEAKS_MEMORY_ALLOC_OWNER_HAL_FRAMEWORK) {
        subNodeTags = std::vector<std::string>(LEAKS_MEMORY_ALLOC_OWNER_FRAMEWORK_BASE_TAGS.begin(),
                                               LEAKS_MEMORY_ALLOC_OWNER_FRAMEWORK_BASE_TAGS.end());
        return subNodeTags;
    }
    long layer = std::count(tag.begin(), tag.end(), OWNER_STRING_DELIMITER);
    for (auto &owner : owners) {
        if (std::count(owner.begin(), owner.end(), OWNER_STRING_DELIMITER) <= layer) {
            continue;
        }
        if (owner.find(tag) == std::string::npos) {
            continue;
        }
        if (CheckIfSubTagIsPrefixOfOwner(subNodeTags, owner)) {
            continue;
        }
        subNodeTags.push_back(owner);
    }
    return subNodeTags;
}

// depth 从1开始
void LeaksMemoryService::BuildMemoryAllocDetailTreeNode(const std::string &deviceId, const uint64_t &timestamp,
                                                        const std::set<std::string> &owners,
                                                        LeaksMemoryDetailTreeNode &curNode, int depth)
{
    // 实际从框架层开始统计为第一层
    if (depth > MAX_TREE_DEPTH) {
        Server::ServerLog::Warn("[LeaksDetail]The depth of current tree has exceeded 8.");
        return;
    }
    if (curNode.size == 0) {
        return;
    }
    auto database = Timeline::DataBaseManager::Instance().GetLeaksMemoryDatabase("");
    if (database == nullptr) {
        Server::ServerLog::Error("[LeaksDetail]Cannot get leaks db connections from database manager");
        return;
    }
    std::vector<std::string> subNodeOwnerTags = FindSubNodeTags(curNode.tag, owners);
    for (auto &subNodeOwnerTag : subNodeOwnerTags) {
        LeaksMemoryDetailTreeNode subNode;
        subNode.tag = subNodeOwnerTag;
        subNode.size = database->QueryTotalSizeUntilTimestampUsingOwner(deviceId, timestamp, subNodeOwnerTag);
        subNode.name = LeaksMemoryDetailTreeNode::GetNodeNameByOwnerTag(subNodeOwnerTag);
        if (subNode.size > 0) {
            // 递归构造子节点
            BuildMemoryAllocDetailTreeNode(deviceId, timestamp, owners, subNode, depth + 1);
            curNode.InsertSubNode(subNode);
        }
    }
}

bool LeaksMemoryService::ParseMemoryAllocDetailTreeByTimestamp(const std::string &deviceId,
                                                               const uint64_t &timestamp,
                                                               const std::string &eventType,
                                                               LeaksMemoryDetailTreeNode &detailTree,
                                                               bool relativeTime)
{
    auto database = Timeline::DataBaseManager::Instance().GetLeaksMemoryDatabase("");
    if (database == nullptr) {
        Server::ServerLog::Error("Cannot get leaks db connections from database manager");
        return false;
    }
    uint64_t minTimestamp = database->QueryMemoryEventExtremumTimestamp(deviceId, true);
    uint64_t maxTimestamp = database->QueryMemoryEventExtremumTimestamp(deviceId, false);
    uint64_t realTimestamp = timestamp;
    if (relativeTime) {
        realTimestamp += minTimestamp;
    }
    if (realTimestamp > maxTimestamp || realTimestamp < minTimestamp) {
        Server::ServerLog::Error("Parse memory alloc details failed: invalid timestamp");
        return false;
    }
    // 构造固定层顶层-进程占用, 来自HAL最后一次分配的总内存
    auto latestHALAllocation = database->QueryLatestAllocationWithinTimestamp(deviceId,
                                                                              LEAKS_DUMP_EVENT_TYPE::MALLOC_FREE_HAL,
                                                                              realTimestamp);
    if (!latestHALAllocation.has_value()) {
        Server::ServerLog::Error("Parse memory alloc details failed: empty HAL allocation data");
        return false;
    }
    detailTree.size = latestHALAllocation->totalSize;
    if (latestHALAllocation->totalSize == 0) {
        Server::ServerLog::Warn("Parse memory alloc details: empty data.");
        return true;
    }
    detailTree.name = LEAKS_MEMORY_ALLOC_OWNER_HAL_NAME;
    if (eventType == LEAKS_DUMP_EVENT_TYPE::MALLOC_FREE_HAL) {
        detailTree.tag = LEAKS_MEMORY_ALLOC_OWNER_HAL_CANN; // 构造CANN层 - ATB/MindSpore/PTA占用，所有owner前缀为带有PTA/ATB/MINDSPORE
    } else {
        detailTree.tag = LEAKS_MEMORY_ALLOC_OWNER_HAL_FRAMEWORK; // 构造框架层 - ATB/MindSpore/PTA占用，所有owner前缀为带有PTA/ATB/MINDSPORE
    }
    std::set<std::string> owners(LEAKS_MEMORY_ALLOC_OWNER_FIXED_TAGS);
    database->QueryMemoryBlocksOwnersReleasedAfterTimestamp(deviceId, eventType, realTimestamp, owners);
    if (owners.empty()) {
        Server::ServerLog::Warn("Parse memory alloc details: empty data.");
        return true;
    }
    HandleOwnerSet(owners);
    BuildMemoryAllocDetailTreeNode(deviceId, realTimestamp, owners, detailTree, 1);
    return true;
}

bool LeaksMemoryService::ParseThreadPythonTrace(LeaksMemoryPythonTrace &trace)
{
    std::stack<PythonTraceSlice *> callStack;
    for (auto &slice : trace.slices) {
        // 弹出栈中已经结束的func
        while (!callStack.empty() && callStack.top()->endTimestamp < slice.startTimestamp) {
            callStack.pop();
        }

        // 当前调用栈深度 = 栈的大小
        size_t callStackDepth = callStack.size();
        if (callStackDepth > INT_MAX) {
            Server::ServerLog::Error("Build python call stack failed: the stack depth exceeds the max of int");
            return false;
        }
        slice.depth = static_cast<int>(callStackDepth);
        // 将当前函数入栈
        callStack.push(&slice);
    }

    return true;
}

bool LeaksMemoryService::IsValidMemoryEventType(const std::string &event, const std::string &eventType)
{
    if (EVENT_TYPE_MAP.find(event) == EVENT_TYPE_MAP.end()) {
        return false;
    }
    const auto& eventTypes = EVENT_TYPE_MAP.at(event);
    return eventTypes.find(eventType) != eventTypes.end();
}
}  // Memory
}  // Module
}  // Dic