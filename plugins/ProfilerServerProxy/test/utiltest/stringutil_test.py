#!/usr/bin/env python
# -*- coding: UTF-8 -*-

# Copyright 2025 Huawei Technologies Co., Ltd
# ============================================================================

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