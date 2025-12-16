#!/usr/bin/python3
# -*- coding: utf-8 -*-
import unittest
import logging
import os
import sys
import glob
import csv
import test_base

sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


class MsprofTaskBasedMemoryAccess(test_base.TestProfiling):
    """
    测试使用msprof命令行采集方式采集task-based模式MemoryAccess分组数据
    """
    OUTPUT_PATH = "/home/result_dir/test_msprof_task_based_memory_access"
    P_LOG = f"{OUTPUT_PATH}/test_msporf_task_based_memory_access.log"
    CMD = (f"source activate smoke_test_env;"
           f"cd /home/msprof_smoke_test/model/task_based_memory_access;"
           f"export ASCEND_GLOBAL_EVENT_ENABLE=1;"
           f"export ASCEND_SLOG_PRINT_TO_STDOUT=1;"
           f"source /usr/local/Ascend/ascend-toolkit/set_env.sh;"
           f"msprof --application=\"python3 loop_matmul.py\" --aic-metrics=MemoryAccess --output={OUTPUT_PATH} > {P_LOG};")
    EXPECT_HEADER = ['Device_id', 'Model ID', 'Task ID', 'Stream ID', 'Op Name', 'OP Type', 'OP State', 'Task Type',
                     'Task Start Time(us)', 'Task Duration(us)', 'Task Wait Time(us)', 'Block Dim', 'Mix Block Dim',
                     'HF32 Eligible', 'Input Shapes', 'Input Data Types', 'Input Formats', 'Output Shapes',
                     'Output Data Types', 'Output Formats', 'Context ID', 'aicore_time(us)', 'aic_total_cycles',
                     'aic_read_main_memory_datas(KB)', 'aic_write_main_memory_datas(KB)', 'aic_GM_to_L1_datas(KB)',
                     'aic_L0C_to_L1_datas(KB)', 'aic_L0C_to_GM_datas(KB)', 'aic_GM_to_UB_datas(KB)', 'aic_UB_to_GM_datas(KB)',
                     'aiv_time(us)', 'aiv_total_cycles', 'aiv_read_main_memory_datas(KB)', 'aiv_write_main_memory_datas(KB)',
                     'aiv_GM_to_L1_datas(KB)', 'aiv_L0C_to_L1_datas(KB)', 'aiv_L0C_to_GM_datas(KB)', 'aiv_GM_to_UB_datas(KB)', 'aiv_UB_to_GM_datas(KB)']
    EXPECT_FILES = ['api_statistic', 'msprof', 'op_statistic', 'op_summary', 'README', 'task_time']

    def getTestCmd(self, scene=None):
        self.msprofbin_cmd = self.CMD

    def checkResDir(self, scene=None):
        prof_lst = []
        for prof_ in os.listdir(self.res_dir):
            if os.path.isfile(os.path.join(self.res_dir, prof_)):
                continue
            prof_lst.append(os.path.join(self.res_dir, prof_))

        for prof_ in prof_lst:
            for dir_ in os.listdir(prof_):
                if "log" in dir_:
                    self.view_error_msg(os.path.join(prof_, dir_), "collection")
                if "output" in dir_:
                    self.check_op_summary_header(os.path.join(prof_, dir_))

    def check_op_summary_header(self, output_path):
        miss_files = []
        all_files = os.listdir(output_path)
        for pattern in self.EXPECT_FILES:
            matched = any(file.startswith(pattern) for file in all_files)
            if not matched:
                miss_files.append(pattern)
        if miss_files:
            self.res += 1
            self.logger.error(f"The files in {output_path} is not expected")
            self.logger.error(f"预期：{self.EXPECT_FILES}")
            self.logger.error(f"实际缺少：{miss_files}")
            return
        # 检查op_summary交付件是不是只有一个
        files = glob.glob(os.path.join(output_path, 'op_summary_*.csv'))
        print(files)
        if len(files) != 1:
            self.res += 1
            self.logger.error(f"the count of current op_summary.csv:{len(files)} is not equal with 1")
            return
        with open(files[0], newline='') as csv_file:
            csv_reader = csv.reader(csv_file)
            header = next(csv_reader)
            if not next(csv_reader, None):
                self.res += 1
                self.logger.error(f"the file {files[0]} has no data")
                return
        if header != self.EXPECT_HEADER:
            self.res += 1
            self.logger.error(f"the header in file {files[0]} is no expected")
            self.logger.error(f"预期：{self.EXPECT_HEADER}")
            self.logger.error(f"实际：{header}")

if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(MsprofTaskBasedMemoryAccess("test_msprof_task_based_memory_access", None, None, None, None))
    unittest.TextTestRunner(verbosity=2).run(suite)
