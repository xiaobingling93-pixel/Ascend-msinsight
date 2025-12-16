#!/usr/bin/env python
# coding=utf-8
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
#
# 构建cluster_analysis.exe
import os

BUILD_DIR = os.path.dirname(os.path.abspath(__name__))
HOME_DIR = os.path.dirname(BUILD_DIR)
THIRD_PARTY_DIR = os.path.join(HOME_DIR, 'third_party')
PROFILER_DIR = os.path.join(THIRD_PARTY_DIR, 'att', 'profiler')
CLUSTER_ANALYSE = 'cluster_analyse'
CLUSTER_ANALYSE_DIR = os.path.join(PROFILER_DIR, 'msprof_analyze', CLUSTER_ANALYSE)
att_main_path = os.path.join(CLUSTER_ANALYSE_DIR, 'cluster_analysis.py')

a = Analysis(
    [att_main_path],
    pathex=[PROFILER_DIR],
    binaries=[],
    datas=[],
    hiddenimports=[],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=['certifi', 'pygments', 'libcrypto-1_1.dll', 'libssl-1_1.dll', 'tkinter', 'cryptography', 'bcrypt'],
    noarchive=False,
    optimize=0,
)

a.binaries = [x for x in a.binaries if 'libcrypto-1_1.dll' not in x[0] and 'libssl-1_1.dll' not in x[0]]
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='cluster_analysis',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=True,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)
