import os
import re

import pandas

import test_base
import argparse
import unittest
import logging

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


class SampleHcclProfiling(test_base.TestProfiling):

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
        target_files = {'op_summary': False, 'task_time': False, 'api_statistic': False, 'communication_statistic': False}
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

        for index, csv in enumerate(sorted(os.listdir(output_dir))):
            if not csv.endswith(".csv"):
                continue
            if "op_summary" in csv:
                def view_summary_content():
                    csv_name = self.get_file_name(csv)
                    cmd = "cat {} | head -2".format(os.path.join(output_dir, csv))
                    res_content = self.subprocess_cmd(cmd)
                    header = res_content.split("\n")[0]
                    cmd = "cat {} | wc -l".format(os.path.join(output_dir, csv))
                    self.op_summary_op_num = int(self.subprocess_cmd(cmd)) - 1
                    except_res = ""
                    if header != except_res:
                        self.logger.error('current header: {}'.format(header))
                        self.logger.info('expect header: {}'.format(except_res))
                        self.res += 1

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
        hccl_sence_to_script = {"sampleSingleOpHcclFftsffOff": self.cfg_path.sampleSingleOpHcclFftsffOff_path,
                                "sampleSingleOpHcclFftsffOn": self.cfg_path.sampleSingleOpHcclFftsffOn_path,
                                "sampleHcclFftsOffGraph": self.cfg_path.sampleGraphHcclFftsOff_path,
                                "sampleHcclFftsOnGraph": self.cfg_path.sampleGraphHcclFftsOn_path}
        script_path = hccl_sence_to_script.get(scene, "")
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
        SampleHcclProfiling(args.id, args.scene, args.mode, args.params,
                            timeout=480 if not args.timeout else int(args.timeout)))
    unittest.TextTestRunner(verbosity=2).run(suite)
