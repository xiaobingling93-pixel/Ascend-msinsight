#!/usr/bin/env python
# -*- coding: UTF-8 -*-

# Copyright 2023 Huawei Technologies Co., Ltd
# ============================================================================

"""build for Insight"""
import logging
import multiprocessing
import os
import platform
import shutil
import subprocess
import stat
import sys
from datetime import datetime, timezone
import json
import zipfile

PROJECT_PATH = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
WORKSPACE_PATH = os.getenv("OCTOPUS_WORKSPACE")
if WORKSPACE_PATH is not None:
    PLUGIN_INSTALL_TMP_PATH = os.path.join(WORKSPACE_PATH, "PluginInstallCache")


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
    SRC_DIR = 'src'
    MANIFEST_DIR = 'manifest'
    DEPENDENCY_DIR = 'dependency'
    CONFIG_INI = 'config.ini'
    VSCODE_PLUGINS_DIR = os.path.join('plugins', 'vscode')
    INTELLIJ_PLUGINS_DIR = os.path.join('plugins', 'intellij')
    JUPYTERLAB_PLUGINS_DIR = os.path.join('plugins', 'jupyterlab')
    PLATFORM_DIR = os.path.join(PROJECT_PATH, 'platform')
    PLATFORM_PREVIEW_DIR = os.path.join(PLATFORM_DIR, 'preview')
    PLATFORM_TARGET_DIR = os.path.join(PLATFORM_DIR, 'target')
    ASCEND_INSIGHT_PREFIX = 'MindStudio-Insight'
    ASCEND_INSIGHT = 'MindStudio_Insight'
    BIN_SUFFIX = '.exe' if platform.system() == WINDOWS_OS else ''
    PRODUCT_FORMAT = {
        # Windows NSI安装程序
        WINDOWS_OS: '.exe',
        # MacOS下的app dmg安装包
        MAC_OS: '.dmg',
        # Linux下压缩文件
        LINUX_OS: '.zip'
    }
    PACKAGE_SUFFIX = PRODUCT_FORMAT.get(platform.system(), '.zip')
    MAC_OS_APPNAME = 'MindStudioInsight.app'
    PYTHON = 'python' if platform.system() == WINDOWS_OS else 'python3'
    NPM = 'npm.cmd' if platform.system() == WINDOWS_OS else 'npm'
    CARGO = 'cargo.exe' if platform.system() == WINDOWS_OS else 'cargo'
    GRADLE = 'gradle.bat' if platform.system() == WINDOWS_OS else 'gradle'
    GRADLEW = 'gradlew.bat' if platform.system() == WINDOWS_OS else 'gradlew'
    JUPYTER = 'jupyter'
    PIP = 'pip' if platform.system() == WINDOWS_OS else 'pip3'
    PLUGINS_VERSION_PLACEHOLDER = '{plugins_version}'
    MAC_ARM_SIGNATURE_CERTIFICATE_ID = "0CC4E29F544EE91874A89DE9C61421E3D3722A79"
    MAC_X86_SIGNATURE_CERTIFICATE_ID = "0B361EE30477593A3766B67157B94FB942EAF20F"
    MAC_SIGNATURE_CERTIFICATE_ID = ""


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
    framework_dist = os.path.join(PROJECT_PATH, Const.MODULES_DIR, Const.FRAMEWORK_DIR, 'build')
    if os.path.exists(framework_dist):
        shutil.rmtree(framework_dist)
    modules = ['cluster', 'memory', 'timeline', 'compute', 'jupyter', 'operator', 'lib']
    for module in modules:
        build_dir = os.path.join(PROJECT_PATH, Const.MODULES_DIR, module, Const.BUILD_DIR)
        if os.path.exists(build_dir):
            shutil.rmtree(build_dir)


# 后台下载npm和cargo依赖，与server并行执行，以缩短构建时间
def download_dependency_background() -> bool:
    modules_dir = os.path.join(PROJECT_PATH, Const.MODULES_DIR)
    build_modules = [Const.NPM, 'install', '--force']
    platform_dir = os.path.join(PROJECT_PATH, Const.PLATFORM_DIR)
    build_platform = [Const.CARGO, 'fetch']
    try:
        subprocess.Popen(build_modules, cwd=modules_dir, stdout=subprocess.PIPE)
        logging.info('[download dependency]Start to download dependency for modules in background.')
        subprocess.Popen(build_platform, cwd=platform_dir, stdout=subprocess.PIPE)
        logging.info('[download dependency]Start to download dependency for platform in background.')
        return True
    except Exception as e:
        logging.error('[download dependency] Failed to start to download dependency in background. %s.', e)
        return False


