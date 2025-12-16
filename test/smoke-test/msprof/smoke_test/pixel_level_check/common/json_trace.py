#!/usr/bin/python3
# -*- coding: utf-8 -*-
# Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.

from dataclasses import dataclass
from dataclasses import field
from typing import Dict, Any


@dataclass
class TraceEvent:
    """
    所有对象共有的元素为pid tid name ph
    """
    ph: str = ""
    bp: str = ""
    cat: str = ""
    name: str = ""
    pid: int = 0
    tid: int = 0
    id: int = 0
    ts: str = ""
    dur: float = 0.0
    args: Dict[str, Any] = field(default_factory=dict)
