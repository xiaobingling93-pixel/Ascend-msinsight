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
