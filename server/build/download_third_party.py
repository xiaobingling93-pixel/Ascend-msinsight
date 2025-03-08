#!/usr/bin/env python
# coding=utf-8
# Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
#
# 下载三方依赖

import os
import subprocess

from build import init_log, log_output

BUILD_DIR = os.path.dirname(os.path.abspath(__file__))
HOME_DIR = os.path.dirname(BUILD_DIR)
THIRD_PARTY_DIR = os.path.join(HOME_DIR, 'third_party')


BUILD_TITLE = '[Download Third Party]'

OPEN_SOURCE = [
    [
        'googletest',
        'v1.13.0',
        'https://szv-open.codehub.huawei.com/OpenSourceCenter/google/googletest.git'
    ],
    [
        'mockcpp',
        'master',
        'https://szv-open.codehub.huawei.com/OpenSourceCenter/sinojelly/mockcpp.git'
    ],
    [
        'libuv',
        'v1.44.2',
        'https://szv-open.codehub.huawei.com/OpenSourceCenter/libuv/libuv.git'
    ],
    [
        'uSockets',
        'v0.8.6',
        'https://szv-open.codehub.huawei.com/OpenSourceCenter/uNetworking/uSockets.git'
    ],
    [
        'uWebSockets',
        'v20.48.0',
        'https://szv-open.codehub.huawei.com/OpenSourceCenter/uNetworking/uWebSockets.git'
    ],
    [
        'rapidjson',
        '6089180ecb704cb2b136777798fa1be303618975-htrunk1',
        'https://szv-open.codehub.huawei.com/OpenSourceCenter/Tencent/rapidjson.git'
    ],
    [
        'sqlite3_src',
        '3.46.1',
        'https://szv-open.codehub.huawei.com/OpenSourceCenter/www.sqlite.org/sqlite.git'
    ],
    [
        'att',
        'v6.0.0-ATT-htrunk13',
        'https://szv-open.codehub.huawei.com/OpenSourceCenter/Ascend/att.git'
    ]
]


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

    log('finish to download third party')


if __name__ == '__main__':
    LOG = init_log('root')
    download_3rd_party()
