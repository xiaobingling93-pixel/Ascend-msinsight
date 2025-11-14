#
# Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
#
import json
import re
import os
import logging
import tqdm
import argparse

logging.basicConfig(level=logging.INFO, format='[%(asctime)s] [%(levelname)s]:%(message)s')
CPU_SCHED_PID = "CPU Scheduling"
PROCESS_SCHED_PID = 'Process Scheduling'
KERNEL_PROCESS = ['migration', 'swapper', 'kworker']


def tran(ts):
    return "{:.3f}".format(ts / 1000)


def is_kernel_process(comm):
    for process in KERNEL_PROCESS:
        if comm.find(process) != -1:
            return True
    return False


class CompleteEvent(object):
    def __init__(self, comm, st, args, cpu):
        self.comm = comm
        self.st = st
        self.args = args
        self.cpu = cpu
        self.running = True
        self.last_time = None

    def end(self, ts):
        self.dur = ts - self.st
        self.es = ts
        self.running = False

    def is_running(self):
        return self.running

    def to_event_json(self):
        return {"name": self.comm, 'ph': 'X', 'pid': CPU_SCHED_PID, 'tid': "CPU " + self.cpu, 'ts': self.st,
                'dur': self.dur, 'end': self.es}


class Process(object):
    def __init__(self, comm, ts):
        self.comm = comm
        self.ts = ts
        self.events = [{'name': self.comm, 'ph': 'M', 'pid': PROCESS_SCHED_PID, 'tid': self.comm}]
        self.state = 'W'  # R-running S-sleep W-runable, X-exit

    def sleep(self, ts):
        if self.state == 'R':
            event = {"name": "Running", 'ph': 'X', 'pid': PROCESS_SCHED_PID, 'tid': self.comm, 'ts': tran(self.ts),
                     'dur': tran(ts - self.ts)}
            self.events.append(event)
        self.ts = ts
        self.state = 'S'

    def wakeup(self, ts):
        self.state = 'W'
        self.ts = ts

    def run(self, ts):
        if self.state == 'W':
            event = {'name': 'Runnable', 'ph': 'X', 'pid': PROCESS_SCHED_PID, 'tid': self.comm, 'ts': tran(self.ts),
                     'dur': tran(ts - self.ts)}
            self.events.append(event)
        self.ts = ts
        self.state = 'R'

    def get_process_events(self, ts):
        self.exit(ts)
        return self.events

    def exit(self, ts):
        if self.state == 'R':
            event = {'name': 'Running', 'ph': 'X', 'pid': PROCESS_SCHED_PID, 'tid': self.comm, 'ts': tran(self.ts),
                     'dur': tran(ts - self.ts)}
            self.events.append(event)
        if self.state == 'W':
            event = {'name': 'Runnable', 'ph': 'X', 'pid': PROCESS_SCHED_PID, 'tid': self.comm, 'ts': tran(self.ts),
                     'dur': tran(ts - self.ts)}
            self.events.append(event)
        self.state = 'X'


