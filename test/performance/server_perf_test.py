#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
"""
-------------------------------------------------------------------------
This file is part of the MindStudio project.
Copyright (c) 2025 Huawei Technologies Co.,Ltd.

MindStudio is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:

         http://license.coscl.org.cn/MulanPSL2

THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details.
-------------------------------------------------------------------------
"""
# # run server performance test
import datetime
import logging
import os
import platform
import shutil
import subprocess
import sys

SCRIPT_PATH = os.path.realpath(__file__)
PROJECT_PATH = os.path.dirname(os.path.dirname(os.path.dirname(SCRIPT_PATH)))


class Const:
    WINDOWS_OS = 'Windows'
    MAC_OS = 'Darwin'
    LINUX_OS = 'Linux'
    SERVER_DIR = 'server'
    BUILD_DIR = 'build'
    OUTPUT_DIR = 'output'
    PYTHON = 'python' if platform.system() == WINDOWS_OS else 'python3'
    TEST_EXEC_FILE = 'insight_performance' + ('.exe' if platform.system() == WINDOWS_OS else '')


def exec_command(cmd, workspace):
    logging.info('Execute %s in the %s', cmd, workspace)
    proc = subprocess.Popen(cmd, cwd=workspace, stdout=subprocess.PIPE)
    for line in iter(proc.stdout.readline, b''):
        logging.info(line.decode('utf-8').strip())
    proc.communicate(timeout=6000)  # 单个命令执行时间不超过6000s
    return proc.returncode


def init():
    output_path = os.path.join(PROJECT_PATH, Const.SERVER_DIR, Const.OUTPUT_DIR)
    if os.path.exists(output_path):
        shutil.rmtree(output_path)


def build_project():
    build_path = os.path.join(PROJECT_PATH, Const.SERVER_DIR, Const.BUILD_DIR)
    logging.info('Start to build server project with test.')
    os.putenv('DEV_TYPE', 'true')
    result = exec_command([Const.PYTHON, 'download_third_party.py'], build_path)
    if result != 0:
        logging.error('Failed to download third party.')
        return -1
    result = exec_command([Const.PYTHON, 'preprocess_third_party.py'], build_path)
    if result != 0:
        logging.error('Failed to preprocess third party.')
        return -1
    result = exec_command([Const.PYTHON, 'build.py', 'clean'], build_path)
    if result != 0:
        logging.error('Failed to clean last build.')
        return -1
    result = exec_command([Const.PYTHON, 'build.py', 'build', '--Release'], build_path)
    if result != 0:
        logging.error('Failed to build project.')
        return -1
    logging.info('Finish to build server project with test.')
    return 0


def execute_test():
    logging.info('Start to execute performance test.')
    output_path = os.path.join(PROJECT_PATH, Const.SERVER_DIR, Const.OUTPUT_DIR)
    test_path = output_path
    for file in os.listdir(output_path):
        test_path = os.path.join(output_path, file, 'bin')
        break
    if not test_path.endswith('bin') or not os.path.exists(test_path):
        logging.error('There is no executable files in %s.', test_path)
        return -1
    result_file = os.path.join(test_path, 'performance_test.csv')
    if os.path.exists(result_file):
        os.remove(result_file)
    result = exec_command([os.path.join(test_path, Const.TEST_EXEC_FILE)], test_path)
    if result != 0:
        logging.error('Failed to execute performance test.')
        return -1
    if not os.path.exists(result_file):
        logging.error('There is no performance result file: %s.', result_file)
        return -1
    result_back_path = os.path.join(os.path.dirname(PROJECT_PATH), 'result_back')
    if not os.path.exists(result_back_path):
        os.mkdir(result_back_path)
    now = datetime.datetime.now(tz=datetime.timezone.utc)
    suffix = now.strftime("_%Y-%m-%d_%H-%M-%S")
    result_back_file = 'performance_test' + suffix + '.csv'
    shutil.copyfile(result_file, os.path.join(result_back_path, result_back_file))
    logging.info('Finish to execute performance test.')
    return 0


def main():
    init()
    result = build_project()
    if result != 0:
        logging.error('Failed to build profiler server.')
        return -1
    result = execute_test()
    if result != 0:
        logging.error('Failed to execute performance test.')
        return -1
    return 0


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    sys.exit(main())
