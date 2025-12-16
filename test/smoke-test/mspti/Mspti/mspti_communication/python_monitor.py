#!/usr/bin/python3
# coding=utf-8
#
# Copyright (C) 2025. Huawei Technologies Co., Ltd. All rights reserved.
#
# The sample demonstrates how to obtain kernel data and hccl data through the MSPTI Python API,
# and how to use MSTX dot to obtain range data
#

import os
import random
import torch
import torch_npu
import torch.nn as nn
import torch.optim as optim

import ctypes

mspti_lib = ctypes.CDLL("./mspti_lib.so")

experimental_config = torch_npu.profiler._ExperimentalConfig(
    export_type=[torch_npu.profiler.ExportType.Text],
    profiler_level=torch_npu.profiler.ProfilerLevel.Level0,
    msprof_tx=False,
    aic_metrics=torch_npu.profiler.AiCMetrics.AiCoreNone,
    l2_cache=False,
    op_attr=False,
    data_simplification=False
)
prof = torch_npu.profiler.profile(
    activities=[
        torch_npu.profiler.ProfilerActivity.CPU,
        torch_npu.profiler.ProfilerActivity.NPU
    ],
    # schedule=torch_npu.profiler.schedule(wait=0, warmup=0, active=2, repeat=1, skip_first=0),
    on_trace_ready=torch_npu.profiler.tensorboard_trace_handler("./result"),
    record_shapes=False,
    profile_memory=False,
    with_stack=False,
    with_modules=False,
    with_flops=False,
    experimental_config=experimental_config
)


class ToyModel(nn.Module):
    def __init__(self, D_in, H, D_out):
        super(ToyModel, self).__init__()
        self.input_linear = torch.nn.Linear(D_in, H)
        self.middle_linear = torch.nn.Linear(H, H)
        self.output_linear = torch.nn.Linear(H, D_out)

    def forward(self, x):
        h_relu = self.input_linear(x).clamp(min=0)
        for i in range(3):
            h_relu = self.middle_linear(h_relu).clamp(min=random.random())
        y_pred = self.output_linear(h_relu)
        torch.distributed.all_reduce(y_pred)
        return y_pred


def init_process(backend="hccl"):
    torch.distributed.init_process_group(backend=backend, init_method='env://')


def test_monitor():
    # enable mspti monitor to collect activity
    mspti_lib.MsptiStart()
    # prof.start()
    init_process()
    device = int(os.getenv('LOCAL_RANK'))
    torch.npu.set_device(device)

    N, D_in, H, D_out = 256, 1024, 4096, 64
    input_data = torch.randn(N, D_in).npu()
    labels = torch.randn(N, D_out).npu()
    model = ToyModel(D_in, H, D_out).npu()

    loss_fn = nn.MSELoss()
    optimizer = optim.SGD(model.parameters(), lr=0.001)
        
    for i in range(5):
        optimizer.zero_grad()
        outputs = model(input_data)
        loss = loss_fn(outputs, labels)
        loss.backward()
        optimizer.step()

    torch.npu.synchronize()
    # prof.stop()
    # stop mspti monitor and consume activity
    mspti_lib.MsptiStop()
    mspti_lib.MsptiFlushAll()


if __name__ == "__main__":
    test_monitor()
