#!/usr/bin/python3
# -*- coding: utf-8 -*-
import unittest
import json
import logging
import os
import sys
import test_base
import pandas as pd
import numpy as np

sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
from check_tools.db_check import DBManager

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


class PytorchSingleProcessMultipleDevices(test_base.TestProfiling):
    """
    测试使用msprof命令行采集方式采集task-based模式MemoryAccess分组数据
    """
    OUTPUT_PATH = "/home/result_dir/test_pytorch_single_process_multiple_devices"
    P_LOG = f"{OUTPUT_PATH}/test_pytorch_single_process_multiple_devices.log"
    CMD = (f"source /root/miniconda3/bin/activate smoke_test_env_pta;"
           f"cd /home/msprof_smoke_test/model/Pytorch_single_process_multiple_devices;"
           f"export ASCEND_GLOBAL_EVENT_ENABLE=1;"
           f"export ASCEND_SLOG_PRINT_TO_STDOUT=1;"
           f"source /usr/local/Ascend/ascend-toolkit/set_env.sh;"
           f"python3 pt_scene_v2_thread.py --result_path {OUTPUT_PATH} --activities CPU,NPU --record_shapes"
           f" --profile_memory --aic_metrics PipeUtilization --profiler_level 1 --record_op_args --op_attr"
           f" --l2_cache --with_stack --with_modules --mode npu_profiler -d 0,1,2,3 > {P_LOG} 2>&1")
    ASCEND_PROFILER_OUTPUT = "ASCEND_PROFILER_OUTPUT"

    def getTestCmd(self, scene=None):
        self.msprofbin_cmd = self.CMD

    def checkResDir(self, scene=None):
        ascend_pt_path = ""
        csv_list = []
        for file in os.listdir(self.res_dir):
            file_path = os.path.join(self.res_dir, file)
            if os.path.isdir(file_path) and file.endswith("ascend_pt"):
                ascend_pt_path = file_path
                continue
        profiler_ouput_path = os.path.join(ascend_pt_path, self.ASCEND_PROFILER_OUTPUT)
        for file in os.listdir(profiler_ouput_path):
            if file.endswith("csv"):
                csv_list.append(os.path.join(profiler_ouput_path, file))
        analysis_db = os.path.join(profiler_ouput_path, "analysis.db")
        trace_view_json = os.path.join(profiler_ouput_path, "trace_view.json")
        self.check_csv_headers(csv_list)
        self.check_db_headers(analysis_db)
        self.check_json_labels(trace_view_json)

    def check_csv_headers(self, csv_list):
        miss_files = []
        expected_devices = {0, 1, 2, 3}
        for csv_path in csv_list:
            df = pd.read_csv(csv_path)
            csv_name = os.path.basename(csv_path)
            devices = expected_devices
            if 'Device_id' not in df.columns and csv_name not in ["memory_record.csv"]:
                miss_files.append(csv_name)
                self.res += 1
                self.logger.error(f"The {csv_name} has no header 'Device_id'!!")
            elif 'Device_id' in df.columns:  # 检查这些交付件里面的device_id是不是符合预期
                if csv_name == "api_statistic.csv":
                    devices = {"host"}
                if set(df['Device_id'].unique()) != devices:
                    self.res += 1
                    self.logger.error(f"The 'Device_id' value in {csv_name} is incomplete.")

    def check_db_headers(self, analysis_db):
        conn, curs = DBManager.create_connect_db(analysis_db)
        if not (conn and curs):
            self.res += 1
            self.logger.error("The analysis.db does not exist!!")
            return
        try:
            # 检查是否有表
            if not DBManager.judge_table_exists(curs, "StepTraceTime"):
                self.res += 1
                self.logger.error("The table 'StepTraceTime' is not exist in analysis.db")
                return
            # 检查Devive_id表头
            sql = f"PRAGMA table_info(StepTraceTime);"
            res = DBManager.fetch_all_data(curs, sql)
            table_set = {table[1] for table in res}  # 第一列有一个cid
            if "deviceId" not in table_set:
                self.res += 1
                self.logger.error("The header 'deviceId' is not exist in StepTraceTime")
                return
            # 检查Devive_id数据是否是0,1,2,3
            sql = f"SELECT DISTINCT deviceId FROM StepTraceTime"
            res = DBManager.fetch_all_data(curs, sql)
            device_ids = {item[0] for item in res}
            expected_devices = {0, 1, 2, 3}
            if device_ids != expected_devices:
                self.res += 1
                self.logger.error(f"The 'deviceId' value in StepTraceTime is incomplete.")
        except Exception as e:
            self.res += 1
            self.logger.error(f"The check of analysis.db failed, reason:{e}")
        finally:
            DBManager.destroy_db_connect(conn, curs)

    def check_json_labels(self, trace_view_json):
        expected_devices = {0, 1, 2, 3}
        device_ids = set()
        df = pd.read_json(trace_view_json, dtype=np.dtype)
        filtered_df = df[df['name'] == 'process_labels']
        for index, row in filtered_df.iterrows():
            args = row['args']
            label = args.get('labels')
            if label is None or label == "NPU":
                self.res += 1
                self.logger.error("The label should be 'CPU' or 'NPU {device_id}'.")
            elif label.startswith("NPU"):
                device_id = int(label.split(" ")[1])
                device_ids.add(device_id)
        if device_ids != expected_devices:
            self.res += 1
            self.logger.error(f"The 'Device_id' value in trace_view_json is incomplete.")


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(PytorchSingleProcessMultipleDevices("test_pytorch_single_process_multiple_devices", None, None,
                                                      None, None))
    unittest.TextTestRunner(verbosity=2).run(suite)