def traverse_folder_and_chmod(path, dir_mode, file_mode):
    os.chmod(path, dir_mode)
    for root, dirs, files in os.walk(path):
        for one in dirs:
            traverse_folder_and_chmod(os.path.join(root, one), dir_mode, file_mode)
        for one in files:
            os.chmod(os.path.join(root, one), file_mode)


def build_server():
    output_path = os.path.join(PROJECT_PATH, Const.SERVER_DIR, 'output')
    if os.path.exists(output_path):
        shutil.rmtree(output_path)
    os.mkdir(output_path)

    build_path = os.path.join(PROJECT_PATH, Const.SERVER_DIR, Const.BUILD_DIR)
    # 开源软件预处理，只要是生成sqlite的源码
    result = exec_command([Const.PYTHON, 'preprocess_third_party.py'], build_path, Const.SERVER_DIR)
    if result != 0:
        return 1
    # 编译代码
    result = exec_command([Const.PYTHON, 'build.py', 'build'], build_path, Const.SERVER_DIR)
    if result != 0:
        return 1
    # 归一化构建产物目录，方便后续其他组件拷贝
    for tmp in os.listdir(output_path):
        tmp_path = os.path.join(output_path, Const.BUILD_DIR, Const.SERVER_DIR)
        os.makedirs(tmp_path)
        bin_path = os.path.join(output_path, tmp, 'bin')
        for file in os.listdir(bin_path):
            if os.path.isdir(os.path.join(bin_path, file)):
                shutil.copytree(os.path.join(bin_path, file), os.path.join(tmp_path, file))
                continue
            if file.endswith('.a'):  # 跳过.a文件
                continue
            shutil.copyfile(os.path.join(bin_path, file), os.path.join(tmp_path, file))
    return 0


def build_frontend():
    os.putenv('npm_config_strict_ssl', 'false')
    os.putenv('npm_config_registry', 'https://cmc.centralrepo.rnd.huawei.com/artifactory/api/npm/npm-central-repo/')

    module_name = 'frontend'
    module_build_path = os.path.join(PROJECT_PATH, Const.MODULES_DIR, Const.BUILD_DIR)
    result = exec_command([Const.PYTHON, 'build.py'], module_build_path, module_name)
    if result != 0:
        return 1

    framework_path = os.path.join(PROJECT_PATH, Const.MODULES_DIR, Const.FRAMEWORK_DIR)
    result = exec_command([Const.NPM, 'install', '--force'], framework_path, module_name)
    if result != 0:
        return 1

    result = exec_command([Const.NPM, 'run', 'build'], framework_path, module_name)
    if result != 0:
        return 1

    shutil.copytree(os.path.join(framework_path, 'plugins'), os.path.join(framework_path, 'build', 'plugins'))
    return 0


def set_npm_config():
    os.putenv('npm_config_strict_ssl', 'false')
    os.putenv('npm_config_registry', 'https://cmc.centralrepo.rnd.huawei.com/artifactory/api/npm/npm-central-repo/')
    os.putenv('npm_config_@cloudsop:registry', 'https://cmc.centralrepo.rnd.huawei.com/artifactory/api/npm/product_npm')


def build_vscode(vscode_version, os_name):
    # Linux和MacOS上默认不编译vscode插件
    if platform.system() != Const.WINDOWS_OS and os.getenv('NO_BUILD_VSCODE'):
        logging.info('The VSCode plugin is not compiled because NO_BUILD_VSCODE is set.')
        return 0

    set_npm_config()

    plugin_path = os.path.join(PROJECT_PATH, Const.VSCODE_PLUGINS_DIR)
    result = exec_command([Const.NPM, 'install', '--force'], plugin_path, 'vscode_plugin')
    if result != 0:
        return 1

    result = exec_command([Const.NPM, 'run', 'vsce:package'], plugin_path, 'vscode_plugin')
    if result != 0:
        return 1

    # copy vscode plugin to out directory
    plugin_name = 'mindstudio-insight-extension_' + vscode_version + '_' + os_name + '.vsix'
    dst_file = os.path.join(PROJECT_PATH, Const.OUT_DIR, plugin_name)
    for file in os.listdir(plugin_path):
        if file.endswith('.vsix'):
            shutil.copy(os.path.join(plugin_path, file), dst_file)

    # zip server and frontend files for huaweicloud
    zip_name = 'mindstudio-insight_' + vscode_version + '_' + os_name
    dst_file = os.path.join(PROJECT_PATH, Const.PRODUCT_DIR, zip_name)
    profiler_path = os.path.join(plugin_path, 'dist', 'profiler')
    shutil.make_archive(dst_file, 'zip', profiler_path)
    return 0


