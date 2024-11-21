#!/usr/bin/env python
# -*- coding: UTF-8 -*-

# Copyright 2024 Huawei Technologies Co., Ltd
# ============================================================================

import os
import platform
import shutil
import site
import sys
import subprocess
import logging

MINDSTUDIO_INSIGHT_NAME = "MindStudio Insight"
ROOT_PATH = os.path.dirname(os.path.realpath(__file__))
FIX_FRONTEND_PORT = 8085

logging.basicConfig(level=logging.INFO, format='%(message)s')


def insight_start(backend_args):
    frontend_dir = os.path.join(ROOT_PATH, "frontend")

    exec_name = "profiler_server.exe" if platform.platform().find("Windows") > -1 else "profiler_server"
    backend_dir = os.path.join(ROOT_PATH, "server")
    backend_path = os.path.join(backend_dir, exec_name)
    # 检查 backend_path 是否存在
    if not os.path.exists(backend_path):
        logging.info("The backend path %s is not exist.", backend_path)
        return

    user_sites = site.getusersitepackages()
    sites = ''.join(site.getsitepackages())
    if platform.platform().find('Windows') == -1:
        # 非阻塞启动前后的进程
        subprocess.Popen([backend_path] + backend_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                         env={'LD_LIBRARY_PATH': backend_dir + ":" + os.getenv('LD_LIBRARY_PATH', ''),
                              'PYTHONPATH': sites + ":" + user_sites + ":" + os.getenv('PYTHONPATH', '')})
    else:
        subprocess.Popen([backend_path] + backend_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    subprocess.Popen([shutil.which('python'), "-m", "http.server", "-d", frontend_dir,
                      str(FIX_FRONTEND_PORT)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def kill_process(pid):
    try:
        execute_command(["kill", "-9", str(pid)])
    except subprocess.CalledProcessError:
        logging.info("Kill process error. Please check the port occupancy status and manually release the port.")
        pass


def get_process_list():
    try:
        result = execute_command(["ps", "-eo", "pid,ppid,comm"])
        return result.splitlines()
    except subprocess.CalledProcessError:
        logging.info("Get process list error.")
        return []


def insight_stop():
    exec_name = "profiler_server"
    process_list = get_process_list()
    temp_process_pid = -1

    for line in process_list:
        parts = line.split()
        if len(parts) < 3:
            continue
        try:
            pid = int(parts[0])
            ppid = int(parts[1])
            name = parts[2]

            if name == exec_name:
                temp_process_pid = pid
                kill_process(pid)
                continue

            if name == 'python' and pid - temp_process_pid < 5:
                kill_process(pid)
                temp_process_pid = -1
        except ValueError: # 过滤非数字行
            pass


def insight_help():
    logging.info("")

    logging.info(f"Usage: %s [command].", MINDSTUDIO_INSIGHT_NAME)
    logging.info(f" %s start [option=arg | option arg]", MINDSTUDIO_INSIGHT_NAME)
    logging.info("   --wsPort <port>         Specify the port number on which the service will start.")
    logging.info("   --wsHost <ip>           Specify the ip address on which the service will start.")
    logging.info("   --logPath <path>        Specify the path where log files will be stored.")
    logging.info("   --logSize <size_in_B>   Specify the maximum size of log files in bytes (B).")
    logging.info("   --logLevel <level>      Specify the level of logging (e.g., INFO, WARN, ERROR, FATAL, DEBUG).")
    logging.info("   --sid <sid>             Specify the sid of the application.")
    logging.info(
        "   --scan <port>           Scan whether the specified port is available, if not, return an available port.")
    logging.info("")
    logging.info(f" %s stop      Stop all related processes.", MINDSTUDIO_INSIGHT_NAME)
    logging.info(f" %s -h        Show this help message and exit.", MINDSTUDIO_INSIGHT_NAME)
    logging.info(f" %s --help    Show this help message and exit.", MINDSTUDIO_INSIGHT_NAME)
    logging.info("")


def execute_command(command):
    result = subprocess.run(command, shell=False, capture_output=True, text=True)
    if result.returncode != 0:
        logging.info(result.stderr)
    return result.stdout


if len(sys.argv) <= 1:
    logging.info("The program startup parameters are not enough.")
    logging.info("Please try executing '-h or --help' to get more information.")
    raise Exception

argvs = sys.argv[2:]

FIRST_ARG = argvs[0]
REMAIN_ARG = argvs[1:]

if FIRST_ARG == "start":
    insight_start(REMAIN_ARG)
elif FIRST_ARG == "stop":
    insight_stop()
elif FIRST_ARG == "-h" or FIRST_ARG == "--help":
    insight_help()
else:
    logging.info(f"Param %s has not been supported.", FIRST_ARG)
    logging.info("Please try executing '-h or --help' to get more information.")
