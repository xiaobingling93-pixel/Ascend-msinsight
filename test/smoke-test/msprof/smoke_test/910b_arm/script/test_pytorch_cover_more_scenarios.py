#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
import sys
import re
import subprocess
import time
import unittest
import logging
from datetime import datetime

sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
from _cfg import ConfigValues
from check_tools.db_check import DBManager
from check_tools.db_check import EmptyClass

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


class PytorchCoverMoreScenarios(unittest.TestCase):
    """
    小而全的用例，看护统一db
    """
    OUTPUT_PATH = "/home/result_dir/test_pytorch_cover_more_scenarios"
    PLOG = f"{OUTPUT_PATH}/test_pytorch_cover_more_scenarios.log"
    DB_0_BASE = ("/home/msprof_smoke_test/model/Pytorch_cover_more_scenarios/test_pytorch_cover_more_scenarios"
                 "/PROF_000001_20241107013723570_GHQRFBMQIQMFPJFC/msprof_20250215160332.db")
    CMD = (f"cd /home/msprof_smoke_test/model/Pytorch_cover_more_scenarios;"
           f"source /root/miniconda3/bin/activate smoke_test_env_pta;"
           f"export ASCEND_GLOBAL_EVENT_ENABLE=1;"
           f"export ASCEND_SLOG_PRINT_TO_STDOUT=1;"
           f"source /usr/local/Ascend/ascend-toolkit/set_env.sh;"
           f"msprof --application=\"python3 demo.py\" --output={OUTPUT_PATH} > {PLOG};"
           f"msprof --export=on --type=db --output={OUTPUT_PATH}")
    AICORE_FREQ = "AICORE_FREQ"
    CANN_API = "CANN_API"
    COMMUNICATION_OP = "COMMUNICATION_OP"
    COMMUNICATION_TASK_INFO = "COMMUNICATION_TASK_INFO"
    COMPUTE_TASK_INFO = "COMPUTE_TASK_INFO"
    ENUM_API_TYPE = "ENUM_API_TYPE"
    ENUM_HCCL_DATA_TYPE = "ENUM_HCCL_DATA_TYPE"
    ENUM_HCCL_LINK_TYPE = "ENUM_HCCL_LINK_TYPE"
    ENUM_HCCL_RDMA_TYPE = "ENUM_HCCL_RDMA_TYPE"
    ENUM_HCCL_TRANSPORT_TYPE = "ENUM_HCCL_TRANSPORT_TYPE"
    ENUM_MSTX_EVENT_TYPE = "ENUM_MSTX_EVENT_TYPE"
    STRING_IDS = "STRING_IDS"
    TASK = "TASK"

    def __init__(self):
        super().__init__("execute")
        self.prof_path_0 = ""
        self.prof_path_1 = ""
        self.db_0 = ""
        self.db_1 = ""
        self.logger = logging
        self.id = "test_pytorch_cover_more_scenarios"  # 用例名字
        self.duration_time = 0  # 对比的时间
        self.res = 0  # 只要不为0，说明这个用例失败
        self.result = ""  # 根据self.res的结果，0的话是pass, 非0为fail，然后写到result.txt里面
        self.conn_0 = EmptyClass("empty conn")
        self.curs_0 = EmptyClass("empty curs")
        self.conn_1 = EmptyClass("empty conn")
        self.curs_1 = EmptyClass("empty curs")
        self.conn_0_base = EmptyClass("empty conn")
        self.curs_0_base = EmptyClass("empty curs")

    def init(self):
        self.logger.info("------------------------------- "
                         "Start execute case {} -------------------------------".format(self.id))
        self.get_db_path()
        if not self.db_0 or not self.db_1:
            return False
        self.conn_0, self.curs_0 = DBManager.create_connect_db(self.db_0)
        self.conn_1, self.curs_1 = DBManager.create_connect_db(self.db_1)
        self.conn_0_base, self.curs_0_base = DBManager.create_connect_db(self.DB_0_BASE)
        if self.conn_0 and self.curs_0 and self.conn_1 and self.curs_1 and self.conn_0_base and self.curs_0_base:
            return True
        return False

    def finalize(self):
        self.logger.info("\n------------------------------- "
                         "End execute case {} -------------------------------".format(self.id))
        DBManager.destroy_db_connect(self.conn_0, self.curs_0)
        DBManager.destroy_db_connect(self.conn_1, self.curs_1)
        DBManager.destroy_db_connect(self.conn_0_base, self.curs_0_base)

    def check_result(self):
        if self.res == 0:
            self.result = ConfigValues.pass_res
        else:
            self.result = ConfigValues.fail_res
        # 记录执行结果到result.txt
        self.write_res()
        self.assertTrue(ConfigValues.pass_res == self.result)

    def check_aicore_freq(self):
        # 校验表里面每一列的名字和类型
        expected_res = [
            (0, 'deviceId', 'INTEGER', 0, None, 0),
            (1, 'timestampNs', 'NUMERIC', 0, None, 0),
            (2, 'freq', 'INTEGER', 0, None, 0)
        ]
        sql = f"PRAGMA table_info({self.AICORE_FREQ});"
        res = DBManager.fetch_all_data(self.curs_0, sql)
        if res != expected_res:
            self.logger.error("AICORE_FREQ: The table structure has changed.")
            self.res += 1

    def check_cann_api(self):
        # 校验表里面每一列的名字和类型
        expected_res = [
            (0, 'startNs', 'INTEGER', 0, None, 0),
            (1, 'endNs', 'INTEGER', 0, None, 0),
            (2, 'type', 'INTEGER', 0, None, 0),
            (3, 'globalTid', 'INTEGER', 0, None, 0),
            (4, 'connectionId', 'INTEGER', 0, None, 1),
            (5, 'name', 'INTEGER', 0, None, 0),
        ]
        sql = f"PRAGMA table_info({self.CANN_API});"
        res = DBManager.fetch_all_data(self.curs_0, sql)
        if res != expected_res:
            self.logger.error("CANN_API: The table structure has changed.")
            self.res += 1

    def check_communication_op(self):
        # 校验表里面每一列的名字和类型
        expected_res = [
            (0, 'opName', 'INTEGER', 0, None, 0),
            (1, 'startNs', 'INTEGER', 0, None, 0),
            (2, 'endNs', 'INTEGER', 0, None, 0),
            (3, 'connectionId', 'INTEGER', 0, None, 0),
            (4, 'groupName', 'INTEGER', 0, None, 0),
            (5, 'opId', 'INTEGER', 0, None, 1),
            (6, 'relay', 'INTEGER', 0, None, 0),
            (7, 'retry', 'INTEGER', 0, None, 0),
            (8, 'dataType', 'INTEGER', 0, None, 0),
            (9, 'algType', 'INTEGER', 0, None, 0),
            (10, 'count', 'NUMERIC', 0, None, 0),
            (11, 'opType', 'INTEGER', 0, None, 0),
            (12, 'deviceId', 'INTEGER', 0, None, 0),
        ]
        sql = f"PRAGMA table_info({self.COMMUNICATION_OP});"
        res = DBManager.fetch_all_data(self.curs_0, sql)
        if res != expected_res:
            self.logger.error("AICORE_FREQ: The table structure has changed.")
            self.res += 1

    def check_communication_task_info(self):
        # 校验表里面每一列的名字和类型
        expected_res = [
            (0, 'name', 'INTEGER', 0, None, 0),
            (1, 'globalTaskId', 'INTEGER', 0, None, 0),
            (2, 'taskType', 'INTEGER', 0, None, 0),
            (3, 'planeId', 'INTEGER', 0, None, 0),
            (4, 'groupName', 'INTEGER', 0, None, 0),
            (5, 'notifyId', 'INTEGER', 0, None, 0),
            (6, 'rdmaType', 'INTEGER', 0, None, 0),
            (7, 'srcRank', 'INTEGER', 0, None, 0),
            (8, 'dstRank', 'INTEGER', 0, None, 0),
            (9, 'transportType', 'INTEGER', 0, None, 0),
            (10, 'size', 'INTEGER', 0, None, 0),
            (11, 'dataType', 'INTEGER', 0, None, 0),
            (12, 'linkType', 'INTEGER', 0, None, 0),
            (13, 'opId', 'INTEGER', 0, None, 0),
            (14, 'isMaster', 'INTEGER', 0, None, 0),
            (15, 'bandwidth', 'NUMERIC', 0, None, 0),
        ]
        sql = f"PRAGMA table_info({self.COMMUNICATION_TASK_INFO});"
        res = DBManager.fetch_all_data(self.curs_0, sql)
        if res != expected_res:
            self.logger.error("COMMUNICATION_TASK_INFO: The table structure has changed.")
            self.res += 1

    def check_compute_task_info(self):
        # 校验表头
        expected_res = [
            (0, 'name', 'INTEGER', 0, None, 0),
            (1, 'globalTaskId', 'INTEGER', 0, None, 1),
            (2, 'blockDim', 'INTEGER', 0, None, 0),
            (3, 'mixBlockDim', 'INTEGER', 0, None, 0),
            (4, 'taskType', 'INTEGER', 0, None, 0),
            (5, 'opType', 'INTEGER', 0, None, 0),
            (6, 'inputFormats', 'INTEGER', 0, None, 0),
            (7, 'inputDataTypes', 'INTEGER', 0, None, 0),
            (8, 'inputShapes', 'INTEGER', 0, None, 0),
            (9, 'outputFormats', 'INTEGER', 0, None, 0),
            (10, 'outputDataTypes', 'INTEGER', 0, None, 0),
            (11, 'outputShapes', 'INTEGER', 0, None, 0),
            (12, 'attrInfo', 'INTEGER', 0, None, 0),
            (13, 'opState', 'INTEGER', 0, None, 0),
            (14, 'hf32Eligible', 'INTEGER', 0, None, 0),
        ]
        sql = f"PRAGMA table_info({self.COMPUTE_TASK_INFO});"
        res = DBManager.fetch_all_data(self.curs_0, sql)
        if res != expected_res:
            self.logger.error("COMPUTE_TASK_INFO: The table structure has changed.")
            self.res += 1

    def check_api_type(self):
        sql = f"SELECT * FROM {self.ENUM_API_TYPE};"
        expected_res = DBManager.fetch_all_data(self.curs_0_base, sql)
        res = DBManager.fetch_all_data(self.curs_0, sql)
        if res != expected_res:
            self.logger.error(f"ENUM_API_TYPE: data error")
            self.res += 1

    def check_hccl_data_type(self):
        sql = f"SELECT * FROM {self.ENUM_HCCL_DATA_TYPE};"
        expected_res = DBManager.fetch_all_data(self.curs_0_base, sql)
        res = DBManager.fetch_all_data(self.curs_0, sql)
        if res != expected_res:
            self.logger.error(f"ENUM_HCCL_DATA_TYPE: data error")
            self.res += 1

    def check_hccl_link_type(self):
        sql = f"SELECT * FROM {self.ENUM_HCCL_LINK_TYPE};"
        expected_res = DBManager.fetch_all_data(self.curs_0_base, sql)
        res = DBManager.fetch_all_data(self.curs_0, sql)
        if res != expected_res:
            self.logger.error(f"ENUM_HCCL_LINK_TYPE: data error")
            self.res += 1

    def check_hccl_rdma_type(self):
        sql = f"SELECT * FROM {self.ENUM_HCCL_RDMA_TYPE};"
        expected_res = DBManager.fetch_all_data(self.curs_0_base, sql)
        res = DBManager.fetch_all_data(self.curs_0, sql)
        if res != expected_res:
            self.logger.error(f"ENUM_HCCL_RDMA_TYPE: data error")
            self.res += 1

    def check_hccl_transport_type(self):
        sql = f"SELECT * FROM {self.ENUM_HCCL_TRANSPORT_TYPE};"
        expected_res = DBManager.fetch_all_data(self.curs_0_base, sql)
        res = DBManager.fetch_all_data(self.curs_0, sql)
        if res != expected_res:
            self.logger.error(f"ENUM_HCCL_TRANSPORT_TYPE: data error")
            self.res += 1

    def check_enum_module(self):
        sql = f"SELECT * FROM {self.ENUM_HCCL_TRANSPORT_TYPE};"
        expected_res = DBManager.fetch_all_data(self.curs_0_base, sql)
        res = DBManager.fetch_all_data(self.curs_0, sql)
        if res != expected_res:
            self.logger.error(f"ENUM_HCCL_TRANSPORT_TYPE: data error")
            self.res += 1

    def check_enum_mstx_event_type(self):
        sql = f"SELECT * FROM {self.ENUM_MSTX_EVENT_TYPE};"
        expected_res = DBManager.fetch_all_data(self.curs_0_base, sql)
        res = DBManager.fetch_all_data(self.curs_0, sql)
        if res != expected_res:
            self.logger.error(f"ENUM_MSTX_EVENT_TYPE: data error")
            self.res += 1

    def check_task(self):
        # 校验表头
        expected_res = [
            (0, 'startNs', 'INTEGER', 0, None, 0),
            (1, 'endNs', 'INTEGER', 0, None, 0),
            (2, 'deviceId', 'INTEGER', 0, None, 0),
            (3, 'connectionId', 'INTEGER', 0, None, 0),
            (4, 'globalTaskId', 'INTEGER', 0, None, 0),
            (5, 'globalPid', 'INTEGER', 0, None, 0),
            (6, 'taskType', 'INTEGER', 0, None, 0),
            (7, 'contextId', 'INTEGER', 0, None, 0),
            (8, 'streamId', 'INTEGER', 0, None, 0),
            (9, 'taskId', 'INTEGER', 0, None, 0),
            (10, 'modelId', 'INTEGER', 0, None, 0),
        ]
        sql = f"PRAGMA table_info({self.TASK});"
        res = DBManager.fetch_all_data(self.curs_0, sql)
        if res != expected_res:
            self.logger.error("TASK: The table structure has changed.")
            self.res += 1

    def check_tables_in_db(self):
        # 检查统一db里面的表是否符合预期
        expected_table = {
            'AICORE_FREQ', 'CANN_API', 'COMMUNICATION_OP', 'COMMUNICATION_SCHEDULE_TASK_INFO',
            'COMMUNICATION_TASK_INFO', 'COMPUTE_TASK_INFO',
            'ENUM_API_TYPE', 'ENUM_HCCL_DATA_TYPE', 'ENUM_HCCL_LINK_TYPE', 'ENUM_HCCL_RDMA_TYPE',
            'ENUM_HCCL_TRANSPORT_TYPE', 'ENUM_MODULE', 'ENUM_MSTX_EVENT_TYPE', 'HOST_INFO', 'META_DATA', 'NPU_INFO',
            'SESSION_TIME_INFO', 'STRING_IDS', 'TASK', 'TASK_PMU_INFO', 'ENUM_MEMCPY_OPERATION', 'RANK_DEVICE_MAP'
        }
        sql = "SELECT name FROM sqlite_master WHERE type='table';"
        res = DBManager.fetch_all_data(self.curs_0, sql)
        table_set = {table[0] for table in res}
        if expected_table != table_set:
            self.res += 1
            self.logger.error("统一db和预期不一致")
            self.logger.error(f"预期：{sorted(list(expected_table))}")
            self.logger.error(f"实际：{sorted(list(table_set))}")
            return False
        return True

    def check_msprof_db(self):
        if not self.conn_0 or not self.curs_0:
            self.res += 1
            return
        if not self.conn_0_base or not self.curs_0_base:
            self.logger.error("base db not exists, check stop")
            self.res += 1
            return
        if not self.check_tables_in_db():
            return
        self.check_aicore_freq()
        self.check_cann_api()
        self.check_communication_op()
        self.check_communication_task_info()
        self.check_compute_task_info()
        self.check_api_type()
        self.check_hccl_data_type()
        self.check_hccl_link_type()
        self.check_hccl_rdma_type()
        self.check_hccl_transport_type()
        self.check_enum_module()
        self.check_enum_mstx_event_type()
        self.check_task()

    def write_res(self):
        with open('result.txt', 'a+') as f:
            f.write('%s %s %s\n' % (self.id, self.result, self.duration_time))

    def subprocess_cmd(self, cmd):
        self.logger.info("host command: {}".format(cmd))
        try:
            result = subprocess.run(['bash', '-c', cmd], capture_output=True, text=True)
            if result.returncode != 0 and len(result.stderr) != 0:
                self.logger.error(result.stderr)
                self.res += 1
                return result.stderr
        except (Exception, TimeoutError) as err:
            self.logger.error(err)
            self.res += 1
            return err
        finally:
            pass
        return result.stdout

    def view_error_msg(self, log_path, log_type=""):
        self.logger.info("start view {} log ...".format(log_type))
        if log_type == "plog":
            cmd = r"grep -rn 'ERROR\] PROFILING' {0}; grep -rn '\[ERROR\] \[MSVP\]' {0}; " \
                  r"grep -rn 'ERROR\] Failed' {0}".format(log_path)
        else:
            cmd = r"grep -rn 'ERROR\]' {0}".format(log_path)
        res = self.subprocess_cmd(cmd)
        if re.search(r"ERROR", res):
            self.logger.error(res)
            self.res += 1

    def view_log(self):
        log0 = os.path.join(self.prof_path_0, "mindstudio_profiler_log")
        log1 = os.path.join(self.prof_path_1, "mindstudio_profiler_log")
        self.view_error_msg(self.PLOG, "plog")
        self.view_error_msg(log0)
        self.view_error_msg(log1)

    def get_db_path(self):
        for file_name in os.listdir(self.OUTPUT_PATH):
            prof_path = os.path.join(self.OUTPUT_PATH, file_name)
            device_0_path = os.path.join(prof_path, "device_0")
            device_1_path = os.path.join(prof_path, "device_1")
            if os.path.exists(device_0_path):
                self.prof_path_0 = prof_path
                self.db_0 = self.get_db(prof_path)
            elif os.path.exists(device_1_path):
                self.prof_path_1 = prof_path
                self.db_1 = self.get_db(prof_path)

    def execute(self):
        self.start_time = time.time_ns()
        start_time = datetime.now()
        self.subprocess_cmd(self.CMD)
        try:
            if self.init():
                self.check_msprof_db()
            else:
                self.logger.error("Failed to connect the dbs.")
                self.res += 1
        except Exception as err:
            self.logger.error(f"Execute {self.id} failed: {err}")
            self.res += 1
        # 执行结果校验
        self.view_log()
        time_diff = datetime.now() - start_time
        self.duration_time = time_diff.total_seconds()
        self.check_result()
        self.finalize()

    @staticmethod
    def get_db(path):
        for file in os.listdir(path):
            if file.startswith("msprof") and file.endswith(".db"):
                return os.path.join(path, file)
        return ""


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(PytorchCoverMoreScenarios())
    unittest.TextTestRunner(verbosity=2).run(suite)
