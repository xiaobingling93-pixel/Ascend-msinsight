#!/usr/bin/env python
# coding=utf-8
# Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.

import os
import shutil
import subprocess
import tarfile
import urllib.request

from build import init_log, log_output

BUILD_DIR = os.path.dirname(os.path.abspath(__file__))
HOME_DIR = os.path.dirname(BUILD_DIR)
THIRD_PARTY_DIR = os.path.join(HOME_DIR, 'third_party')

SQLITE_DIR = 'sqlite'
SQLITE_SRC_DIR = 'sqlite_src'
SQLITE_SRC_TAR = 'sqlite_src.tar.gz'

BUILD_TITLE = '[Download Third Party]'

OPEN_SOURCE = [
    [
        'googletest',
        'release-1.12.1',
        'ssh://git@szv-open.codehub.huawei.com:2222/OpenSourceCenter/google/googletest.git'
    ],
    [
        'libuv',
        'v1.44.2',
        'ssh://git@szv-open.codehub.huawei.com:2222/OpenSourceCenter/libuv/libuv.git'
    ],
    [
        'json',
        'v3.10.1',
        'ssh://git@szv-open.codehub.huawei.com:2222/OpenSourceCenter/nlohmann/json.git'
    ],
    [
        'uSockets',
        'v0.8.1',
        'ssh://git@szv-open.codehub.huawei.com:2222/OpenSourceCenter/uNetworking/uSockets.git'
    ],
    [
        'uWebSockets',
        'v20.10.0',
        'ssh://git@szv-open.codehub.huawei.com:2222/OpenSourceCenter/uNetworking/uWebSockets.git'
    ]
]

CHECK_FILE_LIST = [
    [
        os.path.join(SQLITE_DIR, 'include', 'sqlite3.h'),
        os.path.join(SQLITE_SRC_DIR, 'sqlite-autoconf-3400100', 'sqlite3.h')
    ],
    [
        os.path.join(SQLITE_DIR, 'include', 'sqlite3ext.h'),
        os.path.join(SQLITE_SRC_DIR, 'sqlite-autoconf-3400100', 'sqlite3ext.h')
    ],
    [
        os.path.join(SQLITE_DIR, 'src', 'shell.c'),
        os.path.join(SQLITE_SRC_DIR, 'sqlite-autoconf-3400100', 'shell.c')
    ],
    [
        os.path.join(SQLITE_DIR, 'src', 'sqlite3.c'),
        os.path.join(SQLITE_SRC_DIR, 'sqlite-autoconf-3400100', 'sqlite3.c')
    ],
    [
        os.path.join('json_modern_c++', 'include', 'json.hpp'),
        os.path.join('json', 'single_include', 'nlohmann', 'json.hpp')
    ]
]

SQLITE3_SOURCE_URL = 'https://cmc.cloudartifact.szv.dragon.tools.huawei.com/artifactory/opensource_general/SQLite/' \
                     '3.40.1/package/sqlite-autoconf-3400100.tar.gz'


def log(info):
    LOG.info(BUILD_TITLE + info)


def download_3rd_party():
    log('start to download third party')

    for source in OPEN_SOURCE:
        if os.path.exists(os.path.join(THIRD_PARTY_DIR, source[0])):
            continue
        download_cmd = ['git', 'clone', '-b', source[1], source[2], source[0], '--depth=1']
        output = subprocess.Popen(download_cmd, cwd=THIRD_PARTY_DIR, stdout=subprocess.PIPE)
        log_output(output)

    if not os.path.exists(os.path.join(THIRD_PARTY_DIR, SQLITE_SRC_DIR)):
        tar_path = os.path.join(THIRD_PARTY_DIR, SQLITE_SRC_TAR)
        urllib.request.urlretrieve(SQLITE3_SOURCE_URL, tar_path)
        tar = tarfile.open(tar_path)
        tar.extractall(path=os.path.join(THIRD_PARTY_DIR, SQLITE_SRC_DIR))
        tar.close()
        os.remove(tar_path)


def reorganize_3rd_party():
    log('start to reorganize third party')
    for file in CHECK_FILE_LIST:
        dst_file = os.path.join(THIRD_PARTY_DIR, file[0])
        if not os.path.exists(dst_file):
            src_file = os.path.join(THIRD_PARTY_DIR, file[1])
            if os.path.exists(src_file):
                shutil.copyfile(os.path.join(
                    THIRD_PARTY_DIR, file[1]), dst_file)
            else:
                LOG.error(BUILD_TITLE +
                          'The file is not exist: {}'.format(src_file))
    log('finish to reorganize third party')


if __name__ == '__main__':
    LOG = init_log('root')
    download_3rd_party()
    reorganize_3rd_party()
    log('finish to download third party')
