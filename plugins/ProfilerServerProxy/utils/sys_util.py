#!/usr/bin/env python
# -*- coding: UTF-8 -*-

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
import pwd
import asyncio
import socket
import random
import subprocess
from utils.logutil import common_logger

# 用户空间端口范围
USER_SPACE_PORT_RANGE = (1024, 65535)


async def find_available_port_in_range(start_port: int, end_port: int, host: str = "127.0.0.1",
                                       max_try_times: int = 10):
    for i in range(max_try_times):
        port = random.randint(start_port, end_port)
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            try:
                s.bind((host, port))
            except socket.error:
                common_logger.warning(f"[Times: {i + 1}] Failed to bind to {host}:{port}")
                continue
        await asyncio.sleep(1)
        return port
    return -1


def is_port_in_user_space(port: int) -> bool:
    return USER_SPACE_PORT_RANGE[0] <= port <= USER_SPACE_PORT_RANGE[1]


def lsof_i(port: int):
    if not isinstance(port, int):
        raise ValueError("Invalid port number")
    lsof_command = ["lsof", "-i", f":{port}"]
    result = subprocess.run(lsof_command, shell=False, capture_output=True, text=True)
    if not result.returncode == 0:
        return result.stderr
    return result.stdout


async def start_monitor_process(process: subprocess.Popen, exited_callback=None, delay: int = 1):
    common_logger.info(f"New process[{process.pid}] monitor starting...  Check interval: {delay}s")
    while True:
        if process.poll() is not None:
            common_logger.warning(f"Process[{process.pid}] terminated")

            if exited_callback:
                exited_callback(process)
            break
        await asyncio.sleep(delay)


def check_dir_permissions(path: str, permission: int = os.R_OK):
    try:
        # 检查目录是否具有读和写权限
        if not os.access(path, permission):
            common_logger.error(f"Dir {path} can't be accessed in permission {permission}")
            return False
        return True
    except FileNotFoundError:
        common_logger.error(f"Dir {path} not found")
        return False
    except Exception as e:
        common_logger.error(f"An unknown error occurred while checking dir {path} : {e}")
        return False


def check_dir_owner(path: str):
    try:
        # 获取目录的统计信息
        stats = os.stat(path)
        # 获取当前用户名
        current_user = pwd.getpwuid(os.getuid()).pw_name
        # 获取目录的所有者用户名
        directory_owner = pwd.getpwuid(stats.st_uid).pw_name

        # 检查目录是否属于当前用户
        if directory_owner != current_user:
            common_logger.error(f"Dir {path} is not owned by {current_user}")
            return False
        return True

    except FileNotFoundError:
        common_logger.error(f"Dir {path} not found")
        return False
    except Exception as e:
        common_logger.error(f"An unknown error occurred while checking dir {path} : {e}")
        return False
