#!/usr/bin/env python3
# -*- coding: UTF-8 -*-

import torch
import torch_npu
import torchair as tng
import numpy as np
import os
import sys
import argparse
import logging
import json
import math
import time
import threading

from torchair.configs.compiler_config import CompilerConfig
import torch._dynamo
TORCHDYNAMO_VERBOSE=1
TORCH_LOGS="+dynamo"

# 支持入图的打印宏
from torchair.core.utils import logger
logger.setLevel(logging.ERROR)
config = CompilerConfig()
npu_backend = tng.get_npu_backend(compiler_config=config)

# global settings
logging.basicConfig(format='[%(levelname)s] %(message)s', level=logging.INFO)

activities_dict = {
    "CPU": torch_npu.profiler.ProfilerActivity.CPU,
    "NPU": torch_npu.profiler.ProfilerActivity.NPU
}
aic_metrics_dict = {
    "PipeUtilization": torch_npu.profiler.AiCMetrics.PipeUtilization,
    "ArithmeticUtilization": torch_npu.profiler.AiCMetrics.ArithmeticUtilization,
    "Memory": torch_npu.profiler.AiCMetrics.Memory,
    "MemoryL0": torch_npu.profiler.AiCMetrics.MemoryL0,
    "ResourceConflictRatio": torch_npu.profiler.AiCMetrics.ResourceConflictRatio,
    "MemoryUB": torch_npu.profiler.AiCMetrics.MemoryUB,
    "L2Cache": torch_npu.profiler.AiCMetrics.L2Cache
}
profiler_level_dict = {
    "0": torch_npu.profiler.ProfilerLevel.Level0,
    "1": torch_npu.profiler.ProfilerLevel.Level1,
    "2": torch_npu.profiler.ProfilerLevel.Level2
}


# def process_kwargs(kwargs_temp):
#     kwargs = {}
#     for k, v in kwargs_temp.items():
#         if v is not None:
#             try:
#                 if v.lower() == "true":
#                     v = True
#                 elif v.lower() == "false":
#                     v = False
#             except Exception as e:
#                 logging.debug(e)
#             kwargs.update({k: v})
#     logging.debug(f"generate kwargs: {kwargs}")
#     return kwargs


class Model(torch.nn.Module):
    def __init__(self, func):
        super().__init__()
        self.func = func

    def forward(self):
        return self.func()

class DeviceThread(threading.Thread):
    def __init__(self, func, graph_mode=True, device_id=0, loops=1):
        super().__init__()
        self.func = func
        self.graph_mode = graph_mode
        self.device_id = device_id
        self.loops = loops

    def run(self):
        with torch.no_grad():
            model = Model(self.func)
            model = torch.compile(model, backend=npu_backend, dynamic=False, fullgraph=True)
        for i in range(self.loops):
            if not i % 1:
                logging.info(f"device_{self.device_id} loop times: {i} ")
            self.run_by_graph_mode()

    def run_by_graph_mode(self):
        torch.npu.set_device(f"npu:{self.device_id}")
        if self.graph_mode:
            # with torch.no_grad():
            #     model = Model(self.func)
            #     model = torch.compile(model, backend=npu_backend, dynamic=False, fullgraph=True)
            graph_output = model()
            logging.debug("graph output with mask static:", graph_output, graph_output.shape)

            # with torch.no_grad():
            #     model = Model(self.func)
            #     model = torch.compile(model, backend=npu_backend, dynamic=True, fullgraph=True)
            #     dynamic_graph_output = model()
            #     logging.debug("graph output with mask dynamic:", dynamic_graph_output, dynamic_graph_output.shape)
            
        single_op = self.func()
        logging.debug("single op output with mask:", single_op, single_op.shape)


