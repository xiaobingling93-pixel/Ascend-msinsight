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
import functools
import aiohttp
from aiohttp import web, ClientSession, WSMsgType
from utils import string_util
from utils.pattern import Subject
from utils.decorators import singleton
from utils.logutil import proxy_logger
from core.proxy import ProxyServerConfig, DEFAULT_CONFIG, ProxyWebsocketServerEvent
from core.server import BaseServer, ServerState


@singleton
class MultiplexAIOProxyServer(Subject):
    def __init__(self, proxy_server_config: ProxyServerConfig = DEFAULT_CONFIG):
        super().__init__()
        self.proxy_server_config = proxy_server_config
        self.idle_server_selector = None
        self.initialized = False

    async def forward_to_backend(self, client_ws, backend_ws, backend_ws_uri):
        try:
            async for msg in client_ws:
                if msg.type == WSMsgType.TEXT:
                    await backend_ws.send_str(msg.data)
                elif msg.type == WSMsgType.BINARY:
                    await backend_ws.send_bytes(msg.data)
                elif msg.type == WSMsgType.ERROR:
                    proxy_logger.warning(f'Frontend connection closed with exception {client_ws.exception()}')
                    break
        except Exception as e:
            proxy_logger.warning(f"Error forwarding to backend: {e}")
        finally:
            await backend_ws.close()
            proxy_logger.warning(f'The proxy-server closed the connection to server {backend_ws_uri}.')
            await self.notify(event=ProxyWebsocketServerEvent.pair_released,
                              client_websocket=client_ws,
                              backend_websocket=backend_ws,
                              server_net_location=string_util.parse_net_location_from_url(backend_ws_uri))

    async def forward_to_frontend(self, client_ws, backend_ws):
        try:
            async for msg in backend_ws:
                if msg.type == aiohttp.WSMsgType.TEXT:
                    await client_ws.send_str(msg.data)
                elif msg.type == aiohttp.WSMsgType.BINARY:
                    await client_ws.send_bytes(msg.data)
                elif msg.type == WSMsgType.ERROR:
                    proxy_logger.warning(
                        f'Backend connection closed with exception {backend_ws.exception()}')
                    break
        except Exception as e:
            proxy_logger.warning(f"Error forwarding to frontend: {e}")
        finally:
            proxy_logger.warning(f'The proxy-server closed the connection to client.')
            await client_ws.close()

    async def handle_websocket_connection(self, request, backend_ws_uri):
        """Handle WebSocket connection and proxy to backend server."""
        ws = web.WebSocketResponse()
        proxy_logger.info(f"New websocket client connected.")
        await ws.prepare(request)

        async with ClientSession() as session:
            async with session.ws_connect(backend_ws_uri) as backend_ws:
                proxy_logger.info(f"Connected to backend WebSocket server: {backend_ws_uri}")
                await self.notify(event=ProxyWebsocketServerEvent.pair_established,
                                  request=request,
                                  client_websocket=ws,
                                  backend_websocket=backend_ws,
                                  server_net_location=string_util.parse_net_location_from_url(backend_ws_uri))

                # 启动双向转发任务
                await asyncio.gather(self.forward_to_backend(ws, backend_ws, backend_ws_uri),
                                     self.forward_to_frontend(ws, backend_ws))
        return ws

    async def handle_http_request(self, request, backend_url):
        """Handle HTTP request and proxy to backend server."""
        async with ClientSession() as session:
            # 向后端服务器发送请求
            async with session.request(
                    method=request.method,
                    url=backend_url + request.path_qs,
                    headers=request.headers,
                    data=await request.read()
            ) as resp:
                body = await resp.read()
                return web.Response(body=body, status=resp.status, headers=resp.headers)

    async def default_request_handler(self, request):
        """Determine if the request is HTTP or WebSocket and handle accordingly."""
        if not self.idle_server_selector:
            error = "The proxy server has not completed initialization, and therefore cannot select a backend."
            proxy_logger.error(error)
            raise RuntimeError(error)
        selected_server: BaseServer = await self.idle_server_selector()
        # 如果无可用server, 且已达到server数量上限
        if not selected_server:
            error = "The service-providing server has reached its limit and cannot establish new connections."
            proxy_logger.error(error)
            return web.HTTPBadRequest(text=error)
        if request.headers.get('Upgrade', '').lower() == 'websocket':
            # 通知所有观察者，有新的websocket连接请求
            await self.notify(event=ProxyWebsocketServerEvent.new_connection,
                              request=request)
            # 处理 WebSocket 请求
            return await self.handle_websocket_connection(request,
                                                          f"ws://{selected_server.host}:{selected_server.port}")
        else:
            # 处理 HTTP 请求
            return await self.handle_http_request(request, f"http://{selected_server.host}:{selected_server.port}")

    async def start(self, idle_server_selector, request_handler=default_request_handler):
        if not idle_server_selector:
            raise ValueError(f"Start proxy server failed, illegal idle server selector: None.")
        self.idle_server_selector = idle_server_selector
        app = web.Application()
        app.router.add_route('*', '/{tail:.*}',
                             functools.partial(request_handler, self))

        runner = web.AppRunner(app)
        self.initialized = True
        await runner.setup()
        site = web.TCPSite(runner, self.proxy_server_config.host, self.proxy_server_config.port)
        await site.start()
        proxy_logger.info(f"Proxy server started on {self.proxy_server_config.host}:{self.proxy_server_config.port}")

        # 保持服务器运行
        while True:
            await asyncio.sleep(3600)
