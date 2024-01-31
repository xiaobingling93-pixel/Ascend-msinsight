#!/usr/bin/env python
# -*- coding: UTF-8 -*-

# Copyright 2023 Huawei Technologies Co., Ltd
# ============================================================================

"""build for Insight"""
import logging
import os
import platform
import shutil
import subprocess

PROJECT_PATH = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))


class Const:
    WINDOWS_OS = 'Windows'
    MAC_OS = 'Darwin'
    LINUX_OS = 'Linux'
    OUT_DIR = 'out'
    PRODUCT_DIR = 'product'
    FRAMEWORK_DIR = 'framework'
    MODULES_DIR = 'modules'
    SERVER_DIR = 'server'
    BUILD_DIR = 'build'
    VSCODE_PLUGINS_DIR = os.path.join('plugins', 'vscode')
    INTELLIJ_PLUGINS_DIR = os.path.join('plugins', 'intellij')
    ASCEND_INSIGHT_PREFIX = 'Ascend-Insight'
    ASCEND_INSIGHT = 'ascend_insight'
    BIN_SUFFIX = '.exe' if platform.system() == WINDOWS_OS else ''
    PACKAGE_SUFFIX = '.exe' if platform.system() == WINDOWS_OS else '.zip'
    PYTHON = 'python' if platform.system() == WINDOWS_OS else 'python3'
    NPM = 'npm.cmd' if platform.system() == WINDOWS_OS else 'npm'
    GRADLE = 'gradle.bat' if platform.system() == WINDOWS_OS else 'gradle'
    GRADLEW = 'gradlew.bat' if platform.system() == WINDOWS_OS else 'gradlew'


class ExecError(Exception):
    def __init__(self, message=None):
        super(ExecError, self).__init__(message)


def init():
    clean()
    os.makedirs(os.path.join(PROJECT_PATH, Const.OUT_DIR))
    os.makedirs(os.path.join(PROJECT_PATH, Const.PRODUCT_DIR))


def clean():
    out = os.path.join(PROJECT_PATH, Const.OUT_DIR)
    if os.path.exists(out):
        shutil.rmtree(out)
    ascend_insight = os.path.join(PROJECT_PATH, Const.PRODUCT_DIR)
    if os.path.exists(ascend_insight):
        shutil.rmtree(ascend_insight)
    framework_dist = os.path.join(PROJECT_PATH, Const.FRAMEWORK_DIR, 'dist')
    if os.path.exists(framework_dist):
        shutil.rmtree(framework_dist)
    modules = ['cluster', 'memory', 'timeline']
    for module in modules:
        build_dir = os.path.join(PROJECT_PATH, Const.MODULES_DIR, module, Const.BUILD_DIR)
        if os.path.exists(build_dir):
            shutil.rmtree(build_dir)


def build_server():
    output_path = os.path.join(PROJECT_PATH, Const.SERVER_DIR, 'output')
    if os.path.exists(output_path):
        shutil.rmtree(output_path)
    os.mkdir(output_path)

    build_path = os.path.join(PROJECT_PATH, Const.SERVER_DIR, Const.BUILD_DIR)
    # 开源软件预处理，只要是生成sqlite的源码
    exec_command([Const.PYTHON, 'preprocess_third_party.py'], build_path)
    # 编译代码
    exec_command([Const.PYTHON, 'build.py', 'build', '--release'], build_path)

    # 归一化构建产物目录，方便后续其他组件拷贝
    for tmp in os.listdir(output_path):
        shutil.copytree(os.path.join(output_path, tmp, 'bin'), os.path.join(output_path, Const.BUILD_DIR, 'server'))


def build_frontend():
    os.putenv('npm_config_build_from_source', 'true')
    os.putenv('npm_config_audit', 'false')
    os.putenv('npm_config_strict_ssl', 'false')
    os.putenv('npm_config_disturl', 'http://mirrors.tools.huawei.com/nodejs')
    os.putenv('npm_config_registry', 'https://cmc.centralrepo.rnd.huawei.com/artifactory/api/npm/npm-central-repo/')

    module_build_path = os.path.join(PROJECT_PATH, Const.MODULES_DIR, Const.BUILD_DIR)
    exec_command([Const.PYTHON, 'build.py'], module_build_path)

    framework_path = os.path.join(PROJECT_PATH, Const.FRAMEWORK_DIR)
    exec_command([Const.NPM, 'install', '--force'], framework_path)
    exec_command([Const.NPM, 'run', 'build'], framework_path)

    shutil.copytree(os.path.join(framework_path, 'plugins'), os.path.join(framework_path, 'dist', 'plugins'))


