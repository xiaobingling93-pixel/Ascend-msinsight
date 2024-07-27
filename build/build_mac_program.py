#!/usr/bin/env python
# -*- coding: UTF-8 -*-

# Copyright 2023 Huawei Technologies Co., Ltd
# ============================================================================

"""
build macos program
由于CI缺少MacOS版本的构建工具，因此构建MacOS程序，需要借助一台额外的机器，可以是Linux
"""
import configparser
import logging
import os
import shutil
import sys
from enum import Enum
import threading
import queue

import paramiko
from paramiko.ssh_exception import SSHException

SLASH = '/'  # slash for macos

PROJECT_PATH = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
ASCEND_INSIGHT = 'Ascend-Insight'
ZIP_FILE = 'Ascend-Insight.zip'

MANIFEST_DIR = 'manifest'
DEPENDENCY_DIR = 'dependency'
CONFIG_INI = 'config.ini'
VERSION_CONFIG_FILE = os.path.join(MANIFEST_DIR, DEPENDENCY_DIR, CONFIG_INI)


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


def execute_cmd(ssh_client, cmd):
    """
    通过ssh通道，在远端执行命令，并实时打印日志

    :param ssh_client: ssh
    :param cmd: cmd
    :return:
    """
    logging.info('Start to execute cmd: %s', cmd)
    stdin, stdout, stderr = ssh_client.exec_command(cmd)
    for line in iter(stdout.readline, ""):
        logging.info(line)
    exit_status = stdout.channel.recv_exit_status()

    if exit_status != 0:
        logging.error('Failed to execute cmd: %s, and exit: %s', cmd, exit_status)
        raise SSHException
    else:
        logging.info('Finish to execute cmd: %s', cmd)


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
    cmd = 'unzip -d ' + workspace + SLASH + ASCEND_INSIGHT + ' ' + remote_path + ' || return 0'
    execute_cmd(ssh_client, cmd)
    # 拷贝manifest下的config.ini文件，参见 @init_local_workspace 函数
    ver_config_file = os.path.join(os.path.dirname(PROJECT_PATH), VERSION_CONFIG_FILE)
    if os.path.exists(ver_config_file):
        dependency_path = workspace + SLASH + MANIFEST_DIR + SLASH + DEPENDENCY_DIR
        config_file_path = workspace + SLASH + ASCEND_INSIGHT + SLASH + CONFIG_INI
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
    在执行机上构建Ascend Insight
    """
    build_dir = workspace + SLASH + ASCEND_INSIGHT + SLASH + 'build'
    cmd = 'cd ' + build_dir + '&& source ~/.bash_profile && python3 build.py'
    execute_cmd(ssh_client, cmd)


def copy_file_back(ssh_client, workspace):
    """
    构建完成后将构建结果拷贝回本地
    """
    local_dir = os.path.join(PROJECT_PATH, 'out')
    output_dir = workspace + SLASH + ASCEND_INSIGHT + SLASH + 'out'
    sftp = ssh_client.open_sftp()
    for file in sftp.listdir(output_dir):
        remote_path = output_dir + SLASH + file
        local_path = os.path.join(local_dir, file)
        sftp.get(remote_path, local_path)
        logging.info('Copy remote file %s to local %s', remote_path, local_path)
    sftp.close()


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
    shutil.make_archive(local_path[:-4], 'zip', PROJECT_PATH)


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
        build_project(ssh, ssh_config.workspace)
        copy_file_back(ssh, ssh_config.workspace)
        clean_remote_workspace(ssh, ssh_config.workspace)
        logging.info('Finish to build Ascend Insight for %s on %s.', arch, ssh_config.host)
    except SSHException as e:
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
