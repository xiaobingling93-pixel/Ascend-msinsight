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
# run server performance test script
import configparser
import csv
import logging
import os
import platform
import stat
import subprocess
import sys

from email_sender import EmailSender, PerformanceResult

SCRIPT_PATH = os.path.realpath(__file__)
PROJECT_PATH = os.path.dirname(os.path.dirname(os.path.dirname(SCRIPT_PATH)))


class ReadEmailConfigException(Exception):
    def __init__(self, msg=''):
        self.msg = msg
        super().__init__(self.msg)


class Const:
    WINDOWS_OS = 'Windows'
    MAC_OS = 'Darwin'
    LINUX_OS = 'Linux'
    ASCEND_INSIGHT_DIR = 'Ascend-Insight'
    TEST_DIR = 'test'
    PERFORMANCE_DIR = 'performance'
    SERVER_DIR = 'server'
    TEST_SCRIPT = 'server_perf_test.py'
    OUTPUT_PATH = os.path.join(PROJECT_PATH, SERVER_DIR, 'output')
    TEST_PATH = os.path.join(PROJECT_PATH, TEST_DIR, PERFORMANCE_DIR)
    GIT = 'git.exe' if platform.system() == WINDOWS_OS else 'git'
    PYTHON = 'python' if platform.system() == WINDOWS_OS else 'python3'
    RESULT_FILE_HEADER = ['No.', 'Module', 'CaseName', 'Result', 'Baseline(ms)', 'Real Time(ms)']


def exec_command(cmd, workspace):
    logging.info('Execute %s in the %s', cmd, workspace)
    proc = subprocess.Popen(cmd, cwd=workspace, stdout=subprocess.PIPE)
    for line in iter(proc.stdout.readline, b''):
        logging.info(line.decode('utf-8').strip())
    proc.communicate(timeout=6000)
    return proc.returncode


def read_email_config():
    config = configparser.ConfigParser()
    config_file = os.path.join(os.path.expanduser('~'), '.email_config')
    try:
        with open(config_file, 'r') as f:
            config.read_file(f)
    except FileNotFoundError as e:
        logging.error('The email config file does not exist.')
        raise ReadEmailConfigException from e
    except Exception as e:
        logging.error(f'Failed to read email config file: %s', e)
        raise ReadEmailConfigException from e

    email_config = 'email_config'
    try:
        username = config.get(email_config, 'username')
        passwd = config.get(email_config, 'passwd')
        sender = config.get(email_config, 'sender')
        receiver = config.get(email_config, 'receiver')
        receivers = receiver.split(',')
    except Exception as e:
        raise ReadEmailConfigException from e

    return EmailSender(sender, username, passwd, receivers)


def send_email():
    result_file = ''
    workspace = ''
    if not os.path.exists(Const.OUTPUT_PATH) or len(os.listdir(Const.OUTPUT_PATH)) < 1:
        return 1
    for file in os.listdir(Const.OUTPUT_PATH):
        if os.path.isdir(os.path.join(Const.OUTPUT_PATH, file)):
            workspace = os.path.join(Const.OUTPUT_PATH, file, 'bin')
    if len(workspace) == 0 or not os.path.exists(workspace):
        logging.error('Can\'t find workspace.')
        return 1
    for file in os.listdir(workspace):
        if file.startswith('performance_test') and file.endswith('.csv'):
            result_file = os.path.join(workspace, file)
            break
    if not result_file:
        logging.error('Can\'t find result file.')
        return 1

    try:
        email_sender = read_email_config()
        results_list = []
        with open(result_file, newline='') as csvfile:
            reader = csv.reader(csvfile, delimiter=',', quotechar='"')
            header = next(reader)
            if header != Const.RESULT_FILE_HEADER:
                logging.error('The result file is not correct.')
                return 1
            for row in reader:
                one = PerformanceResult(row[0], row[1], row[2], row[3], row[4], row[5])
                results_list.append(one)
        content = email_sender.generate_email_content(results_list)
        email_sender.send(content.get('subject'), content.get('html_content'), '', email_sender.receivers, [])
        return 0
    except ReadEmailConfigException:
        logging.error('Failed to get email config.')
        return 1


def main():
    logging.basicConfig(level=logging.INFO)

    # 拉起运行脚本
    result = exec_command([Const.PYTHON, Const.TEST_SCRIPT], Const.TEST_PATH)
    if result != 0:
        logging.error('Failed to run server performance test.')
        return 1

    result = send_email()
    if result != 0:
        logging.error('Failed to send email.')
    return 0


if __name__ == "__main__":
    sys.exit(main())
