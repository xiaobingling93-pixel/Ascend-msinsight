import os
import glob
import pandas as pd
import test_base
import argparse
import unittest
import logging
import re
from collections import defaultdict

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


class MsptiCommunication(test_base.TestProfiling):
    def getTestCmd(self, scene=None):
        script_path = self.cfg_path.mspti_communication_path
        self.msprofbin_cmd += ("cd {}; source {}/set_env.sh; "
                               "source /root/miniconda3/bin/activate /root/miniconda3/envs/smoke_test_env_pta;"
                               "bash run.sh > {} 2>&1").format(script_path,
                                                               self.cfg_path.ascend_toolkit_path,
                                                               self.slog_stdout)

    def getCommActivity(self, content):
        pattern = r"\[Communication\](.*?)correlationId:\s*\d+"
        matches = re.finditer(r"\[Communication\]\s*(.*?correlationId:\s*\d+)", content, re.DOTALL)
        results = []
        for match in matches:
            data_str = match.group(1)
            kv_pairs = re.findall(r"(\w+):\s*([^,]+)", data_str)
            result = {k: v.strip() for k, v in kv_pairs}
            results.append(result)
        return results

    def getApiActivity(self, content):
        pattern = r"\[Api\](.*?)correlationId:\s*\d+"
        matches = re.finditer(r"\[Api\]\s*(.*?correlationId:\s*\d+)", content, re.DOTALL)
        results = []
        for match in matches:
            data_str = match.group(1)
            kv_pairs = re.findall(r"(\w+):\s*([^,]+)", data_str)
            result = {k: v.strip() for k, v in kv_pairs}
            results.append(result)
        return results

    def check_comm_data(self, comm_datas):
        logging.info("check_comm_data start")
        assert len(comm_datas) == 20, "少了通信算子"
        for comm_data in comm_datas:
            assert comm_data["name"] == "hcom_allReduce_", "名字不对"
            assert comm_data["start"] <= comm_data["end"], "时间不对"
            assert int(comm_data["kind"]) == 9, f"communication kind must be 9, but is {comm_data['kind']}"

    def check_api_data(self, api_datas):
        logging.info("check_api_data start")
        for api_data in api_datas:
            assert api_data["start"] <= api_data["end"], "时间不对"
            assert int(api_data["kind"]) == 3, f"api kind must be 3, but is {api_data['kind']}"

    def check_correlation(self, comm_datas, api_datas):
        logging.info("check_correlation start")
        api_group = defaultdict(list)

        # 把所有 Api 分组（按 correlationId）
        for r in api_datas:
            api_group[r['correlationId']].append(r)

        for comm in comm_datas:
            corr_id = comm.get('correlationId')
            comm_name = comm.get('name', '').strip()
            api_candidates = api_group.get(corr_id, [])

            matched = any(api.get('name', '').strip() == comm_name for api in api_candidates)
            assert matched, f'Mismatch: Communication="{comm_name}, corr_id={corr_id}"'

    def checkResDir(self, scend=None):
        comm_data = []
        api_data = []
        with open(self.slog_stdout, 'r', encoding='utf-8') as txtfile:
            content = txtfile.read()
            comm_data = self.getCommActivity(content)
            api_data = self.getApiActivity(content)
        self.check_comm_data(comm_data)
        self.check_api_data(api_data)
        self.check_correlation(comm_data, api_data)


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(
        MsptiCommunication("test_mspti_communication", "mspti_c", "", "", timeout=480))
    unittest.TextTestRunner(verbosity=2).run(suite)
