#!/usr/bin/python3
# -*- coding: utf-8 -*-

import torch
import torch_npu


def test1():
    """
    计算类算子 单算子
    """
    a = torch.rand(2, 3).to("npu")
    b = torch.rand(2, 3).to("npu")
    c = a + b


if __name__ == "__main__":
    test1()
