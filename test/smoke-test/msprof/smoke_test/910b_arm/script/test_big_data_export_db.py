import csv
import glob
import os
import re

import pandas as pd

import test_base
import argparse
import unittest
import logging
import json
import sys
sys.path.append(os.path.abspath("../../../../../smoke_test"))
from check_tools.file_check import FileChecker
from _cfg import ConfigExportDbTableHeaders

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')

class BigDataExportDbProfiling(test_base.TestProfiling):
    def checkResDir(self, scene=None):
        output_paths = glob.glob(os.path.join(self.res_dir, "*_ascend_pt/ASCEND_PROFILER_OUTPUT"))[0]
        db_paths = glob.glob(os.path.join(output_paths, "ascend_pytorch_profiler_*.db"))[0]
        FileChecker.check_file_exists(db_paths)
        expect_tables = ["ACC_PMU","AICORE_FREQ","CANN_API","COMMUNICATION_OP","COMMUNICATION_TASK_INFO",
                         "COMPUTE_TASK_INFO","CONNECTION_IDS","ENUM_API_TYPE","ENUM_HCCL_DATA_TYPE",
                         "ENUM_HCCL_LINK_TYPE","ENUM_HCCL_RDMA_TYPE","ENUM_HCCL_TRANSPORT_TYPE","ENUM_MODULE",
                         "ENUM_MSTX_EVENT_TYPE","HBM","HOST_INFO","LLC","MEMORY_RECORD","META_DATA","NPU_INFO",
                         "NPU_MEM","NPU_MODULE_MEM","NPU_OP_MEM","OP_MEMORY","PYTORCH_API","RANK_DEVICE_MAP",
                         "SESSION_TIME_INFO","SOC_BANDWIDTH_LEVEL","STEP_TIME","STRING_IDS","TASK","TASK_PMU_INFO"]
        for table in expect_tables:
            FileChecker.check_db_table_exist(db_paths, table)
            FileChecker.check_db_table_struct(
                db_paths,
                table,
                ConfigExportDbTableHeaders.table_header.get(table, ConfigExportDbTableHeaders.table_header.get(table)))

    def changeExportType2Db(self):
        json_files = glob.glob(f'{self.res_dir}/*_ascend_pt/profiler_info*.json')
        # 遍历找到的文件
        for file_path in json_files:
            with open(file_path, 'r', encoding='utf-8') as file:
                data = json.load(file)
            data['config']['experimental_config']['export_type'] = 'db'
            with open(file_path, 'w', encoding='utf-8') as file:
                json.dump(data, file, indent=4)

    def exportDb(self):
        from torch_npu.profiler.profiler import analyse
        if __name__ == "__main__":
            analyse(profiler_path=f"{self.res_dir}")

    def getTestCmd(self, scene=None):
        mode_to_script = {"llama2": self.cfg_path.ModellinkLlama2}
        script_path = mode_to_script.get(self.mode, "")
        self.run_path = script_path
        # 拷贝profile_dir_db下数据，
        self.subprocess_cmd(f"cd {script_path}; cd ./profile_dir; cp -r *_ascend_pt {self.res_dir};")
        # 修改profiler_info_json下的expterimental_config配置，修改export_type为db
        self.changeExportType2Db()
        # 执行python离线解析
        self.exportDb()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-id', '--id', help='test case')
    parser.add_argument('-s', '--scene', required=True,
                        help='run App or Sys or All')
    parser.add_argument('-m', '--mode', required=True,
                        help='mode in slogConfig')
    parser.add_argument('-p', '--params',
                        help='params of runtime test script')
    parser.add_argument('-t', '--timeout',
                        help='timeout of test case')
    args = parser.parse_args()
    suite = unittest.TestSuite()
    suite.addTest(
        BigDataExportDbProfiling(args.id, args.scene, args.mode, args.params,
                           timeout=480 if not args.timeout else int(args.timeout)))
    unittest.TextTestRunner(verbosity=2).run(suite)
