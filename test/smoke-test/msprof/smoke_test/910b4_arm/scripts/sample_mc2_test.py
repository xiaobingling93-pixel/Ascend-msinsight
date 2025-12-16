import glob
import os
import json
import test_base
import argparse
import unittest
import sys
sys.path.append(os.path.abspath("../../../../../smoke_test"))
from check_tools.file_check import FileChecker


class SampleMc2Profiling(test_base.TestProfiling):

    def checkPtaProfDir(self, pt_prof: str):
        target_files = ['kernel_details.csv', 'npu_module_mem.csv', 'op_statistic.csv', 'trace_view.json',
                        'operator_memory.csv']
        output_path = os.path.join(pt_prof, "ASCEND_PROFILER_OUTPUT")
        for target_file in target_files:
            file_path = os.path.join(output_path, target_file)
            FileChecker.check_file_exists(file_path)
            FileChecker.check_txt_not_empty(file_path)

    def checkMetaData(self, pt_prof: str):
        target_groups = ["default_group", "tp", "pp"]
        metadata = {}
        metadata_path = os.path.join(pt_prof, "profiler_metadata.json")
        with open(metadata_path) as fs:
            metadata = json.load(fs)
        assert "distributed_args" in metadata
        assert "parallel_group_info" in metadata
        all_group_names = [group_info['group_name'] for group_info in metadata["parallel_group_info"].values()]
        for group_name in target_groups:
            assert group_name in all_group_names

    def checkMsprofDir(self, pt_prof: str):
        prof_path = glob.glob(f"{pt_prof}/PROF_*")[0]
        output_path = os.path.join(prof_path, "mindstudio_profiler_output")
        target_files = ['api_statistic_*.csv', 'communication_statistic_*.csv', 'op_statistic_*.csv', 'op_summary_*.csv',
                        'msprof_*.json']
        for target_file in target_files:
            file_path = glob.glob(os.path.join(output_path, target_file))
            if len(file_path) == 0:
                self.logger.error("target file %s is not existing", target_file)
                continue
            FileChecker.check_file_exists(file_path[0])
            FileChecker.check_txt_not_empty(file_path[0])

    def checkMc2Operator(self):
        target_device_path = glob.glob(f"{self.res_dir}/*_ascend_pt/PROF_*/device_0")[0]
        prof_path = os.path.dirname(target_device_path)
        op_summary_path = glob.glob(os.path.join(prof_path, "mindstudio_profiler_output/op_summary_*.csv"))[0]
        target_csv_item = {"Op Name" : ["MatmulReduceScatterMc2*", "AllGatherMatmulMc2*"]}
        FileChecker.check_csv_items(op_summary_path, target_csv_item)

    def checkResDir(self, scene=None):
        pt_prof_lst = glob.glob(f"{self.res_dir}/*_ascend_pt")
        for pt_prof in pt_prof_lst:
            self.checkPtaProfDir(pt_prof)
            self.checkMsprofDir(pt_prof)
            self.checkMetaData(pt_prof)
        self.checkMc2Operator()

    def getTestCmd(self, scene=None):
        scene = self.scene if scene is None else scene
        mc2_sence_to_script = {"sample_mc2": self.cfg_path.MindSpeedLLMGPTMc2}
        script_path = mc2_sence_to_script.get(scene, "")
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
