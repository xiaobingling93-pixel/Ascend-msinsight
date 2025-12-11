#!/usr/bin/env python
# -*- coding: UTF-8 -*-

# Copyright 2025 Huawei Technologies Co., Ltd
# ============================================================================

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