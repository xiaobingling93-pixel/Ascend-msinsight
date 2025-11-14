#
# Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
#
import os
import argparse
import logging
import subprocess
import time
import signal

logging.basicConfig(level=logging.DEBUG, format='[%(asctime)s] [%(levelname)s]:%(message)s')


def ftrace_record_start(cpu_mask=None):
    try:
        if os.getuid() != 0:
            logging.critical('Please run  this script as root.')
            return False
        subprocess.run(['/usr/bin/trace-cmd', 'clear'], check=True)
        start_command = ['/usr/bin/trace-cmd', 'start', '-b', '2820', '-e', 'sched:*', '-C', 'mono_raw']
        if cpu_mask is not None:
            start_command.append('-M')
            start_command.append(cpus_to_cpumask(cpu_mask))
        subprocess.run(start_command, check=True)
    except subprocess.CalledProcessError as e:
        logging.error("Cmd run failed, error={}".format(e))
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
    try:
        subprocess.run(['/usr/bin/trace-cmd', 'stop'], check=True)
        with open(output, 'w') as file:
            subprocess.run(['/usr/bin/trace-cmd', 'show'], stdout=file, check=True)
            subprocess.run(['/usr/bin/trace-cmd', 'clear'], check=True)
            subprocess.run(['/usr/bin/trace-cmd', 'reset'], check=True)
    except subprocess.CalledProcessError as e:
        logging.error("command run failed, error={}".format(e))
        return
    logging.info("Write finish")


def on_exit():
    subprocess.run(['/usr/bin/trace-cmd', 'stop'])
    return


if __name__ == "__main__":
    confirm = input(
        "This script requires root privileges, irreversible actions may occur. Continue? (y/N): ").strip().lower()
    if confirm not in ('y', 'yes'):
        print("Aborted.")
        exit(1)
    parser = argparse.ArgumentParser()
    parser.add_argument('--cpu', type=str, default=None)
    parser.add_argument('--output', type=str, default='ftrace.txt')
    parser.add_argument('--record_time', type=int, required=True,
                        help='record time, if pass <=0 will start long term record that user should attention the disk space')
    args = parser.parse_args()
    if args.cpu:
        args.cpu = [int(i) for i in args.cpu.split(',')]
    signal.signal(signal.SIGTERM, on_exit)
    try:
        ftrace_record_start(args.cpu)
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