def build_intellij(idea_version, os_name):
    # MacOS上不编译intellij插件
    if platform.system() == Const.MAC_OS:
        return 0

    module_name = 'intellij_plugin'
    plugins_path = os.path.join(PROJECT_PATH, Const.INTELLIJ_PLUGINS_DIR)
    url = os.getenv('GRADLE_URL')
    if url is None:
        result = exec_command([Const.GRADLE, 'wrapper'], plugins_path, module_name)
    else:
        result = exec_command([Const.GRADLE, 'wrapper', '--gradle-distribution-url', url], plugins_path, module_name)
    if result != 0:
        return 1

    gradlew = os.path.join(plugins_path, Const.GRADLEW)
    if platform.system() != Const.WINDOWS_OS:
        result = exec_command(['chmod', 'a+x', 'gradlew'], plugins_path, module_name)
        if result != 0:
            return 1

    result = exec_command([gradlew, 'clean'], plugins_path, module_name)
    if result != 0:
        return 1

    result = exec_command([gradlew, 'mindstudio-insight:copyFrontendBuild'], plugins_path, module_name)
    if result != 0:
        return 1

    result = exec_command([gradlew, 'buildPlugin'], plugins_path, module_name)
    if result != 0:
        return 1

    distributions_path = os.path.join(PROJECT_PATH, Const.INTELLIJ_PLUGINS_DIR, Const.BUILD_DIR, 'distributions')
    plugin_name = 'mindstudio-insight-plugin_' + idea_version + '_' + os_name + '.zip'
    dst_file = os.path.join(PROJECT_PATH, Const.OUT_DIR, plugin_name)
    for file in os.listdir(distributions_path):
        if file.endswith('.zip'):
            shutil.copy(os.path.join(distributions_path, file), dst_file)

    return 0


def update_jupyterlab_plugin_version(jupyterlab_version, plugin_path):
    package_json_path = os.path.join(plugin_path, 'package.json')
    if not os.path.exists(package_json_path):
        return 1

    with open(package_json_path, 'r', encoding='utf-8') as file:
        try:
            package_data = json.load(file)
        except json.JSONDecodeError:
            return 1

    package_data['version'] = jupyterlab_version

    with open(package_json_path, 'w', encoding='utf-8') as file:
        try:
            json.dump(package_data, file, indent=2)
        except json.JSONDecodeError:
            return 1
    return 0


def get_os_platform():
    os_system = platform.system().lower()
    os_machine = platform.machine().lower()
    if 'windows' in os_system:
        return 'win_' + os_machine
    return os_system + '_' + os_machine


