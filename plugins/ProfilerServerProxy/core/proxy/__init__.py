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
from enum import Enum
from utils.time_util import TimeDimension
from utils.pattern import Subject, Observer


class ProxyServerConfig:
    def __init__(self, host: str = '127.0.0.1', port: int = 9000):
        self.host = host
        self.port = port
        self.enable_heartbeat: bool = False
        self.heartbeat_interval: int = 0
        self.enable_server_pool: bool = False
        self.core_size: int = 1
        self.max_size: int = 3
        self.max_idle_time: int = 30 * TimeDimension.SECONDS

    def enable_heartbeat(self, heartbeat_interval: int = 10 * TimeDimension.SECONDS):
        self.heartbeat_interval = heartbeat_interval
        self.enable_heartbeat = True

    def enable_server_pool(self, core_size: int = 1, max_size: int = 3,
                           max_idle_time: int = 30 * TimeDimension.SECONDS):
        self.core_size = core_size
        self.max_size = max_size
        self.max_idle_time = max_idle_time
        self.enable_server_pool = True


DEFAULT_CONFIG = ProxyServerConfig()


class ProxyWebsocketServerEvent(Enum):
    new_connection = 1
    pair_established = 2
    pair_released = 3


class ProxyWebsocketServerObserver(Observer):
    async def update(self, *args, **kwargs):
        event = kwargs.get('event', None)
        if event is None:
            return
        server_net_location = kwargs.get('server_net_location', None)
        request = kwargs.get('request', None)
        client_websocket = kwargs.get('client_websocket', None)
        server_websocket = kwargs.get('backend_websocket', None)

        if event == ProxyWebsocketServerEvent.new_connection:
            return await self.on_new_conn_request(request)
        if event == ProxyWebsocketServerEvent.pair_established:
            return await self.on_pair_established(client_websocket, server_websocket, server_net_location)
        if event == ProxyWebsocketServerEvent.pair_released:
            return await self.on_pair_released(client_websocket, server_websocket, server_net_location)

    async def on_new_conn_request(self, request):
        ...

    async def on_pair_established(self, client_websocket, server_websocket,
                                  server_net_location):
        ...

    async def on_pair_released(self, client_websocket, server_websocket,
                               server_net_location):
        ...
