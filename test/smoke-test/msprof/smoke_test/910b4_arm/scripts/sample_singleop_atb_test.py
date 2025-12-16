import os
import re

import pandas
import csv
import test_base
import argparse
import unittest
import logging
import sys
sys.path.append(os.path.abspath("../../../../../smoke_test"))
from check_tools.file_check import FileChecker
logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


class SampleMc2Profiling(test_base.TestProfiling):
    def get_file_name(self, file_name: str) -> str:
        """
        get filemane like "op_summary"
        """
        match = re.search(r'(_\d)?(_slice_\d+)?_\d+', file_name)
        if match and match.start() > 0:
            return file_name[:match.start()]
        logging.warning("The file name  %s is invalid!", file_name)
        return "invalid"

    def check_output_file(self, output_dir):
        target_files = {'op_summary': False, 'task_time': False, 'api_statistic': False, 'op_statistic': False}
        output_dir = os.path.join(self.res_dir, output_dir)
        for file in os.listdir(output_dir):
            flag = False
            for file_name, _ in target_files.items():
                if file_name in file:
                    target_files[file_name] = True

        for file_name, exist in target_files.items():
            if not exist:
                self.logger.error("%s is not exist", file_name)
                self.res += 1

        for index, file_csv in enumerate(sorted(os.listdir(output_dir))):
            if not file_csv.endswith(".csv"):
                continue
            if "op_summary" in file_csv:
                def view_summary_header():
                    expect_header = ["Device_id", "Model ID", "Task ID", "Stream ID", "Op Name", "OP Type", "OP State",
                    "Task Type",
                    "Task Start Time(us)", "Task Duration(us)", "Task Wait Time(us)", "Block Dim", "Mix Block Dim",
                    "HF32 Eligible", "Input Shapes", "Input Data Types", "Input Formats", "Output Shapes",
                    "Output Data Types", "Output Formats", "Context ID", "aicore_time(us)", "aic_total_cycles",
                    "aic_mac_time(us)", "aic_mac_ratio", "aic_scalar_time(us)", "aic_scalar_ratio",
                    "aic_mte1_time(us)", "aic_mte1_ratio", "aic_mte2_time(us)", "aic_mte2_ratio",
                    "aic_fixpipe_time(us)", "aic_fixpipe_ratio", "aic_icache_miss_rate", "aiv_time(us)",
                    "aiv_total_cycles", "aiv_vec_time(us)", "aiv_vec_ratio", "aiv_scalar_time(us)", "aiv_scalar_ratio",
                    "aiv_mte2_time(us)", "aiv_mte2_ratio", "aiv_mte3_time(us)", "aiv_mte3_ratio",
                    "aiv_icache_miss_rate", "cube_utilization(%)"]
                    FileChecker.check_csv_headers(os.path.join(output_dir, file_csv), expect_header)

                def view_summary_content():
                    # CSV文件路径
                    csv_file_path = os.path.join(output_dir, file_csv)
                    # 要检查的字段和值
                    field_to_check = 'Op Name'
                    required_value = 'KVCacheNdKernel'
                    with open(csv_file_path, mode='r', newline='', encoding='utf-8') as file:
                        reader = csv.DictReader(file)
                        for row in reader:
                            if row[field_to_check] == required_value:
                                logging.info(f'Found required value "{required_value}" in field "{field_to_check}".')
                                return True
                    logging.error(f'No row with value "{required_value}" found in field "{field_to_check}".')
                    return False

                view_summary_header()
                view_summary_content()

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
                    self.check_output_file(os.path.join(prof_, dir_))

    def getTestCmd(self, scene=None):
        scene = self.scene if scene is None else scene
        atb_sence_to_script = {"singleop_atb": self.cfg_path.AtbSingleOp}
        script_path = atb_sence_to_script.get(scene, "")
        self.run_path = script_path
        self.msprofbin_cmd = "cd {}; {}; bash run_train.sh {} > {}".format(script_path, self.cfg_path.toolkit_env_path,
                                                                           self.res_dir, self.slog_stdout)


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
        SampleMc2Profiling(args.id, args.scene, args.mode, args.params,
                            timeout=480 if not args.timeout else int(args.timeout)))
    unittest.TextTestRunner(verbosity=2).run(suite)