def build_jupyterlab(jupyterlab_version, os_name):
    # 设置环境变量暂时不构建jupyterlab
    if not os.getenv('BUILD_JUPYTERLAB'):
        logging.info('The JupyterLab extension is not compiled because BUILD_JUPYTERLAB is not set.')
        return 0

    plugin_path = os.path.join(PROJECT_PATH, Const.JUPYTERLAB_PLUGINS_DIR)

    # 下载构建依赖
    result = exec_command([Const.PIP, 'install', '--trusted-host', 'mirrors.tools.huawei.com', '-i',
            'https://mirrors.tools.huawei.com/pypi/simple',
            'jupyterlab==4.3.2', 'tornado==6.2', 'jupyter_packaging~=0.12.3'],
            plugin_path, 'jupyterlab_plugin')
    if result != 0:
        return 1

    # 设置npm代理
    os.putenv('npm_config_registry', 'https://mirrors.tools.huawei.com/npm/')

    # 拷贝前后端资源
    jupyterlab_path = 'mindstudio_insight_jupyterlab'
    resources_dir = 'resources'
    resources_path = os.path.join(plugin_path, jupyterlab_path, resources_dir)
    if not os.path.exists(resources_path):
        os.makedirs(resources_path, 0o750)
    shutil.copytree(os.path.join(PROJECT_PATH, Const.MODULES_DIR, Const.FRAMEWORK_DIR, 'build'),
                    os.path.join(resources_path, 'frontend'))
    shutil.copytree(os.path.join(PROJECT_PATH, Const.SERVER_DIR, 'output', 'build', 'server'),
                    os.path.join(resources_path, 'server'))

    # 修改jupyterlab中package.json版本
    result = update_jupyterlab_plugin_version(jupyterlab_version, plugin_path)
    if result != 0:
        return 1

    # 1. 清理jupyterlab环境
    result = exec_command([Const.JUPYTER, 'lab', 'clean'], plugin_path, 'jupyterlab_plugin')
    if result != 0:
        return 1
    # 2. 构建whl包
    setup_path, output_path = 'setup.py', 'output'
    result = exec_command([Const.PYTHON, setup_path, 'bdist_wheel',
                           '--plat-name=' + get_os_platform(), '--dist-dir', output_path],
                          plugin_path, 'jupyterlab_plugin')
    if result != 0:
        return 1

    # 3. 此处暂时需要构建两次
    setup_path, output_path = 'setup.py', 'output'
    result = exec_command([Const.PYTHON, setup_path, 'bdist_wheel',
                           '--plat-name=' + get_os_platform(), '--dist-dir', output_path],
                          plugin_path, 'jupyterlab_plugin')
    if result != 0:
        return 1

    # copy jupyterlab plugin to out directory
    plugin_name = 'mindstudio_insight_jupyterlab-' + jupyterlab_version + '-py3-none-' + get_os_platform() + '.whl'
    dst_file = os.path.join(PROJECT_PATH, Const.OUT_DIR, plugin_name)
    whl_source_path = os.path.join(plugin_path, 'output')
    for file in os.listdir(whl_source_path):
        if file.endswith('.whl'):
            shutil.copy(os.path.join(whl_source_path, file), dst_file)

    return 0


def build_package(version, os_name):
    return build_light_package(version, os_name, False) & build_light_package(version, os_name, True)


def build_light_package(version, os_name, is_huaweicloud):
    if is_huaweicloud and os_name != "linux-aarch64" and os_name != "linux-x86_64":
        return 0
    os.putenv('CARGO_REGISTRY', 'https://mirrors.tools.huawei.com/rust/crates.io-index/')
    if os.getenv('BEPHOME') is not None:  # 规避目前cargo不能跑bep问题
        os.putenv('LD_PRELOAD', '')

    # 清理构建缓存
    resource_dir = 'resources'
    build_cache_paths = [Const.PLATFORM_PREVIEW_DIR, Const.PLATFORM_TARGET_DIR]
    for tmp_path in build_cache_paths:
        if os.path.exists(tmp_path):
            traverse_folder_and_chmod(tmp_path, 0o750, 0o750)
            shutil.rmtree(tmp_path)
        os.mkdir(tmp_path)
    # 拷贝前后端文件
    shutil.copytree(os.path.join(Const.PLATFORM_DIR, resource_dir),
                    os.path.join(Const.PLATFORM_PREVIEW_DIR, resource_dir))
    profiler_path = os.path.join(Const.PLATFORM_PREVIEW_DIR, resource_dir, 'profiler')
    os.mkdir(profiler_path, 0o750)
    shutil.copytree(os.path.join(PROJECT_PATH, Const.MODULES_DIR, Const.FRAMEWORK_DIR, 'build'),
                    os.path.join(profiler_path, 'frontend'))
    shutil.copytree(os.path.join(PROJECT_PATH, Const.SERVER_DIR, 'output', 'build', 'server'),
                    os.path.join(profiler_path, 'server'))
    # 华为云构建将插件一并打包
    if is_huaweicloud:
        huaweicloud_install_plugin(profiler_path=profiler_path)
    # 在macos下使用cargo bundle --release直接构建为app
    if platform.system() == Const.MAC_OS:
        cmd_list = [Const.CARGO, 'bundle', '--release']
        set_mac_app_signature_certificate_id(os_name)
    else:
        cmd_list = [Const.CARGO, 'build', '--release']
    package_name = Const.ASCEND_INSIGHT_PREFIX + '_' + version + '_' + os_name + Const.PACKAGE_SUFFIX
    if is_huaweicloud:
        shutil.copyfile(os.path.join(PROJECT_PATH, "build", "huaweicloud_start_script.py"),
                        os.path.join(profiler_path, "start_script.py"))
        cmd_list = cmd_list + ["--no-default-features"]
        package_name = Const.ASCEND_INSIGHT_PREFIX + '_huaweicloud_' + version + '_' + os_name + Const.PACKAGE_SUFFIX

    shutil.copyfile(os.path.join(PROJECT_PATH, "build", "plugin_install.py"),
                    os.path.join(profiler_path, "plugin_install.py"))

    result = exec_command(cmd_list, Const.PLATFORM_DIR, 'bin_package')
    if result != 0:
        return 1
    return zip_package(profiler_path, package_name)


