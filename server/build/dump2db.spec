# -*- mode: python ; coding: utf-8 -*-
import os
BUILD_DIR = os.path.dirname(os.path.abspath(__name__))
HOME_DIR = os.path.dirname(BUILD_DIR)
INSIGHT_DIR = os.path.dirname(HOME_DIR)
MEMSNAPDUMP_DIR = os.path.join(INSIGHT_DIR, 'scripts', 'MemSnapDump')
DUMP2DB_PY = os.path.join(MEMSNAPDUMP_DIR, 'tools', 'dump2db.py')
a = Analysis(
    [DUMP2DB_PY],
    pathex=[MEMSNAPDUMP_DIR],
    binaries=[],
    datas=[],
    hiddenimports=[],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='dump2db',
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
