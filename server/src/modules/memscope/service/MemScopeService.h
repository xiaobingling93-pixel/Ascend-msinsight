/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

#ifndef PROFILER_SERVER_MEM_SCOPE_SERVICE_H
#define PROFILER_SERVER_MEM_SCOPE_SERVICE_H

#include <utility>
#include "pch.h"
#include "DataBaseManager.h"
#include "MemScopeDefs.h"
#include "MemScopeEntities.h"
#include "MemScopeDatabase.h"
#include "MemScopeEventTree.h"
#include "MemScopeProtocolResponse.h"

namespace Dic {
namespace Module {
namespace MemScope {
struct ParseContext {
    std::vector<MemScopeEvent> events;
    std::unordered_map<std::string, std::map<std::string, const MemScopeEvent *>> deviceMallocMap;
    std::unordered_map<std::string, uint64_t> deviceTotalSize;
    std::unordered_map<uint64_t, EventGroup> eventGroupMap;
    std::shared_ptr<FullDb::MemScopeDatabase> db;
    std::set<std::string> deviceIds;

    bool CheckDeviceIdValid(const std::string &deviceId);
};

class MemScopeService {
public:
    static void ParseEventsToBlockAndAllocations(ParseContext &context);
    static bool ParseMemoryMemScopeDumpEventsAndPythonTraces(const std::string &fileId);
    static void ParserEnd(const std::string &rankId, bool result);
    static void ParseCallBack(const std::string &fileId, bool result, const std::string &msg);
    static bool ParseMemoryAllocDetailTreeByTimestamp(const std::string &deviceId,
                                                      const uint64_t &timestamp,
                                                      const std::string &eventType,
                                                      MemScopeMemoryDetailTreeNode &detailTree,
                                                      bool relativeTime);
    // 传入slices必须为已按照startTimestamp升序排序的数组
    static bool ParseThreadPythonTrace(MemScopePythonTrace &trace, ParseContext &context);
    // 判断eventType是否合法
    static bool IsValidMemoryEventType(const std::string &event, const std::string &eventType);
    static std::optional<ParseContext> BuildContext(std::shared_ptr<FullDb::MemScopeDatabase> &db);
private:
    static void ParseRemainMallocEvents(ParseContext &context);
    static bool SingleDeviceEventParse(const MemScopeEvent &event,
                                       ParseContext &context);
    // 递归,depth从0开始
    static void BuildMemoryAllocDetailTreeNode(const std::string &deviceId,
                                        const uint64_t &timestamp,
                                        const std::set<std::string> &owners,
                                        MemScopeMemoryDetailTreeNode &curNode, int depth);
    static void SetMemoryBlockExtendByEventGroup(MemoryBlock& block, const uint64_t groupId,
                                                 ParseContext &context);
};
}  // Memory
}  // Module
}  // Dic

#endif  // PROFILER_SERVER_MEM_SCOPE_SERVICE_H