file_names = {
    (Const.WINDOWS_OS, 'bin'): 'MindStudioInsight.exe',
    (Const.WINDOWS_OS, 'target'): 'MindStudio-Insight.exe',
    (Const.MAC_OS, 'target'): 'MindStudio Insight'
}


def zip_package(profiler_path, package_name):
    system = platform.system()
    bin_file = file_names.get((system, 'bin'), 'MindStudioInsight')
    target_file = file_names.get((system, 'target'), 'MindStudio-Insight')
    # MacOs通过cargo bundle打包后的产物为app, 不拷贝二进制可执行文件
    if not system == Const.MAC_OS:
        shutil.copyfile(os.path.join(Const.PLATFORM_TARGET_DIR, 'release', bin_file),
                        os.path.join(Const.PLATFORM_PREVIEW_DIR, target_file))
    # 打包
    dst_file = os.path.join(PROJECT_PATH, Const.OUT_DIR, package_name)
    if system == Const.WINDOWS_OS:
        shutil.copytree(os.path.join(Const.PLATFORM_DIR, 'config'),
                        os.path.join(Const.PLATFORM_PREVIEW_DIR, 'config'))  # 仅Windows需要
        bundle_path = os.path.join(Const.PLATFORM_DIR, 'bundle')
        shutil.copyfile(os.path.join(bundle_path, 'installer.nsi'),
                        os.path.join(Const.PLATFORM_PREVIEW_DIR, 'installer.nsi'))
        nsis_cmd = os.path.join('C:\\Program Files (x86)\\NSIS', 'bin', 'makensis.exe')
        result = exec_command([nsis_cmd, os.path.join('preview', 'installer.nsi')], Const.PLATFORM_DIR, 'bin_package')
        if result != 0:
            return 1
        for tmp in os.listdir(Const.PLATFORM_PREVIEW_DIR):
            if not tmp.startswith(Const.ASCEND_INSIGHT_PREFIX + '_'):
                continue
            shutil.copyfile(os.path.join(Const.PLATFORM_PREVIEW_DIR, tmp), dst_file)
            break
    else:
        traverse_folder_and_chmod(Const.PLATFORM_PREVIEW_DIR, 0o750, 0o640)  # 1、统一修改为文件夹750，文件640
        traverse_folder_and_chmod(os.path.join(profiler_path, Const.SERVER_DIR), 0o750, 0o550)  # 2、server下的文件550
        # 非MacOS、windows场景，即linux场景，打包为zip即可
        if system != Const.MAC_OS:
            os.chmod(os.path.join(Const.PLATFORM_PREVIEW_DIR, target_file), 0o550)  # 3、ascend_insight 550
            shutil.make_archive(dst_file[:-4], 'zip', Const.PLATFORM_PREVIEW_DIR)
            return 0
        # [AR] 新增额外的macOS场景下打包为app->dmg的流程
        app_dir = os.path.join(Const.PLATFORM_TARGET_DIR, 'release', 'bundle', 'osx', Const.MAC_OS_APPNAME)
        app_bin_file_dir = os.path.join(app_dir, 'Contents', 'MacOS')
        preview_app = os.path.join(PROJECT_PATH, Const.PLATFORM_PREVIEW_DIR, Const.MAC_OS_APPNAME)
        os.chmod(os.path.join(app_bin_file_dir, bin_file), 0o550)  # 4、app内二进制文件 ascend_insight 550
        shutil.copytree(os.path.join(Const.PLATFORM_PREVIEW_DIR, 'resources'),
                            os.path.join(app_bin_file_dir, 'resources'))
        shutil.move(app_dir, preview_app)
        # 签名app
        if "aarch64" in package_name:
            # 清除旧bundle临时签名
            if not clear_mac_app_signature(preview_app):
                return 1
            # 重签
            if not sign_mac_app(preview_app, Const.MAC_SIGNATURE_CERTIFICATE_ID):
                return 1
            logging.info('[%s] %s', 'bin_package', 'MacOS application resigned successfully, start to build dmg')
        if not chmod_mac_app(preview_app, 'aarch64' if 'aarch64' in package_name else 'x86_64'):
            return 1
        # 通过dmgbuild打包
        if not build_dmg_for_mac_app(dst_file):
            logging.info('[%s] %s', 'bin_package', 'Build dmg for application failed.')
            return 1
        # 将dmg文件设置为640
        os.chmod(dst_file, 0o640)
    return 0


