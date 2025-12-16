#!/usr/bin/env python
# -*- coding: UTF-8 -*-

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

"""
build macos program
由于CI缺少MacOS版本的构建工具，因此构建MacOS程序，需要借助一台额外的机器，可以是Linux
"""
import configparser
import logging
import os
import shutil
import sys
import zipfile
from enum import Enum
import threading
import queue

import paramiko
from paramiko.ssh_exception import SSHException

SLASH = '/'  # slash for macos

PROJECT_PATH = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
MINDSTUDIO_INSIGHT = 'MindStudio-Insight'
ZIP_FILE = 'MindStudio-Insight.zip'
EXCLUDE_DIRS = [
    "dist",
    "node_modules",
    "target",
    "out",
    ".idea",
    ".git",
    "cmake-build-debug-mingw",
    "output"
]

MANIFEST_DIR = 'manifest'
DEPENDENCY_DIR = 'dependency'
CONFIG_INI = 'config.ini'
VERSION_CONFIG_FILE = os.path.join(MANIFEST_DIR, DEPENDENCY_DIR, CONFIG_INI)
SENSITIVE_CMD_PLACEHOLDER = "{***}"


class SshConfig:
    def __init__(self, host, port, name, passwd, workspace):
        self.host = host
        self.port = port
        self.name = name
        self.passwd = passwd
        self.workspace = workspace


class Arch(Enum):
    aarch64 = 'aarch64'
    x86_64 = 'x86_64'


class BuildException(Exception):
    def __init__(self, message):
        self.message = message

    def __str__(self):
        return self.message


def read_config_file():
    """
    读取执行机ssh配置
    """
    ssh_config = {}
    config = configparser.ConfigParser()
    config_file = os.path.join(os.path.expanduser('~'), '.ssh_config')
    try:
        with open(config_file, 'r') as f:
            config.read_file(f)
    except FileNotFoundError:
        logging.error('The ssh config file does not exist.')
        return ssh_config
    except Exception as e:
        logging.error(f'Failed to read ssh config file: %s', e)
        return ssh_config

    for arch in Arch:
        try:
            port = int(config.get(arch.value, 'port'))
        except ValueError:
            logging.error('Invalid port number in ssh config file for %s.', arch.value)
            continue
        except Exception as e:
            logging.warning(f'Failed to read port number from ssh config file for %s : %s', arch.value, e)
            continue

        hostname = config.get(arch.value, 'hostname', fallback=None)
        username = config.get(arch.value, 'username', fallback=None)
        password = config.get(arch.value, 'password', fallback=None)
        workspace = config.get(arch.value, 'workspace', fallback=None)
        if not all([hostname, username, password, workspace]):
            logging.error('The ssh config file is not complete for %s.', arch.value)
            continue

        logging.info('Find new ssh config, host, %s@%s, workspace, %s', username, hostname, workspace)
        ssh_config[arch.value] = SshConfig(hostname, port, username, password, workspace)

    return ssh_config


def create_ssh_connect(ssh_config):
    """
    创建ssh连接
    """
    ssh_client = paramiko.SSHClient()
    ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy)
    ssh_client.connect(hostname=ssh_config.host, port=ssh_config.port,
                       username=ssh_config.name, password=ssh_config.passwd)
    return ssh_client


def destroy_ssh_connect(ssh_client):
    """
    销毁ssh连接
    """
    ssh_client.close()


def execute_cmd(ssh_client, cmd, timeout: int = 3*60, sensitive: bool = False, *args):
    """
    通过ssh通道，在远端执行命令，并实时打印日志

    :param ssh_client: ssh
    :param cmd: cmd
    :param timeout: 命令执行的超时时间
    :param sensitive: bool 是否包含敏感指令(包含敏感字符串，占位符{***})
    :param *args: 需要填充到敏感字符串占位符的值，按顺序填充
    :return:
    """
    log_cmd: str = cmd
    if sensitive:
        try:
            log_cmd = cmd.replace(SENSITIVE_CMD_PLACEHOLDER, '***')
            cmd = cmd.replace(SENSITIVE_CMD_PLACEHOLDER, "%s")
            cmd = cmd % args
        except TypeError as e:
            logging.error("[Sensitive Cmd] Usage error: %s" % e)
            raise BuildException("Cmd init failed") from e


    logging.info('Start to execute cmd: %s', log_cmd)

    stdin, stdout, stderr = ssh_client.exec_command(cmd, timeout=timeout)
    for line in stdout:
        logging.info(line.strip())

    exit_status = stdout.channel.recv_exit_status()

    if exit_status != 0:
        stderr_output = stderr.read().decode('utf-8')
        logging.error('Failed to execute cmd: %s, and exit: %s, detail: %s', log_cmd, exit_status, stderr_output)
        raise BuildException("Command execute failed")
    else:
        logging.info('Finish to execute cmd: %s', log_cmd)


def transfer_remote_and_unzip(ssh_client, workspace):
    """
    压缩工程文件，拷贝至远端，然后解压

    :param ssh_client: ssh
    :param workspace: workspace
    :return:
    """
    local_path = os.path.join(os.path.dirname(PROJECT_PATH), ZIP_FILE)
    remote_path = workspace + SLASH + ZIP_FILE
    sftp = ssh_client.open_sftp()
    sftp.put(local_path, remote_path)
    sftp.close()
    logging.info('Copy local file %s to remote %s', local_path, remote_path)
    cmd = 'unzip -d ' + workspace + SLASH + MINDSTUDIO_INSIGHT + ' ' + remote_path + ' || return 0'
    execute_cmd(ssh_client, cmd)
    # 拷贝manifest下的config.ini文件，参见 @init_local_workspace 函数
    ver_config_file = os.path.join(os.path.dirname(PROJECT_PATH), VERSION_CONFIG_FILE)
    if os.path.exists(ver_config_file):
        dependency_path = workspace + SLASH + MANIFEST_DIR + SLASH + DEPENDENCY_DIR
        config_file_path = workspace + SLASH + MINDSTUDIO_INSIGHT + SLASH + CONFIG_INI
        cmd = 'mkdir -p ' + dependency_path + ' && cp ' + config_file_path + ' ' + dependency_path
        execute_cmd(ssh_client, cmd)


