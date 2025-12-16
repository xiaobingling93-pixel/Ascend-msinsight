import os
import glob
import pandas as pd
import test_base
import argparse
import unittest
import logging
import json
from check_tools.file_check import FileChecker

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')

class MsprofInfoCluster(test_base.TestProfiling):
    def getTestCmd(self, scene=None):
        script_path = self.cfg_path.msprof_info_cluster_path
        target_py_path = os.path.join(self.cfg_path.ascend_toolkit_path,
                            "latest/tools/profiler/profiler_tool/analysis/interface")
        self.msprofbin_cmd = "cd {}; python3 get_msprof_info.py -dir {} > {}".format(target_py_path,
                                                                         script_path, self.slog_stdout)

    def checkResDir(self, scend=None):
        target_json = "query/cluster_info.json"
        res_path = os.path.join(self.cfg_path.msprof_info_cluster_path, target_json)
        FileChecker.check_file_exists(res_path)
        check_keys = ["Rank Id", "Device Id", "Prof Dir", "Device Dir", "Models"]
        with open(res_path, 'r', encoding='utf-8') as jsonfile:
            data = json.load(jsonfile)
            for rank_data in data:
                for key in check_keys:
                    assert key in rank_data, f"Key '{key}' not found in JSON file."

class MsprofInfoNonCluster(test_base.TestProfiling):
    def getTestCmd(self, scene=None):
        script_path = self.cfg_path.msprof_info_cluster_path
        target_py_path = os.path.join(self.cfg_path.ascend_toolkit_path,
                            "latest/tools/profiler/profiler_tool/analysis/interface")
        self.msprofbin_cmd = "cd {}; python3 get_msprof_info.py -dir {} > {}".format(target_py_path,
                                                                         self.cfg_path.msprof_info_non_cluster_path,
                                                                         self.slog_stdout)

    def checkResDir(self, scend=None):
        FileChecker.check_file_exists(self.slog_stdout)
        FileChecker.check_json_nested_keys(self.slog_stdout,
                                    ["data.collection_info", "data.device_info", "data.host_info", "data.model_info", "data.version_info"])

if __name__ == '__main__':
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
    if args.scene == 'Cluster':
        suite.addTest(MsprofInfoCluster(args.id, args.scene, args.mode, args.params,
                        timeout=480 if not args.timeout else int(args.timeout)))
    else:
        suite.addTest(MsprofInfoNonCluster(args.id, args.scene, args.mode, args.params,
                        timeout=480 if not args.timeout else int(args.timeout)))
    unittest.TextTestRunner(verbosity=2).run(suite)