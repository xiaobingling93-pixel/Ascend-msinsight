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
# sqlite编译和三方依赖整理

import os
import platform
import shutil
import stat
import tarfile
import urllib.request
import subprocess

from build import init_log

BUILD_DIR = os.path.dirname(os.path.abspath(__file__))
HOME_DIR = os.path.dirname(BUILD_DIR)
THIRD_PARTY_DIR = os.path.join(HOME_DIR, 'third_party')

SQLITE_DIR = 'sqlite'
SQLITE3_SRC_DIR = 'sqlite3_src'
SQLITE3_AUTOCONF_DIR = 'sqlite-autoconf-3400100'
SQLITE_SRC_TAR = 'sqlite_src.tar.gz'

BUILD_TITLE = '[Pre Process Third Party]'

CHECK_FILE_LIST = [
    [
        os.path.join(SQLITE_DIR, 'include', 'sqlite3.h'),
        os.path.join(SQLITE3_AUTOCONF_DIR, 'sqlite3.h'),
        os.path.join(SQLITE3_SRC_DIR, 'build', 'sqlite3.h')
    ],
    [
        os.path.join(SQLITE_DIR, 'include', 'sqlite3ext.h'),
        os.path.join(SQLITE3_AUTOCONF_DIR, 'sqlite3ext.h'),
        os.path.join(SQLITE3_SRC_DIR, 'build', 'sqlite3ext.h')
    ],
    [
        os.path.join(SQLITE_DIR, 'src', 'shell.c'),
        os.path.join(SQLITE3_AUTOCONF_DIR, 'shell.c'),
        os.path.join(SQLITE3_SRC_DIR, 'build', 'shell.c')
    ],
    [
        os.path.join(SQLITE_DIR, 'src', 'sqlite3.c'),
        os.path.join(SQLITE3_AUTOCONF_DIR, 'sqlite3.c'),
        os.path.join(SQLITE3_SRC_DIR, 'build', 'sqlite3.c')
    ]
]

SQLITE3_SOURCE_URL = 'https://www.sqlite.org/2022/sqlite-autoconf-3400100.tar.gz'


def log(info):
    LOG.info(BUILD_TITLE + info)


def exec_command(command, path):
    process = subprocess.Popen(command, cwd=path, stdout=subprocess.PIPE)
    for line in iter(process.stdout.readline, b''):
        log(line.decode('utf-8').strip())
    process.communicate(timeout=600)
    if process.returncode != 0:
        log('Failed to execute '.join(command))
    return process.returncode


def prepare_sqlite_src():
    log('start to prepare sqlite src')
    if platform.system() == "Windows":
        if not os.path.exists(os.path.join(THIRD_PARTY_DIR, SQLITE3_AUTOCONF_DIR)):
            tar_path = os.path.join(THIRD_PARTY_DIR, SQLITE_SRC_TAR)
            urllib.request.urlretrieve(SQLITE3_SOURCE_URL, tar_path)
            tar = tarfile.open(tar_path)
            tar.extractall(THIRD_PARTY_DIR)
            tar.close()
            os.remove(tar_path)
    else:
        build_path = os.path.join(THIRD_PARTY_DIR, SQLITE3_SRC_DIR, 'build')
        if os.path.exists(build_path):
            shutil.rmtree(build_path)
        os.mkdir(build_path)
        config_cmd = os.path.join(THIRD_PARTY_DIR, SQLITE3_SRC_DIR, 'configure')
        os.chmod(config_cmd, stat.S_IXUSR | stat.S_IRUSR)
        os.chdir(build_path)
        ret = exec_command(["../configure"], build_path)
        if ret != 0:
            LOG.error('Failed to configure parameter for sqlite')
        ret = exec_command(["make", "sqlite3.c"], build_path)
        if ret != 0:
            LOG.error('Failed to make sqlite3.c')

    log('finish to prepare sqlite3 src')


def reorganize_3rd_party():
    log('start to reorganize third party')
    for file in CHECK_FILE_LIST:
        dst_file = os.path.join(THIRD_PARTY_DIR, file[0])
        if not os.path.exists(dst_file):
            src_file = os.path.join(THIRD_PARTY_DIR, file[1] if platform.system() == "Windows" else file[2])
            if os.path.exists(src_file):
                shutil.copyfile(src_file, dst_file)
            else:
                LOG.error('%s The file is not exist: %s', BUILD_TITLE, src_file)

    log('finish to reorganize third party')


if __name__ == '__main__':
    LOG = init_log('root')
    prepare_sqlite_src()
    reorganize_3rd_party()
