/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_LEAKSMEMORYSERVICE_H
#define PROFILER_SERVER_LEAKSMEMORYSERVICE_H

#include <utility>
#include "pch.h"
#include "DataBaseManager.h"
#include "MemoryDetailDefs.h"
#include "MemoryDetailEntities.h"
#include "LeaksMemoryDatabase.h"
#include "LeaksMemoryDetailTreeNode.h"
#include "MemoryDetailProtocolResponse.h"

namespace Dic {
namespace Module {
namespace MemoryDetail {
struct ParseContext {
    std::vector<MemoryEvent> events;
    std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> deviceExtremumTsMap;
    std::unordered_map<std::string, std::map<std::string, const MemoryEvent *>> deviceMallocMap;
    std::unordered_map<std::string, uint64_t> deviceTotalSize;
    std::unordered_map<uint64_t, EventGroup> eventGroupMap;
    std::shared_ptr<FullDb::LeaksMemoryDatabase> db;

    bool CheckDeviceIdValid(const std::string &deviceId);
};

class LeaksMemoryService {
public:
    static void ParseEventsToBlockAndAllocations(ParseContext &context);
    static void BuildEventAttrs(const MemoryEvent &event, MemoryEventAttrs &eventAttr);
    static bool ParseMemoryLeaksDumpEvents(const std::string &fileId);
    static void ParserEnd(const std::string &rankId, bool result);
    static void ParseCallBack(const std::string &fileId, bool result, const std::string &msg);
    static bool ParseMemoryAllocDetailTreeByTimestamp(const std::string &deviceId,
                                                      const uint64_t &timestamp,
                                                      const std::string &eventType,
                                                      LeaksMemoryDetailTreeNode &detailTree,
                                                      bool relativeTime);
    // 传入slices必须为已按照startTimestamp升序排序的数组
    static bool ParseThreadPythonTrace(LeaksMemoryPythonTrace &trace);
    // 判断eventType是否合法
    static bool IsValidMemoryEventType(const std::string &event, const std::string &eventType);
    static std::optional<ParseContext> BuildContext(std::shared_ptr<FullDb::LeaksMemoryDatabase> &db);
private:
    static void ParseRemainMallocEvents(ParseContext &context);
    static bool SingleDeviceEventParse(const MemoryEvent &event,
                                       const MemoryEventAttrs &eventAttrs,
                                       ParseContext &context);
    // 递归,depth从0开始
    static void BuildMemoryAllocDetailTreeNode(const std::string &deviceId,
                                        const uint64_t &timestamp,
                                        const std::set<std::string> &owners,
                                        LeaksMemoryDetailTreeNode &curNode, int depth);
    static void GetEventAttrWithDefaultValueByJson(json_t &json, MemoryEventAttrs &eventAttrs);
    static std::optional<MemoryBlockAttrs> BuildMemoryBlockAttrsByMallocEvent(const MemoryEvent& mallocEvent,
                                                                              const EventGroup& eventGroup,
                                                                              const uint64_t minTimestamp = 0);
};
}  // Memory
}  // Module
}  // Dic

#endif  // PROFILER_SERVER_LEAKSMEMORYSERVICE_H
