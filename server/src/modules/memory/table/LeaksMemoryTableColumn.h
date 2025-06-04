/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_LEAKSMEMORYCOLUMN_H
#define PROFILER_SERVER_LEAKSMEMORYCOLUMN_H
#include <string>
namespace Dic {
namespace Module {
namespace Memory {
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
}
namespace MemoryAllocationTableColumn {
    constexpr std::string_view ID = "id";
    constexpr std::string_view TIMESTAMP = "timestamp";
    constexpr std::string_view TOTAL_SIZE = "totalSize";
    constexpr std::string_view OPTIMIZED = "optimized";
    constexpr std::string_view DEVICE_ID = "deviceId";
    constexpr std::string_view EVENT_TYPE = "eventType";
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
}
namespace MemoryPythonTraceTableColumn {
    constexpr std::string_view FUNC_INFO = "FuncInfo";
    constexpr std::string_view START_TIME = "`StartTime(ns)`";
    constexpr std::string_view END_TIME = "`EndTime(ns)`";
    constexpr std::string_view THREAD_ID = "`Thread Id`";
    constexpr std::string_view PROCESS_ID = "`Process Id`";
}
}
}
}
#endif // PROFILER_SERVER_LEAKSMEMORYCOLUMN_H
