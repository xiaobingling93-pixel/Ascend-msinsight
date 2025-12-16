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

import unittest
import time
import asyncio
from utils import time_util


class TestTimeUtilAsync(unittest.IsolatedAsyncioTestCase):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.end_time = 0

    async def test_timer_end(self):
        start_time = int(time.time())
        duration = 5
        self.end_time = 0

        async def simple_async_callback():
            self.end_time = int(time.time())

        timer = time_util.Timer(duration, simple_async_callback)
        timer.start()
        await asyncio.sleep(duration + 1)
        self.assertEqual(duration, self.end_time - start_time)
        self.end_time = 0

    async def test_timer_reset_end(self):
        start_time = int(time.time())
        duration = 5
        reset = 1
        self.end_time = 0

        async def simple_async_callback():
            self.end_time = int(time.time())

        timer = time_util.Timer(duration, simple_async_callback)
        timer.start()
        await asyncio.sleep(reset)
        timer.reset()
        await asyncio.sleep(duration + 1)
        self.assertEqual(duration + reset, self.end_time - start_time)
        self.end_time = 0

    async def test_timer_cancel(self):
        duration = 5
        self.end_time = 0

        async def simple_async_callback():
            self.end_time = int(time.time())

        timer = time_util.Timer(duration, simple_async_callback)
        timer.start()
        await asyncio.sleep(1)
        timer.remove()
        await asyncio.sleep(duration + 1)
        self.assertEqual(0, self.end_time)