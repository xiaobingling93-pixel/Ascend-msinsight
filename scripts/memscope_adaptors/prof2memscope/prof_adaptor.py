"""
-------------------------------------------------------------------------
This file is part of the MindStudio project.
Copyright (c) 2026 Huawei Technologies Co.,Ltd.

MindStudio is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:

         http://license.coscl.org.cn/MulanPSL2

THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details.
-------------------------------------------------------------------------
"""
from dataclasses import asdict
from torch_npu.profiler.analysis.prof_parse._event_tree_parser import (
    _EventType,
    _ProfilerEvent,
    _ExtraFields_Allocation,
    _ExtraFields_PyCall,
)
from memscope.event import MemoryEvent, PythonTraceEvent, MallocFreeEventAttr
from memscope.defs import Event, EventType


class EventBuilder:
    @staticmethod
    def build_memory_event_by_prof_event(profiler_event: _ProfilerEvent):
        extra: _ExtraFields_Allocation = profiler_event.extra_fields
        alloc_size = extra.alloc_size
        return MemoryEvent(
            event=Event.ALLOC if alloc_size >= 0 else Event.FREE,
            event_type=EventType.PTA,
            name="N/A",
            timestamp=profiler_event.start_time_ns,
            tid=extra.tid,
            pid=extra.pid,
            did=extra.device_index,
            ptr=hex(extra.ptr),
            attr=asdict(MallocFreeEventAttr(hex(extra.ptr), str(extra.alloc_size), str(extra.total_allocated),
                                            str(extra.total_reserved), str(extra.stream_ptr)))
        )

    @staticmethod
    def build_python_trace_event_by_prof_event(profiler_event: _ProfilerEvent):
        extra: _ExtraFields_PyCall = profiler_event.extra_fields
        return PythonTraceEvent(
            func_info=extra.name,
            start_time_ns=profiler_event.start_time_ns,
            end_time_ns=extra.end_time_ns,
            pid=extra.pid,
            tid=extra.tid
        )
