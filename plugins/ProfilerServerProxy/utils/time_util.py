#!/usr/bin/env python
# -*- coding: UTF-8 -*-

# Copyright 2025 Huawei Technologies Co., Ltd
# ============================================================================

from enum import IntEnum
import asyncio
from typing import Callable


class TimeDimension(IntEnum):
    SECONDS = 1000
    MINUTES = 60 * SECONDS
    HOURS = 60 * MINUTES
    DAYS = 24 * HOURS


class Timer:
    def __init__(self, duration: int = 600, async_callback=lambda: None):
        """
        初始化计时器。

        :param duration: 计时时间（秒）
        :param callback: 回调函数，计时结束后将被调用
        """
        self._duration = duration
        self._callback = async_callback
        self._task = None
        self._lock = asyncio.Lock()

    def start(self):
        """启动计时器"""

        async def start_timer():
            async with self._lock:
                if self._task is not None and not self._task.done():
                    self._task.cancel()
                self._task = asyncio.create_task(self._timer())

        asyncio.create_task(start_timer())

    def reset(self, new_duration=None):
        """重置计时器"""

        async def reset_timer():
            async with self._lock:
                if self._task is not None:
                    self._task.cancel()
                if new_duration is not None:
                    self._duration = new_duration
                self._task = asyncio.create_task(self._timer())

        asyncio.create_task(reset_timer())

    def remove(self):
        async def cancel_timer():
            async with self._lock:
                if self._task is not None:
                    self._task.cancel()
                    self._task = None

    async def _timer(self):
        """内部方法，用于执行计时操作"""
        try:
            await asyncio.sleep(self._duration)
            if callable(self._callback):
                await self._callback()
        except asyncio.CancelledError:
            pass  # 忽略取消异常
