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
import logging
import time
import warnings
from enum import Enum
from abc import ABC, abstractmethod
from utils import sys_util
from utils.logutil import server_logger
import common


class ServerState(Enum):
    Initial = 0
    Starting = 1
    Pending = 2
    Working = 3
    Exited = 4


# 默认检查server是否拉起 最大重试次数
DEFAULT_CHECK_SERVER_PS_MAX_RETRIES = 5
# 默认延时
DEFAULT_CHECK_SERVER_PS_DELAY = 1

DEFAULT_RANDOM_SERVER_PORT = -1


class ServerEvent(Enum):
    server_started_successfully = 0
    server_started_failed = 1
    server_exited_on_idle = 2
    server_exited_by_command = 3
    server_exited_on_error = 4
    server_picked_up_successfully = 5
    server_picked_up_failed = 6
    server_released_successfully = 7
    server_released_failed = 8
    server_state_changed = 9
    server_reached_max_idle_time = 10


class BaseServer(ABC):
    def __init__(self,
                 port: int,
                 host: str = '127.0.0.1',
                 server_event_callback=lambda event, server, msg="": None):
        self.state = ServerState.Initial
        self.port = port
        if not sys_util.is_port_in_user_space(port):
            raise ValueError(f'Port {port} is not valid')
        self.host = host
        self.protocol = 'ws/http'
        self._async_lock = asyncio.Lock()
        self._subprocess = None
        self._initialized = False
        self._server_event_callback = server_event_callback
        self._initialize()

    # 同步启动
    def start(self):
        if not self._is_initial():
            err = f"Server {self.net_location()} state: {self.state} cannot be start again."
            server_logger.warning(err)
            return False
        if not self._initialized:
            err = f"Server {self.net_location()} not initialized"
            server_logger.warning(err)
            return False
        self._set_state(ServerState.Starting)
        if not self._do_start():
            server_logger.error(f"Server {self.net_location()} process failed to start.")
            return False
        server_logger.info(
            f"Server {self.net_location()} is in the process of starting up."
            f"Beginning to check whether it has started successfully.")
        for i in range(DEFAULT_CHECK_SERVER_PS_MAX_RETRIES):
            time.sleep(DEFAULT_CHECK_SERVER_PS_DELAY)
            is_running = self._check_start()
            server_logger.info(
                f"Performing {i + 1}/{DEFAULT_CHECK_SERVER_PS_MAX_RETRIES} checks")
            if is_running:
                self._set_state(ServerState.Pending)
                self._server_event_callback(ServerEvent.server_started_successfully, self)
                server_logger.info(f'Server {self.net_location()} start successfully')
                return True
            server_logger.warning(
                f"Completed {i + 1}/{DEFAULT_CHECK_SERVER_PS_MAX_RETRIES} checks. Check result: {is_running}")
        self._terminate()
        self._server_event_callback(ServerEvent.server_started_failed, self)
        server_logger.error(f'Server {self.net_location()} failed to start.')
        return False

    def get_state(self):
        return self.state

    async def terminate(self, *args, **kwargs):
        async with self._async_lock:
            self._terminate()

    async def pick(self):
        async with self._async_lock:
            self._pick()

    async def release(self):
        async with self._async_lock:
            return self._release()

    async def is_pending(self):
        async with self._async_lock:
            return self.state == ServerState.Pending

    async def is_working(self):
        async with self._async_lock:
            return self.state == ServerState.Working

    async def is_exited(self):
        async with self._async_lock:
            return self.state == ServerState.Exited

    async def is_initial(self):
        async with self._async_lock:
            return self.state == ServerState.Initial

    async def is_starting(self):
        async with self._async_lock:
            return self.state == ServerState.Starting

    def net_location(self):
        return f"{self.host}:{self.port}"

    async def set_state(self, state: ServerState):
        async with self._async_lock:
            self._set_state(state)

    def can_be_remove(self):
        return self._is_initial() or self._is_exited() or self._is_pending()

    @abstractmethod
    def _initialize(self):
        self._initialized = True

    @abstractmethod
    def _check_start(self) -> bool:
        ...

    @abstractmethod
    def _do_terminate(self):
        ...

    @abstractmethod
    def _do_start(self):
        ...

    @abstractmethod
    def _do_pick(self):
        ...

    @abstractmethod
    def _do_release(self):
        ...

    def _terminate(self):
        self._do_terminate()
        self._set_state(ServerState.Exited)
        self._server_event_callback(ServerEvent.server_exited_by_command, self)

    def _pick(self):
        if not self._is_pending():
            self._server_event_callback(ServerEvent.server_picked_up_failed, self,
                                        f"Cannot pick server: {self.net_location()} in state: {self.state}")
            return None
        try:
            self._do_pick()
            self._set_state(ServerState.Working)
        except Exception as e:
            self._server_event_callback(ServerEvent.server_picked_up_failed, self,
                                        f"Pick server: {self.net_location()} failed: {e}")
            return None
        self._server_event_callback(ServerEvent.server_picked_up_successfully, self)
        return self

    def _release(self):
        if not self._is_working():
            self._server_event_callback(ServerEvent.server_released_failed, self,
                                        f"Cannot release server: {self.net_location()} in state: {self.state}")
            return False
        try:
            self._do_release()
            self._set_state(ServerState.Pending)
        except Exception as e:
            self._server_event_callback(ServerEvent.server_picked_up_failed, self,
                                        f"Pick server: {self.net_location()} failed: {e}")
            return False
        self._server_event_callback(ServerEvent.server_released_successfully, self)
        return True

    def _set_state(self, state: ServerState):
        if self.state == state:
            return
        self._server_event_callback(ServerEvent.server_state_changed, self,
                                    f"from {self.state} to {state}")
        self.state = state

    def _is_pending(self):
        return self.state == ServerState.Pending

    def _is_working(self):
        return self.state == ServerState.Working

    def _is_exited(self):
        return self.state == ServerState.Exited

    def _is_starting(self):
        return self.state == ServerState.Starting

    def _is_initial(self):
        return self.state == ServerState.Initial


