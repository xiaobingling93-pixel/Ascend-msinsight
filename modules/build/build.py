#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#
# Copyright 2023 Huawei Technologies Co., Ltd
# ============================================================================
#
# build modules

import logging
import multiprocessing
import os
import platform
import shutil
import subprocess
import sys

BUILD_DIR = os.path.dirname(os.path.abspath(__file__))
MODULES_DIR = os.path.dirname(BUILD_DIR)
PLUGIN_DIR = os.path.join(os.path.dirname(MODULES_DIR), MODULES_DIR, 'framework', 'plugins')

MODULES_MAP = {
    'cluster': 'Cluster',
    'memory': 'Memory',
    'operator': 'Operator',
    'timeline': 'Timeline',
    'compute': 'Compute',
    'jupyter': 'Jupyter',
    'statistic': 'Statistic',
}


def clean():
    modules = list(MODULES_MAP.keys())
    for module in modules:
        plugin_dir = os.path.join(PLUGIN_DIR, MODULES_MAP.get(module))
        if os.path.exists(plugin_dir):
            shutil.rmtree(plugin_dir)


def execute_cmd(module, module_dir, cmd):
    proc = subprocess.Popen(cmd, cwd=module_dir, stdout=subprocess.PIPE)
    for line in iter(proc.stdout.readline, b''):
        logging.info('[%s]%s', module, line.decode('utf-8').strip())
    proc.communicate(timeout=600)
    return proc.returncode


def build_module(module):
    """
    构建单个模块，首先先npm install --force安装依赖，然后npm run build进行编译，最终拷贝结果到framework下的plugins对应目录里

    :param module: 子模块
    :return: 0 表示构建成功， 1表示构建失败
    """
    logging.basicConfig(level=logging.INFO)
    logging.info('[%s]Start to build %s', MODULES_MAP.get(module), module)
    module_dir = os.path.join(MODULES_DIR, module)
    plugin_dir = os.path.join(PLUGIN_DIR, MODULES_MAP.get(module))
    if os.path.exists(plugin_dir):
        shutil.rmtree(plugin_dir)

    npm_cmd = 'npm.cmd' if platform.system() == 'Windows' else 'npm'

    result = execute_cmd(MODULES_MAP.get(module), module_dir, [npm_cmd, 'run', 'build'])
    if result != 0:
        logging.error('[%s]Failed to build %s, %s', MODULES_MAP.get(module), module, result)
        return 1

    shutil.copytree(os.path.join(module_dir, 'build'), plugin_dir)
    logging.info('[%s]Finish to build %s', MODULES_MAP.get(module), module)
    return result


def parallel_build():
    """
    采用多进程实现并行构建，

    :return: 0 表示构建成功， 1表示构建失败；如果单个模块（进程）构建失败，则返回modules构建失败
    """
    logging.info('Start to build modules')

    npm_cmd = 'npm.cmd' if platform.system() == 'Windows' else 'npm'
    result = execute_cmd('modules', MODULES_DIR, [npm_cmd, 'install', '--force'])
    if result != 0:
        logging.error('Failed to install dependencies, %s', result)
        return 1
    result = execute_cmd('lib', os.path.join(MODULES_DIR, 'lib'), [npm_cmd, 'run', 'build'])
    if result != 0:
        logging.error('Failed to build lib, %s', result)
        return 1

    modules = list(MODULES_MAP.keys())
    pool = multiprocessing.Pool(processes=min(multiprocessing.cpu_count(), len(MODULES_MAP)))
    results = pool.map(build_module, modules)
    pool.close()
    pool.join()

    for _, result in enumerate(results):
        if result != 0:
            return 1

    logging.info('Finish to build modules')
    return 0


def main():
    logging.basicConfig(level=logging.INFO)
    if platform.system() != 'Windows':
        multiprocessing.set_start_method('fork')

    clean()

    return parallel_build()


if __name__ == '__main__':
    sys.exit(main())
