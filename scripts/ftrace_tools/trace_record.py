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
import os
import argparse
import logging
import subprocess
import time
import signal
import threading
import json
from typing import Optional

logging.basicConfig(level=logging.DEBUG, format='[%(asctime)s] [%(levelname)s]:%(message)s')

DEFAULT_TRACE_BUFFER_SIZE = 2820 # 单位: KB(trace-cmd环形缓冲区大小默认值)
DEFAULT_TRACE_RECORD_TIME = 30 # 单位: 秒（trace-cmd采集时长默认值）

# trace-cmd 事件白名单
# cpu调度
SCHED_EVENT_LIST = {
    "sched_switch", "sched_wakeup", "sched_waking", "sched_wakeup_new", "sched_migrate_task", "sched_stat_runtime",
    "sched_process_fork", "sched_process_exec", "sched_process_exit"
}
# 中断
IRQ_EVENT_LIST = {
    "irq_handler_entry", "irq_handler_exit",
    "softirq_raise", "softirq_entry", "softirq_exit"
}
# 锁竞争
FUTEX_EVENT_LIST = {
    "syscalls:sys_enter_futex", "syscalls:sys_exit_futex",
}

def ftrace_record_start(cpu_mask=None, output="trace.dat", bf_size=DEFAULT_TRACE_BUFFER_SIZE, event_cfg: Optional["TraceEventConfig"]=None,
                        args=None):
    if os.getuid() != 0:
        logging.critical('Please run this script as root')
        return False
    cpu_mask = CPUParser.normalize_cpu_mask(cpu_mask)

    if event_cfg is None:
        event_cfg = TraceEventConfig()
    if args is not None:
        event_cfg.sched = args.sched
        event_cfg.irq = args.irq
        event_cfg.futex = args.futex

    TraceRecord.trace_clear()
    TraceRecord.trace_start(cpu_mask, output, event_cfg=event_cfg, buffer_size=bf_size)
    return True

def ftrace_record_stop(output = "ftrace.txt"):
    logging.info("Ending record, cleaning up...")
    TraceRecord.trace_stop()
    TraceRecord.trace_clear()
    TraceRecord.trace_reset()
    logging.info("Cleanup finished")

def on_exit():
    TraceRecord.trace_stop()
    return

class TraceEventConfig:
    def __init__(self, sched=1, irq=1, futex=0):
        self.sched = sched
        self.irq = irq
        self.futex = futex

class CPUParser:
    """CPU参数解析器，支持字符串和列表格式的CPU掩码"""

    @staticmethod
    def parse_cpu_arg(cpu_str):
        cpus = set()
        if not cpu_str:
            return None

        format_hint = "Please use non-negative integers (e.g., '0-3,5'). Negative values are not supported."

        try:
            parts = cpu_str.split(',')
            for part in parts:
                part = part.strip()
                if not part:
                    continue

                if '-' in part:
                    sub_parts = part.split('-')
                    # 是两个部分，且两个部分都是纯数字
                    if len(sub_parts) != 2 or not (sub_parts[0].isdigit() and sub_parts[1].isdigit()):
                        logging.error(f"Invalid range format or negative value detected: '{part}'. {format_hint}")
                        return None

                    start, end = int(sub_parts[0]), int(sub_parts[1])
                    # 避免前大后小
                    if start > end:
                        logging.error(f"Range start must be not greater than end: '{part}'.")
                        return None
                    cpus.update(range(start, end + 1))
                else:
                    # 检查是否为数字
                    if not part.isdigit():
                        logging.error(f"Invalid CPU ID detected: '{part}'. {format_hint}")
                        return None
                    cpus.add(int(part))
        except Exception:
            logging.error(f"Unexpected error parsing CPU arguments. {format_hint}")
            return None

        return sorted(list(cpus))
    @staticmethod
    def normalize_cpu_mask(cpu_mask):
        format_hint = "Hint: CPU mask should be a string (e.g., '0-3,5') or a list of integers (e.g., [0, 1, 2])."

        # 合法：表示不绑定 CPU
        if cpu_mask is None:
            return None

        # 处理字符串输入
        if isinstance(cpu_mask, str):
            logging.info(f"Received cpu mask string input: '{cpu_mask}', parsing...")
            parsed_mask = CPUParser.parse_cpu_arg(cpu_mask)
            if parsed_mask is None:
                #在内部已有详细错误信息，直接返回None
                return None
            return parsed_mask

       # 处理列表输入
        elif isinstance(cpu_mask, list):
            invalid_elements = [c for c in cpu_mask if not (isinstance(c, int) and c >= 0)]
            if invalid_elements:
                logging.error(
                    f"Invalid CPU ID(s) detected in list: {invalid_elements}. {format_hint}"
                )
                return None
            return sorted(list(set(cpu_mask)))

        # 非法类型
        elif cpu_mask is not None:
            logging.error(f"Unsupported CPU mask type. {format_hint}")
            return None
    @staticmethod
    def cpus_to_cpumask(cpus):
        # 确保输入合法
        for cpu in cpus:
            if not isinstance(cpu, int) or cpu < 0:
                raise ValueError(f"Invalid CPU ID: {cpu}")

        # 构建位掩码（支持任意大小）
        mask = 0
        for cpu in cpus:
            mask |= (1 << cpu)
        if mask == 0:
            return "0"
        # 每 32 位一组，生成掩码字符串
        parts = []
        while mask:
            # 取低 32 位
            part = mask & 0xFFFFFFFF
            parts.append(f"{part:08x}")  # 8位十六进制，左补0
            mask >>= 32
        # 如果为空，说明 mask 为 0
        if not parts:
            return "0"
        # 反转顺序（低位在右，高位在左），用逗号连接
        cpumask_str = ",".join(reversed(parts))
        return cpumask_str

