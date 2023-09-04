#!/usr/bin/env python
# -*- coding: UTF-8 -*-

# Copyright 2023 Huawei Technologies Co., Ltd
# ============================================================================

"""build for Insight"""
import os
import platform
import shutil
import subprocess
import logging

SCRIPT_PATH = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))


class ExecError(Exception):
    def __init__(self, message=None):
        super(ExecError, self).__init__(message)


def init():
    clean()
    os.makedirs(os.path.join(SCRIPT_PATH, 'out'))
    os.makedirs(os.path.join(SCRIPT_PATH, 'product'))


def clean():
    out = os.path.join(SCRIPT_PATH, 'out')
    if os.path.exists(out):
        shutil.rmtree(out)
    ascend_insight = os.path.join(SCRIPT_PATH, 'product')
    if os.path.exists(ascend_insight):
        shutil.rmtree(ascend_insight)
    build = os.path.join(SCRIPT_PATH, 'packages', 'frontend', 'build')
    if os.path.exists(build):
        shutil.rmtree(build)


def build_vscode(vscode_version, os_name):
    os.putenv('npm_config_build_from_source', 'true')
    os.putenv('npm_config_audit', 'false')
    os.putenv('npm_config_strict_ssl', 'false')
    os.putenv('npm_config_disturl', 'http://mirrors.tools.huawei.com/nodejs')
    os.putenv('npm_config_registry', 'https://cmc.centralrepo.rnd.huawei.com/artifactory/api/npm/npm-central-repo/')
    if os_name == 'win':
        exec_command(['npm.cmd', 'run', 'buildWin'], SCRIPT_PATH)
    elif os_name.endswith('x86_64'):
        exec_command(['npm', 'run', 'buildLinuxX64'], SCRIPT_PATH)
    elif os_name.endswith('aarch64'):
        exec_command(['npm', 'run', 'buildLinuxArm'], SCRIPT_PATH)
    src = os.path.join(SCRIPT_PATH, 'packages/extension')
    # copy vscode plugin
    dst_file = os.path.join(SCRIPT_PATH, 'out/ascend-insight-extension_' + vscode_version + '_' + os_name + '.vsix')
    for file in os.listdir(src):
        if file.endswith('.vsix'):
            shutil.copy(os.path.join(src, file), dst_file)
    profiler_path = os.path.join(src, 'dist/profiler')
    # Lightweight base middleware
    dst_file = os.path.join(SCRIPT_PATH, 'product/ascend-insight_' + vscode_version + '_' + os_name)
    shutil.make_archive(dst_file, 'zip', profiler_path)


def build_intellij(idea_version, os_name):
    url = os.getenv('GRADLE_URL')
    plugins_path = os.path.join(SCRIPT_PATH, 'plugins')
    is_linux = os_name.startswith('linux')
    gradle_exec = 'gradle' if is_linux else 'gradle.bat'
    if url is None:
        exec_command([gradle_exec, 'wrapper'], plugins_path)
    else:
        exec_command([gradle_exec, 'wrapper', '--gradle-distribution-url', url], plugins_path)
    gradlew = os.path.join(plugins_path, 'gradlew.bat')
    if is_linux:
        exec_command(['chmod', 'a+x', 'gradlew'], plugins_path)
        gradlew = os.path.join(plugins_path, 'gradlew')
    exec_command([gradlew, 'clean'], plugins_path)
    exec_command([gradlew, 'ascend-insight:copyFrontendBuild'], plugins_path)
    exec_command([gradlew, 'buildPlugin'], plugins_path)
    src = os.path.join(SCRIPT_PATH, 'plugins', 'build', 'distributions')
    dst_file = os.path.join(SCRIPT_PATH, 'out/ascend-insight-plugin_' + idea_version + '_' + os_name + '.zip')
    for file in os.listdir(src):
        if file.endswith('.zip'):
            shutil.copy(os.path.join(src, file), dst_file)


def exec_command(command, path):
    process = subprocess.run(command, cwd=path)
    if process.returncode != 0:
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