def chmod_mac_app(app_path: str, arch: str) -> bool:
    path_list = get_mac_app_structure(app_path, arch)
    if not path_list:
        logging.warning(f'Failed to get structure of {app_path}, '
                        f'no further permission modification actions will be performed.')
        return False
    # 将目录设置为750, 文件设置为640
    for path in path_list:
        try:
            os.chmod(path, 0o750 if os.path.isdir(path) else 0o640)
        except Exception as e:
            logging.error(f'An exception occurred while performing chmod.Path:{path}, Error: {e}')
            return False
    return True


def get_mac_app_structure(app_path: str, arch: str) -> list:
    """
    获取mac app捆绑包特定的目录结构
    :param app_path: app路径 如 /tmp/example.app
    :return: 捆绑包内的文件、目录
    """
    app_structure_paths = [app_path]
    contents_dir = os.path.join(app_path, 'Contents')
    macos_dir = os.path.join(contents_dir, 'MacOS')
    resources_dir = os.path.join(contents_dir, 'Resources')
    info_file = os.path.join(contents_dir, 'Info.plist')
    icon_file = os.path.join(resources_dir, 'MindStudioInsight.icns')
    app_structure_paths.extend([contents_dir, macos_dir, resources_dir, info_file, icon_file])
    if 'aarch64' in arch:
        sign_dir = os.path.join(contents_dir, '_CodeSignature')
        sign_file = os.path.join(sign_dir, 'CodeResources')
        app_structure_paths.extend([sign_dir, sign_file])
    for path in app_structure_paths:
        if not os.path.exists(path):
            logging.warning(f'{path} not found.')
            return []
    return app_structure_paths


def clear_mac_app_signature(app_path: str) -> bool:
    if not os.path.exists(app_path):
        return False
    # 清除现有的bundle签名
    logging.info('[%s] %s', 'bin_package', 'MacOS application old signature removing.')
    remove_sign_cmd_list = ["codesign", "--remove-signature", app_path]
    result = exec_command(remove_sign_cmd_list,
                          os.path.join(PROJECT_PATH, Const.PLATFORM_PREVIEW_DIR), 'bin_package')
    if result != 0:
        logging.error('[%s] %s', 'bin_package', 'MacOS application old signature removed failed.')
        return False
    return True


def sign_mac_app(app_path: str, certificate_id: str = Const.MAC_SIGNATURE_CERTIFICATE_ID) -> bool:
    if not os.path.exists(app_path):
        return False
    logging.info('[%s] %s', 'bin_package',
                 'Start to sign/resign MacOS application, using certificate %s' % certificate_id)
    sign_cmd_list = ["codesign", "--force", "-s", certificate_id,
                     "--deep", "--timestamp=none", app_path]
    result = exec_command(sign_cmd_list, os.path.join(PROJECT_PATH, Const.PLATFORM_PREVIEW_DIR), 'bin_package')
    if result != 0:
        logging.error('[%s] %s', 'bin_package', 'MacOS application signed failed.')
        return False
    return True


def build_dmg_for_mac_app(dst_file) -> bool:
    cmd_list = ["dmgbuild", "-s", "macos_dmg_settings.json", '\"MindStudio Installer\"', dst_file]
    result = exec_command(cmd_list, os.path.join(PROJECT_PATH, Const.PLATFORM_DIR, 'bundle'), 'bin_package')
    if result != 0:
        return False
    return True


def exec_command(command, path, module_name):
    logging.basicConfig(level=logging.INFO)
    process = subprocess.Popen(command, cwd=path, stdout=subprocess.PIPE)
    for line in iter(process.stdout.readline, b''):
        logging.info('[%s]%s', module_name, line.decode('utf-8').strip())
    process.communicate(timeout=600)
    if process.returncode != 0:
        logging.error('[%s]Failed to execute %s.', module_name, ' '.join(command))
    return process.returncode


