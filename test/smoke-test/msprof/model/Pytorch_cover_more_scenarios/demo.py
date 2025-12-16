#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
import random
import torch
import torch_npu
import torch.distributed as dist
import torch.multiprocessing as mp
import numpy as np
from torch.distributed.distributed_c10d import ReduceOp
from torch_npu.dynamo import torchair
from torchair.configs.compiler_config import CompilerConfig
import torchair.ge_concrete_graph.ge_converter.experimental.patch_for_hcom_allreduce


def test1():
    """
    计算类算子 单算子
    """
    a = torch.rand(2, 3).to("npu")
    b = torch.rand(2, 3).to("npu")
    c = a + b

class DynamicNet(torch.nn.Module):
    def __init__(self, D_in, H, D_out):
        """
        构造3个nn.Linear实例
        """
        super(DynamicNet, self).__init__()
        self.input_linear = torch.nn.Linear(D_in, H)
        self.middle_linear_0 = torch.nn.Linear(H, H)
        self.middle_linear_1 = torch.nn.Linear(H, H)
        self.output_linear = torch.nn.Linear(H, D_out)

    def forward(self, x):
        h_relu = self.input_linear(x).clamp(min=0)
        for i in range(6):
            if i & 1:
                h_relu = self.middle_linear_1(h_relu).clamp(min=random.random())
            else:
                h_relu = self.middle_linear_0(h_relu).clamp(min=random.random())
        y_pred = self.output_linear(h_relu)
        return y_pred

def test2():
    """
    event打点，通信算子，mix算子
    :return:
    """
    N, D_in, H, D_out = 256, 1024, 4096, 64
    x = torch.randn(N, D_in, requires_grad=True).npu()
    y = torch.randn(N, D_out).npu()
    model = DynamicNet(D_in, H, D_out).npu()
    criterion = torch.nn.MSELoss(size_average=False).npu()
    start = torch_npu.npu.Event(enable_timing=True)
    end = torch_npu.npu.Event(enable_timing=True)
    for t in range(2):
        start.record()
        range_id = torch_npu.npu.mstx.range_start("@SUM(cmd|'/c calc'!A0)")
        y_pred = model(x)
        torch_npu.npu.mstx.range_end(range_id)
        end.record()
        end.wait()
        end.synchronize()
        print("Model exec time", start.elapsed_time(end))
        loss = criterion(y_pred, y)
        loss.backward()



class AllReduceSingeGroup(torch.nn.Module):
    def __init__(self):
        super().__init__()

    def forward(self, x, y):
        x = x + y
        torch.distributed.all_reduce(x, op=torch.distributed.ReduceOp.SUM)
        return x

def example(rank, world_size):
    os.environ["MASTER_ADDR"] = "localhost"
    os.environ["MASTER_PORT"] = "29506"
    x = torch.ones([2, 2], dtype=torch.int32).to("npu:"+str(rank))
    y = torch.ones([2, 2], dtype=torch.int32).to("npu:"+str(rank))
    config = CompilerConfig()
    npu_backend = torchair.get_npu_backend(compiler_config=config)
    model = torch.compile(AllReduceSingeGroup().to("npu:"+str(rank)), backend=npu_backend, dynamic=False)
    out = torch.ones([2, 2], dtype=torch.int32).npu() * 2 * world_size
    ret = model(x, y)
    assert out.equal(ret)
    torch.distributed.destroy_process_group()

def run_mm_all_reduce_base(rank, B, S, K, N, dtype):
    torch_npu.npu.set_device(rank)
    dist.init_process_group(backend="hccl", rank=rank, world_size=2, init_method='tcp://127.0.0.1:50001')
    from torch.distributed.distributed_c10d import _get_default_group
    default_pg = _get_default_group()
    if torch.__version__ > '2.0.1':
        hcom_info = default_pg._get_backend(torch.device("npu")).get_hccl_comm_name(rank)
    else:
        hcom_info = default_pg.get_hccl_comm_name(rank)

    # prepare input data
    np_input = np.random.uniform(-5, 5, size=[S, K])
    input = torch.tensor(np_input).to(dtype).npu()
    np_weight = np.random.uniform(-20, 20, size=[K, N])
    weight = torch.tensor(np_weight).to(dtype).npu()
    for i in range(2):
        output = torch_npu.npu_mm_all_reduce_base(input, weight, hcom_info, reduce_op='sum')
    print("rank: {} allreduce output : {}".format(rank, output))
    example(rank, world_size=2)
    test1()
    test2()


def test_0():
    """
    mc2算子
    """
    B = 1
    S = 122940
    K = 1536
    N = 12288
    dtype = torch.bfloat16
    mp.spawn(run_mm_all_reduce_base, args=(B, S, K, N, dtype), nprocs=2)


if __name__ == "__main__":
    test_0()
