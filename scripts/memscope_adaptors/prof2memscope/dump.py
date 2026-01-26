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
import os.path
import logging
import argparse
from enum import Enum
from typing import List

from torch_npu.profiler.analysis._profiler_config import ProfilerConfig
from torch_npu.profiler.analysis.prof_parse._event_tree_parser import EventTree, _EventType

from profiler_event_analyze_patch import init_patch
from memscope import MemoryEvent, PythonTraceEvent, MemScopeDb
from prof_adaptor import EventBuilder

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[logging.StreamHandler()]  # 输出到 stdout
)

# profiler解析事件时的patch
init_patch()


def analyze_profiler_events() -> (List[MemoryEvent], List[PythonTraceEvent]):
    logging.info(f"Starting to parse profiler data...")
    event_tree = EventTree(DumpConfig.profiler_path)
    sorted_events = event_tree.sorted_events
    if not sorted_events:
        raise RuntimeError("No events were parsed from the profiler data.")
    min_time = sorted_events[0].start_time_ns
    max_time = sorted_events[-1].start_time_ns
    logging.info(f"Parsing profiler data finished. "
                 f"(The first profiler event start timestamp: {min_time}, "
                 f"The last profiler event start timestamp {max_time})")
    DumpConfig.reset_time_range_by_events_time(min_time, max_time)
    allocation_events: List[MemoryEvent] = []
    python_trace_events: List[PythonTraceEvent] = []
    for event in sorted_events:
        if event.tag is _EventType.Allocation:
            if DumpConfig.crop_mode is CropMode.NO_CROPPING or DumpConfig.start <= event.start_time_ns <= DumpConfig.end:
                allocation_events.append(EventBuilder.build_memory_event_by_prof_event(event))
        elif event.tag is _EventType.PyCall:
            # 无剪裁模式
            if DumpConfig.crop_mode is CropMode.NO_CROPPING:
                python_trace_events.append(EventBuilder.build_python_trace_event_by_prof_event(event))
                continue
            # 如果python调用与剪裁区间无交集，直接丢弃
            if event.start_time_ns > DumpConfig.end or event.end_time_ns < DumpConfig.start:
                continue
            trace_event = EventBuilder.build_python_trace_event_by_prof_event(event)
            # 取调用事件区间与剪裁起终点的交集
            trace_event.start_time_ns = max(DumpConfig.start, trace_event.start_time_ns)
            trace_event.end_time_ns = min(DumpConfig.end, trace_event.end_time_ns)
            python_trace_events.append(trace_event)
    return allocation_events, python_trace_events


def save_memory_events(events: List[MemoryEvent], db: MemScopeDb):
    dump_event_table = db.get_dump_table()
    event_records = [event.to_dict() for event in events]
    dump_event_table.insert_records(db.conn, event_records)


def save_python_trace_events(events: List[PythonTraceEvent], db: MemScopeDb):
    pid_python_trace_map = {}
    for event in events:
        if event.pid not in pid_python_trace_map:
            pid_python_trace_map[event.pid] = []
        pid_python_trace_map[event.pid].append(event.to_dict())

    for pid, event_records in pid_python_trace_map.items():
        python_trace_table = db.get_python_trace_table(pid)
        python_trace_table.insert_records(db.conn, event_records)


def dump_profiler_data_to_memscope():
    # Parse sorted allocation events and python trace events from the raw profiler data.
    allocation_events, python_trace_events = analyze_profiler_events()
    # Save the data in memscope format.
    try:
        db = MemScopeDb(DumpConfig.output_path)
    except Exception as e:
        raise RuntimeError("Failed to create output db file.") from e
    logging.info(
        f"Starting save to db, allocation_events:{len(allocation_events)}, trace_events:{len(python_trace_events)} ...")
    save_memory_events(allocation_events, db)
    save_python_trace_events(python_trace_events, db)
    db.conn.close()
    logging.info(f"Dump profiler data to memscope data success, output path: {DumpConfig.output_path}")


class CropMode(Enum):
    NO_CROPPING = 1  # 不做剪裁
    CROP_FIXED_DURATION_FROM_MIN_EVENT_TIME = 2  # 从解析事件列表的最小时间剪裁指定duration
    CROP_FIXED_DURATION_FROM_SPECIFIED_START_TIME = 3  # 从指定时间开始剪裁指定duration
    CROP_FROM_SPECIFIED_START_TIME_TO_THE_END = 4  # 从指定时间开始剪裁到结尾