# 获取版本信息，将从config.ini中读取到的版本后去掉最后一个后缀
def get_version_from_config(file_path, default_version):
    version = default_version
    with open(file_path, 'r') as file:
        for line in file:
            kv = line.split('=')
            if len(kv) != 2:
                continue
            if kv[0].strip() == "version":
                # 读取到的版本后去掉最后一个后缀
                version_info = kv[1].strip().split('.')
                version = '.'.join(version_info[:-1])
    return version


# 加载版本相关信息，参数为默认的版本数据
def load_version_info(default_version):
    # 获取config.ini文件路径，该文件位于Ascend-Insight项目同级目录下的/manifest/dependency/config.ini
    file_path = os.path.join(os.path.dirname(PROJECT_PATH), Const.MANIFEST_DIR, Const.DEPENDENCY_DIR, Const.CONFIG_INI)
    version = default_version
    modify_time = datetime.now(tz=timezone.utc).strftime("%Y/%m/%d")
    # 判断文件是否存在，不存在直接使用默认版本号和修改时间
    if os.path.exists(file_path):
        version = get_version_from_config(file_path, default_version)
    # 创建（覆盖）版本信息文件
    create_version_info_file(version, modify_time)
    return version


# 创建、修改版本信息文件，文件目录在framework/src/下，文件名为version_info.json
def create_version_info_file(version, modify_time):
    output_path = os.path.join(PROJECT_PATH, Const.MODULES_DIR, Const.FRAMEWORK_DIR, Const.SRC_DIR, 'version_info.json')
    # os.O_WRONLY表示只写入，os.O_CREAT在文件不存在时会创建文件，os.O_TRUNC会清空原文件内容
    flags = os.O_WRONLY | os.O_CREAT | os.O_TRUNC
    mode = stat.S_IWUSR
    with os.fdopen(os.open(output_path, flags, mode), "w") as f:
        data = {'version': version, 'modifyTime': modify_time}
        f.write(json.dumps(data))


def build_product_parallel(vscode_version, idea_version, os_name):
    logging.info('Start to build products')
    funcs = [build_package, build_jupyterlab]
    args_list = [
        (idea_version, os_name),
        (vscode_version, os_name)
    ]
    if os.getenv('BUILD_INTELLIJ'):
        funcs.append(build_intellij)
        args_list.append((idea_version, os_name))
    if os.getenv('BUILD_VSCODE'):
        funcs.append(build_vscode)
        args_list.append((vscode_version, os_name))

    pool = multiprocessing.Pool(processes=min(multiprocessing.cpu_count(), len(funcs)))
    results = []
    for func, args in zip(funcs, args_list):
        results.append(pool.apply_async(func, args))
    pool.close()
    pool.join()

    for func, result in zip(funcs, results):
        if result.get() != 0:
            logging.error('Failed to execute %s, and see the log for the error cause.', func)
            return 1

    logging.info('Finish to build products.')
    return 0


# 替换文件内容，对文件内容中的占位符使用指定内容替换
def replace_placeholders_in_file(file_path, placeholder, replacement):
    with os.fdopen(os.open(file_path, os.O_RDWR, stat.S_IWUSR), "r+", encoding='utf-8') as file:
        # 读取文件内容
        content = file.read()
        # 进行占位符替换
        content = content.replace(placeholder, replacement)
        # 将文件指针移到开头以便重写文件内容
        file.seek(0)
        file.write(content)
        file.truncate()  # 清除文件指针当前位置后面的内容


def update_plugins_version(version):
    # 替换gradle.properties中的版本信息
    gradle_properties = os.path.join(PROJECT_PATH, Const.INTELLIJ_PLUGINS_DIR, 'foundation',
                                     'ideaplugin', 'gradle.properties')
    replace_placeholders_in_file(gradle_properties, Const.PLUGINS_VERSION_PLACEHOLDER, version)

    # 替换installer.nsi中的版本信息
    installer_nsi_path = os.path.join(PROJECT_PATH, Const.PLATFORM_DIR, 'bundle', 'installer.nsi')
    replace_placeholders_in_file(installer_nsi_path, Const.PLUGINS_VERSION_PLACEHOLDER, version)

    # 替换Cargo.toml中的版本信息
    cargo_toml_path = os.path.join(PROJECT_PATH, Const.PLATFORM_DIR, "Cargo.toml")
    replace_placeholders_in_file(cargo_toml_path, Const.PLUGINS_VERSION_PLACEHOLDER, version)


