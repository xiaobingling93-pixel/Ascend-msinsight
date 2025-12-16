#!/usr/bin/python3
# -*- coding: utf-8 -*-
import glob
import unittest
import logging
import os
import sys
import test_base
import pandas as pd

sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


class PytorchCaptureWithBeginEnd(test_base.TestProfiling):
    """
    校验capture场景，无stepId，通过profiler对象采集，采集范围包含begin end
    校验项
    1、kernel_details.csv中相关字段，
    """
    OUTPUT_PATH = "/home/result_dir/test_torch_capture_without_schedule_by_object_profiler/PytorchCaptureWithBeginEnd"
    CMD = (f"source /root/miniconda3/bin/activate smoke_test_env_pta;"
           f"cd /home/msprof_smoke_test/model/easy_capture_testcase;"
           f"export ASCEND_GLOBAL_EVENT_ENABLE=1;"
           f"export ASCEND_SLOG_PRINT_TO_STDOUT=0;"
           f"export ASCEND_PROCESS_LOG_PATH={OUTPUT_PATH}/begin_end;"
           f"source /usr/local/Ascend/ascend-toolkit/set_env.sh;"
           f"torchrun --nnodes=1 --nproc_per_node=2 --node_rank=0 --master_port=29500 ./test_begin_end.py")
    ASCEND_PROFILER_OUTPUT = "ASCEND_PROFILER_OUTPUT"

    def getTestCmd(self, scene=None):
        self.msprofbin_cmd = self.CMD

    def checkResDir(self, scene=None):
        pt_list = []
        for file in os.listdir(self.res_dir):
            file_path = os.path.join(self.res_dir, file)
            if os.path.isdir(file_path) and file.endswith("ascend_pt"):
                pt_list.append(file_path)
        if len(pt_list) == 0:
            self.res += 1
            self.logger.error(f"没有在 {self.res_dir} 下找到任何pt文件。")
            return
        for pt_path in pt_list:
            self.check_kernel_details(pt_path)

    def check_kernel_details(self, pt_path):
        kernel_details_files = glob.glob(os.path.join(pt_path, self.ASCEND_PROFILER_OUTPUT, 'kernel_details.csv'))
        print(kernel_details_files)
        if len(kernel_details_files) != 1:
            self.res += 1
            self.logger.error(f"没找到kernel_details.csv,路径是 {pt_path}")
            return


class PytorchCaptureOnlyReplay(test_base.TestProfiling):
    """
    校验capture场景，无stepId，通过profiler对象采集，采集范围不包含begin end
    校验项
    1、kernel_details.csv中相关字段，
    """
    OUTPUT_PATH = "/home/result_dir/test_torch_capture_without_schedule_by_object_profiler/PytorchCaptureOnlyReplay"
    CMD = (f"source /root/miniconda3/bin/activate smoke_test_env_pta;"
           f"cd /home/msprof_smoke_test/model/easy_capture_testcase;"
           f"export ASCEND_GLOBAL_EVENT_ENABLE=1;"
           f"export ASCEND_SLOG_PRINT_TO_STDOUT=0;"
           f"export ASCEND_PROCESS_LOG_PATH={OUTPUT_PATH}/only_replay;"
           f"source /usr/local/Ascend/ascend-toolkit/set_env.sh;"
           f"torchrun --nnodes=1 --nproc_per_node=2 --node_rank=0 --master_port=29500 ./test_replay.py")
    ASCEND_PROFILER_OUTPUT = "ASCEND_PROFILER_OUTPUT"
    NA_HEADERS_LIST = ["OP State", "HF32 Eligible", "Input Shapes", "Input Data Types", "Input Formats",
                       "Output Shapes", "Output Data Types", "Output Formats"]

    def getTestCmd(self, scene=None):
        self.msprofbin_cmd = self.CMD

    def checkResDir(self, scene=None):
        pt_list = []
        for file in os.listdir(self.res_dir):
            file_path = os.path.join(self.res_dir, file)
            if os.path.isdir(file_path) and file.endswith("ascend_pt"):
                pt_list.append(file_path)
        if len(pt_list) == 0:
            self.res += 1
            self.logger.error(f"没有在 {self.res_dir} 下找到任何pt文件。")
            return
        for pt_path in pt_list:
            self.check_kernel_details(pt_path)

    def check_kernel_details(self, pt_path):
        kernel_details_files = glob.glob(os.path.join(pt_path, self.ASCEND_PROFILER_OUTPUT, 'kernel_details.csv'))
        print(kernel_details_files)
        if len(kernel_details_files) != 1:
            self.res += 1
            self.logger.error(f"没找到kernel_details.csv,路径是 {pt_path}")
            return

        df = pd.read_csv(kernel_details_files[0])
        self.logger.info(f"start to check kernel_details.csv, path is {kernel_details_files[0]}")

        # 所有的值都应该是NA
        for na_header in self.NA_HEADERS_LIST:
            if not (df[na_header].isna()).all():
                self.res += 1
                self.logger.error(f"{na_header} 有非NA值")


if __name__ == '__main__':
    suite = unittest.TestSuite()

    args = sys.argv
    if args[1] == "begin_end":
        suite.addTest(PytorchCaptureWithBeginEnd(
            "test_torch_capture_without_schedule_by_object_profiler/PytorchCaptureWithBeginEnd", None, None, None, None))
    elif args[1] == "replay":
        suite.addTest(PytorchCaptureOnlyReplay(
            "test_torch_capture_without_schedule_by_object_profiler/PytorchCaptureOnlyReplay", None, None, None, None))

    unittest.TextTestRunner(verbosity=2).run(suite)
