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
import os
import site
import subprocess

import common
from core.server import BaseServer, BaseServerManager, ServerEvent, ServerState
from utils.sys_util import start_monitor_process, lsof_i
from utils.logutil import server_logger
from utils.time_util import Timer


class ProfilerServer(BaseServer):
    TAG = "ProfilerServer"

    def __init__(self,
                 port: int = -1,
                 host: str = '127.0.0.1',
                 log_path: str = common.LOG_PATH,
                 log_size: int = common.LOG_SIZE,
                 log_level: str = common.LOG_LEVEL,
                 event_dir: str = common.PROFILER_SERVER_EVENT_DIR,
                 sid: int = 0,
                 server_event_callback=lambda event, server, msg="": None,
                 max_idle_time: int = common.MAX_SERVER_IDLE_TIME):
        self._log_path = log_path
        self._sid = sid
        self._log_size = log_size
        self._log_level = log_level
        self._sub_process = None
        self._env = {}
        self._start_cmd_list = []
        self._event_dir = event_dir
        self._idle_timer = Timer(max_idle_time, self._on_idle_timeout) if max_idle_time > 0 else None
        super().__init__(port, host, server_event_callback)

    def on_subprocess_exited_callback(self, process: subprocess.Popen):
        server_logger.error(
            f"Server[{self.net_location()}] subprocess[{process.pid}] exited(code={process.returncode}) abnormally.")
        self._terminate()

    def has_timer(self):
        return self._idle_timer is not None

    def set_timer(self, max_idle_time: int = common.MAX_SERVER_IDLE_TIME):
        if self.has_timer():
            return
        self._idle_timer = Timer(max_idle_time, self._on_idle_timeout)

    async def _on_idle_timeout(self):
        msg = f"Server {self.net_location()} has reached the max idle time"
        server_logger.info(msg)
        self._server_event_callback(ServerEvent.server_reached_max_idle_time, self, msg)

    def _initialize(self):
        if self._initialized:
            return
        args = {
            '--wsPort': self.port,
            '--wsHost': self.host,
            '--logPath': self._log_path,
            '--logSize': self._log_size,
            '--logLevel': self._log_level,
            '--sid': self._sid,
            '--eventDir': self._event_dir
        }
        start_args_list = []
        server_bin = "profiler_server"
        abs_server_bin = os.path.join(common.SERVER_PATH, server_bin)

        for k, v in args.items():
            if v:
                start_args_list.append(f"{k}={v}")
        self._start_cmd_list = [abs_server_bin] + start_args_list
        user_sites = site.getusersitepackages()
        sys_sites = ''.join(site.getsitepackages())
        self._env = {
            'LD_LIBRARY_PATH': f"{common.SERVER_PATH}:{os.getenv('LD_LIBRARY_PATH', '')}",
            'PYTHONPATH': f"{sys_sites}:{user_sites}:{os.getenv('PYTHONPATH', '')}",
        }
        super()._initialize()

    def _do_terminate(self):
        if self._sub_process.poll():
            server_logger.warning(
                f"Server[{self.net_location()}] has been terminated with return code {self._sub_process.poll()}")
            return
        try:
            self._sub_process.terminate()
            server_logger.info(f"Sent terminate signal to the server[{self.net_location()}:{self._sub_process.pid}].")
            self._sub_process.wait(timeout=3)
            server_logger.info(f"Server[{self.net_location()}] terminated.")
        except subprocess.TimeoutExpired as e:
            self._sub_process.kill()
            self._sub_process.wait()

    def _do_start(self):
        server_logger.info(
            f"try to start profiler_server using \ncmd = {' '.join(self._start_cmd_list)}, \nenv = {self._env}")
        self._sub_process = subprocess.Popen(self._start_cmd_list,
                                             stdout=subprocess.DEVNULL,
                                             stderr=subprocess.PIPE,
                                             env=self._env)
        asyncio.get_event_loop().create_task(
            start_monitor_process(self._sub_process,
                                  exited_callback=lambda process: self.on_subprocess_exited_callback(process),
                                  delay=3))
        return self._sub_process.poll() is None

    def _check_start(self) -> bool:
        try:
            result = lsof_i(self.port)
            server_logger.info(f"lsof result: \n{result}")
            return result and "profiler" in result and f"{self.port}" in result
        except Exception as e:
            server_logger.error(f"Unknown error in checking start state: {e}")
            return False

    def _do_pick(self):
        server_logger.info(f"Server[{self.net_location()}] has been picked.")

    def _do_release(self):
        server_logger.info(f"Server[{self.net_location()}] has been released.")

    def _set_state(self, state: ServerState):
        super()._set_state(state)
        if state == ServerState.Pending and self._idle_timer:
            self._idle_timer.reset()
        if state == ServerState.Working and self._idle_timer:
            self._idle_timer.remove()


class ProfilerServerManager(BaseServerManager):
    def __init__(self,
                 core_server_size: int = common.CORE_SERVER_SIZE,
                 max_server_size: int = common.MAX_SERVER_SIZE,
                 random_port_range: tuple = common.PROFILER_SERVER_DEFAULT_PORT_RANGE,
                 server_max_idle_time: int = common.MAX_SERVER_IDLE_TIME):
        super().__init__(core_server_size, max_server_size, random_port_range)
        self._server_max_idle_time = server_max_idle_time

    def server_event_sync_handler(self, event: ServerEvent, server: BaseServer, msg: str = ""):
        server_logger.info(f"event:{event}, server:{server.port}, msg:{msg}")
        if event == ServerEvent.server_started_successfully:
            self._on_server_start_finished(True, server)
            return
        if event == ServerEvent.server_started_failed:
            self._on_server_start_finished(False, server)
            return
        if event == ServerEvent.server_exited_by_command:
            self._on_server_terminated(server)
        if event == ServerEvent.server_reached_max_idle_time:
            self._on_server_reached_max_idle_time(server)

    def _on_server_reached_max_idle_time(self, server: BaseServer):
        server_logger.info(f"Server[{server.net_location()}] has reached max idle time will be terminate.")
        self._remove_server(server.net_location())

    def _on_server_start_finished(self, is_success: bool, server: BaseServer):
        if is_success:
            self._register_server(server)
            return
        server_logger.warning(f"Server[{server.net_location()}] start failed.]")

    def _on_server_terminated(self, server: ProfilerServer):
        if not server.has_timer():
            server_logger.info(f"Core server[{server.net_location()}] exited.Try to start new server")
            create_task = asyncio.create_task(self._add_server(is_core_server=True))
        self._remove_server(server.net_location())
        server_logger.info(f"Server[{server.net_location()}] has been terminated.")

    def _do_remove_server(self, server: BaseServer):
        server_logger.info(f"Remove profiler_server : {server.net_location()}")

    def _do_add_server(self, port: int, host: str = '127.0.0.1', is_core_server: bool = False, *args, **kwargs) -> bool:
        new_server = ProfilerServer(port=port,
                                    host=host,
                                    log_path=common.LOG_PATH,
                                    log_level=common.LOG_LEVEL,
                                    log_size=common.LOG_SIZE,
                                    server_event_callback=self.server_event_sync_handler,
                                    max_idle_time=self._server_max_idle_time if not is_core_server else 0)
        return new_server.start()
