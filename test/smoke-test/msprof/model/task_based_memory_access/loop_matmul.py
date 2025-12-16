#!/usr/bin/python3
# -*- coding: utf-8 -*-

import random
import torch
import torch_npu

def test():
    """
    计算类算子 单算子
    """
    for i in range(10):
        a = torch.rand(2, 3).to("npu")
        b = torch.rand(3, 2).to("npu")
        c = torch.matmul(a, b)
        print(c)

if __name__ == "__main__":
    torch_npu.npu.set_device(0)
    test()

