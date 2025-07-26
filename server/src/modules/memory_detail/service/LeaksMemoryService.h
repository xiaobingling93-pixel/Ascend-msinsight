/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_LEAKSMEMORYSERVICE_H
#define PROFILER_SERVER_LEAKSMEMORYSERVICE_H

#include <utility>
#include "pch.h"
#include "MemoryDetailDefs.h"
#include "LeaksMemoryDatabase.h"
#include "LeaksMemoryDetailTreeNode.h"
#include "MemoryDetailProtocolResponse.h"

namespace Dic {
namespace Module {
namespace MemoryDetail {
struct BlockEventAttr {
    std::string addr;
    int64_t size;
    std::string owner;
    uint64_t total;
    uint64_t used;
    int64_t mid;
};
const std::string START_PATTERN = "start:";
const std::string STOP_PATTERN = "stop:";
const char OWNER_STRING_DELIMITER = '@';
// 内存事件
struct LEAKS_DUMP_EVENT {
    inline const static std::string MALLOC = "MALLOC"; // 内存申请事件
    inline const static std::string FREE = "FREE"; // 内存释放事件
    inline const static std::string ACCESS = "ACCESS"; // 内存访问
    inline const static std::string OP_LAUNCH = "OP_LAUNCH"; // 算子launch
    inline const static std::string KERNEL_LAUNCH = "KERNEL_LAUNCH"; // kernel launch
    inline const static std::string SYSTEM = "SYSTEM"; // 系统事件
};
// 内存事件类型
struct LEAKS_DUMP_EVENT_TYPE {
    // 内存申请/释放事件 子类型
    inline const static std::string MALLOC_FREE_PTA = "PTA"; // PTA内存池分配(从PTA内存池申请)
    inline const static std::string MALLOC_FREE_MINDSPORE = "MINDSPORE"; // MINDSPORE内存池分配(从MINDSPORE内存池申请)
    inline const static std::string MALLOC_FREE_ATB = "ATB"; // ATB申请
    inline const static std::string MALLOC_FREE_HAL = "HAL"; // 从HAL申请
    // 内存访问事件 子类型
    inline const static std::string ACCESS_READ = "READ"; // 内存读事件
    inline const static std::string ACCESS_WRITE = "WRITE"; // 内存写事件
    // 算子launch事件 子类型
    inline const static std::string OP_LAUNCH_ATEN_START = "ATEN_START"; // aten开始, name字段标识aten算子名
    inline const static std::string OP_LAUNCH_ATEN_END = "ATEN_END"; // aten结束
    // kernel launch事件 子类型
    inline const static std::string KERNEL_LAUNCH = "KERNEL_LAUNCH"; // kernel launch
    inline const static std::string KERNEL_LAUNCH_START = "KERNEL_EXECUTE_START"; // kernel launch 开始
    inline const static std::string KERNEL_LAUNCH_END = "KERNEL_EXECUTE_END"; // kernel launch 结束
    // 系统事件 子类型
    inline const static std::string SYSTEM_ACL_INIT = "ACL_INIT"; // ACL初始化
    inline const static std::string SYSTEM_ACL_FINI = "ACL_FINI"; // ACL结束
};
// 内存事件 - 类型映射表
const std::unordered_map<std::string, std::set<std::string>> EVENT_TYPE_MAP = {
    {LEAKS_DUMP_EVENT::MALLOC, {LEAKS_DUMP_EVENT_TYPE::MALLOC_FREE_PTA,
                                LEAKS_DUMP_EVENT_TYPE::MALLOC_FREE_MINDSPORE,
                                LEAKS_DUMP_EVENT_TYPE::MALLOC_FREE_ATB,
                                LEAKS_DUMP_EVENT_TYPE::MALLOC_FREE_HAL}},
    {LEAKS_DUMP_EVENT::FREE, {LEAKS_DUMP_EVENT_TYPE::MALLOC_FREE_PTA,
                              LEAKS_DUMP_EVENT_TYPE::MALLOC_FREE_MINDSPORE,
                              LEAKS_DUMP_EVENT_TYPE::MALLOC_FREE_ATB,
                              LEAKS_DUMP_EVENT_TYPE::MALLOC_FREE_HAL}},
    {LEAKS_DUMP_EVENT::ACCESS, {LEAKS_DUMP_EVENT_TYPE::ACCESS_READ,
                                LEAKS_DUMP_EVENT_TYPE::ACCESS_WRITE}},
    {LEAKS_DUMP_EVENT::OP_LAUNCH, {LEAKS_DUMP_EVENT_TYPE::OP_LAUNCH_ATEN_START,
                                   LEAKS_DUMP_EVENT_TYPE::OP_LAUNCH_ATEN_END}},
    {LEAKS_DUMP_EVENT::KERNEL_LAUNCH, {LEAKS_DUMP_EVENT_TYPE::KERNEL_LAUNCH,
                                       LEAKS_DUMP_EVENT_TYPE::KERNEL_LAUNCH_START,
                                       LEAKS_DUMP_EVENT_TYPE::KERNEL_LAUNCH_END}},
    {LEAKS_DUMP_EVENT::SYSTEM, {LEAKS_DUMP_EVENT_TYPE::SYSTEM_ACL_INIT,
                                LEAKS_DUMP_EVENT_TYPE::SYSTEM_ACL_FINI}}
};
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