class TraceRecord:
    _INT_RETRY_COUNT = 3
    _INT_INTERVAL_SEC = 0.5
    _WAIT_TIMEOUT_SEC = 10
    # Holds the running trace-cmd record subprocess (if any)
    _record_process = None
    @staticmethod
    def trace_clear():
        TraceRecord.__run(['/usr/bin/trace-cmd', 'clear'])

    @staticmethod
    def trace_reset():
        TraceRecord.__run(['/usr/bin/trace-cmd', 'reset'])

    @staticmethod
    def trace_start(cpu_mask, output, event_cfg, buffer_size=DEFAULT_TRACE_BUFFER_SIZE, ):
        start_command = ['/usr/bin/trace-cmd', 'record', '-b', str(buffer_size), '-C', 'mono_raw']

        if event_cfg.sched:
            for event in SCHED_EVENT_LIST:
                start_command.extend(['-e', event])
        if event_cfg.irq:
            for event in IRQ_EVENT_LIST:
                start_command.extend(['-e', event])
        if event_cfg.futex:
            for event in FUTEX_EVENT_LIST:
                start_command.extend(['-e', event])

        if cpu_mask is not None:
            start_command.append('-M')
            start_command.append(CPUParser.cpus_to_cpumask(cpu_mask))

        start_command.extend(['-o', output])

        # Start trace-cmd in background so we can stop it dynamically later
        try:
            logging.info("Starting trace-cmd record process" + " ".join(start_command))
            TraceRecord._record_process = subprocess.Popen(start_command, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        except Exception as e:
            logging.error(f"Failed to start trace-cmd record: {e}")

    @staticmethod
    def trace_stop():
        # If we launched a trace-cmd record subprocess, try to stop it gracefully
        proc = TraceRecord._record_process
        if proc:
            try:
                logging.info("Stopping trace-cmd record process")
                # Try sending SIGINT a few times to let trace-cmd finish cleanly
                for i in range(TraceRecord._INT_RETRY_COUNT):
                    try:
                        os.kill(proc.pid, signal.SIGINT)
                    except Exception:
                        pass
                    time.sleep(TraceRecord._INT_INTERVAL_SEC)
                    if proc.poll() is not None:
                        break
                proc.wait(timeout=TraceRecord._WAIT_TIMEOUT_SEC)
                logging.info("trace-cmd record process stopped")
            except subprocess.TimeoutExpired:
                try:
                    proc.terminate()
                except Exception:
                    pass
                logging.warning("Force stopped trace-cmd record process")
            finally:
                TraceRecord._record_process = None
        else:
            # Fallback: run trace-cmd stop if we don't have a process handle
            logging.error("No trace-cmd record process found.")

    @staticmethod
    def trace_show(output):
        logging.info("Write result to file")
        with open(output, 'w') as file:
            TraceRecord.__run(['/usr/bin/trace-cmd', 'show'], stdout=file)
        logging.info("Write end")

    @staticmethod
    def __run(commands: list, stdout=None):
        logging.info("Run command" + " ".join(commands))
        subprocess.run(commands, stdout=subprocess.DEVNULL if stdout is None else stdout, check=True)


def normal_mode(args):
    signal.signal(signal.SIGTERM, on_exit)
    try:
        if args.NSpid:
            nspid_recorder = ContainerPidMapper()
            nspid_recorder.start(None)
        ftrace_record_start(args.cpu, args.output, args.bf_size, args)
    except KeyboardInterrupt:
        ftrace_record_stop(args.output)
        return
    if args.record_time <= 0:
        logging.warning('Record time equals -1, start long term record')
        while True:
            try:
                time.sleep(1)
            except KeyboardInterrupt:
                break
    else:
        time.sleep(args.record_time)
    ftrace_record_stop(args.output)


class ContainerPidMapper:
    def __init__(self, output_file: str = "pid_mapping.json"):
        self.output_file = output_file
        self.stop_flag = threading.Event()
        self.work_thread = None
        self.pid_dict = {}

    def read_process_status(self, host_pid: int):
        """从 /proc/<pid>/status 中读取 NSpid，返回容器内 PID（若存在）"""

        try:
            status_path = f"/proc/{host_pid}/status"
            with open(status_path, "r") as f:
                return self.parse_process_status_file(f)
        except FileNotFoundError:
            pass
        return {}

    def parse_process_status_file(self, file):
        status = {
            "name": "",
            "NSpid": "",
            "Tgid": "",
            "PPid": ""
        }
        for line in file:
            if line.startswith("NSpid:"):
                status['NSpid'] = self.status_value_get(line, 2)
            if line.startswith("Name:"):
                status['name'] = self.status_value_get(line, 1)
            if line.startswith("Tgid"):
                status['Tgid'] = self.status_value_get(line, 1)
            if line.startswith('PPid'):
                status['PPid'] = self.status_value_get(line, 1)
        return status

    def status_value_get(self, line: str, index: int):
        parts = line.strip().split()
        if len(parts) <= index:
            return ""
        return parts[index]

    def list_all_host_pids(self):
        """列出 /proc 下所有数字目录（即当前系统所有进程 PID）"""
        pids = []
        try:
            for entry in os.listdir("/proc"):
                if entry.isdigit():
                    pids.append(int(entry))
        except OSError as e:
            logging.error("Find pid status error, error={}".format(e))
            pass
        return pids

    def get_pid_mapping(self):
        """获取容器内所有进程的 {host_pid: container_pid} 映射"""
        all_pids = self.list_all_host_pids()
        for pid in all_pids:
            status = self.read_process_status(pid)
            self.pid_dict[pid] = status

    def _write_json(self, data: dict):
        try:
            with open(self.output_file, "w", encoding="utf-8") as f:
                json.dump(data, f, indent=2)
            logging.info("Write nspid collect result to file")
        except Exception as e:
            print(f"Write json failed: {e}")

    def _worker(self, duration):
        """后台工作线程主循环"""
        if duration is None:
            self.get_pid_mapping()
            self._write_json(self.pid_dict)
            return
        while not self.stop_flag.is_set():
            self.get_pid_mapping()
            time.sleep(duration)

    def start(self, duration=None):
        """启动后台采集线程（非阻塞）"""
        if duration is None:
            self._worker(duration)
            return
        self.work_thread = threading.Thread(target=self._worker, args=(duration,))
        self.work_thread.start()

    def stop(self):
        """停止采集"""
        self.stop_flag.set()
        if self.work_thread and self.work_thread.is_alive():
            self.work_thread.join(timeout=5)
        self._worker(None)
        self._write_json(self.pid_dict)

    def is_running(self) -> bool:
        """检查是否正在运行"""
        return not self.stop_flag.is_set()


class TraceRecordDaemon(TraceRecord):
    def __init__(self, output="ftrace.txt", rotation_time=30, backup_count=4, buffer_size=2820):
        self.rotation_time = rotation_time
        self.backup_count = backup_count
        self.output_prefix = output
        self.buffer_size = buffer_size
        self.record_process = None
        self.show_thread = None
        self.output_files = []
        self.cur_output_file = None
        self.cur_file_path = None
        self.stop_event = threading.Event()
        self.read_end_event = threading.Event()
        self.pid_mapping_recorder = ContainerPidMapper()


    def trace_record(self, cpu_mask, duration=None):
        TraceRecord.trace_clear()
        TraceRecord.trace_reset()
        self.pid_mapping_recorder.start(duration=duration)
        # 强制不使用IO缓冲区
        logging.info("Start record process")
        logging.info("Start capture thread")
        self.show_thread = threading.Thread(target=self.capture_trace_show_out)
        self.show_thread.start()

    def capture_trace_show_out(self):
        last_time = int(time.time())
        self.cur_output_file = self.get_file_handler()
        count = 0
        while True:
            # 每100次检查一次,避免过于频繁
            cur_time = int(time.time())
            if count > 100 and cur_time - last_time > self.rotation_time:
                self.cur_output_file.close()
                self.cur_output_file = self.get_file_handler()
                count = 0
                last_time = cur_time
                logging.info(f"Switch to file {self.cur_file_path}")
            # 非堵塞读取，每次最多读取40Kb
            context = self.show_process.stdout.read1(40960)
            if not context:
                if self.stop_event.is_set():
                    logging.info("Capture thread read file end")
                    self.read_end_event.set()
                    return
                else:
                    time.sleep(0.2)
                    continue
            self.cur_output_file.write(context)
            count += 1
            time.sleep(0.1)

    def trace_record_stop(self):
        logging.info("End trace record")
        TraceRecord.trace_stop()
        TraceRecord.trace_show(self.output_prefix)
        TraceRecord.trace_clear()
        TraceRecord.trace_reset()
        self.pid_mapping_recorder.stop()

    def stop_process(self, process, name):
        try:
            for i in range(0, 3):
                os.kill(process.pid, signal.SIGINT)
                time.sleep(0.5)
            process.wait(timeout=10)
            logging.info(f"Stop process {name} normally")
        except subprocess.TimeoutExpired:
            process.terminate()
            logging.warning(f"Force stop process {name}")

    def get_file_handler(self):
        cur_time = int(time.time())
        self.cur_file_path = self.output_prefix + "_" + str(cur_time)
        if self.backup_count > 0:
            self.output_files.append(self.cur_file_path)
            if len(self.output_files) > self.backup_count:
                try:
                    os.remove(self.output_files[0])
                    self.output_files.pop(0)
                except Exception as e:
                    logging.warning("This is an exception when remove file, exception={}".format(e))
        return open(self.cur_file_path, 'wb')


def daemon_mode(args):
    recorder = TraceRecordDaemon(args.output, args.rotation, args.backup_count)
    if args.NSpid:
        recorder.trace_record(args.cpu, args.duration)
    else:
        recorder.trace_record(args.cpu)
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        recorder.trace_record_stop()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--cpu', type=str, default=None,
                        help="Specify CPU cores to collect. Supports single numbers, commas, and hyphen ranges. e.g., '0,1,4' or '0-3,8'. Default: collect all CPUs.")
    parser.add_argument('--output', type=str, default='trace.dat')
    parser.add_argument('--record_time', type=int, default=DEFAULT_TRACE_RECORD_TIME,
                        help='record time, if pass <=0 will start long term record that user should attention the disk space')
    parser.add_argument('--bf_size', type=int, default=DEFAULT_TRACE_BUFFER_SIZE,
                        help = 'trace-cmd ring buffer size in KB, used to store ftrace events during tracing. '
                               'Increase this value to avoid event loss when trace volume is large.')
    parser.add_argument('--sched', type=int, choices=[0, 1], default=1,
                        help='Enable sched events (default on). Use 0 to disable.')
    parser.add_argument('--irq', type=int, choices=[0, 1], default=1,
                        help='Enable irq/softirq events (default on). Use 0 to disable.')
    parser.add_argument('--futex', type=int, choices=[0, 1], default=0,
                        help='Enable futex syscall events (default off). Use 1 to enable.')
    parser.add_argument('--NSpid', action='store_true', help='will try to record the pid flex map')
    parser.add_argument('--duration', type=int, default=30)
    confirm = input(
        "This script requires root privileges, irreverisble action may occur. Continue? (y/N):").strip().lower()
    if confirm not in ('y', 'yes'):
        logging.critical("Aborted")
        exit(1)

    args = parser.parse_args()
    normal_mode(args)
