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
import sys
import asyncio
import argparse
import common
from utils import sys_util, logutil


def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument('--eventDir', type=str, required=False, default="")
    parser.add_argument('--port', type=int, required=False)
    parser.add_argument('--logPath', type=str, required=False, default="")
    parser.add_argument('--logSize', type=int, required=False, default=0)
    args = parser.parse_args()
    if args.eventDir and sys_util.check_dir_permissions(args.eventDir, os.R_OK):
        common.PROFILER_SERVER_EVENT_DIR = args.eventDir
    # 日志目录要求属主正确且可读可写
    if args.logPath and sys_util.check_dir_owner(args.logPath):
        if sys_util.check_dir_permissions(args.logPath, os.R_OK) and sys_util.check_dir_permissions(args.logPath,
                                                                                                    os.W_OK):
            common.LOG_PATH = args.logPath
    if args.logSize > 0:
        common.LOG_SIZE = args.logSize
    if args.port and sys_util.is_port_in_user_space(args.port):
        common.PORT = args.port


def init_log_util():
    logutil.proxy_logger = logutil.LogUtil.get_logger("PROXY")
    logutil.server_logger = logutil.LogUtil.get_logger("PROFILER")
    logutil.common_logger = logutil.LogUtil.get_logger("COMMON")
    logutil.logger = logutil.LogUtil.get_logger()


if __name__ == '__main__':
    parse_arguments()
    init_log_util()
    from simple_profiler_server_proxy import start_proxy
    asyncio.run(start_proxy())