def run_unit():
    input_dtype = "float32"
    x1_shape = [128, 32]
    x2_shape = [32, 128]
    x3_shape = [128]
    x1 = torch.from_numpy(np.random.uniform(-1, 1, x1_shape).astype("float16").astype(input_dtype))
    x2 = torch.from_numpy(np.random.uniform(-1, 1, x2_shape).astype("float16").astype(input_dtype))
    x3 = torch.from_numpy(np.random.uniform(-1, 1, x3_shape).astype("int64").astype(input_dtype))

    x1_npu = x1.type(torch.float16).to("npu")
    x2_npu = x2.type(torch.float16).to("npu")
    x3_npu = x3.type(torch.int64).to("npu")

    # AICORE:matmul
    out_npu = torch.matmul(x1_npu, x2_npu)
    x3_npu = x3_npu.type(torch.float32).to("npu")

    # AIV:mul
    out_npu = torch.mul(out_npu, x3_npu)

    # MIX_AIC:PFA
    # 生成随机数据，并发送到npu
    q = torch.randn(1, 8, 164, 128, dtype=torch.bfloat16).npu()
    k = torch.randn(1, 8, 1024, 128, dtype=torch.bfloat16).npu()
    v = torch.randn(1, 8, 1024, 128, dtype=torch.bfloat16).npu()
    scale = 1/math.sqrt(128.0)
    actseqlen = [164]
    actseqlenkv = [1024]
    out = torch_npu.npu_prompt_flash_attention(q, k, v, 
    actual_seq_lengths = actseqlen, actual_seq_lengths_kv = actseqlenkv,
    num_heads = 8, input_layout = "BNSD", scale_value=scale, pre_tokens=65535, next_tokens=65535)

    return out


