#!/usr/bin/env python
# -*- coding: UTF-8 -*-

# Copyright 2025 Huawei Technologies Co., Ltd
# ============================================================================

import asyncio


class Observer:
    async def update(self, *args, **kwargs):
        ...


class Subject:
    def __init__(self):
        self._observers = []
        self._lock = asyncio.Lock()

    async def attach(self, observer: Observer):
        async with self._lock:
            if observer not in self._observers:
                self._observers.append(observer)

    async def detach(self, observer: Observer):
        async with self._lock:
            try:
                self._observers.remove(observer)
            except ValueError:
                pass

    async def notify(self, *args, **kwargs):
        async with self._lock:
            for observer in self._observers:
                await observer.update(*args, **kwargs)
