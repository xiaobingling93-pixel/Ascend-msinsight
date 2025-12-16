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

import asyncio
import websockets

import core.server
from core.server import BaseServer, BaseServerManager, ServerEvent, DEFAULT_CHECK_SERVER_PS_DELAY
from utils.sys_util import check_listen_server_exists
from utils.logutil import server_logger

core.server.DEFAULT_CHECK_SERVER_PS_DELAY = 3


class SimpleEchoWebsocketServer(BaseServer):
    def _initialize(self):
        self.stop_event = asyncio.Event()
        self.server_task = None
        super()._initialize()

    def _check_start(self) -> bool:
        return True

    def _do_terminate(self):
        self.stop_event.set()
        self.server_task.cancel()

    def _do_start(self):
        async def echo(websocket):
            """
                处理 WebSocket 连接并回响消息。
                """
            async for message in websocket:
                await websocket.send(f"[{self.net_location()}]Echo: {message}")

        async def start_echo_server():
            async with websockets.serve(echo, self.host, self.port):
                server_logger.info(f"Started: {self.port}")
                await self.stop_event.wait()

        self.server_task = asyncio.create_task(start_echo_server())
        return True

    def _do_pick(self):
        pass

    def _do_release(self):
        pass


class SimpleEchoWebsocketServerManager(BaseServerManager):

    def _do_add_server(self, port: int, host: str = '127.0.0.1', *args, **kwargs) -> bool:
        new_server = SimpleEchoWebsocketServer(host=host, port=port,
                                               server_event_callback=self.server_event_sync_handler)
        return new_server.start()

    def _do_remove_server(self, server: BaseServer):
        pass

    def _on_server_reached_max_idle_time(self, server: BaseServer):
        server_logger.info(f"Server[{server.net_location()}] has reached max idle time will be terminate.")
        self._remove_server(server.net_location())

    def _on_server_start_finished(self, is_success: bool, server: BaseServer):
        if is_success:
            self._register_server(server)
            return
        server_logger.warning(f"Server[{server.net_location()}] start failed.]")

    def _on_server_terminated(self, server: BaseServer):
        server_logger.info(f"Server[{server.net_location()}] exited.")
        asyncio.create_task(self._add_server(is_core_server=True))
        self._remove_server(server.net_location())
        server_logger.info(f"Server[{server.net_location()}] has been terminated.")