class TraceConverter:
    def __init__(self, trace_file_path, cpu_filter, profiling_data):
        self.trace_file_path = trace_file_path
        self.sched_pattern = re.compile(
            r'(?P<task>.*)-(?P<pid>\d+)\s+\[(?P<cpu>\d+)\]\s.{4,5}\s(?P<timestamp>[\d.]+):\s+(?P<action>.*):\s+(?P<args>.*)')
        self.parse_result = []
        self.utc_start_timestamp = None
        self.timestamp_offset = None
        self.profiling_data = profiling_data
        self.set_timestamp_offset(profiling_data)
        self.trace_event = {}
        self.cpu_stats = {}
        self.cpu_set = set()
        self.process_state = {}
        self.process_set = set()
        if cpu_filter is None:
            self.cpu_filter = None
        else:
            self.cpu_filter = [int(i) for i in cpu_filter.split(',')]

    def parse(self):
        self.parse_trace_file()
        self.trans_to_trace_event()
        result = []
        for k, events in self.trace_event.items():
            result.extend(events)
        result.append({'name': 'Process Scheduling', 'ph': 'M', 'pid': PROCESS_SCHED_PID, 'tid': PROCESS_SCHED_PID})
        for k, value in self.process_state.items():
            if is_kernel_process(k):
                continue
            result.extend(value.get_process_events(self.last_time))
        return result

    def add_trace_event(self, event_type, event):
        if event_type not in self.trace_event.keys():
            self.trace_event[event_type] = []
        # swapper进程为内核的特殊进程，忽略
        if is_kernel_process(event['name']):
            return
        if 'ts' in event.keys():
            event['ts'] = tran(event['ts'])
        if 'dur' in event.keys():
            event['dur'] = tran(event['dur'])
        self.trace_event[event_type].append(event)

    def parse_trace_file(self):
        # check whether file exist
        if not os.path.exists(self.trace_file_path):
            logging.critical(f"File not exists: {self.trace_file_path}")
            return False
        with open(self.trace_file_path, 'r') as file:
            lines = file.readlines()
            for line in tqdm.tqdm(lines, desc="Parsing trace file", unit="line"):
                self.parse_trace_file_line(line)
        return True

    def parse_trace_file_line(self, line: str):
        if len(line) == 0:
            return
        match = self.sched_pattern.search(line.strip())
        if match is None:
            logging.debug("Not match regex:{}", line)
            return
        task = match.group('task')
        pid = match.group('pid')
        cpu = match.group('cpu')
        timestamp = match.group('timestamp')
        action = match.group('action')
        args = match.group('args')
        result = {"task": task, "pid": pid, "cpu": cpu, "timestamp": timestamp, "action": action, "args": args}
        self.parse_result.append(result)

    def trans_to_trace_event(self):
        for event in self.parse_result:
            if event['action'].find('sched') != -1:
                self.parse_sched_event(event)

    def parse_sched_event(self, sched_event):
        cpu = sched_event['cpu']
        if cpu not in self.cpu_set:
            self.cpu_set.add(cpu)
            cpu_thread = {"name": "CPU " + cpu, "ph": "M", "pid": CPU_SCHED_PID, "tid": "CPU " + cpu}
            self.add_trace_event('sched', cpu_thread)
        self.last_time = self.get_utc_timestamp(sched_event['timestamp'])
        if sched_event['action'] == 'sched_switch':
            self.parse_sched_switch(sched_event)
        if sched_event['action'] == 'sched_wakeup' or sched_event['action'] == 'sched_wakeup_new':
            self.parse_sched_wakeup(sched_event)
        if sched_event['action'] == 'sched_process_free':
            self.parse_sched_free(sched_event)

    def parse_sched_switch(self, entry: dict):
        # draw kernel part
        cpu = entry['cpu']
        timestamp = self.get_utc_timestamp(entry['timestamp'])
        kwargs = self.parse_sched_param(entry['args'])
        prev_comm = kwargs['prev_comm'] + ':' + kwargs['prev_pid']
        if cpu not in self.cpu_stats.keys():
            self.cpu_stats[cpu] = {}
        if prev_comm in self.cpu_stats[cpu].keys():
            self.cpu_stats[cpu][prev_comm].end(timestamp)
            self.add_trace_event('sched', self.cpu_stats[cpu][prev_comm].to_event_json())
            del self.cpu_stats[cpu][prev_comm]
        next_name = kwargs['next_comm'] + ':' + kwargs['next_pid']
        self.cpu_stats[cpu][next_name] = CompleteEvent(next_name, timestamp, args=entry['args'], cpu=cpu)
        if prev_comm not in self.process_state.keys():
            self.process_state[prev_comm] = Process(prev_comm, self.utc_start_timestamp)
        self.process_state[prev_comm].sleep(timestamp)
        if next_name not in self.process_state.keys():
            self.process_state[next_name] = Process(next_name, timestamp)
            self.process_state[next_name].run(timestamp)
        else:
            self.process_state[next_name].run(timestamp)

    def parse_sched_wakeup(self, entry):
        timestamp = self.get_utc_timestamp(entry['timestamp'])
        args = entry['args']
        args_dict = self.parse_sched_param(args)
        if self.cpu_filter is not None and int(args_dict['target_cpu']) not in self.cpu_filter:
            return
        comm = args_dict['comm'] + ':' + args_dict['pid']
        if comm in self.process_state.keys():
            self.process_state[comm].wakeup(timestamp)
        else:
            self.process_state[comm] = Process(comm, timestamp)

    def parse_sched_free(self, entry):
        cpu = entry['cpu']
        args_dict = self.parse_sched_param(entry['args'])
        timestamp = self.get_utc_timestamp(entry['timestamp'])
        comm = args_dict['comm'] + ':' + args_dict['pid']
        if comm in self.process_state.keys():
            self.process_state[comm].exit(timestamp)
            del self.process_state[comm]
        if comm in self.process_set:
            self.process_set.remove(comm)

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

    def get_utc_timestamp(self, uptime: str):
        # ns
        timestamp = self.str_to_int(uptime)
        return (timestamp - self.mono_raw_start) + self.utc_start_timestamp

    def set_timestamp_offset(self, profiling_data):
        start_info = self.get_profiling_start_info()
        if start_info is None:
            logging.critical("Can't find start info")
            return
        self.mono_raw_start = int(start_info['clockMonotonicRaw'])
        self.utc_start_timestamp = int(start_info['collectionTimeBegin']) * 1000

    def get_profiling_start_info(self):
        if not os.path.exists(self.profiling_data):
            logging.error("Profiling data path not exist")
            # 递归查找start_info
            return None
        start_info_path = None
        for dirpath, dirnames, filenames in os.walk(self.profiling_data):
            if "start_info" in filenames:
                full_path = os.path.join(dirpath, "start_info")
                start_info_path = full_path
                break
        if start_info_path is None:
            logging.error("Not find start_info in profling data")
            return None
        with open(start_info_path, 'r') as f:
            start_info = json.load(f)
        return start_info

    def str_to_int(self, num_str):
        pos = num_str.find('.')
        return (int(num_str[:pos]) * 1000000 + int(num_str[pos + 1:])) * 1000


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', type=str, default='ftrace.txt')
    parser.add_argument('--output', type=str, default='output.json')
    parser.add_argument('--cpu_list', type=str, default=None)
    parser.add_argument('--profiling_data', type=str, help='use profiling data to adjust start time')
    args = parser.parse_args()
    con = TraceConverter(args.input, args.cpu_list, args.profiling_data)
    trace_event = con.parse()
    with open(args.output, 'w') as f:
        json.dump(trace_event, f)
