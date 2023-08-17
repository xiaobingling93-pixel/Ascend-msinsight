#!/usr/bin/env python
# -*- coding: UTF-8 -*-

# Copyright 2023 Huawei Technologies Co., Ltd
# ============================================================================

"""build for Insight"""
import os
import platform
import shutil
import sys
import logging

SCRIPT_PATH = os.path.dirname(os.path.realpath(__file__))


class ExecError(Exception):
    def __init__(self, message=None):
        super(ExecError, self).__init__(message)


def init():
    out = os.path.join(SCRIPT_PATH, 'out')
    if os.path.exists(out):
        for file in os.listdir(out):
            os.remove(os.path.join(out, file))
        os.rmdir(out)
    os.makedirs(os.path.join(SCRIPT_PATH, 'out'))
    sys.platform


def build_vscode(vscode_version, os_name):
    os.putenv('npm_config_build_from_source', 'true')
    if os_name == 'win':
        exec_command('cd ' + SCRIPT_PATH + ' && npm run buildWin')
    elif os_name.endswith('x86_64'):
        exec_command('cd ' + SCRIPT_PATH + ' && npm run buildLinuxX64')
    elif os_name.endswith('aarch64'):
        exec_command('cd ' + SCRIPT_PATH + ' && npm run buildLinuxArm')
    src = os.path.join(SCRIPT_PATH, 'packages/extension')
    dst_file = os.path.join(SCRIPT_PATH, 'out/ascend-insight-extension_' + vscode_version + '_' + os_name + '.vsix')
    for file in os.listdir(src):
        if file.endswith('.vsix'):
            shutil.copy(os.path.join(src, file), dst_file)


def build_intellij(idea_version, os_name):
    url = os.getenv('GRADLE_URL')
    plugins_path = os.path.join(SCRIPT_PATH, 'plugins')
    if url is None:
        exec_command('cd ' + plugins_path + ' && gradle wrapper')
    else:
        exec_command('cd ' + plugins_path + ' && gradle wrapper --gradle-distribution-url ' + url)
    gradlew = 'gradlew'
    if os_name.startswith('linux'):
        exec_command('cd ' + plugins_path + ' && chmod a+x gradlew')
        gradlew = './gradlew'
    exec_command('cd ' + plugins_path + ' && ' + gradlew + ' clean')
    exec_command('cd ' + plugins_path + ' && ' + gradlew + ' ascend-insight:copyFrontendBuild')
    exec_command('cd ' + plugins_path + ' && ' + gradlew + ' buildPlugin')
    src = os.path.join(SCRIPT_PATH, 'plugins', 'build', 'distributions')
    dst_file = os.path.join(SCRIPT_PATH, 'out/ascend-insight-plugin_' + idea_version + '_' + os_name + '.zip')
    for file in os.listdir(src):
        if file.endswith('.zip'):
            shutil.copy(os.path.join(src, file), dst_file)


def exec_command(command):
    if os.system(command) == 1:
        logging.error('execute %s failed', command)
        raise ExecError()


def main():
    vscode_version = '6.0.3'
    idea_version = '6.0.RC3'
    init()
    os_info = platform.platform()
    framework = 'x86_64' if os_info.find('x86_64') > -1 else 'aarch64'
    os_name = 'win' if os_info.find('Windows') > -1 else 'linux-' + framework
    build_vscode(vscode_version, os_name)
    build_intellij(idea_version, os_name)


if __name__ == "__main__":
    main()
