#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# -------------------------------------------------------------------------
# This file is part of the MindStudio project.
# Copyright (c) 2025 Huawei Technologies Co.,Ltd.
#
# MindStudio is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#
#          http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.
# -------------------------------------------------------------------------
# # run server ut test

import logging
import os
import platform
import subprocess
import sys
from datetime import datetime, timezone
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from email_sender import read_email_config, EmailSender, ReadEmailConfigException

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))


class Const:
    SERVER_PATH = os.path.join(ROOT_DIR, 'server')
    SERVER_BUILD_PATH = os.path.join(SERVER_PATH, 'build')
    TEST_PATH = os.path.join(ROOT_DIR, 'test')
    TEST_UT_PATH = os.path.join(TEST_PATH, 'ut')
    PYTHON = 'python' if platform.system() == 'Windows' else 'python3'
    LOG_FILE = os.path.join(TEST_UT_PATH, 'server_ut_test.log')
    EMAIL_CONFIG_FILE = os.path.join(os.path.expanduser('~'), '.email_config')


def exec_command(command, path, stage_name):
    logging.basicConfig(level=logging.INFO)
    process = subprocess.Popen(command, cwd=path, stdout=subprocess.PIPE)
    for line in iter(process.stdout.readline, b''):
        logging.info('[%s]%s', stage_name, line.decode('utf-8').strip())
    try:
        process.communicate(timeout=600)
    except ValueError or TimeoutError:
        logging.error('[%s]Failed to execute %s due to timeout or wrong input.', stage_name, ' '.join(command))
        return 1
    if process.returncode != 0:
        logging.error('[%s]Failed to execute %s.', stage_name, ' '.join(command))
    return process.returncode


def send_email(result=False):
    # read email config
    if not os.path.exists(Const.EMAIL_CONFIG_FILE):
        logging.error('Email config file is not existed')
        return 1
    try:
        sender, username, passwd, receivers = read_email_config(Const.EMAIL_CONFIG_FILE)
    except ReadEmailConfigException:
        logging.error('Email config file is not existed')
        return 1

    email_sender = EmailSender(sender, username, passwd, receivers)
    test_time = datetime.now(tz=timezone.utc).strftime("%Y-%m-%d")
    subject = f'[MindStudio-Insight][master]每日DT用例测试报告({platform.system()})_{test_time}_result:{result}'
    if result:
        html_content = '<div>Test succeeded, see the log for details.</div>'
    else:
        html_content = '<div>Test failed, see the log for details.</div>'
    attachment_path = Const.LOG_FILE
    email_sender.send(subject, html_content, attachment_path)
    return 0


def main():
    # download and preprocess third party dependency
    download_deps_cmd = [Const.PYTHON, 'download_third_party.py']
    ret = exec_command(download_deps_cmd, Const.SERVER_BUILD_PATH, 'Download Dependencies')
    if ret != 0:
        logging.error('Failed to download dependencies')
        return 1

    preprocess_deps_cmd = [Const.PYTHON, 'preprocess_third_party.py']
    ret = exec_command(preprocess_deps_cmd, Const.SERVER_BUILD_PATH, 'Preprocess Dependencies')
    if ret != 0:
        logging.error('Failed to preprocess dependencies')
        return 1

    # build server with test
    build_server_test_cmd = [Const.PYTHON, 'build.py', 'test']
    ret = exec_command(build_server_test_cmd, Const.SERVER_BUILD_PATH, 'Build Server')
    if ret != 0:
        logging.error('Failed to build server with test')
        return 1

    # execute ut case and logging the result
    if platform.system() == 'Windows':
        sub_build_dir = 'win_mingw64'
    elif platform.system() == 'Darwin':
        sub_build_dir = 'darwin'
    else:
        sub_build_dir = 'linux-x86_64' if platform.machine() == 'x86_64' else 'linux-aarch64'
    test_file_path = os.path.join(Const.SERVER_PATH, 'output', sub_build_dir, 'bin')
    run_ut_test_cmd = [os.path.join(test_file_path, 'insight_test')]
    logging.info('Start to run ut test')
    ret = exec_command(run_ut_test_cmd, test_file_path, 'Run UT Test')
    if ret != 0:
        logging.error('Failed to run ut test')
    else:
        logging.info('Success to run ut test')

    # package log file and send email with result
    ret = send_email(True if ret == 0 else False)
    if ret != 0:
        logging.error('Failed to send email')
        return 1
    return 0


if __name__ == "__main__":
    logging.basicConfig(filename='server_ut_test.log', filemode='w', level=logging.INFO,
                        format='%(levelname)s - %(message)s')
    sys.exit(main())
