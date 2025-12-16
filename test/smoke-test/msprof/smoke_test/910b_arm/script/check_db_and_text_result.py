#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
import sys
import re
import subprocess
import unittest
import logging
from datetime import datetime
import numpy as np
import pandas as pd
sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
from _cfg import ConfigValues
from check_tools.db_check import DBManager
from check_tools.db_check import EmptyClass

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


class ResultComparer(unittest.TestCase):
    """
    这个冒烟用例用来测试导出统一db和导出text(csv和json)的结果是否一致，实现准备好一份原始的数据，分别用--type=db和--type=text解析，
    对比两者的结果是否一致，数据来源目前是Smoke_test_IsAicModeIsTaskBased_LlcIsWrite_AicMetricsIsPipeUtilization.sh运行的结果，
    随着默认导出统一db功能上线，这里改为校验默认导出text+db的同一路径下文件，--type=db有test_pixel_level_check进行看护
    """
    MAX_JSON_SIZE = 1 * 1024 * 1024 * 1024
    # PROF_DB_PATH = "/home/result_dir/test_analysis_db_result_is_the_same_as_text_result/PROF_db"
    PROF_TEXT_PATH = "/home/result_dir/test_analysis_db_result_is_the_same_as_text_result/PROF_text"
    PROF_DB_PATH = PROF_TEXT_PATH
    CANN_API = "CANN_API"
    NPU_MEM = "NPU_MEM"
    NPU_MODULE_MEM = "NPU_MODULE_MEM"

    def __init__(self):
        super().__init__("execute")
        self.logger = logging
        self.id = "test_Analysis_db_result_is_the_same_as_text_result"  # 用例名字
        self.duration_time = 0  # 对比的时间
        self.res = 0  # 只要不为0，说明这个用例失败
        self.result = ""  # 根据self.res的结果，0的话是pass, 非0为fail，然后写到result.txt里面
        self.msprof_db_path = ""  # PROF_DB_PATH下面的统一db的路径
        self.api_statistic_csv_path = ""  # PROF_TEXT_PATH下面的output目录中的api_statistic表的路径
        self.npu_mem_csv_path = ""  # PROF_TEXT_PATH下面的output目录中的npu_mem表的路径
        self.npu_module_mem_csv_path = ""  # PROF_TEXT_PATH下面的output目录中的npu_mem表的路径
        self.msprof_json_path = ""  # PROF_TEXT_PATH下面的output目录中的msprof_xxx.json的路径
        self.df = None  # pandas读取msprof_xxx.json
        self.conn = EmptyClass("empty conn")
        self.curs = EmptyClass("empty curs")

    def init(self):
        self.logger.info("------------------------------- "
                         "Start execute case {} -------------------------------".format(self.id))
        self.get_file_path()
        self.df = self.read_json_file(self.msprof_json_path)
        if self.df is None:
            self.res += 1

    def finalize(self):
        self.logger.info("\n------------------------------- "
                         "End execute case {} -------------------------------".format(self.id))
        DBManager.destroy_db_connect(self.conn, self.curs)

    def check_result(self):
        if self.res == 0:
            self.result = ConfigValues.pass_res
        else:
            self.result = ConfigValues.fail_res
        # 记录执行结果到result.txt
        self.write_res()
        self.assertTrue(ConfigValues.pass_res == self.result)

    def compare_cann_api(self):
        """
        对比统一db的CANN_API表里面的数据量和api_statistic表里面的api的count之和
        """
        total_count = 0
        if self.api_statistic_csv_path:
            df = pd.read_csv(self.api_statistic_csv_path)
            total_count = df["Count"].sum()
        # 查询数据库中api数量
        sql = f"SELECT COUNT(*) FROM {self.CANN_API};"
        res = DBManager.fetch_all_data(self.curs, sql)
        data_num = res[0][0]  # 业务保证
        if data_num != total_count:
            self.res += 1

    def compare_npu_mem(self):
        data_len = 0
        if self.npu_mem_csv_path:
            df = pd.read_csv(self.npu_mem_csv_path)
            data_len = len(df)
        # 查询数据库中api数量
        sql = f"SELECT COUNT(*) FROM {self.NPU_MEM};"
        res = DBManager.fetch_all_data(self.curs, sql)
        data_num = res[0][0]  # 业务保证
        if data_num != data_len:
            self.res += 1

    def compare_npu_module_mem(self):
        data_len = 0
        if self.npu_module_mem_csv_path:
            df = pd.read_csv(self.npu_module_mem_csv_path)
            data_len = len(df)
        # 查询数据库中api数量
        sql = f"SELECT COUNT(*) FROM {self.NPU_MODULE_MEM};"
        res = DBManager.fetch_all_data(self.curs, sql)
        data_num = res[0][0]  # 业务保证
        if data_num != data_len:
            self.res += 1

    def check_data_integrity(self):
        expected_json_layer = {
            'HCCS', 'LLC', 'Stars Chip Trans', 'NPU MEM', 'HBM', 'AI Core Freq', 'SIO', 'QoS', 'NIC',
            'Ascend Hardware', 'Acc PMU', 'Overlap Analysis', 'RoCE', 'PCIe', 'Stars Soc Info', 'CANN'
        }
        json_layer = set()
        df = self.df
        filtered_df = df[df['name'] == 'process_name']
        args_name_values = filtered_df['args'].tolist()
        for args_name in args_name_values:
            if "name" in args_name:
                json_layer.add(args_name.get("name"))
        if json_layer != expected_json_layer:
            self.res += 1
            self.logger.error(f"msprof_xxx.json和预期不一致,预期：{expected_json_layer}，实际：{json_layer}")
        # 检查统一db里面的表是否符合预期
        expected_table = {
            'COMPUTE_TASK_INFO', 'HCCS', 'SESSION_TIME_INFO', 'ENUM_HCCL_RDMA_TYPE', 'NIC', 'NPU_MEM', 'ROCE',
            'ACC_PMU', 'ENUM_HCCL_LINK_TYPE', 'ENUM_HCCL_DATA_TYPE', 'HBM', 'SOC_BANDWIDTH_LEVEL', 'STRING_IDS',
            'ENUM_MODULE', 'TASK_PMU_INFO', 'PCIE', 'TASK', 'LLC', 'ENUM_MSTX_EVENT_TYPE', 'ENUM_API_TYPE',
            'META_DATA', 'HOST_INFO', 'ENUM_HCCL_TRANSPORT_TYPE', 'CANN_API', 'AICORE_FREQ', 'NPU_MODULE_MEM',
            'NPU_INFO', 'ENUM_MEMCPY_OPERATION', 'MEMCPY_INFO', 'QOS', 'RANK_DEVICE_MAP'
        }
        sql = "SELECT name FROM sqlite_master WHERE type='table';"
        res = DBManager.fetch_all_data(self.curs, sql)
        table_set = {table[0] for table in res}
        if expected_table != table_set:
            self.res += 1
            self.logger.error(f"预期：{sorted(list(expected_table))}")
            self.logger.error(f"实际：{sorted(list(table_set))}")

    def compare_analysis_result(self):
        if not self.conn or not self.curs:
            self.res += 1
            return
        self.compare_cann_api()
        self.compare_npu_mem()
        self.compare_npu_module_mem()
        if self.df is not None:
            self.check_data_integrity()


    def get_file_path(self):
        output_path = os.path.join(self.PROF_TEXT_PATH, "mindstudio_profiler_output")
        for file_name in os.listdir(output_path):
            # 获取api_statistic表的路径
            if file_name.startswith("api_statistic") and file_name.endswith(".csv"):
                self.api_statistic_csv_path = os.path.join(output_path, file_name)
            elif file_name.startswith("npu_mem_") and file_name.endswith(".csv"):
                self.npu_mem_csv_path = os.path.join(output_path, file_name)
            elif file_name.startswith("npu_module_mem") and file_name.endswith(".csv"):
                self.npu_module_mem_csv_path = os.path.join(output_path, file_name)
            elif file_name.startswith("msprof") and file_name.endswith(".json"):
                self.msprof_json_path = os.path.join(output_path, file_name)

        # 获取统一db的路径
        for file_name in os.listdir(self.PROF_DB_PATH):
            if file_name.startswith("msprof_") and file_name.endswith(".db"):
                self.msprof_db_path = os.path.join(self.PROF_DB_PATH, file_name)
                break
        if not self.msprof_db_path:
            return
        self.conn, self.curs = DBManager.create_connect_db(self.msprof_db_path)

    def write_res(self):
        with open('result.txt', 'a+') as f:
            f.write('%s %s %s\n' % (self.id, self.result, self.duration_time))

    def subprocess_cmd(self, cmd):
        self.logger.info("host command: {}".format(cmd))
        try:
            result = subprocess.run(['bash', '-c', cmd], capture_output=True, text=True)
            if result.returncode != 0 and len(result.stderr) != 0:
                self.logger.error(result.stderr)
                self.res += 1
                return result.stderr
        except (Exception, TimeoutError) as err:
            self.logger.error(err)
            self.res += 1
            return err
        finally:
            pass
        return result.stdout

    def view_error_msg(self, log_path, log_type=""):
        self.logger.info("start view {} log ...".format(log_type))
        if log_type == "plog":
            cmd = r"grep -rn 'ERROR\] PROFILING' {0}; grep -rn '\[ERROR\] \[MSVP\]' {0}; " \
                  r"grep -rn 'ERROR\] Failed' {0}".format(log_path)
        else:
            cmd = r"grep -rn 'ERROR\]' {0}".format(log_path)
        res = self.subprocess_cmd(cmd)
        if re.search(r"ERROR", res):
            self.logger.error(res)
            self.res += 1

    def view_analysis_log(self):
        """
        首先对导出text和db的日志进行检查，看看是否有ERROR日志，看护两边的解析代码
        """
        text_log_dir = os.path.join(self.PROF_TEXT_PATH, "mindstudio_profiler_log")
        db_log_dir = os.path.join(self.PROF_DB_PATH, "mindstudio_profiler_log")
        self.view_error_msg(text_log_dir)
        self.view_error_msg(db_log_dir)

    def read_json_file(self, file: str) -> any:
        if not os.path.isfile(file):
            self.logger.error("the file {} is not exist".format(file))
            return None
        if os.path.getsize(file) > self.MAX_JSON_SIZE:
            self.logger.error("the size of file {} is over 1 G")
            return None
        df = None
        try:
            df = pd.read_json(file, dtype=np.dtype)
        except Exception as e:
            self.logger.error("read {0} failed, errmsg is {1}".format(file, e.args))
        return df

    def execute(self):
        self.init()
        try:
            start_time = datetime.now()
            self.compare_analysis_result()
            time_diff = datetime.now() - start_time
            self.duration_time = time_diff.total_seconds()
        except Exception as err:
            self.logger.error(f"Execute {self.id} failed: {err}")
            self.res += 1
        # 执行结果校验
        self.check_result()
        self.finalize()


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(ResultComparer())
    unittest.TextTestRunner(verbosity=2).run(suite)
