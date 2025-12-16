import glob
import os

import test_base
import argparse
import unittest
import logging
from check_tools.file_check import FileChecker
from _cfg import ConfigTableHeaders
from _cfg import ConfigExportDbTableHeaders
from check_tools.db_check import DBManager
import pandas as pd

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')

class AnalysisTxt(test_base.TestProfiling):

    def check_output_file_header(self, output_dir):
        logging.info("check_output_file_header")
        normal_expect_files = ['api_statistic', 'task_time', 'communication_statistic']
        for expect_file in normal_expect_files:
            expect_statistic_headers = ConfigTableHeaders.table_header.get(expect_file, '').split(',')
            FileChecker.check_csv_headers(glob.glob(os.path.join(output_dir, f'{expect_file}_*.csv'))[0],
                                          expect_statistic_headers)

        # op_statistic 和 op_summary需要单独检查
        expect_op_summary_headers = ConfigTableHeaders.table_header['op_summary']['task-based'][
            'PipeUtilization'].split(',')
        expect_op_summary_headers.append('Device_id')
        FileChecker.check_csv_headers(glob.glob(os.path.join(output_dir, 'op_summary_*.csv'))[0],
                                      expect_op_summary_headers)

        expect_op_statistic_headers = ConfigTableHeaders.table_header['op_statistic']['AllData'].split(',')
        FileChecker.check_csv_headers(glob.glob(os.path.join(output_dir, 'op_statistic_*.csv'))[0],
                                      expect_op_statistic_headers)

    def check_output_file_exist(self, output_dir):
        logging.info("check_output_file_exist")
        expect_files = ['op_statistic', 'api_statistic', 'op_summary', 'task_time', 'communication_statistic']
        # 需要校验内容有msprof.json, communication_statistic, api_statistic., op_summary, task_time
        for expect_file in expect_files:
            FileChecker.check_txt_not_empty(glob.glob(os.path.join(output_dir, expect_file.join('*')))[0])

    def check_output_db_header(self, target_db_path):
        logging.info("check_output_db_header")
        expect_tables = ['AICORE_FREQ' 'CANN_API', 'COMMUNICATION_OP', 'COMMUNICATION_SCHEDULE_TASK_INFO',
                         'COMMUNICATION_TASK_INFO', 'COMPUTE_TASK_INFO', 'ENUM_API_TYPE', 'ENUM_HCCL_DATA_TYPE',
                         'ENUM_HCCL_LINK_TYPE', 'ENUM_HCCL_RDMA_TYPE', 'ENUM_HCCL_TRANSPORT_TYPE',
                         'ENUM_MEMCPY_OPERATION', 'ENUM_MODULE', 'ENUM_MSTX_EVENT_TYPE', 'HOST_INFO', 'META_DATA',
                         'NPU_INFO', 'SESSION_TIME_INFO', 'STRING_IDS', 'TASK', 'TASK_PMU_INFO', 'RANK_DEVICE_MAP']
        for table in expect_tables:
            FileChecker.check_db_table_struct(
                target_db_path, table, ConfigExportDbTableHeaders.table_header.get(table, []))

    def check_db_exist(self, target_db_path):
        logging.info("check_db_exist")
        FileChecker.check_file_exists(target_db_path)

    def compare_cann_api(self, output_dir, target_db_path):
        logging.info("compare_cann_api")
        api_statistic_path = glob.glob(os.path.join(output_dir, 'api_statistic_*.csv'))[0]
        FileChecker.compare_csv_num_with_table(api_statistic_path, target_db_path, 'CANN_API')

    def check_db_result_same_with_text(self, output_dir, target_db_path):
        logging.info("check_db_result_same_with_text")
        self.compare_cann_api(output_dir, target_db_path)

    def checkResDir(self, scene=None):
        target_db_path = glob.glob(os.path.join(self.res_dir, 'PROF_data/msprof_*.db'))[0]
        output_dir = os.path.join(self.res_dir, 'PROF_data/mindstudio_profiler_output')
        self.check_output_file_exist(output_dir)
        self.check_output_file_header(output_dir)
        self.check_db_exist(target_db_path)
        self.check_output_db_header(target_db_path)
        self.check_db_result_same_with_text(output_dir, target_db_path)

    def getTestCmd(self, scene=None):
        pass
        scene = self.scene if scene is None else scene
        hccl_sence_to_script = {"analysis_A3": self.cfg_path.analysis_a3_path}
        script_path = hccl_sence_to_script.get(scene, "")
        self.run_path = script_path
        self.msprofbin_cmd = "cd {}; {}; bash run_analysis.sh {} > {}".format(script_path,
                                                                             self.cfg_path.toolkit_env_path,
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
        AnalysisTxt("test_analysis_A3_text", args.scene, args.mode, args.params,
                    timeout=480 if not args.timeout else int(args.timeout)))
    unittest.TextTestRunner(verbosity=2).run(suite)