class BaseServerManager(ABC):
    def __init__(self, core_server_size: int = common.CORE_SERVER_SIZE, max_server_size: int = common.MAX_SERVER_SIZE,
                 random_port_range: tuple = common.PROFILER_SERVER_DEFAULT_PORT_RANGE):
        if not 0 < core_server_size <= max_server_size:
            raise ValueError(f'core_server_size {core_server_size} is not valid')
        if not len(random_port_range) == 2:
            raise ValueError("Invalid port range, illegal range tuple")
        if random_port_range[1] - random_port_range[0] + 1 < core_server_size:
            raise ValueError("Invalid port range, the number of available values should be "
                             "at least the number of the core server size.")
        self._server_map = {}
        self._max_server_map_size = max_server_size
        self._core_server_size = core_server_size
        self._random_port_range = random_port_range
        self._lock = asyncio.Lock()

    async def start(self):
        async with self._lock:
            server_logger.info("Trying to start the profiler server manager...")
            server_logger.info(f"Trying to start {self._core_server_size} core servers.")
            while True:
                if len(self._server_map) >= self._core_server_size:
                    break
                server_logger.info(f"Starting the {len(self._server_map) + 1}/{self._core_server_size} server")
                if not await self._add_server(is_core_server=True):
                    server_logger.warning(
                        f"Failed to add the {len(self._server_map) + 1}/{self._core_server_size} server.")

    async def add_server(self, port: int = DEFAULT_RANDOM_SERVER_PORT, host: str = '127.0.0.1', *args, **kwargs):
        async with self._lock:
            return await self._add_server(port, host, False, *args, **kwargs)

    async def is_available_port_for_register(self, port: int):
        async with self._lock:
            self._is_available_port_register(port)

    async def register_server(self, server: BaseServer):
        async with self._lock:
            return self._register_server(server)

    async def pick_server_by_location(self, server_net_location):
        async with self._lock:
            return await self._pick_server_by_location(server_net_location)

    async def release_server_by_location(self, server_net_location):
        async with self._lock:
            return await self._release_server_by_location(server_net_location)

    async def find_available_random_port(self) -> int:
        random_port = await sys_util.find_available_port_in_range(*self._random_port_range, host="127.0.0.1",
                                                                  max_try_times=self._max_server_map_size * 3)
        if random_port == -1:
            raise RuntimeError(
                f"Unable to find an available random port after {self._max_server_map_size * 3} attempts.")
        return random_port

    async def get_idle_server(self, auto_create: bool = False):
        async with self._lock:
            for server in self._server_map.values():
                if await server.is_pending():
                    return server
            server_logger.info("There is no idle server.")
            if auto_create:
                server_logger.info("Trying to start a new server in background.")
                create_task = asyncio.create_task(self._add_server())

    async def remove_server(self, server_net_location: str):
        async with self._lock:
            server_count = len(self._server_map)
            if server_count < self._core_server_size + 1:
                raise RuntimeError(
                    f"Cannot remove server: {server_net_location}, there are only {self._core_server_size} servers.")
            if server_net_location not in self._server_map:
                raise RuntimeError(f"Cannot remove server[{server_net_location}], not found.")
            if self._server_map[server_net_location].is_pending():
                raise RuntimeWarning(f"Try to remove server[{server_net_location}] in pending.")
            server = self._server_map.pop(server_net_location)
            await server.terminate()
            self._do_remove_server(server)

    @abstractmethod
    def _do_add_server(self, port: int, host: str = '127.0.0.1', *args, **kwargs) -> bool:
        ...

    @abstractmethod
    def _do_remove_server(self, server: BaseServer):
        ...

    def _is_available_port_register(self, port: int, host: str = '127.0.0.1'):
        if not port:
            return False
        is_in_range = self._random_port_range[0] <= port <= self._random_port_range[1]
        is_unregistered = f"{host}:{port}" not in self._server_map
        # 在端口范围内且未注册
        return is_in_range and is_unregistered

    def _remove_server(self, server_net_location: str):
        server_count = len(self._server_map)
        if server_net_location not in self._server_map:
            server_logger.error(f"Cannot remove server[{server_net_location}], not found.")
            return False
        server = self._server_map[server_net_location]
        if not server.can_be_remove():
            server_logger.warning(f"Try to remove server[{server_net_location}] in state {server.get_state()}.")
            return False
        server = self._server_map.pop(server_net_location)
        self._do_remove_server(server)
        return True

    async def _add_server(self,
                          port: int = DEFAULT_RANDOM_SERVER_PORT,
                          host: str = '127.0.0.1',
                          is_core_server: bool = False,
                          *args, **kwargs):
        if port == DEFAULT_RANDOM_SERVER_PORT or f"{host}:{port}" in self._server_map:
            port = await self.find_available_random_port()
        current_server_size = len(self._server_map)
        if current_server_size >= self._max_server_map_size:
            server_logger.warning("Max server size reached, cannot add more server.")
            return False
        server_logger.info(f"Trying to create and run new server: {host}:{port}")
        if self._do_add_server(port, host, is_core_server, *args, **kwargs):
            server_logger.info(f"Server {host}:{port} added successfully.")
            return True
        else:
            server_logger.warning(f"Failed to added server: {host}:{port}")
            return False

    def _register_server(self, server: BaseServer):
        if not self._is_available_port_register(server.port):
            server_logger.warning(f"Cannot register server[{server.host}:{server.port}], port is already registered.")
            return False
        if server.state == ServerState.Exited:
            server_logger.error(f"Cannot register server[{server.host}:{server.port}], server is already exited.")
            return False
        self._server_map[f"{server.host}:{server.port}"] = server
        return True

    async def _pick_server_by_location(self, server_net_location):
        if server_net_location not in self._server_map:
            logging.error(f"Cannot find server[{server_net_location}]")
            return False
        return await self._server_map[server_net_location].pick()

    async def _release_server_by_location(self, server_net_location):
        if server_net_location not in self._server_map:
            logging.error(f"Cannot find server[{server_net_location}]")
            return False
        return await self._server_map[server_net_location].release()
