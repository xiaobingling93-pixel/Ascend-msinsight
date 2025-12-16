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

import unittest
import socket
from utils import sys_util
from utils.logutil import common_logger


class TestSysUtilAsync(unittest.IsolatedAsyncioTestCase):
    async def test_find_available_port_in_range_valid(self):
        start_port = 1024
        end_port = 65535
        able = await sys_util.find_available_port_in_range(start_port, end_port)
        if able != -1:
            self.assertTrue(start_port <= able <= end_port)

    async def test_find_available_port_in_invalid_range_with(self):
        ranges = [
            [-2, -1],  # 全部在范围外
            [65536, 100000000000000],
            [-2, 65535],  # 存在交集但起始点在范围外
            [80, 65536],  # 存在交集但终止点在范围外
            [65535, 80],  # 起始点大于终止点
        ]
        able_list = [await sys_util.find_available_port_in_range(_range[0], _range[1]) for _range in ranges]
        for able in able_list:
            self.assertEqual(able, -1)

    async def test_find_processes_by_port(self):
        able_port = await sys_util.find_available_port_in_range(1024, 65535)
        if able_port != -1:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                try:
                    s.bind(("127.0.0.1", able_port))
                    s.listen()
                    processes = sys_util.find_processes_by_port(able_port)
                    self.assertEqual(1, len(processes))
                    self.assertIn("python", processes[0].name())
                except socket.error:
                    common_logger.warning(f"Failed to bind port {able_port}, try again")
