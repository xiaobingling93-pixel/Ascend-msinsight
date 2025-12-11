#!/usr/bin/env python
# -*- coding: UTF-8 -*-

# Copyright 2025 Huawei Technologies Co., Ltd
# ============================================================================
import asyncio
from core.proxy.multi_aio_proxy_server import MultiplexAIOProxyServer
from core.proxy import ProxyServerConfig
from core.proxy import ProxyWebsocketServerObserver
from core.server.profiler_server import ProfilerServerManager
import common

common.COMMON_LOOP = asyncio.get_event_loop()

## profiler_server_manager
profiler_server_manager = ProfilerServerManager(core_server_size=common.CORE_SERVER_SIZE,
                                                max_server_size=common.MAX_SERVER_SIZE)


class SimpleObserver(ProxyWebsocketServerObserver):
    async def on_new_conn_request(self, request):
        ...

    async def on_pair_established(self, client_websocket, server_websocket,
                                  server_net_location):
        await profiler_server_manager.pick_server_by_location(server_net_location)

    async def on_pair_released(self, client_websocket, server_websocket,
                               server_net_location):
        await profiler_server_manager.release_server_by_location(server_net_location)


all_host_proxy_config = ProxyServerConfig(host=common.PROXY_SERVER_HOST, port=common.PROXY_SERVER_PORT)

## 代理
proxy_server = MultiplexAIOProxyServer(all_host_proxy_config)
simple_observer = SimpleObserver()


async def simple_server_selector():
    return await profiler_server_manager.get_idle_server(auto_create=True)


async def start_proxy():
    await profiler_server_manager.start()
    await proxy_server.attach(simple_observer)
    await proxy_server.start(simple_server_selector)
