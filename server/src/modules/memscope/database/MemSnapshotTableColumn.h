/*
* -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
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

#ifndef PROFILER_SERVER_MEMSNAPSHOTTABLECOLUMN_H
#define PROFILER_SERVER_MEMSNAPSHOTTABLECOLUMN_H
#include "CommonRequests.h"

namespace Dic::Module::MemSnapshot {
namespace TraceEntryTableColumn {
constexpr std::string_view ID = "id";
constexpr std::string_view ACTION = "action";
constexpr std::string_view ADDRESS = "address";
constexpr std::string_view SIZE = "size";
constexpr std::string_view STREAM = "stream";
constexpr std::string_view ALLOCATED = "allocated";
constexpr std::string_view ACTIVE = "active";
constexpr std::string_view RESERVED = "reserved";
constexpr std::string_view CALLSTACK = "callstack";

inline const std::vector<Dic::Protocol::TableViewColumn> FIELD_FULL_COLUMNS = {
    {ID, "id", true, true, false, true}, // ID, 事件索引
    {ACTION, "action", true, true, true, false}, // 事件类型
    {ADDRESS, "address", true, true, true, false}, // 事件地址
    {SIZE, "size", true, true, false, true}, // 事件关联内存大小，单位bytes
    {STREAM, "stream", true, true, true, false}, // 事件所属流ptr
    {ALLOCATED, "allocated", true, true, false, true}, // 事件发生时刻allocated内存
    {ACTIVE, "active", true, true, false, true}, // 事件发生时刻active内存
    {RESERVED, "reserved", true, true, false, true}, // 事件发生时刻预留内存大小
    {CALLSTACK, "callstack", true, false, true, false} // 事件发生的调用栈
};
}

namespace BlockTableColumn {
constexpr std::string_view ID = "id";
constexpr std::string_view ADDRESS = "address";
constexpr std::string_view SIZE = "size";
constexpr std::string_view REQUESTED_SIZE = "requestedSize";
constexpr std::string_view STATE = "state";
constexpr std::string_view ALLOC_EVENT_ID = "allocEventId";
constexpr std::string_view FREE_EVENT_ID = "freeEventId";
inline const std::vector<Dic::Protocol::TableViewColumn> FIELD_FULL_COLUMNS = {
    {ID, "id", true, true, false, true},
    {ADDRESS, "address", true, true, true, false},
    {SIZE, "size", true, true, false, true},
    {REQUESTED_SIZE, "requestedSize", true, true, false, true},
    {STATE, "state", true, true, true, false},
    {ALLOC_EVENT_ID, "allocEventId", true, true, true, false},
    {FREE_EVENT_ID, "freeEventId", true, true, true, false}
};
}
}
#endif //PROFILER_SERVER_MEMSNAPSHOTTABLECOLUMN_H
