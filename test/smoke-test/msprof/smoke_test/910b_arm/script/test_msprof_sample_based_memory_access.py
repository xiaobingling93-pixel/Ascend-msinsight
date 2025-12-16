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


class MsprofSampleBasedMemoryAccess(test_base.TestProfiling):
    """
    测试使用msprof命令行采集方式采集sample-based模式MemoryAccess分组数据
    """
    OUTPUT_PATH = "/home/result_dir/test_msprof_sample_based_memory_access"
    P_LOG = f"{OUTPUT_PATH}/test_msporf_sample_based_memory_access.log"
    SAMPLE_CMD = (f"source activate smoke_test_env;"
                  f"cd /home/msprof_smoke_test/model/task_based_memory_access;"
                  f"export ASCEND_GLOBAL_EVENT_ENABLE=1;"
                  f"export ASCEND_SLOG_PRINT_TO_STDOUT=1;"
                  f"source /usr/local/Ascend/ascend-toolkit/set_env.sh;"
                  f"msprof --application=\"python3 loop_matmul.py\" --aic-mode=sample-based --aic-metrics=MemoryAccess --output={OUTPUT_PATH} > {P_LOG};")
    EXPECT_OP_HEADER = ['Device_id', 'Model ID', 'Task ID', 'Stream ID', 'Op Name', 'OP Type', 'OP State', 'Task Type',
                        'Task Start Time(us)', 'Task Duration(us)', 'Task Wait Time(us)', 'Block Dim', 'Mix Block Dim',
                        'HF32 Eligible', 'Input Shapes', 'Input Data Types', 'Input Formats', 'Output Shapes',
                        'Output Data Types', 'Output Formats', 'Context ID']
    EXPECT_FILES = ['ai_core_utilization', 'ai_vector_core_utilization', 'api_statistic', 'msprof', 'op_statistic', 'op_summary', 'README', 'task_time']
    PATTERNS = ['op_summary_*.csv', 'ai_core_utilization_*.csv', 'ai_vector_core_utilization_*.csv']
    EXPECT_CORE_HEADER = ['Device_id', 'Core ID', 'read_main_memory_datas(KB)', 'write_main_memory_datas(KB)',
                          'gm_to_l1_datas(KB)', 'l0c_to_l1_datas(KB)', 'l0c_to_gm_datas(KB)', 'gm_to_ub_datas(KB)', 'ub_to_gm_datas(KB)']

    def getTestCmd(self, scene=None):
        self.msprofbin_cmd = self.SAMPLE_CMD

    def checkResDir(self, scene=None):
        prof_path = ''
        for prof_ in os.listdir(self.res_dir):
            if os.path.isfile(os.path.join(self.res_dir, prof_)):
                continue
            prof_path = os.path.join(self.res_dir, prof_)
        for dir_ in os.listdir(prof_path):
            if "log" in dir_:
                self.view_error_msg(os.path.join(prof_path, dir_), "collection")
            if "output" in dir_:
                self.check_summary_header(os.path.join(prof_path, dir_))

    def check_summary_header(self, output_path):
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
        # 检查每类summary交付件是不是只有一个
        headers = [self.EXPECT_OP_HEADER, self.EXPECT_CORE_HEADER, self.EXPECT_CORE_HEADER]
        i = 0
        for pattern in self.PATTERNS:
            files = glob.glob(os.path.join(output_path, pattern))
            if len(files) != 1:
                self.res += 1
                self.logger.error(f"the count of current {pattern}:{len(files)} is not equal with 1")
                return
            with open(files[0]) as csv_file:
                csv_reader = csv.reader(csv_file)
                header = next(csv_reader)
                if not next(csv_reader, None):
                    self.res += 1
                    self.logger.error(f"the file {files[0]} has no data")
                    return
            if header != headers[i]:
                self.res += 1
                self.logger.error(f"the header in file {files[0]} is no expected")
                self.logger.error(f"预期：{self.EXPECT_OP_HEADER}")
                self.logger.error(f"实际：{header}")
            i += 1

if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(MsprofSampleBasedMemoryAccess("test_msprof_sample_based_memory_access", None, None, None, None))
    unittest.TextTestRunner(verbosity=2).run(suite)
