#!/usr/bin/env python
# -*- coding: UTF-8 -*-

# Copyright 2025 Huawei Technologies Co., Ltd
# ============================================================================

import asyncio
from threading import Lock as ThreadLock


def singleton(cls):
    instances = {}
    lock = asyncio.Lock()
    thread_lock = ThreadLock()

    def get_instance_sync(*args, **kwargs):
        with thread_lock:
            if cls not in instances:
                instances[cls] = cls(*args, **kwargs)
            return instances[cls]

    async def get_instance_async(*args, **kwargs):
        async with lock:
            if cls not in instances:
                instances[cls] = cls(*args, **kwargs)
            return instances[cls]

    def wrapper(*args, **kwargs):
        try:
            loop = asyncio.get_running_loop()
            if loop and loop.is_running():
                return get_instance_async(*args, **kwargs)
            else:
                return get_instance_sync(*args, **kwargs)
        except RuntimeError:
            return get_instance_sync(*args, **kwargs)

    return wrapper
