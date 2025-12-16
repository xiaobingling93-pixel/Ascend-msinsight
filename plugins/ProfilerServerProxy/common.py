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
from pathlib import Path

COMMON_LOOP = None
ROOT_PATH = Path(__file__).resolve().parent
PROFILER_PATH = ROOT_PATH.parent
FRONTEND_PATH = PROFILER_PATH / "frontend"
SERVER_PATH = PROFILER_PATH / "server"

# ServerProxy相关配置
CORE_SERVER_SIZE = 1  # 核心server数 默认1
MAX_SERVER_SIZE = 3  # 最大server数 默认3
MAX_SERVER_IDLE_TIME = 600  # 最大空闲时间 默认10分钟
PROXY_SERVER_HOST = "127.0.0.1"
PROXY_SERVER_PORT = 9000

# profiler_server相关配置
PROFILER_SERVER_DEFAULT_PORT_RANGE = (9001, 9099)
PROFILER_SERVER_EVENT_DIR = ""

# 日志相关配置
LOG_PATH = ROOT_PATH / "log"
LOG_SIZE = 0
LOG_LEVEL = ""
LOG_BACKUP_COUNT = 5
