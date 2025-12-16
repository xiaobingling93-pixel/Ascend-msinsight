#!/usr/bin/env python
# coding=utf-8
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
#
# build server
# python build.py build --release
#
# build test
# python build.py test --profile --project_type=test

import argparse
import logging
import multiprocessing
import os
import platform
import shutil
import subprocess
import sys
from logging.handlers import TimedRotatingFileHandler

IS_WINDOWS = platform.system() == 'Windows'
IS_LINUX = platform.system() == 'Linux'
IS_DARWIN = platform.system() == 'Darwin'
FRAMEWORK = 'x86_64' if platform.platform().find('x86_64') > -1 else 'aarch64'
MAKE_JOBS = multiprocessing.cpu_count()

BUILD_DIR = os.path.dirname(os.path.abspath(__file__))
CMAKE_BUILD_DIR = os.path.join(BUILD_DIR, 'build')

HOME_DIR = os.path.dirname(BUILD_DIR)
THIRD_PARTY_DIR = os.path.join(HOME_DIR, 'third_party')
SRC_DIR = os.path.join(HOME_DIR, 'src')
CLUSTER_ANALYSE = 'cluster_analyse'
ADVISOR = 'advisor'
PROF_COMMON = 'prof_common'
CLUSTER_ANALYSE_DIR = os.path.join(THIRD_PARTY_DIR, 'att', 'profiler', 'msprof_analyze', CLUSTER_ANALYSE)
ADVISOR_DIR = os.path.join(THIRD_PARTY_DIR, 'att', 'profiler', 'msprof_analyze', ADVISOR)
PROF_COMMON_DIR = os.path.join(THIRD_PARTY_DIR, 'att', 'profiler', 'msprof_analyze', PROF_COMMON)
PROTO_DIR = os.path.join(SRC_DIR, 'protos')

OUTPUT_DIR = os.path.join(HOME_DIR, 'output')
OUTPUT_OS_DIR = ''
OUTPUT_BIN_DIR = ''
OUTPUT_EXE_DIR = ''
Spec_Path = os.path.join(BUILD_DIR, 'cluster_analysis.spec')

LOG_DIR = os.path.join(BUILD_DIR, 'logs')
LOG_FILE = os.path.join(LOG_DIR, 'build.log')

BUILD_TITLE = '[Server]'


def build_log(build_info):
    LOG.info(BUILD_TITLE + build_info)


def execute_cmd(cmd, work_path):
    proc = subprocess.Popen(cmd, cwd=work_path, stdout=subprocess.PIPE)
    for line in iter(proc.stdout.readline, b''):
        build_log(line.decode('utf-8').strip())
    try:
        proc.communicate(timeout=600)
    except subprocess.TimeoutExpired:
        LOG.error('Subprocess timed out')
        return 1
    return proc.returncode


def log_output(output):
    """log command output"""
    while True:
        line = output.stdout.readline()
        if not line:
            ret = output.poll()
            if ret and ret != 0:
                LOG.error('build error: %d!\n', ret)
                return False
            break
        build_log(line.decode('UTF-8').rstrip())
    return True


def get_gxx_type():
    if IS_WINDOWS:
        gxx_type = 'win_mingw64'
    elif IS_DARWIN:
        gxx_type = 'darwin'
    else:
        gxx_type = 'linux-' + FRAMEWORK
    return gxx_type


def build():
    build_log('begin build...\n')
    gxx_type = get_gxx_type()
    build_dir = os.path.join(CMAKE_BUILD_DIR, gxx_type)
    if not os.path.exists(build_dir):
        os.makedirs(build_dir)

    build_cmds = [
        'cmake', HOME_DIR, '-G', 'Ninja', '-DCMAKE_BUILD_TYPE=Release'
    ]

    result = execute_cmd(build_cmds, build_dir)
    if result != 0:
        build_log('Failed to execute cmake command.')
        return result

    build_cmds = ['cmake', '--build', '.', '-j', str(MAKE_JOBS)]
    result = execute_cmd(build_cmds, build_dir)
    if result != 0:
        build_log('Failed to execute cmake build command.')
        return result

    att_dir = os.path.join(OUTPUT_DIR, gxx_type)
    att_bin_dir = os.path.join(att_dir, 'bin')
    if IS_LINUX:
        script_path = os.path.join(att_bin_dir, 'msprof_analyze')
        if os.path.exists(script_path):
            shutil.rmtree(script_path)
        shutil.copytree(CLUSTER_ANALYSE_DIR, os.path.join(script_path, CLUSTER_ANALYSE), copy_function=shutil.copy2)
        shutil.copytree(PROF_COMMON_DIR, os.path.join(script_path, PROF_COMMON), copy_function=shutil.copy2)
    else:
        build_att = [
            'pyinstaller', '--distpath=' + att_bin_dir, Spec_Path
        ]
        result = execute_cmd(build_att, BUILD_DIR)
        if result != 0:
            build_log('Failed to execute build att command.')
            return result

    build_log('end build.\n')

    return 0


def init_log(name):
    """init log config"""
    if not os.path.exists(LOG_DIR):
        os.makedirs(LOG_DIR)
    building_log = logging.getLogger(name)
    building_log.setLevel(logging.DEBUG)
    formatter = logging.Formatter('[%(asctime)s] %(message)s')
    streamhandler = logging.StreamHandler(sys.stdout)
    streamhandler.setLevel(logging.DEBUG)
    streamhandler.setFormatter(formatter)
    building_log.addHandler(streamhandler)
    filehandler = TimedRotatingFileHandler(LOG_FILE, when='W6', interval=1, backupCount=60)
    filehandler.setLevel(logging.DEBUG)
    filehandler.setFormatter(formatter)
    building_log.addHandler(filehandler)
    return building_log


def build_test():
    build_log('begin test build...\n')
    generator = 'MinGW Makefiles' if IS_WINDOWS else 'Ninja'
    build_dir = os.path.join(CMAKE_BUILD_DIR, "test")

    if not os.path.exists(build_dir):
        os.makedirs(build_dir)

    build_cmds = [
        'cmake', HOME_DIR, '-G', generator, '-DCMAKE_BUILD_TYPE=Release',
        '-D_PROJECT_TYPE=test'
    ]

    result = execute_cmd(build_cmds, build_dir)
    if result != 0:
        build_log('Failed to execute cmake command in test.')
        return result

    build_cmds = ['cmake', '--build', '.', '--target', 'insight_test', '-j', str(MAKE_JOBS)]
    result = execute_cmd(build_cmds, build_dir)
    if result != 0:
        build_log('Failed to execute cmake build command in test.')
        return result

    build_log('end test build.\n')
    return 0


def clean():
    build_dir = os.path.join(CMAKE_BUILD_DIR)
    shutil.rmtree(build_dir)


def main():
    """build entry"""
    parser = argparse.ArgumentParser(description='build dic server project')
    subparsers = parser.add_subparsers(help='sub command help')
    parser_build = subparsers.add_parser('build', help='build dic server')
    parser_build.set_defaults(func=build)

    parser_test = subparsers.add_parser('test', help='test build')
    parser_test.set_defaults(func=build_test)

    parser_clean = subparsers.add_parser('clean', help='make clean')
    parser_clean.set_defaults(func=clean)

    args = parser.parse_args()
    if not hasattr(args, 'func'):
        args = parser.parse_args(['build'] + sys.argv[1:])

    return args.func()


if __name__ == '__main__':
    LOG = init_log('root')
    os.environ['LANG'] = 'C'
    sys.exit(main())
