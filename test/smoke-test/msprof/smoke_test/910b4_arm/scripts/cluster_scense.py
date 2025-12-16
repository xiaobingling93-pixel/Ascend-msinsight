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

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


class SampleMc2Profiling(test_base.TestProfiling):

    def checkPtaFileExist(self, pt_prof: str):
        # 文件存在校验
        target_files = ['communication_matrix.json', 'step_trace_time.csv', 'trace_view.json',
                        'api_statistic.csv', 'kernel_details.csv', 'operator_details.csv', 'op_statistic.csv']
        output_path = os.path.join(pt_prof, "ASCEND_PROFILER_OUTPUT")
        for target_file in target_files:
            file_path = os.path.join(output_path, target_file)
            FileChecker.check_file_exists(file_path)
            FileChecker.check_txt_not_empty(file_path)

    def checkProfInfoJson(self):
        # 预期上prof_info_json应该有8个
        expect_rank = [0, 1, 2, 3, 4, 5, 6, 7]
        prof_info_json_path = glob.glob(f"{self.res_dir}/*_ascend_pt/profiler_info_*.json")
        current_rank = []
        for file_path in prof_info_json_path:
            match = re.search(r"profiler_info_(\d+).json", file_path)
            if match:
                # 将提取的数字添加到列表中
                current_rank.append(int(match.group(1)))
        if set(current_rank) != set(expect_rank):
            logging.error(f"prof rank is {current_rank}, different from {expect_rank}")
            self.res += 1

    def checkPtaProfDir(self):
        # prof_info_{}_json中rank_id存在,且与预期相同
        self.checkProfInfoJson()
        # 校验其中一个hccl算子数量大致相等
        self.checkCommunication()
        pt_prof_lst = glob.glob(f"{self.res_dir}/*_ascend_pt")
        for pt_prof in pt_prof_lst:
            self.checkPtaFileExist(pt_prof)

    def checkMsprofDir(self):
        prof_path_lst = glob.glob(f"{self.res_dir}/*_ascend_pt/PROF_*")
        for prof_path in prof_path_lst:
            output_path = os.path.join(prof_path, "mindstudio_profiler_output")
            target_files = ['api_statistic_*.csv', 'hccl_statistic_*.csv', 'op_statistic_*.csv', 'op_summary_*.csv',
                            'msprof_*.json']
            for target_file in target_files:
                file_path = glob.glob(os.path.join(output_path, target_file))
                if len(file_path) == 0:
                    self.logger.error("target file %s is not existing", target_file)
                    continue
                FileChecker.check_file_exists(file_path[0])
                FileChecker.check_txt_not_empty(file_path[0])

    def checkCommunication(self):
        target_device_path = glob.glob(f"{self.res_dir}/*_ascend_pt/PROF_*/device_2")[0]
        pt_prof_path = os.path.dirname(os.path.dirname(target_device_path))
        target_communication_path = os.path.join(pt_prof_path, "ASCEND_PROFILER_OUTPUT/communication.json")
        baseline_path = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                                     "baseline/communication.json")

        baseline_dict = self.countHcclOp(baseline_path)
        current_dict = self.countHcclOp(target_communication_path)
        if baseline_dict != current_dict:
            logging.error(f"current hcclop is {current_dict}, different from baseline: {baseline_dict}")
            self.res += 1

    def countHcclOp(self, baseline_path: str):
        target_hccl_op_count = {"hcom_broadcast_*": 0, "hcom_allReduce_*": 0, "hcom_reduceScatter_*": 0,
                                "hcom_allGather_*": 0,
                                "hcom_batchSendRecv_*": 0}
        # 根据target_hccl_op的名称获取baseline中的所有op算子数据
        with open(baseline_path, 'r', encoding='utf-8') as baseline_data:
            data = json.load(baseline_data)
            collect_op_info = data['step4']['collective']
            p2p_op_info = data['step4']['p2p']
            if isinstance(collect_op_info, dict) and isinstance(p2p_op_info, dict):
                for op_name in collect_op_info.keys():
                    for key, value in target_hccl_op_count.items():
                        value += 1 if re.compile(re.escape(key), re.IGNORECASE).search(op_name) is None else 0

                for op_name in p2p_op_info.keys():
                    for key, value in target_hccl_op_count.items():
                        value += 1 if re.compile(re.escape(key), re.IGNORECASE).search(op_name) is None else 0
        return target_hccl_op_count

    def checkClusterFileExist(self):
        output_path = glob.glob(f"{self.res_dir}/cluster_analysis_output")[0]
        # 文件存在校验
        target_files = ['cluster_communication.json', 'cluster_communication_matrix.json',
                        'cluster_step_trace_time.csv',
                        'communication_group.json']
        for target_file in target_files:
            file_path = os.path.join(output_path, target_file)
            FileChecker.check_file_exists(file_path)
            FileChecker.check_txt_not_empty(file_path)

    def checkClusterFileContent(self):
        output_path = glob.glob(f"{self.res_dir}/cluster_analysis_output")[0]
        # 内容校验, 校验表头
        expect_header = ["Step", "Type", "Index", "Computing", "Communication(Not Overlapped)", "Overlapped",
                         "Communication", "Free", "Stage", "Bubble", "Communication(Not Overlapped and Exclude "
                                                                     "Receive)", "Preparing", "DP Index", "PP Index",
                         "TP Index"]
        cluster_step_trace_path = os.path.join(output_path, "cluster_step_trace_time.csv")
        FileChecker.check_csv_headers(cluster_step_trace_path, expect_header)

        # 计算校验值准确性, 应该有Stage - Computing - NotOverlapped = Free (这个值会有误差, 在3以内都算没问题)
        df = pd.read_csv(cluster_step_trace_path)
        matched_df = df[df['Index'] == 'rank']
        free = matched_df['Free']
        # 计算 Stage - Computing - NotOverlapped
        stage_computing_notoverlapped = (
                matched_df['Stage'] - matched_df['Computing'] - matched_df['Communication(Not Overlapped)'])
        difference = (stage_computing_notoverlapped - free).abs()
        is_within_tolerance = difference <= 3
        out_of_tolerance_rows = matched_df[~is_within_tolerance]
        if not out_of_tolerance_rows.empty:
            logging.error(f"target row : {out_of_tolerance_rows} has error")
            self.res += 1

    def checkMsprofCluster(self):
        self.checkClusterFileExist()
        self.checkClusterFileContent()

    def checkResDir(self, scene=None):
        self.checkPtaProfDir()
        self.checkMsprofDir()
        self.checkMsprofCluster()

    def getTestCmd(self, scene=None):
        scene = self.scene if scene is None else scene
        sence_to_script = {"llama2": self.cfg_path.ModellinkLlama2}
        script_path = sence_to_script.get(scene, "")
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
