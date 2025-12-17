"""
-------------------------------------------------------------------------
This file is part of the MindStudio project.
Copyright (c) 2025 Huawei Technologies Co.,Ltd.

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
import json
import re
import os
import logging
from abc import abstractmethod

import tqdm
import argparse
import glob

logging.basicConfig(level=logging.INFO, format='[%(asctime)s] [%(levelname)s]:%(message)s')
CPU_SCHED_PID = "CPU Scheduling"
PROCESS_SCHED_PID = 'Process Scheduling'
KERNEL_PROCESS = ['migration', 'swapper', 'kworker']


def tran(ts):
    if not isinstance(ts, (int, float)):
        logging.warning("Invalid timestamp")
        return "0"
    return "{:.3f}".format(ts / 1000)


def is_kernel_process(comm):
    if not isinstance(comm, str):
        logging.error("Param must be string")
        return False
    for process in KERNEL_PROCESS:
        if comm.find(process) != -1:
            return True
    return False


def get_trace_event(name, pid, tid, ts, dur, args=None):
    event = {"name": name, "ph": "X", "pid": pid, "tid": tid, "ts": ts, "dur": dur}
    if args is not None:
        event['args'] = args
    return event


def get_meta_event(pid, tid):
    return {"name": "process_name", 'ph': 'M', 'pid': pid, 'tid': tid}


class CompleteEvent(object):
    def __init__(self, comm, pid, st, cpu, prio):
        self.comm = comm
        self.pid = pid
        self.st = st
        self.cpu = "CPU " + cpu
        self.total_runtime = 0
        self.vruntime = 0
        self.end_state = ""
        self.prio = prio

    def update_runtime(self, runtime, vruntime):
        if runtime is not None:
            self.total_runtime += int(runtime.split()[0])
        if vruntime is not None:
            self.vruntime = vruntime

    def end(self, ts, end_state, prio):
        self.dur = ts - self.st
        self.end_state = end_state
        self.prio = prio

    def to_event_json(self):
        return get_trace_event(self.comm + ":" + self.pid, CPU_SCHED_PID, self.cpu, self.st, self.dur,
                               {'host_pid': self.pid, 'total_runtime': str(self.total_runtime) + " [ns]",
                                'vruntime': self.vruntime, 'end_state': self.end_state, 'prio': self.prio})


class Process(object):
    def __init__(self, comm, pid, ts):
        self.pid = pid
        self.ppid = PROCESS_SCHED_PID
        self.ts = ts
        self.comm = comm
        self.process_name = self.comm + ":" + self.pid
        self.events = []
        self.add_process_meta()
        self.state = 'W'  # R-running S-sleep W-runable, X-exit

    def add_process_meta(self):
        # 存在父进程, 挂在父进程下
        self.events.append(get_meta_event(self.ppid, self.pid))

    def get_event_name(self):
        if self.state == 'W':
            return 'Runnable'
        elif self.state == 'R':
            return 'Running'
        return 'Unknown'

    def add_event(self, ts):
        event_name = self.get_event_name()
        event = get_trace_event(event_name, self.ppid, self.process_name, tran(self.ts), tran(ts - self.ts))
        self.events.append(event)

    def sleep(self, ts):
        if self.state == 'R':
            self.add_event(ts)
        self.ts = ts
        self.state = 'S'

    def wakeup(self, ts):
        self.state = 'W'
        self.ts = ts

    def run(self, ts):
        if self.state == 'W':
            self.add_event(ts)
        self.ts = ts
        self.state = 'R'

    def get_process_events(self, ts):
        self.exit(ts)
        return self.events

    def exit(self, ts):
        self.add_event(ts)
        self.state = 'X'


def singleton(cls):
    instances = {}

    def get_instance(*args, **kwargs):
        if cls not in instances:
            instances[cls] = cls(*args, **kwargs)
        return instances[cls]

    return get_instance


@singleton
class PidTran:
    def __init__(self):
        self.pid_mapping_path = None
        self.pid_status = None

    def init(self, pid_mapping=None):
        self.pid_mapping_path = pid_mapping
        self.load_pid_mapping(self.pid_mapping_path)

    def load_pid_mapping(self, pid_status_file):
        if pid_status_file is None:
            return None
        try:
            with open(pid_status_file, 'r') as file:
                self.pid_status = json.load(file)
                return None
        except IOError as e:
            logging.error("Open pid status file failed, exception={}".format(e))
            return None

    def get_ns_pid(self, pid):
        if self.pid_status is None:
            return pid
        if pid not in self.pid_status:
            return pid
        return self.pid_status[pid]['Nspid']


@singleton
class TimeStampTran:
    def __init__(self):
        self.mono_raw_start = None
        self.utc_start_timestamp = None
        pass

    def init(self, profiling_data):
        start_info = self.__get_profiling_start_info(profiling_data)
        if start_info is None:
            logging.critical("Can't find start info")
            return
        self.mono_raw_start = int(start_info['clockMonotonicRaw'])
        self.utc_start_timestamp = int(start_info['collectionTimeBegin']) * 1000

    def __get_profiling_start_info(self, profiling_data):
        if profiling_data is None:
            return {'clockMonotonicRaw': 0, 'collectionTimeBegin': 0}
        if not os.path.exists(profiling_data):
            logging.error("Profiling data path not exist")
            # 递归查找start_info
            return None
        start_info_path = glob.glob(os.path.join(profiling_data, "**", "start_info"), recursive=True)
        if len(start_info_path) == 0:
            logging.error("Not find start_info in profling data")
            return None
        with open(start_info_path[0], 'r') as f:
            start_info = json.load(f)
        return start_info

    def get_utc_timestamp(self, uptime: str):
        # ns
        timestamp = self.__str_to_int(uptime)
        return (timestamp - self.mono_raw_start) + self.utc_start_timestamp

    def __str_to_int(self, num_str):
        if not all((c.isdigit() or c == '.') for c in num_str) or num_str.count('.') > 1:
            logging.error("Invalid format")
            return 0
        pos = num_str.find('.')
        if pos == -1:
            return int(num_str) * 1000000
        return (int(num_str[:pos]) * 1000000 + int(num_str[pos + 1:])) * 1000


class FtraceParse:
    @abstractmethod
    def parse_one_event(self, event: str):
        return

    @abstractmethod
    def belong(self, event: str) -> bool:
        return True

    @abstractmethod
    def get_result(self):
        return


class SchedFtraceParse(FtraceParse):
    def __init__(self):
        self.cpu_stats = dict()
        self.cpu_set = set()
        self.process_state = {}
        self.process_set = set()
        self.sched_pattern = re.compile(
            r'(?P<task>.*)-(?P<pid>\d+)\s+\[(?P<cpu>\d+)\]\s.{4,5}\s(?P<timestamp>[\d.]+):\s+(?P<action>.*):\s+(?P<args>.*)')

        self.trace_event = []

    def belong(self, event: str) -> bool:
        match = self.sched_pattern.search(event.strip())
        return match is not None

    def parse_one_event(self, event: str):
        if len(event) == 0:
            return
        match = self.sched_pattern.search(event.strip())
        if match is None:
            logging.debug("Not match regex:{}", event)
            return
        task = match.group('task')
        pid = match.group('pid')
        pid = PidTran().get_ns_pid(pid)
        cpu = match.group('cpu')
        timestamp = TimeStampTran().get_utc_timestamp(match.group('timestamp'))
        action = match.group('action')
        args = match.group('args')
        result = {"task": task, "pid": pid, "cpu": cpu, "timestamp": timestamp, "action": action, "args": args}
        self.trans_to_trace_event(result)

    def trans_to_trace_event(self, event):
        if 'action' not in event.keys():
            return
        if 'sched' in event['action']:
            self.parse_sched_event(event)

    def parse_sched_event(self, sched_event):
        cpu = sched_event['cpu']
        if cpu not in self.cpu_set:
            self.cpu_set.add(cpu)
            self.cpu_stats[cpu] = {}
            cpu_thread = {"name": "CPU " + cpu, "ph": "M", "pid": CPU_SCHED_PID, "tid": "CPU " + cpu}
            self.add_trace_event(cpu_thread)
        self.last_time = sched_event['timestamp']
        if sched_event['action'] == 'sched_switch':
            self.parse_sched_switch(sched_event)
        if sched_event['action'] == 'sched_wakeup' or sched_event['action'] == 'sched_wakeup_new':
            self.parse_sched_wakeup(sched_event)
        if sched_event['action'] == 'sched_process_free':
            self.parse_sched_free(sched_event)
        if sched_event['action'] == 'sched_process_exec':
            self.parse_sched_process_exec(sched_event)
        if sched_event['action'] == 'sched_stat_runtime':
            self.parse_sched_stat_runtime(sched_event)

    def parse_sched_switch(self, entry: dict):
        # draw kernel part
        cpu = entry['cpu']
        timestamp = entry['timestamp']
        kwargs = self.parse_sched_param(entry['args'])
        prev_comm = kwargs['prev_comm'] + ':' + kwargs['prev_pid']
        if prev_comm in self.cpu_stats[cpu].keys():
            self.cpu_stats[cpu][prev_comm].end(timestamp, kwargs['prev_state'], kwargs['prev_prio'])
            self.add_trace_event(self.cpu_stats[cpu][prev_comm].to_event_json())
            del self.cpu_stats[cpu][prev_comm]
        next_name = kwargs['next_comm'] + ':' + kwargs['next_pid']
        self.cpu_stats[cpu][next_name] = CompleteEvent(kwargs['next_comm'], kwargs['next_pid'], timestamp,
                                                       cpu, kwargs['next_prio'])
        if prev_comm not in self.process_state.keys():
            self.process_state[prev_comm] = Process(kwargs['prev_comm'], kwargs['prev_pid'], timestamp)
        self.process_state[prev_comm].sleep(timestamp)
        if next_name not in self.process_state.keys():
            self.process_state[next_name] = Process(kwargs['next_comm'], kwargs['next_pid'], timestamp)
            self.process_state[next_name].run(timestamp)
        else:
            self.process_state[next_name].run(timestamp)

    def parse_sched_wakeup(self, entry):
        timestamp = entry['timestamp']
        args_dict = self.parse_sched_param(entry['args'])
        comm = args_dict['comm'] + ':' + args_dict['pid']
        if comm in self.process_state.keys():
            self.process_state[comm].wakeup(timestamp)
        else:
            self.process_state[comm] = Process(args_dict['comm'], args_dict['pid'], timestamp)

    def parse_sched_free(self, entry):
        args_dict = self.parse_sched_param(entry['args'])
        timestamp = entry['timestamp']
        comm = args_dict['comm'] + ':' + args_dict['pid']
        if comm in self.process_state.keys():
            self.process_state[comm].exit(timestamp)
            del self.process_state[comm]

    def parse_sched_process_exec(self, entry):
        ts = entry['timestamp']
        cpu = entry['cpu']
        kwargs = self.parse_sched_param(entry['args'])
        pid = kwargs['pid']
        filename = kwargs['filename'].split('/')[-1]
        process = filename + ':' + PidTran().get_ns_pid(pid)
        self.cpu_stats[cpu][process] = CompleteEvent(filename, pid, ts, cpu, "unkown")
        self.process_state[process] = Process(filename, pid, ts)
        self.process_state[process].run(ts)

    def parse_sched_stat_runtime(self, entry):
        ts = entry['timestamp']
        cpu = entry['cpu']
        kwargs = self.parse_sched_param(entry['args'])
        comm = kwargs['comm']
        pid = kwargs['pid']
        process = comm + ':' + PidTran().get_ns_pid(pid)
        # 当前进程已存在，更新运行时间
        if process in self.cpu_stats[cpu]:
            self.cpu_stats[cpu][process].update_runtime(kwargs.get('runtime'), kwargs.get('vruntime', None))
        else:
            self.cpu_stats[cpu][process] = CompleteEvent(comm, pid, ts, cpu, "unknown")

    def parse_sched_param(self, string: str):
        kv = string.split(' ')
        result = [kv[0]]
        for i in range(1, len(kv)):
            if '=' in kv[i]:
                result.append(kv[i])
            else:
                result[-1] += " " + kv[i]
        kv_dic = {}
        for item in result:
            if item == '==>':
                continue
            k, v = item.split("=")
            kv_dic[k] = v
        return kv_dic

    def add_trace_event(self, event):
        # swapper进程为内核的特殊进程，忽略
        if is_kernel_process(event['name']):
            return
        if 'ts' in event.keys():
            event['ts'] = tran(event['ts'])
        if 'dur' in event.keys():
            event['dur'] = tran(event['dur'])
        self.trace_event.append(event)

    def get_result(self):
        result = []
        result.extend(self.trace_event)
        result.append({'name': 'Process Scheduling', 'ph': 'M', 'pid': PROCESS_SCHED_PID, 'tid': PROCESS_SCHED_PID})
        for k, value in self.process_state.items():
            if is_kernel_process(k):
                continue
            result.extend(value.get_process_events(self.last_time))
        return result


class TraceConverter:
    def __init__(self, trace_file_path, profiling_data=None, pid_mapping=None):
        self.trace_file_path = trace_file_path
        self.parse_func_map = []
        self.pid_status = PidTran()
        self.pid_status.init(pid_mapping)
        self.time_tran = TimeStampTran()
        self.time_tran.init(profiling_data)
        self.register_parser()

    def register_parser(self):
        self.parse_func_map.append(SchedFtraceParse())

    def parse(self):
        self.parse_trace_file()
        result = []
        for parser in self.parse_func_map:
            result.extend(parser.get_result())
        return result

    def parse_trace_file(self):
        # check whether file exist
        if not os.path.exists(self.trace_file_path):
            logging.critical(f"File not exists: {self.trace_file_path}")
            return False
        file = open(self.trace_file_path, 'r')
        lines = file.readlines()
        for line in tqdm.tqdm(lines, desc="Parsing trace file", unit="line"):
            for parser in self.parse_func_map:
                if parser.belong(line):
                    parser.parse_one_event(line)
        file.close()
        return True


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', type=str, default='ftrace.txt')
    parser.add_argument('--output', type=str, default='output.json')
    parser.add_argument('--profiling_data', type=str, help='use profiling data to adjust start time')
    parser.add_argument('--pid_mapping', type=str, help='container pid map file')
    args = parser.parse_args()
    con = TraceConverter(args.input, args.profiling_data, args.pid_mapping)
    trace_event = con.parse()
    logging.info("Parse End, Start Write to file....")
    with open(args.output, 'w') as f:
        json.dump(trace_event, f, indent=2)
