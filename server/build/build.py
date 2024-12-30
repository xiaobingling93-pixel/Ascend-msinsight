#!/usr/bin/env python
# coding=utf-8
# Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
CLUSTER_ANALYSE_DIR = os.path.join(THIRD_PARTY_DIR, 'att', 'profiler', CLUSTER_ANALYSE)
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
    proc.communicate(timeout=600)
    return proc.returncode


def information():
    build_log('os platform = {}'.format(platform.system()))
    build_log('home directory = {}'.format(HOME_DIR))
    build_log('third_party directory = {}'.format(THIRD_PARTY_DIR))
    build_log('src directory = {}'.format(SRC_DIR))
    build_log('output directory = {}'.format(OUTPUT_DIR))
    build_log('build directory = {}'.format(BUILD_DIR))
    build_log('log directory = {}'.format(LOG_DIR))


def info(args):
    information()


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


def build_bin(args):
    build_log('begin build...\n')
    generator = 'Ninja'
    gxx_type = get_gxx_type()
    build_dir = os.path.join(CMAKE_BUILD_DIR, gxx_type)
    if not os.path.exists(build_dir):
        os.makedirs(build_dir)
    if args.build_release:
        build_type = 'release'
    else:
        build_type = 'debug'

    build_cmds = [
        'cmake', HOME_DIR, '-G', generator, '-DCMAKE_BUILD_TYPE=' + build_type,
        '-D_PROJECT_TYPE=' + args.project_type
    ]

    if args.cross_compile:
        toolchain = os.path.join(HOME_DIR, 'toolchain.cmake')
        build_cmds.append('-DCMAKE_TOOLCHAIN_FILE=' + toolchain)

    if args.project_type == 'true':
        build_cmds.append('-DWASM_MJS_ENABLE=ON')

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
        script_path = os.path.join(att_bin_dir, CLUSTER_ANALYSE)
        if os.path.exists(script_path):
            shutil.rmtree(script_path)
        shutil.copytree(CLUSTER_ANALYSE_DIR, os.path.join(att_bin_dir, CLUSTER_ANALYSE), copy_function=shutil.copy2)
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


def build(args):
    return build_bin(args)


def clean(args):
    """clean build outputs and logs"""
    build_log('begin clean...\n')
    output_dirs = ['output', LOG_DIR, CMAKE_BUILD_DIR]
    for file_path in output_dirs:
        abs_file_path = os.path.join(HOME_DIR, file_path)
        if os.path.isdir(abs_file_path):
            shutil.rmtree(abs_file_path, ignore_errors=True)
        if os.path.isfile(abs_file_path):
            os.remove(abs_file_path)
    build_log('end clean.\n')
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


def build_test(args):
    build_log('begin test build...\n')
    generator = 'MinGW Makefiles' if IS_WINDOWS else 'Unix Makefiles'
    build_dir = os.path.join(CMAKE_BUILD_DIR, "test")

    if not os.path.exists(build_dir):
        os.makedirs(build_dir)
    build_type = 'release'

    build_cmds = [
        'cmake', HOME_DIR, '-G', generator, '-DCMAKE_BUILD_TYPE=' + build_type,
        '-D_PROJECT_TYPE=' + args.project_type
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


def main():
    """build entry"""
    parser = argparse.ArgumentParser(description='build dic server project')
    subparsers = parser.add_subparsers(help='sub command help')
    parser_build = subparsers.add_parser('build', help='build dic server')
    parser_build.set_defaults(func=build)
    parser_build.add_argument('--release',
                              action='store_true',
                              dest='build_release',
                              help='release build')
    parser_build.add_argument('--debug',
                              action='store_false',
                              dest='build_release',
                              help='debug build')
    parser_build.add_argument('--project_type',
                              nargs='?',
                              type=str,
                              dest='project_type',
                              default='',
                              help='project type. CentricServer or EdgeServer')

    parser_build.add_argument('--cross_compile',
                              action='store_true',
                              help='cross compile')

    parser_clean = subparsers.add_parser('clean', help='clean build')
    parser_clean.set_defaults(func=clean)

    parser_clean = subparsers.add_parser('info', help='build information')
    parser_clean.set_defaults(func=info)

    parser_test = subparsers.add_parser('test', help='test build')
    parser_test.set_defaults(func=build_test)
    parser_test.add_argument('--profile',
                             action='store_true',
                             dest='profile',
                             help='build type setting. just profile')
    parser_test.add_argument('--project_type',
                             nargs='?',
                             type=str,
                             dest='project_type',
                             default='test',
                             help='project_type. just test')

    args = parser.parse_args()
    if not hasattr(args, 'func'):
        args = parser.parse_args(['build'] + sys.argv[1:])

    return args.func(args)


if __name__ == '__main__':
    LOG = init_log('root')
    os.environ['LANG'] = 'C'
    sys.exit(main())
