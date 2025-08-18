/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_LEAKSMEMORYCOLUMN_H
#define PROFILER_SERVER_LEAKSMEMORYCOLUMN_H

#include "pch.h"

namespace Dic::Module::MemoryDetail {
struct SqliteDbTableColumn {
    std::string_view name;
    std::string_view key;
    bool visible{true};
    bool sortable{false};
    bool searchable{false};

    SqliteDbTableColumn(std::string_view name, std::string_view key, bool visible, bool sortable, bool searchable)
        : name(name),
          key(key),
          visible(visible),
          sortable(sortable),
          searchable(searchable) {}

    SqliteDbTableColumn(std::string_view name, std::string_view key)
        : name(name),
          key(key)
    {
        visible = false;
    }
};

inline std::vector<SqliteDbTableColumn>::const_iterator FindColumnByKey(std::string_view key,
                                                                        const std::vector<SqliteDbTableColumn> &columns)
{
    return std::find_if(columns.begin(), columns.end(), [key](const SqliteDbTableColumn& col) {
            return key == col.key;
    });
}

namespace MemoryEventTableColumn {
    constexpr std::string_view ID = "ID";
    constexpr std::string_view EVENT = "Event";
    constexpr std::string_view EVENT_TYPE = "`Event Type`";
    constexpr std::string_view NAME = "Name";
    constexpr std::string_view TIMESTAMP = "`Timestamp(ns)`";
    constexpr std::string_view PROCESS_ID = "`Process Id`";
    constexpr std::string_view THREAD_ID = "`Thread Id`";
    constexpr std::string_view DEVICE_ID = "`Device Id`";
    constexpr std::string_view PTR = "Ptr";
    constexpr std::string_view ATTR = "Attr";
    // 可能存在的列
    constexpr std::string_view CALL_STACK_PYTHON = "`Call Stack(Python)`";
    constexpr std::string_view CALL_STACK_C = "`Call Stack(C)`";
    inline const std::vector<SqliteDbTableColumn> FIELD_FULL_COLUMNS = {
        {ID, "id", true, true, false}, // ID, 事件ID
        {EVENT, "event", true, false, true}, // Event, 事件类型
        {EVENT_TYPE, "eventType", true, true, true}, // Event Type, 事件子类型
        {NAME, "name", true, true, true}, // Name, 名称
        {TIMESTAMP, "timestamp", true, true, false}, // Timestamp(ns), 发生时间
        {PROCESS_ID, "processId", true, true, true}, // Process ID, 进程ID
        {THREAD_ID, "threadId", true, true, true}, // Thread ID, 线程ID
        {DEVICE_ID, "deviceId"},
        {PTR, "ptr", true, true, true}, // Ptr, 内存地址
        {ATTR, "attr", true, false, true}, // Attr, 特有属性
        // 可选列
        {CALL_STACK_PYTHON, "callStackPython", true, false, true}, // Call Stack(Python), Python调用栈
        {CALL_STACK_C, "callStackC", true, false, true} // Call Stack(C), C调用栈
    };
}
namespace MemoryAllocationTableColumn {
    constexpr std::string_view ID = "id";
    constexpr std::string_view TIMESTAMP = "timestamp";
    constexpr std::string_view TOTAL_SIZE = "totalSize";
    constexpr std::string_view OPTIMIZED = "optimized";
    constexpr std::string_view DEVICE_ID = "deviceId";
    constexpr std::string_view EVENT_TYPE = "eventType";
    constexpr std::string_view FULL_COLUMNS_WITHOUT_ID[] = {TIMESTAMP, TOTAL_SIZE, OPTIMIZED,
                                                            DEVICE_ID, EVENT_TYPE};
    constexpr std::string_view FULL_COLUMNS[] = {ID, TIMESTAMP, TOTAL_SIZE,
                                                 OPTIMIZED, DEVICE_ID, EVENT_TYPE};
}
namespace MemoryBlockTableColumn {
    constexpr std::string_view ID = "id";
    constexpr std::string_view DEVICE_ID = "deviceId";
    constexpr std::string_view ADDR = "addr";
    constexpr std::string_view SIZE = "size";
    constexpr std::string_view START_TIMESTAMP = "startTimestamp";
    constexpr std::string_view END_TIMESTAMP = "endTimestamp";
    constexpr std::string_view EVENT_TYPE = "eventType";
    constexpr std::string_view OWNER = "owner";
    constexpr std::string_view ATTR = "attr";
    constexpr std::string_view PROCESS_ID = "processId";
    constexpr std::string_view THREAD_ID = "threadId";
    constexpr std::string_view FULL_COLUMNS_WITHOUT_ID[] = {DEVICE_ID, ADDR, SIZE, START_TIMESTAMP,
                                                            END_TIMESTAMP, EVENT_TYPE, OWNER,
                                                            ATTR, PROCESS_ID, THREAD_ID};
    inline const std::vector<SqliteDbTableColumn>  FIELD_FULL_COLUMNS = {
        {ID, "id", true, true, false}, // ID, 内存块ID
        {DEVICE_ID, "deviceId"},
        {ADDR, "addr", true, true, true}, // Addr, 内存地址
        {SIZE, "size", true, true, false}, // Size, 内存块大小
        {START_TIMESTAMP, "startTimestamp", true, true, false}, // Malloc Timestamp(ns), 申请时间
        {END_TIMESTAMP, "endTimestamp", true, true, false}, // Free Timestamp(ns), 释放时间
        {EVENT_TYPE, "eventType", false, true, true}, // 事件子类型
        {OWNER, "owner", true, true, true}, // 内存块持有者(标签)
        {PROCESS_ID, "processId", true, true, true}, // Process Id, 进程ID
        {THREAD_ID, "threadId", true, true, true}, // Thread Id, 线程ID
        {ATTR, "attr", true, false, true} // Attr, 特有属性
    };
}
namespace MemoryPythonTraceTableColumn {
    constexpr std::string_view ID = "ROWID"; // 表中无该列, 为sqlite表的隐藏列, 用于标识唯一一行
    constexpr std::string_view FUNC_INFO = "FuncInfo";
    constexpr std::string_view START_TIME = "`StartTime(ns)`";
    constexpr std::string_view END_TIME = "`EndTime(ns)`";
    constexpr std::string_view THREAD_ID = "`Thread Id`";
    constexpr std::string_view PROCESS_ID = "`Process Id`";
    constexpr std::string_view DEPTH = "Depth"; // 源数据中无该列, 为便于后续做查询抽样, 在解析时生成
}
}
#endif // PROFILER_SERVER_LEAKSMEMORYCOLUMN_H
