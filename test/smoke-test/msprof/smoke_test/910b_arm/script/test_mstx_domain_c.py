import os
import glob
import pandas as pd
import test_base
import argparse
import unittest
import logging
import re
from check_tools.file_check import FileChecker
import pandas as pd

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


class MsptiDomainProfiling(test_base.TestProfiling):
    def getTestCmd(self, scene=None):
        script_path = self.cfg_path.mspti_domain_c_path
        self.msprofbin_cmd += ("cd {}; source {}/set_env.sh; "
                               "bash run.sh {} > {} 2>&1").format(script_path,
                                                                 self.cfg_path.ascend_toolkit_path,
                                                                 self.res_dir, self.slog_stdout)

    def checkResDir(self, scend=None):
        target_mspti_log_path = glob.glob(f"{self.slog_stdout}")[0]
        FileChecker.check_file_exists(target_mspti_log_path)
        # 检查domain名称与预期名称
        pattern = r'timestamp:\s*(\d+),\s*markId:\s*(\d+),\s*name:\s*([^,]*),\s*domain:\s*([^\n,]*)'
        with open(target_mspti_log_path, 'r', encoding='utf-8') as txtfile:
            content = txtfile.read()
            matches = re.findall(pattern, content)
            assert (len(matches) == 2), (f"Expected markdata lenth is 2, "
                                         f"but find {len(matches)} data.")
            for match in matches:
                timestamp, markId, name, domain = match
                if len(name):
                    assert (domain.strip() == "range"), (f"Expected domain name is  range, "
                                                 f"but domain in file is {domain}.")

class PtaMsptiDomainProfiling(test_base.TestProfiling):
    def getTestCmd(self, scene=None):
        script_path = self.cfg_path.mspti_domain_pta_path
        self.msprofbin_cmd += ("cd {}; source {}/set_env.sh; "
                               "bash run.sh {} > {}").format(script_path,
                                                                 self.cfg_path.ascend_toolkit_path,
                                                                 self.res_dir, self.slog_stdout)

    def checkResDir(self, scend=None):
        target_mspti_log_paths = glob.glob(f"{self.res_dir}/mspti_output/*.csv")
        assert len(target_mspti_log_paths) == 4, f"expect 4 csv, but we have {len(target_mspti_log_paths)} device"

        expect_all_communication_count = 17324
        all_communication_count = 0
        for target_mspti_log_path in target_mspti_log_paths:
            df = pd.read_csv(target_mspti_log_path)
            communication_count = df[df['communication'].notna()].shape[0]
            all_communication_count += communication_count
        assert expect_all_communication_count == expect_all_communication_count, \
                (f"expect domain communication mark is {expect_all_communication_count}, "
                 f"but only {all_communication_count}")

class MsMsptiCkptProfiling(test_base.TestProfiling):
    def getTestCmd(self, scene=None):
        script_path = os.path.join(self.cfg_path.mspti_domain_ms_path, "checkpoint")
        self.msprofbin_cmd += ("cd {}; source {}/set_env.sh; "
                               "bash run.sh {} > {}").format(script_path,
                                                                 self.cfg_path.ascend_toolkit_path,
                                                                 self.res_dir, self.slog_stdout)

    def checkResDir(self, scend=None):
        target_mspti_log_paths = glob.glob(f"{self.res_dir}/mspti_output/*.csv")
        assert len(target_mspti_log_paths) == 9, f"expect 9 csv, but we have {len(target_mspti_log_paths)} devices"

        for target_mspti_log_path in target_mspti_log_paths:
            df = pd.read_csv(target_mspti_log_path)
            for index, row in df.iterrows():
                assert row[3] == 'save_checkpoint', f"{target_mspti_log_path}, 第 {index + 1} 行data msg不符合要求:"
                assert row[10] == 'default', f"{target_mspti_log_path}, 第 {index + 1} 行domain不符合要求:"


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(
        MsptiDomainProfiling("test_mstx_domain_cpp", "mspti_c", "", "", timeout=480))
    suite.addTest(
        PtaMsptiDomainProfiling("test_mstx_allreduce_pta", "mspti_python", "", "", timeout=480))
    suite.addTest(
        MsMsptiCkptProfiling("test_mstx_ckpt_mindspore", "mspti_python", "", "", timeout=480))
    unittest.TextTestRunner(verbosity=2).run(suite)
