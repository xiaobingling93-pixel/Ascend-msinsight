#!/usr/bin/python3
# -*- coding: utf-8 -*-

import unittest
import logging
import shutil
import subprocess
import re
from _datetime import datetime
from abc import ABCMeta, abstractmethod

from _cfg import *

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


class MsptiBaseCase(unittest.TestCase):
    def __init__(self, func: str):
        super(MsptiBaseCase, self).__init__(func)
        self.mspti_base_dir = ConfigPaths().mspti_base_testcase_path
        self.toolkit_setenv = ConfigPaths().toolkit_env_path
        self.ascend_home_path = ConfigPaths().latest_path
        self.log_env = "export ASCEND_GLOBAL_LOG_LEVEL=3; export ASCEND_SLOG_PRINT_TO_STDOUT=1"
        self.res = 0
        self.logger = logging
        self.res_dir = ConfigPaths().result_path
        self.cfg_value = ConfigValues()
        self.plog_path = ""
        self.duration_time = 0
        self.exec_cmd = ""

    def subprocess_cmd(self, cmd):
        self.logger.info("host command: {}".format(cmd))
        try:
            result = subprocess.run(['bash', '-c', cmd], capture_output=True, text=True)
            if result.returncode != 0 and len(result.stderr) != 0:
                self.logger.error(result.stderr)
                self.res += 1
        except (Exception, TimeoutError) as err:
            self.logger.error(err)
            self.res += 1
            return (result.returncode, err)
        finally:
            pass
        return (result.returncode, result.stdout)

    def executeCmd(self):
        start_time = datetime.now()
        status, exe_res = self.subprocess_cmd(self.exec_cmd)
        time_diff = datetime.now() - start_time
        self.duration_time = time_diff.total_seconds()
        self.logger.info("\n------------------------------- "
                           "End execute case {} -------------------------------".format(self.id))
        return status

    def view_plog_error(self, log_path: str):
        self.logger.info("start view {} log ...".format(log_path))
        cmd = r"grep -rn 'ERROR\] PROFILING' {0};".format(log_path)
        status, res = self.subprocess_cmd(cmd)
        if re.search(r"ERROR", res):
            self.logger.error(res)
            self.res += 1

    def write_res(self, id: str, res: str):
        with open('result.txt', 'a+') as f:
            f.write('%s %s %s\n' % (id, res, self.duration_time))


class MsptiActivityCallbackCase(MsptiBaseCase):
    def __init__(self):
        super(MsptiActivityCallbackCase, self).__init__("execute")
        self.__result = ""
        self.id = "test_MsptiActivityCallbackCase"
        self.case_path = os.path.join(self.mspti_base_dir, "activity_callback_all")
        self.res_dir = os.path.join(self.res_dir, self.id)
        self.build_path = os.path.join(self.case_path, "build")
        self.plog_path = os.path.join(self.build_path, "plog.txt")
        self.exec_cmd = f"cmake -S {self.case_path} -B {self.build_path} -DASCEND_HOME_PATH={self.ascend_home_path}; \
        make -C {self.build_path} -j$(nproc); {self.toolkit_setenv}; {self.log_env}; {self.build_path}/main > {self.plog_path}"

    def initTest(self):
        self.logger.info("------------------------------- "
                           "Start execute case {} -------------------------------".format(self.id))
        if os.path.exists(self.res_dir):
            shutil.rmtree(self.res_dir)
        os.makedirs(self.res_dir)
        if os.path.exists(self.build_path):
            shutil.rmtree(self.build_path)
        os.makedirs(self.build_path)

    def checkRes(self):
        self.view_plog_error(self.plog_path)
        if self.res == 0:
            self.__result = self.cfg_value.pass_res
        else:
            self.__result = self.cfg_value.fail_res
        # 记录执行结果到result.txt
        self.write_res(self.id, self.__result)
        self.assertTrue(self.cfg_value.pass_res == self.__result)

    def execute(self):
        # 初始化工作空间
        self.initTest()
        # 执行用例
        self.executeCmd()
        # 执行结果校验
        self.checkRes()


