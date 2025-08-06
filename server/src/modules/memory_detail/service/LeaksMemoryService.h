/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_LEAKSMEMORYSERVICE_H
#define PROFILER_SERVER_LEAKSMEMORYSERVICE_H

#include <utility>
#include "pch.h"
#include "MemoryDetailDefs.h"
#include "MemoryDetailEntities.h"
#include "LeaksMemoryDatabase.h"
#include "LeaksMemoryDetailTreeNode.h"
#include "MemoryDetailProtocolResponse.h"

namespace Dic {
namespace Module {
namespace MemoryDetail {


class LeaksMemoryService {
public:
    static void ParseEventsToBlockAndAllocations(const std::vector<MemoryEvent> &events,
                                                 const std::shared_ptr<FullDb::LeaksMemoryDatabase> &db);
    static void BuildBlockEventAttrFromEvent(const MemoryEvent &event, BlockEventAttr &eventAttr);
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
private:
    static bool SingleDeviceEventParse(const std::shared_ptr<FullDb::LeaksMemoryDatabase> &db, const MemoryEvent &event,
                                       std::map<std::string, const MemoryEvent *> &allocMap,
                                       const BlockEventAttr &eventExtendAttr);
    // 递归,depth从0开始
    static void BuildMemoryAllocDetailTreeNode(const std::string &deviceId,
                                        const uint64_t &timestamp,
                                        const std::set<std::string> &owners,
                                        LeaksMemoryDetailTreeNode &curNode, int depth);
    static void GetEventAttrWithDefaultValueByJson(json_t &json, BlockEventAttr &eventAttr);
    inline static const std::string BLOCK_EVENT_ATTR_SIZE_FIELD = "size";
    inline static const std::string BLOCK_EVENT_ATTR_OWNER_FIELD = "owner";
    inline static const std::string BLOCK_EVENT_ATTR_ADDR_FIELD = "addr";
    inline static const std::string BLOCK_EVENT_ATTR_TOTAL_FIELD = "total";
    inline static const std::string BLOCK_EVENT_ATTR_USED_FIELD = "used";
    inline static const std::string BLOCK_EVENT_ATTR_MID_FIELD = "MID";
};
}  // Memory
}  // Module
}  // Dic

#endif  // PROFILER_SERVER_LEAKSMEMORYSERVICE_H