def init_remote_workspace(ssh_client, workspace):
    """
    在执行机上创建workspace
    """
    cmd = 'rm -rf ' + workspace + ' && mkdir -p ' + workspace
    execute_cmd(ssh_client, cmd)


def clean_remote_workspace(ssh_client, workspace):
    """
    构建完成后，清理执行机上的workspace
    """
    cmd = 'chmod -R +w ' + workspace + ' && rm -rf ' + workspace
    execute_cmd(ssh_client, cmd)


def build_project(ssh_client, workspace):
    """
    在执行机上构建MindStudio-Insight
    """
    build_dir = workspace + SLASH + MINDSTUDIO_INSIGHT + SLASH + 'build'
    log_file = build_dir + SLASH + 'daily_build_mac.log'
    cmd = 'cd ' + build_dir + '&& source ~/.bash_profile && python3 -u build.py 2>&1'
    execute_cmd(ssh_client, cmd, timeout=20 * 60)


def copy_file_back(ssh_client, workspace):
    """
    构建完成后将构建结果拷贝回本地
    """
    local_dir = os.path.join(PROJECT_PATH, 'out')
    output_dir = workspace + SLASH + MINDSTUDIO_INSIGHT + SLASH + 'out'
    sftp = ssh_client.open_sftp()
    if len(sftp.listdir(output_dir)) == 0:
        logging.error("Remote product file not found")
        raise BuildException("No product file found")
    for file in sftp.listdir(output_dir):
        remote_path = f"{output_dir}{SLASH}{file}"
        local_path = os.path.join(local_dir, file)
        sftp.get(remote_path, local_path)
        logging.info('Copy remote file %s to local %s', remote_path, local_path)
    sftp.close()


def unlock_signature_keychain(ssh_client, ssh_config):
    """
    远程ssh链接需要解锁签名钥匙串，以供后续签名使用
    """
    cmd = "security unlock-keychain -p %s ~/Library/Keychains/login.keychain-db" % SENSITIVE_CMD_PLACEHOLDER
    execute_cmd(ssh_client, cmd, 3*60, True, ssh_config.passwd)


def init_local_workspace():
    """
    清理上一次构建结果，并打包源码
    """
    local_dir = os.path.join(PROJECT_PATH, 'out')
    if os.path.exists(local_dir):
        shutil.rmtree(local_dir)
    os.mkdir(local_dir)
    local_path = os.path.join(os.path.dirname(PROJECT_PATH), ZIP_FILE)
    if os.path.exists(local_path):
        os.remove(local_path)
    # B版本配置文件，用于修正发布件版本号
    ver_config_file = os.path.join(os.path.dirname(PROJECT_PATH), VERSION_CONFIG_FILE)
    if os.path.exists(ver_config_file):
        shutil.copyfile(ver_config_file, os.path.join(PROJECT_PATH, CONFIG_INI))
    else:
        logging.warning('Can not find config.ini, and use default version configuration.')

    with zipfile.ZipFile(local_path, "w", zipfile.ZIP_DEFLATED) as zipf:
        for root, dirs, files in os.walk(PROJECT_PATH):
            dirs[:] = [d for d in dirs if d not in EXCLUDE_DIRS]

            for file in files:
                # 用于规避IDE类型检查的warning
                root_str = str(root)
                file_str = str(file)
                project_path_str = str(PROJECT_PATH)

                zip_path = os.path.relpath(os.path.join(root_str, file_str), project_path_str)
                zipf.write(os.path.join(root_str, file_str), arcname=zip_path)


def build_mac(arch, ssh_config, result_queue):
    result = 0
    logging.info('Start to build Ascend Insight for %s on %s.', arch, ssh_config.host)
    try:
        ssh = create_ssh_connect(ssh_config)
    except Exception as e:
        logging.error('Failed to connect ssh for %s : %s', arch, e)
        result_queue.put(-1)
        return

    try:
        init_remote_workspace(ssh, ssh_config.workspace)
        transfer_remote_and_unzip(ssh, ssh_config.workspace)
        unlock_signature_keychain(ssh, ssh_config)
        build_project(ssh, ssh_config.workspace)
        copy_file_back(ssh, ssh_config.workspace)
        clean_remote_workspace(ssh, ssh_config.workspace)
        logging.info('Finish to build Ascend Insight for %s on %s.', arch, ssh_config.host)
    except (SSHException, BuildException) as e:
        logging.error('Failed to build Ascend Insight for %s on %s : %s', arch, ssh_config.host, e)
        result = -1
    finally:
        destroy_ssh_connect(ssh)
    result_queue.put(result)


def main():
    logging.basicConfig(level=logging.INFO)
    ssh_configs = read_config_file()
    if len(ssh_configs) == 0:
        logging.error('Failed to get ssh config')
        return -1
    init_local_workspace()
    # 创建一个队列用于存放结果
    result_queue = queue.Queue()
    # 创建线程列表
    threads = []
    for arch, ssh_config in ssh_configs.items():
        thread = threading.Thread(target=build_mac, args=(arch, ssh_config, result_queue))
        thread.start()
        threads.append(thread)
    # 等待所有线程完成
    for thread in threads:
        thread.join()
    while not result_queue.empty():
        if result_queue.get() != 0:
            return -1
    return 0


if __name__ == '__main__':
    sys.exit(main())
