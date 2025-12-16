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

logging.basicConfig(level=logging.DEBUG, format='[%(asctime)s] [%(levelname)s]:%(message)s')


def ftrace_record_start(cpu_mask=None):
    if os.getuid() != 0:
        logging.critical('Please run this script as root')
        return False
    TraceRecord.trace_clear()
    TraceRecord.trace_start(cpu_mask)
    return True


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


def ftrace_record_stop(output):
    logging.info("Ending record, writing result to file...")
    TraceRecord.trace_stop()
    TraceRecord.trace_show(output)
    TraceRecord.trace_clear()
    TraceRecord.trace_reset()
    logging.info("Write finish")


def on_exit():
    TraceRecord.trace_stop()
    return


class TraceRecord:
    @staticmethod
    def trace_clear():
        TraceRecord.__run(['/usr/bin/trace-cmd', 'clear'])

    @staticmethod
    def trace_reset():
        TraceRecord.__run(['/usr/bin/trace-cmd', 'reset'])

    @staticmethod
    def trace_start(cpu_mask, buffer_size='2820'):
        start_command = ['/usr/bin/trace-cmd', 'start', '-b', buffer_size, '-e', 'sched:*', '-C', 'mono_raw']
        if cpu_mask is not None:
            start_command.append('-M')
            start_command.append(cpus_to_cpumask(cpu_mask))
        TraceRecord.__run(start_command)

    @staticmethod
    def trace_stop():
        TraceRecord.__run(['/usr/bin/trace-cmd', 'stop'])

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
        ftrace_record_start(args.cpu)
        if args.NSpid:
            nspid_recorder = ContainerPidMapper()
            nspid_recorder.start(None)
    except KeyboardInterrupt:
        ftrace_record_stop(args.output)
    logging.info("Start recording")
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
    parser.add_argument('--cpu', type=str, default=None)
    parser.add_argument('--output', type=str, default='ftrace.txt')
    parser.add_argument('--record_time', type=int, default=60,
                        help='record time, if pass <=0 will start long term record that user should attention the disk space')
    parser.add_argument('--daemon', action='store_true', default=False, help='daemon mode')
    parser.add_argument('--rotation', type=int, default=30, help='rotation time, unit sec')
    parser.add_argument('--backup_count', type=int, default=6)
    parser.add_argument('--NSpid', action='store_true', help='will try to record the pid flex map')
    parser.add_argument('--duration', type=int, default=30)
    confirm = input(
        "This script requires root privileges, irreverisble action may occur. Continue? (y/N):").strip().lower()
    if confirm not in ('y', 'yes'):
        logging.critical("Aborted")
        exit(1)

    args = parser.parse_args()
    if args.cpu:
        args.cpu = [int(i) for i in args.cpu.split(',')]
    if args.daemon:
        daemon_mode(args)
    else:
        normal_mode(args)
