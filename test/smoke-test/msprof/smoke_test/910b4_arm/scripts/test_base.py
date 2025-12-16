from _datetime import datetime
import os
import re
import json
import time
import shutil
import logging

import subprocess
import unittest
from abc import abstractmethod
from collections import defaultdict
import pandas
import sys

sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

from _cfg import *
from check_tools.summary_check import OpSummaryDataCheck

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


class TestProfiling(unittest.TestCase):
    def __init__(self, tc, scene, mode, params, timeout):
        self.id = tc
        self.scene = scene
        self.mode = mode
        self.params = params
        self.timeout = timeout
        self.logger = logging
        self.cfg_path = ConfigPaths()
        self.duration_time = 0
        super(TestProfiling, self).__init__("execute")

    def initTest(self):
        self.cfg_value = ConfigValues()
        self.msprofbin_cmd = ""
        self.res_dir = os.path.join(self.cfg_path.result_path, self.id)
        self.res = 0
        self.__result = self.cfg_value.pass_res
        self.slog_stdout = "{}.log".format(os.path.join(self.res_dir, self.id))
        self.logger.info("------------------------------- "
                         "Start execute case {} -------------------------------".format(self.id))

    def write_res(self):
        if self.res != 0:
            self.__result = self.cfg_value.fail_res
        self.logger.info("\n------------------------------- "
                         "End execute case {} -------------------------------".format(self.id))
        with open('result.txt', 'a+') as f:
            f.write('%s %s %s\n' % (self.id, self.__result, self.duration_time))

    def execute(self):
        self.initTest()
        self.clear_result_dir()
        self.getTestCmd()
        try:
            self.executeCmd()
            self.checkResDir()
        except Exception as e:
            self.__result = self.cfg_value.fail_res
        self.write_res()
        self.assertFalse(self.cfg_value.fail_res == self.__result)

    @abstractmethod
    def getTestCmd(self, scene=None):
        pass

    @abstractmethod
    def checkResDir(self, scene=None):
        self.__result = "pass"
        pass

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
            return err
        finally:
            pass
        return result.stdout

    def view_error_msg(self, argv_path, log_type):
        self.logger.info("start view {} log ...".format(log_type))
        if log_type == "plog":
            cmd = r"grep -rn 'ERROR\] PROFILING' {0}; grep -rn '\[ERROR\] \[MSVP\]' {0}; " \
                  r"grep -rn 'ERROR\] Failed' {0}".format(argv_path)
        else:
            cmd = r"grep -rn 'ERROR\]' {0}".format(argv_path)
        res = self.subprocess_cmd(cmd)
        if re.search(r"ERROR", res):
            self.logger.error(res)
            self.res += 1

    def clear_result_dir(self):
        if os.path.isdir(self.res_dir):
            shutil.rmtree(self.res_dir)
            self.logger.info("clear output ok ...")
        os.makedirs(self.res_dir)

    def executeCmd(self):
        start_time = datetime.now()
        self.subprocess_cmd(self.msprofbin_cmd)
        time_diff = datetime.now() - start_time
        self.duration_time = time_diff.total_seconds()
        self.view_error_msg(self.res_dir, "plog")