def run_extra_unit():
    input_dtype = "float32"
    x1_shape = [16, 32]
    x1 = torch.from_numpy(np.random.uniform(-1, 1, x1_shape).astype("float16").astype(input_dtype))
    x2 = torch.from_numpy(np.random.uniform(-1, 1, x1_shape).astype("float16").astype(input_dtype))

    x1_npu = x1.type(torch.float16).to("npu")
    x2_npu = x2.type(torch.float16).to("npu")

    out_npu = torch.add(x1_npu, x2_npu)
    out_npu = torch.add(x1_npu, x2_npu)

    return out_npu


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='run the testcase')
    # profiling config
    parser.add_argument("--result_path", default='.')
    parser.add_argument("--activities", help="CPU,NPU")
    parser.add_argument("--record_shapes", action="store_true")
    parser.add_argument("--profile_memory", action="store_true")
    parser.add_argument("--with_stack", action="store_true")
    parser.add_argument("--with_flops", action="store_true")
    parser.add_argument("--with_modules", action="store_true")
    parser.add_argument("--aic_metrics", choices=aic_metrics_dict.keys())
    parser.add_argument("--profiler_level", choices=["0", "1", "2"])
    parser.add_argument("--l2_cache", action="store_true")
    parser.add_argument("--data_simplification", action="store_true")
    parser.add_argument("--record_op_args", action="store_true")
    parser.add_argument("--op_attr", action="store_true")
    parser.add_argument("--mode", default="no_prof", choices=["no_prof", "npu_profiler"])
    # dump config
    parser.add_argument("--dump_json")
    # app config
    parser.add_argument("--aclop", action="store_true")
    parser.add_argument("--dynamo", action="store_true")
    parser.add_argument("--extra_task", action="store_true")
    parser.add_argument("--run_mode", default="single", choices=["single", "long"], help="决定了多个iteration下，是多次开启关闭profiling和dump还是仅一次")
    parser.add_argument("-d", "--device_ids", default="0")
    parser.add_argument("-l", "--iterations", type=int, default=1)
    parser.add_argument("--loops", type=int, default=1)
    args = parser.parse_args()

    if args.aclop:
        torch.npu.set_compile_mode(jit_compile=True)

    if args.activities is not None:
        activities = []
        try:
            activities_tmp = args.activities.split(",")
            for item in activities_tmp:
                activities.append(activities_dict.get(item, item))
        except Exception as e:
            logging.debug(f"parse activities failed, {e}")
    else:
        activities = args.activities

    if args.profiler_level or args.l2_cache or args.data_simplification or args.aic_metrics or args.record_op_args or args.op_attr:
        experimental_config = {
            "aic_metrics": aic_metrics_dict.get(args.aic_metrics),
            "profiler_level": profiler_level_dict.get(args.profiler_level),
            "l2_cache": args.l2_cache,
            "data_simplification": args.data_simplification,
            "record_op_args": args.record_op_args,
            "op_attr": args.op_attr,
            "export_type": [
                torch_npu.profiler.ExportType.Db,
                torch_npu.profiler.ExportType.Text,
            ],
        }
        experimental_config = torch_npu.profiler._ExperimentalConfig(**experimental_config)
    else:
        experimental_config = None

    kwargs = {
        "activities": activities,
        # 使用start、stop方式控制启停，无需schedule
        # "schedule": torch_npu.profiler.schedule(wait=0, warmup=0, active=1, repeat=args.iterations, skip_first=0),
        "on_trace_ready": torch_npu.profiler.tensorboard_trace_handler(args.result_path),
        "record_shapes": args.record_shapes,
        "profile_memory": args.profile_memory,
        "with_stack": args.with_stack,
        "with_flops": args.with_flops,
        "with_modules": args.with_modules,
        "experimental_config": experimental_config
    }

    if args.mode == "npu_profiler":
        prof_handle = torch_npu.profiler.profile(**kwargs)

    device_list = args.device_ids.split(",")

    start_time = time.time()
    if args.run_mode == "single":
        for i in range(args.iterations):
            logging.info(f"iteration: {i}")
            if args.mode == "npu_profiler":
                prof_handle.start()
            if args.dump_json:
                torch_npu.npu.init_dump()
                torch_npu.npu.set_dump(args.dump_json)

            thread_list = []
            for dev in device_list:
                thread = DeviceThread(run_unit, args.dynamo, dev, args.loops)
                thread_list.append(thread)
            for th in thread_list:
                th.start()
            for th in thread_list:
                th.join()

            if args.mode == "npu_profiler":
                prof_handle.stop()
            if args.dump_json:
                torch_npu.npu.finalize_dump()

            if args.extra_task:
                logging.info(f"extra_task...")
                if args.mode == "npu_profiler":
                    prof_handle.start()
                if args.dump_json:
                    torch_npu.npu.init_dump()
                    torch_npu.npu.set_dump(args.dump_json)

                thread_list_extra = []
                for dev in device_list:
                    thread = DeviceThread(run_extra_unit, args.dynamo, dev, args.loops)
                    thread_list_extra.append(thread)
                for th in thread_list_extra:
                    th.start()
                for th in thread_list_extra:
                    th.join()

                if args.mode == "npu_profiler":
                    prof_handle.stop()
                if args.dump_json:
                    torch_npu.npu.finalize_dump()

            if args.mode == "npu_profiler":
                prof_handle.step()
    elif args.run_mode == "long":
        if args.mode == "npu_profiler":
            prof_handle.start()
        if args.dump_json:
            torch_npu.npu.init_dump()
            torch_npu.npu.set_dump(args.dump_json)
        for i in range(args.iterations):
            logging.info(f"iteration: {i}")
            thread_list = []
            for dev in device_list:
                thread = DeviceThread(run_unit, args.dynamo, dev, args.loops)
                thread_list.append(thread)
            for th in thread_list:
                th.start()
            for th in thread_list:
                th.join()
        if args.mode == "npu_profiler":
            prof_handle.stop()
        if args.dump_json:
            torch_npu.npu.finalize_dump()
    end_time = time.time()
    logging.info(f"mode: {args.mode}, device_id: {args.device_ids}, iterations: {args.iterations}, loops: {args.loops}, graph_mode: {args.dynamo}, run_mode: {args.run_mode}, cost_time: {end_time - start_time:.2f}s")


