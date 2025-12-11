#!/usr/bin/env python
# -*- coding: UTF-8 -*-

# Copyright 2025 Huawei Technologies Co., Ltd
# ============================================================================

import unittest
import asyncio
from test.servertest import SimpleEchoWebsocketServerManager
import websockets
from utils.sys_util import find_available_port_in_range
from utils.logutil import common_logger
from core.proxy.multi_aio_proxy_server import MultiplexAIOProxyServer
from core.proxy import ProxyWebsocketServerObserver, ProxyServerConfig


class SimpleObserver(ProxyWebsocketServerObserver):
    def __init__(self, profiler_server_manager):
        super().__init__()
        self.profiler_server_manager = profiler_server_manager

    async def on_new_conn_request(self, request):
        ...

    async def on_pair_established(self, client_websocket, server_websocket, server_net_location):
        await self.profiler_server_manager.pick_server_by_location(server_net_location)

    async def on_pair_released(self, client_websocket, server_websocket,
                               server_net_location):
        await self.profiler_server_manager.release_server_by_location(server_net_location)


class TestMultiplexAIOProxyServer(unittest.IsolatedAsyncioTestCase):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.proxy_server_port = -1
        self.simple_observer = None
        self.simple_profiler_server_manager = None
        self.simple_proxy_server = None

    async def asyncSetUp(self):
        self.simple_profiler_server_manager = SimpleEchoWebsocketServerManager(core_server_size=1, max_server_size=1)
        self.simple_observer = SimpleObserver(self.simple_profiler_server_manager)
        await self.simple_profiler_server_manager.start()
        self.proxy_server_port = await find_available_port_in_range(10000, 20000)
        all_host_proxy_config = ProxyServerConfig(host="127.0.0.1", port=self.proxy_server_port)

        ## 被测代理
        self.simple_proxy_server = await MultiplexAIOProxyServer(all_host_proxy_config)

        async def simple_server_selector():
            return await self.simple_profiler_server_manager.get_idle_server(auto_create=True)

        await self.simple_proxy_server.attach(self.simple_observer)
        asyncio.create_task(self.simple_proxy_server.start(simple_server_selector))

    async def asyncTearDown(self):
        await self.simple_profiler_server_manager.stop()

    async def test_single_client_connect_and_request(self):
        uri = f"ws://localhost:{self.proxy_server_port}"
        try:
            async with websockets.connect(uri) as soc:
                await soc.send("hello, proxy!")
                resp = await soc.recv()
                self.assertIn("hello, proxy!", resp)
        except Exception as e:
            common_logger.error(f"test_single_client_connect_and_request error: {e}")
            self.fail(f"{e}")