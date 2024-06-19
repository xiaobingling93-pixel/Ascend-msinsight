#!/usr/bin/env python
# -*- coding: UTF-8 -*-

# Copyright 2024 Huawei Technologies Co., Ltd
# ============================================================================

import os
import platform
import shutil
import sys
import subprocess
import logging

ASCEND_INSIGHT_NAME = "MindStudio Insight"
PY_MEI_DIR = getattr(sys, '_MEIPASS', None)
AI_TEMP_DIR = os.path.join(os.path.dirname(PY_MEI_DIR), "MindStudioInsightTemp")
FIX_FRONTEND_PORT = 8085

logging.basicConfig(level=logging.INFO, format='%(message)s')


def insight_start(backend_args):
    if os.path.exists(AI_TEMP_DIR):
        shutil.rmtree(AI_TEMP_DIR)
    shutil.copytree(PY_MEI_DIR, AI_TEMP_DIR)

    frontend_dir = os.path.join(AI_TEMP_DIR, "server", "frontend")

    exec_name = "profiler_server.exe" if platform.platform().find("Windows") > -1 else "profiler_server"
    backend_path = os.path.join(os.path.join(AI_TEMP_DIR, "server", "backend"), exec_name)

    # 非阻塞启动前后的进程
    subprocess.Popen([shutil.which('python'), "-m", "http.server", "-d", frontend_dir, str(FIX_FRONTEND_PORT)],
                     stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    subprocess.Popen([backend_path] + backend_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)


def insight_stop():
    frontend_dir = os.path.join(AI_TEMP_DIR, "server", "frontend")

    exec_name = "profiler_server.exe" if platform.platform().find("Windows") > -1 else "profiler_server"
    backend_path = os.path.join(os.path.join(AI_TEMP_DIR, "server", "backend"), exec_name)

    execute_command(["pkill", "-f", frontend_dir])
    execute_command(["pkill", "-f", backend_path])

    shutil.rmtree(AI_TEMP_DIR, ignore_errors=True)
    logging.info(f"%s has stopped.", ASCEND_INSIGHT_NAME)


def insight_help():
    logging.info("")

    logging.info(f"Usage: %s [command].", ASCEND_INSIGHT_NAME)
    logging.info(f" %s start [option=arg | option arg]", ASCEND_INSIGHT_NAME)
    logging.info("   --wsPort <port>         Specify the port number on which the service will start.")
    logging.info("   --wsHost <ip>           Specify the ip address on which the service will start.")
    logging.info("   --logPath <path>        Specify the path where log files will be stored.")
    logging.info("   --logSize <size_in_B>   Specify the maximum size of log files in bytes (B).")
    logging.info("   --logLevel <level>      Specify the level of logging (e.g., INFO, WARN, ERROR, FATAL, DEBUG).")
    logging.info("   --sid <sid>             Specify the sid of the application.")
    logging.info(
        "   --scan <port>           Scan whether the specified port is available, if not, return an available port.")
    logging.info("")
    logging.info(f" %s stop      Stop all related processes.", ASCEND_INSIGHT_NAME)
    logging.info(f" %s -h        Show this help message and exit.", ASCEND_INSIGHT_NAME)
    logging.info(f" %s --help    Show this help message and exit.", ASCEND_INSIGHT_NAME)
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

argvs = sys.argv[1:]

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
