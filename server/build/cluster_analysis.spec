#!/usr/bin/env python
# coding=utf-8
# Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
#
# 构建cluster_analysis.exe
from PyInstaller.utils.hooks import collect_submodules
import os

hiddenimports = []
hiddenimports += collect_submodules('cluster_analyse')

BUILD_DIR = os.path.dirname(os.path.abspath(__name__))
HOME_DIR = os.path.dirname(BUILD_DIR)
THIRD_PARTY_DIR = os.path.join(HOME_DIR, 'third_party')
CLUSTER_ANALYSE = 'cluster_analyse'
CLUSTER_ANALYSE_DIR = os.path.join(THIRD_PARTY_DIR, 'att', 'profiler', CLUSTER_ANALYSE)
att_main_path = os.path.join(CLUSTER_ANALYSE_DIR, 'cluster_analysis.py')

a = Analysis(
    [att_main_path],
    pathex=[CLUSTER_ANALYSE_DIR],
    binaries=[],
    datas=[],
    hiddenimports=hiddenimports,
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=['libcrypto-1_1.dll', 'libssl-1_1.dll'],
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