class MsptiMarkMultiThreadCase(MsptiBaseCase):
    def __init__(self):
        super(MsptiMarkMultiThreadCase, self).__init__("execute")
        self.__result = ""
        self.id = "test_MsptiMarkMultiThreadCase"
        self.case_path = os.path.join(self.mspti_base_dir, "mark_multithread")
        self.res_dir = os.path.join(self.res_dir, self.id)
        self.build_path = os.path.join(self.case_path, "build")
        self.plog_path = os.path.join(self.build_path, "plog.txt")
        self.thread_num = 16
        self.mark_num_per_thread = 40000
        self.exec_cmd = f"cmake -S {self.case_path} -B {self.build_path} -DASCEND_HOME_PATH={self.ascend_home_path}; \
        make -C {self.build_path} -j$(nproc); {self.toolkit_setenv}; {self.log_env}; {self.build_path}/main {self.thread_num} > {self.plog_path}"

    def initTest(self):
        self.logger.info("------------------------------- "
                           "Start execute case {} -------------------------------".format(self.id))
        if os.path.exists(self.res_dir):
            shutil.rmtree(self.res_dir)
        os.makedirs(self.res_dir)
        if os.path.exists(self.build_path):
            shutil.rmtree(self.build_path)
        os.makedirs(self.build_path)

    def get_mark_num(self):
        cmd = r"grep -r 'MSPTI_SMOKE_MARK_NUM' {0}".format(self.plog_path)
        status, res = self.subprocess_cmd(cmd)
        return res.split(" ")[1] if status == 0 else 0

    def checkRes(self):
        self.view_plog_error(self.plog_path)
        mark_num = int(self.get_mark_num())
        if mark_num != self.thread_num * self.mark_num_per_thread:
            self.logger.error(f"Execute {self.id} failed. mark total num error.")
            self.res += 1
        if self.res == 0:
            self.__result = self.cfg_value.pass_res
        else:
            self.__result = self.cfg_value.fail_res
        self.write_res(self.id, self.__result)
        self.assertTrue(self.cfg_value.pass_res == self.__result)

    def execute(self):
        # 初始化工作空间
        self.initTest()
        # 执行用例
        if self.executeCmd() != 0:
            self.logger.error(f"Execute {self.id} failed.")
            self.res += 1
        # 执行结果校验
        self.checkRes()


class MsptiPythonMonitorCase(MsptiBaseCase):
    def __init__(self):
        super(MsptiPythonMonitorCase, self).__init__("execute")
        self.__result = ""
        self.id = "test_MsptiPythonMonitorCase"
        self.case_path = os.path.join(self.mspti_base_dir, "monitor_minst")
        self.res_dir = os.path.join(self.res_dir, self.id)
        self.plog_path = os.path.join(self.res_dir, "plog.txt")
        self.exec_cmd = f"cd {self.case_path}; source {ConfigPaths().ascend_toolkit_path}/set_env.sh; " \
                        f"export LD_PRELOAD={ConfigPaths().ascend_toolkit_path}/latest/lib64/libmspti.so; " \
                        f"source activate smoke_test_env_bak; {self.log_env}; " \
                        r"python3 mnist.py --addr='127.0.0.1' --workers 160 --lr 0.8 --print-freq 1 --dist-url 'tcp://127.0.0.1:50005' " \
                        r"--dist-backend 'hccl' --multiprocessing-distributed --world-size 1 --epochs 1 --rank 0 --device-list '4,5,6,7' --amp " \
                        f"--output {self.res_dir} > {self.plog_path}"

    def initTest(self):
        self.logger.info("------------------------------- "
                           "Start execute case {} -------------------------------".format(self.id))
        if os.path.exists(self.res_dir):
            shutil.rmtree(self.res_dir)
        os.makedirs(self.res_dir)

    def check_marker_count(self):
        marker_count_set = set()
        for csv_file in [path for path in os.listdir(self.res_dir) if path.endswith('csv')]:
            csv_file = os.path.join(self.res_dir, csv_file)
            cmd = "grep -r 'MSPTI_ACTIVITY_KIND_MARKER' {0} | wc -l".format(csv_file)
            status, res = self.subprocess_cmd(cmd)
            marker_count_set.add(int(res) if status == 0 else 0)
        self.logger.info(f"MarkerData count {marker_count_set}.")
        if len(marker_count_set) != 1:
            self.logger.error(f"Execute {self.id} failed. MarkerData num error.")
            self.res += 1

    def check_hccl_count(self):
        hccl_count_set = set()
        for csv_file in [path for path in os.listdir(self.res_dir) if path.endswith('csv')]:
            csv_file = os.path.join(self.res_dir, csv_file)
            cmd = "grep -r 'MSPTI_ACTIVITY_KIND_HCCL' {0} | wc -l".format(csv_file)
            status, res = self.subprocess_cmd(cmd)
            hccl_count_set.add(int(res) if status == 0 else 0)
        self.logger.info(f"HcclData count {hccl_count_set}.")
        if len(hccl_count_set) != 1:
            self.logger.error(f"Execute {self.id} failed. HcclData num error.")
            self.res += 1

    def checkRes(self):
        self.view_plog_error(self.plog_path)
        self.check_marker_count()
        self.check_hccl_count()
        if self.res == 0:
            self.__result = self.cfg_value.pass_res
        else:
            self.__result = self.cfg_value.fail_res
        self.write_res(self.id, self.__result)
        self.assertTrue(self.cfg_value.pass_res == self.__result)

    def execute(self):
        # 初始化工作空间
        self.initTest()
        # 执行用例
        if self.executeCmd() != 0:
            self.logger.error(f"Execute {self.id} failed.")
            self.res += 1
        # 执行结果校验
        self.checkRes()


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(MsptiActivityCallbackCase())
    suite.addTest(MsptiMarkMultiThreadCase())
    suite.addTest(MsptiPythonMonitorCase())
    unittest.TextTestRunner(verbosity=2).run(suite)
