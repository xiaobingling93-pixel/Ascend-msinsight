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

import unittest
from utils import string_util


class TestStringUtil(unittest.TestCase):
    def test_parse_net_location_from_protocol_url(self):
        urls = [
            "http://www.apache.org/licenses/LICENSE-2.0",
            "https://somehost:someport/xxx",
            "file://gitee.com/ascend/mstt/tree/master/plugins/tensorboard-plugins/tb_graph_ascend",
            "ws://npms.io",
            "wss://127.0.0.1:8080/111"
        ]
        net_locations = [string_util.parse_net_location_from_url(url) for url in urls]
        assert_result = [
            "www.apache.org",
            "somehost:someport",
            "gitee.com",
            "npms.io",
            "127.0.0.1:8080"
        ]
        self.assertListEqual(assert_result, net_locations)

    def test_parse_net_location_from_invalid_url(self):
        urls = [
            "//www.apache.org/licenses/LICENSE-2.0",
            "://somehost:someport/xxx",
            "file//gitee.com/ascend/mstt/tree/master/plugins/tensorboard-plugins/tb_graph_ascend",
        ]
        net_locations = [string_util.parse_net_location_from_url(url) for url in urls]
        assert_result = [
            "www.apache.org",
            "",
            ""
        ]
        self.assertListEqual(assert_result, net_locations)