def build_vscode(vscode_version, os_name):
    # Linux和MacOS上默认不编译vscode插件
    if platform.system() != Const.WINDOWS_OS and os.getenv('NO_BUILD_VSCODE') is not None:
        logging.info('The VSCode plugin is not compiled because NO_BUILD_VSCODE is set.')
        return

    os.putenv('npm_config_build_from_source', 'true')
    os.putenv('npm_config_audit', 'false')
    os.putenv('npm_config_strict_ssl', 'false')
    os.putenv('npm_config_disturl', 'http://mirrors.tools.huawei.com/nodejs')
    os.putenv('npm_config_registry', 'https://cmc.centralrepo.rnd.huawei.com/artifactory/api/npm/npm-central-repo/')

    plugin_path = os.path.join(PROJECT_PATH, Const.VSCODE_PLUGINS_DIR)
    exec_command([Const.NPM, 'install', '--force'], plugin_path)
    exec_command([Const.NPM, 'run', 'vsce:package'], plugin_path)

    # copy vscode plugin to out directory
    plugin_name = 'ascend-insight-extension_' + vscode_version + '_' + os_name + '.vsix'
    dst_file = os.path.join(PROJECT_PATH, Const.OUT_DIR, plugin_name)
    for file in os.listdir(plugin_path):
        if file.endswith('.vsix'):
            shutil.copy(os.path.join(plugin_path, file), dst_file)

    # zip server and frontend files for huaweicloud
    zip_name = 'ascend-insight_' + vscode_version + '_' + os_name
    dst_file = os.path.join(PROJECT_PATH, Const.PRODUCT_DIR, zip_name)
    profiler_path = os.path.join(plugin_path, 'dist', 'profiler')
    shutil.make_archive(dst_file, 'zip', profiler_path)


def build_intellij(idea_version, os_name):
    # MacOS上不编译intellij插件
    if platform.system() == Const.MAC_OS:
        return

    plugins_path = os.path.join(PROJECT_PATH, Const.INTELLIJ_PLUGINS_DIR)
    url = os.getenv('GRADLE_URL')
    if url is None:
        exec_command([Const.GRADLE, 'wrapper'], plugins_path)
    else:
        exec_command([Const.GRADLE, 'wrapper', '--gradle-distribution-url', url], plugins_path)
    gradlew = os.path.join(plugins_path, Const.GRADLEW)
    if platform.system() != Const.WINDOWS_OS:
        exec_command(['chmod', 'a+x', 'gradlew'], plugins_path)
    exec_command([gradlew, 'clean'], plugins_path)
    exec_command([gradlew, 'ascend-insight:copyFrontendBuild'], plugins_path)
    exec_command([gradlew, 'buildPlugin'], plugins_path)

    distributions_path = os.path.join(PROJECT_PATH, Const.INTELLIJ_PLUGINS_DIR, Const.BUILD_DIR, 'distributions')
    plugin_name = 'ascend-insight-plugin_' + idea_version + '_' + os_name + '.zip'
    dst_file = os.path.join(PROJECT_PATH, Const.OUT_DIR, plugin_name)
    for file in os.listdir(distributions_path):
        if file.endswith('.zip'):
            shutil.copy(os.path.join(distributions_path, file), dst_file)