def huaweicloud_install_plugin(profiler_path):
    if WORKSPACE_PATH is None:
        return
    plugins_dir = os.path.join(WORKSPACE_PATH, "Msi_plugin_package")
    if not os.path.exists(plugins_dir):
        logging.warning("Plugin dir not found")
        return
    for item in os.listdir(plugins_dir):
        logging.info("Install plugin:{}".format(item))
        item_path = os.path.join(plugins_dir, item)
        plugin_install_init(plugin_zip=item_path)
        install_opt = read_plugin_install_opt()
        if install_opt is None:
            continue
        plugin_name = install_opt.get('pluginName')
        if not plugin_install_frontend(profiler_path, plugin_name, install_opt):
            continue
        plugin_install_backend(profiler_path, plugin_name, install_opt)


def plugin_install_init(plugin_zip):
    if os.path.exists(PLUGIN_INSTALL_TMP_PATH):
        shutil.rmtree(PLUGIN_INSTALL_TMP_PATH)
    with zipfile.ZipFile(plugin_zip, "r") as zip_file:
        zip_file.extractall(PLUGIN_INSTALL_TMP_PATH)


def read_plugin_install_opt():
    config_path = os.path.join(PLUGIN_INSTALL_TMP_PATH, "config.json")
    if not os.path.exists(config_path):
        logging.error("Config file not found in plugin")
        return None
    try:
        with open(config_path) as file:
            return json.load(file)
    except IOError:
        logging.error("Failed open config file")
        return None


def plugin_install_frontend(profiler_path, plugin_name, config):
    front_dir = os.path.join(profiler_path, 'frontend')
    front_dis_dir = os.path.join(front_dir, "plugins", plugin_name)
    os.makedirs(front_dis_dir, exist_ok=True)
    front_zip = os.path.join(PLUGIN_INSTALL_TMP_PATH, config['frontend'])
    try:
        with zipfile.ZipFile(front_zip, 'r') as zip_file:
            zip_file.extractall(front_dis_dir)
    except Exception as _:
        logging.error("Extract failed")
        return False
    return True


def plugin_install_backend(profiler_path, plugin_name, config):
    backend_dir = os.path.join(profiler_path, 'server')
    os_info = platform.platform()
    backend_so = config['backend_x86'] if os_info.find('x86_64') > -1 else config['backend_arm']
    os.makedirs(os.path.join(backend_dir, "plugins", plugin_name), exist_ok=True)
    backend_dist = os.path.join(backend_dir, "plugins", plugin_name, backend_so)
    if not os.path.exists(os.path.join(PLUGIN_INSTALL_TMP_PATH, backend_so)):
        logging.error("Target plugin library not exist")
        return False
    shutil.copyfile(os.path.join(PLUGIN_INSTALL_TMP_PATH, backend_so), backend_dist)
    return True


def set_mac_app_signature_certificate_id(framework: str):
    Const.MAC_SIGNATURE_CERTIFICATE_ID = Const.MAC_ARM_SIGNATURE_CERTIFICATE_ID \
        if 'aarch64' in framework else Const.MAC_X86_SIGNATURE_CERTIFICATE_ID


def main():
    logging.basicConfig(level=logging.INFO)
    idea_version = load_version_info('7.0.RC3')
    update_plugins_version(idea_version)
    # vscode_version不允许存在字母，因此这里做进一步处理，将字母内容去掉
    vscode_version = ''.join(ch for ch in idea_version if not ch.isalpha())
    init()
    os_info = platform.platform()
    framework = 'x86_64' if os_info.find('x86_64') > -1 else 'aarch64'
    os_name = 'linux-' + framework
    if os_info.find(Const.WINDOWS_OS) > -1:
        os_name = 'win'
    elif os_info.find('mac') > -1:
        os_name = 'darwin-' + framework
    result = build_server()
    if result != 0:
        logging.error('Failed to build server.')
        return 1
    result = build_frontend()
    if result != 0:
        logging.error('Failed to build frontend.')
        return 1

    return build_product_parallel(vscode_version, idea_version, os_name)


if __name__ == "__main__":
    sys.exit(main())
