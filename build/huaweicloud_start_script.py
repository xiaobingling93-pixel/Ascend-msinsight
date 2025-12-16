#!/usr/bin/env python
# -*- coding: UTF-8 -*-

# -------------------------------------------------------------------------
# This file is part of the MindStudio project.
# Copyright (c) 2025 Huawei Technologies Co.,Ltd.
#
# MindStudio is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#
#          http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.
# -------------------------------------------------------------------------

import os
import platform
import shutil
import site
import sys
import subprocess
import logging
from enum import Enum
import argparse
import socket

MINDSTUDIO_INSIGHT_NAME = "MindStudio Insight"
ROOT_PATH = os.path.dirname(os.path.realpath(__file__))
FIX_FRONTEND_PORT = 8085

logging.basicConfig(level=logging.INFO, format='%(message)s', stream=sys.stdout)


class ErrCode(Enum):
    OK = 0
    PortNotAvailable = 1
    PortReuse = 2
    BackEndPathNotExist = 3
    UnKnown = 4


def error_code_to_int(enum_value):
    if not isinstance(enum_value, ErrCode):
        return ErrCode.UnKnown.value
    return enum_value.value


def read_backend_port(backend_args):
    parser = argparse.ArgumentParser();
    parser.add_argument("--wsPort", dest='port', type=str)
    args, _ = parser.parse_known_args(args=backend_args)
    port = args.port
    if len(port) == 0:
        return None
    try:
        port = int(port)
    except ValueError:
        return None
    return port


def check_port_in_listen(port):
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        result = sock.connect_ex(("localhost", int(port)))
        sock.close()
        return True if result == 0 else False
    except OSError:
        return False


def check_port(port):
    if not check_port_in_listen(port):
        return ErrCode.OK
    result = execute_command(["/bin/ps", "aux"])
    if "profiler_server" not in result or str(port) not in result:
        return ErrCode.PortNotAvailable
    else:
        return ErrCode.PortReuse


def check_event_dir(backend_args):
    if "--eventDir" not in " ".join(backend_args):
        return True
    parser = argparse.ArgumentParser()
    parser.add_argument("--eventDir", dest='event_dir', type=str)
    args, _ = parser.parse_known_args(args=backend_args)
    event_dir = args.event_dir
    if len(event_dir) == 0:
        return False
    if not os.path.exists(event_dir) or not os.path.isdir(event_dir):
        return False
    if os.path.islink(event_dir):
        return False
    return True


def insight_start(backend_args):
    frontend_dir = os.path.join(ROOT_PATH, "frontend")

    exec_name = "profiler_server.exe" if platform.platform().find("Windows") > -1 else "profiler_server"
    backend_dir = os.path.join(ROOT_PATH, "server")
    backend_path = os.path.join(backend_dir, exec_name)
    # 检查 backend_path 是否存在
    if not os.path.exists(backend_path):
        logging.info("The backend path %s is not exist.", backend_path)
        return ErrCode.BackEndPathNotExist

    user_sites = site.getusersitepackages()
    sites = ''.join(site.getsitepackages())
    if platform.platform().find('Windows') == -1:
        # 非阻塞启动前后的进程
        port = read_backend_port(backend_args)
        if port is None:
            return ErrCode.UnKnown
        result = check_port(port)
        if result != ErrCode.OK:
            return result
        if not check_event_dir(backend_args):
            return ErrCode.UnKnown
        subprocess.Popen([backend_path] + backend_args, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                         env={'LD_LIBRARY_PATH': backend_dir + ":" + os.getenv('LD_LIBRARY_PATH', ''),
                              'PYTHONPATH': sites + ":" + user_sites + ":" + os.getenv('PYTHONPATH', '')})

    else:
        subprocess.Popen([backend_path] + backend_args, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    subprocess.Popen([shutil.which('python'), "-m", "http.server", "-d", frontend_dir,
                      str(FIX_FRONTEND_PORT)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    result = execute_command(["/bin/ps", "aux"])
    if "profiler_server" not in result:
        return ErrCode.UnKnown
    return ErrCode.OK


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
        except ValueError:  # 过滤非数字行
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
    code = insight_start(REMAIN_ARG)
    code = error_code_to_int(code)
    sys.exit(code)
elif FIRST_ARG == "stop":
    insight_stop()
elif FIRST_ARG == "-h" or FIRST_ARG == "--help":
    insight_help()
else:
    logging.info(f"Param %s has not been supported.", FIRST_ARG)
    logging.info("Please try executing '-h or --help' to get more information.")