class DumpConfig:
    # 可选参数
    profiler_path: str
    start: int
    duration: int
    output_path: str

    # 剪裁参数(分析生成)
    end: int
    crop_mode: CropMode

    @classmethod
    def init(cls):
        parser = argparse.ArgumentParser(description="Parse PyTorch Profiler data into memscope format.")
        parser.add_argument("profiler_path", type=str,
                            help="Specify the PyTorch Profiler directory to be parsed.")
        parser.add_argument("-s", "--start",
                            type=lambda x: int(x) if int(x) > 0 else parser.error(f"{x} is not a positive integer"),
                            default=-1, required=False,
                            help="Specify the start time for data parsing and trimming, in nanoseconds (ns)."
                                 "If not provided, trimming will start from the beginning of the profiler data.")
        parser.add_argument("-d", "--duration",
                            type=lambda x: int(x) if int(x) > 0 else parser.error(f"{x} is not a positive integer"),
                            default=-1, required=False,
                            help="Specify the duration of data to be trimmed, starting from the start time, in nanoseconds (ns)."
                                 "If not provided, the data will be trimmed from the start time to the end of the profiler data.")
        parser.add_argument("-o", "--output_path", type=str, required=False,
                            help="Specify the output file path for the parsed results; "
                                 "defaults to the dump_data subdirectory under the profiler path.")
        args = parser.parse_args()
        cls.profiler_path = args.profiler_path
        if not os.path.exists(cls.profiler_path):
            raise RuntimeError(f"The specified profiler path does not exist: {cls.profiler_path}.")
        cls.set_time_range_and_crop_mode(args.start, args.duration)
        cls.output_path = args.output_path
        if not cls.output_path:
            import time
            cls.output_path = os.path.join(cls.profiler_path, "dump_data", f"leaks_dump_{time.time_ns()}.db")
        ProfilerConfig().load_info(cls.profiler_path)

    @classmethod
    def set_time_range_and_crop_mode(cls, start: int, duration: int):
        """
            从输入指定的start与duration确定剪裁模式
        :param start: 指定的剪裁开始时间，缺省为-1
        :param duration: 指定的剪裁时长，缺省为-1
        :return:
        """
        if start == -1 and duration == -1:
            cls.crop_mode = CropMode.NO_CROPPING
            return
        # 从指定时间开始剪裁固定时间
        if start > 0 and duration > 0:
            cls.start = start
            cls.duration = duration
            cls.end = start + duration
            cls.crop_mode = CropMode.CROP_FIXED_DURATION_FROM_SPECIFIED_START_TIME
            return
        # 从最小时间开始剪裁固定时间
        if start == -1:
            cls.start = -1
            cls.duration = duration
            cls.end = -1
            cls.crop_mode = CropMode.CROP_FIXED_DURATION_FROM_MIN_EVENT_TIME
            return
        # 从指定时间开始剪裁到结尾
        cls.start = start
        cls.duration = -1
        cls.end = -1
        cls.crop_mode = CropMode.CROP_FROM_SPECIFIED_START_TIME_TO_THE_END

    @classmethod
    def reset_time_range_by_events_time(cls, min_time: int, max_time: int):
        """
            用户所指定的剪裁范围可能超过实际的数据范围，在解析完event事件后进行重置时间范围(取交集)
        :param min_time: profiler事件序列的最小时间
        :param max_time: profiler事件序列的最大时间
        """
        if cls.crop_mode is CropMode.NO_CROPPING:
            return
        if cls.crop_mode is CropMode.CROP_FIXED_DURATION_FROM_SPECIFIED_START_TIME:
            cls.start = max(min_time, cls.start)
            cls.end = min(max_time, cls.end)
            return
        if cls.crop_mode is CropMode.CROP_FIXED_DURATION_FROM_MIN_EVENT_TIME:
            cls.start = min_time
            cls.end = min(max_time, cls.start + cls.duration)
            return
        cls.start = max(min_time, cls.start)
        cls.end = max_time


if __name__ == '__main__':
    DumpConfig.init()
    dump_profiler_data_to_memscope()
