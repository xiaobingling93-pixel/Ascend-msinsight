#!/usr/bin/env python
# -*- coding: UTF-8 -*-

# Copyright 2025 Huawei Technologies Co., Ltd
# ============================================================================

from urllib.parse import urlparse


def parse_net_location_from_url(url: str) -> str:
    parse_url = urlparse(url)
    return parse_url.netloc if parse_url else ""