def build_light_package(version, os_name):
    # MR门禁不跑
    package_path = os.path.join(PROJECT_PATH, 'opensource', 'package')
    if not os.path.exists(package_path):
        return

    # Linux环境CI尚未打通二进制包，若要编译二进制包，需要设置BUILD_BIN_PACKAGE环境变量
    if platform.system() == Const.LINUX_OS and os.getenv('BUILD_BIN_PACKAGE') is None:
        return

    os.putenv('RUSTUP_UPDATE_ROOT', 'http://rust.inhuawei.com/rustup-static/rustup')
    os.putenv('RUSTUP_DIST_SERVER', 'http://rust.inhuawei.com/rustup-static')
    os.putenv('CARGO_REGISTRY', 'https://mirrors.tools.huawei.com/rust/crates.io-index/')

    # 清理构建缓存
    resource_dir = 'resources'
    preview_path = os.path.join(package_path, 'preview')
    profiler_path = os.path.join(package_path, resource_dir, 'profiler')
    target_path = os.path.join(package_path, 'target')
    build_cache_paths = [profiler_path, preview_path, target_path]
    for tmp_path in build_cache_paths:
        if os.path.exists(tmp_path):
            shutil.rmtree(tmp_path)
        os.mkdir(tmp_path)

    # 拷贝前后端文件
    shutil.copytree(os.path.join(PROJECT_PATH, Const.FRAMEWORK_DIR, 'dist'), os.path.join(profiler_path, 'frontend'))
    shutil.copytree(os.path.join(PROJECT_PATH, Const.SERVER_DIR, 'output', 'build', 'server'),
                    os.path.join(profiler_path, 'server'))
    shutil.copytree(os.path.join(package_path, resource_dir), os.path.join(preview_path, resource_dir))

    # 构建底座
    cargo_cmd = 'cargo.exe' if platform.system() == Const.WINDOWS_OS else 'cargo'
    bin_file = 'ascend_insight.exe' if platform.system() == Const.WINDOWS_OS else 'ascend_insight'
    exec_command([cargo_cmd, 'build', '--release'], package_path)
    shutil.copyfile(os.path.join(target_path, 'release', bin_file), os.path.join(preview_path, bin_file))

    # 打包
    package_name = Const.ASCEND_INSIGHT_PREFIX + '_' + version + '_' + os_name + Const.PACKAGE_SUFFIX
    dst_file = os.path.join(PROJECT_PATH, Const.OUT_DIR, package_name)
    if platform.system() == Const.WINDOWS_OS:
        bundle_path = os.path.join(package_path, 'bundle')
        dependencies_dir = os.path.join(package_path, 'dependencies')
        for file in os.listdir(dependencies_dir):
            shutil.copyfile(os.path.join(dependencies_dir, file), os.path.join(preview_path, file))
        shutil.copyfile(os.path.join(bundle_path, 'installer.nsi'), os.path.join(preview_path, 'installer.nsi'))
        nsis_cmd = os.path.join(bundle_path, 'nsis', 'bin', 'makensis.exe')
        exec_command([nsis_cmd, os.path.join('preview', 'installer.nsi')], package_path)
        for tmp in os.listdir(preview_path):
            if not tmp.startswith(Const.ASCEND_INSIGHT_PREFIX):
                continue
            shutil.copyfile(os.path.join(preview_path, tmp), dst_file)
            break
    else:
        os.chmod(os.path.join(preview_path, Const.ASCEND_INSIGHT), 0o550)
        server_path = os.path.join(preview_path, resource_dir, 'profiler', 'server')
        for file in os.listdir(server_path):
            os.chmod(os.path.join(server_path, file), 0o550)
        shutil.make_archive(dst_file[:-4], 'zip', preview_path)


def exec_command(command, path):
    process = subprocess.run(command, cwd=path)
    if process.returncode != 0:
        logging.error('execute %s failed', command)
        raise ExecError()


def main():
    vscode_version = '7.0.1'
    idea_version = '7.0.RC1'
    init()
    os_info = platform.platform()
    framework = 'x86_64' if os_info.find('x86_64') > -1 else 'aarch64'
    os_name = 'linux-' + framework
    if os_info.find(Const.WINDOWS_OS) > -1:
        os_name = 'win'
    elif os_info.find('mac') > -1:
        os_name = 'darwin-' + framework
    build_server()
    build_frontend()
    build_vscode(vscode_version, os_name)
    build_intellij(idea_version, os_name)
    build_light_package(idea_version, os_name)


if __name__ == "__main__":
    main()
