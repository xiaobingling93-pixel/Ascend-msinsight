#!/usr/bin/env python
# -*- coding: UTF-8 -*-
#  Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
import os
import logging
import argparse
import platform
import sys
import shutil
import zipfile
import json
import stat

logging.basicConfig(level=logging.INFO)

PROFILER_DIR = os.path.dirname(os.path.abspath(__file__))
MINDSTUDIO_INSIGHT_DIR = os.path.dirname(os.path.dirname(PROFILER_DIR))
MAX_ZIP_FILE_SIZE = 1024 * 1024 * 1024  # 1GB
MAX_DEPTH = 14
MAX_UNZIP_COUNT = 100000

if sys.platform == 'win32':
    CACHE_DIR = os.path.join(MINDSTUDIO_INSIGHT_DIR, '.mindstudio_insight')
else:
    CACHE_DIR = os.path.join(os.getenv('HOME'), '.mindstudio_insight')


def common_path_check(path: str, exist_only=True):
    """
    :param exist_only: 是否进行文件是否存在和文件读写权限校验
    :param path: 路径
    :brief: 检查路径长度、非法字符、路径是否存在、软连接、读写权限、属主校验
    :return:
    """
    if len(path) > 260:
        logging.error("Path length exceed the limit 260")
        return False
    if sys.platform == "win32":
        invalid_char = [
            "\n", "\f", "\r", "\b", "\t", "\v", "\x7F", "\u007F", "\"", "\'", "%", ">", "<", "|", "&", "$",
            ";", "`"
        ]
    else:
        invalid_char = [
            "\n", "\f", "\r", "\b", "\t", "\v", "\x7F", "\u007F", "\"", "\'", "%", ">", "<", "|", "&", "$",
            ";", "`", "\\"
        ]
    if any(char in invalid_char for char in path):
        logging.error("Path contains invail charactor")
        return False
    if not exist_only:
        return True
    if not os.path.exists(path):
        logging.error("Path not exist")
        return False
    if os.path.islink(path):
        logging.error("Path is a soft link, not allowed")
        return False
    if not os.access(path, os.R_OK | os.W_OK):
        logging.error("Need path has read and write permission")
        return False
    if sys.platform != "win32" and os.getuid() != os.stat(path).st_uid:
        logging.error("File or Dir owner is not current user")
        return False
    return True


def is_within_directory(src, dst):
    """ 检查是否存在压缩跨路径覆盖攻击"""
    abs_src_dir = os.path.abspath(os.path.join(dst, src))
    abs_dst_dir = os.path.abspath(dst)
    return os.path.commonpath([abs_dst_dir, abs_src_dir]) == abs_dst_dir


def is_symlink(file_info):
    # windows平台无软链接问题
    return stat.S_ISLNK(file_info.external_attr >> 16)


def unzip_safety(zip_file: str, dist_path):
    max_extract_count = 1000
    max_extract_file_size = 200 * 1024 * 1024  # 200MB
    with zipfile.ZipFile(zip_file, 'r') as zip_file:
        file_list = zip_file.namelist()
        for file in file_list:
            if zip_file.getinfo(file).file_size > max_extract_file_size:
                logging.error(f"File size exceeds max extract file limit(200MB), file={file}")
                return False
            if is_symlink(zip_file.getinfo(file)):
                logging.error("Detect extract symbol link file")
                return False
            if not is_within_directory(file, dist_path):
                logging.error("Detected extract file cross dist path")
                return False
            if not common_path_check(os.path.join(dist_path, file), exist_only=False):
                logging.error("Extract file path is invalided")
                return False
            if max_extract_count == 0:
                logging.error("Extract file count exceeds limit")
                return False
            max_extract_count = max_extract_count - 1
            zip_file.extract(file, dist_path)
    return True


def parse_plugin_config(config_file: str):
    if not common_path_check(config_file):
        logging.error("Missing plugin config file")
        return None, None, None
    with open(config_file, 'r') as config_fp:
        try:
            config = json.load(config_fp)
        except json.JSONDecodeError as e:
            logging.error(f"Plugin config decode into json failed, error={e}")
            return None, None, None
        plugin_name = config.get("pluginName")
        frontend = config.get("frontend")
        backend_suffix = f"_{sys.platform}_{platform.machine().lower()}"
        backend = config.get("backend" + backend_suffix)
        if any(item is None for item in (plugin_name, frontend, backend)):
            logging.error("Plugin config wrong")
            return None, None, None
    return plugin_name, frontend, backend


def unzip_plugin(zip_path, dst_path):
    if not unzip_safety(zip_path, dst_path):
        logging.error("Unzip failed")
        return None, None, None
    plugin_name, frontend_name, backend_name = parse_plugin_config(os.path.join(dst_path, "config.json"))
    if any(item is None for item in (plugin_name, frontend_name, backend_name)):
        logging.error("Plugin config wrong, missing config or not support this platform")
        return None, None, None
    frontend = os.path.join(dst_path, frontend_name)
    backend = os.path.join(dst_path, backend_name)
    if not os.path.exists(frontend):
        logging.error("Missing frontend file")
        return None, None, None
    if not os.path.exists(backend):
        logging.error("Missing backend file")
        return None, None, None
    if not frontend.endswith(".zip"):
        logging.error("Frontend file not in zip")
        return None, None, None
    frontend_dst_dir = os.path.join(dst_path, "frontend")
    if not unzip_safety(frontend, frontend_dst_dir):
        logging.error("Extract frontend failed")
        return None, None, None
    backend_dst_dir = os.path.join(dst_path, "backend")
    if backend.endswith(".zip"):
        if not unzip_safety(backend, backend_dst_dir):
            logging.error("Extract backend failed")
            return None, None, None
    else:
        os.makedirs(backend_dst_dir, 0o750, exist_ok=False)
        shutil.copyfile(backend, os.path.join(backend_dst_dir, backend_name))
    return plugin_name, frontend_dst_dir, backend_dst_dir


def copy_file(plugin_name, frontend, backend):
    backend_dst = os.path.join(PROFILER_DIR, 'server', 'plugins', plugin_name)
    os.makedirs(backend_dst, exist_ok=True)
    shutil.copytree(backend, backend_dst, dirs_exist_ok=True)
    frontend_dst = os.path.join(PROFILER_DIR, 'frontend', 'plugins', plugin_name)
    shutil.copytree(frontend, frontend_dst, dirs_exist_ok=True)


def check_cache_dir():
    global CACHE_DIR
    return common_path_check(CACHE_DIR)


def check_plugin_path(plugin_path: str):
    if not common_path_check(plugin_path):
        return False
    if not os.path.isfile(plugin_path):
        logging.error("The plugin path should be a file, not a directory")
        return False
    if not plugin_path.endswith('.zip'):
        logging.error("The plugin path is not in zip format")
        return False
    return True


def install_plugin(path: str):
    global CACHE_DIR
    logging.info(f"Start to install the plugin, plugin path={path}")
    if not check_cache_dir():
        logging.error("Cache dir check failed")
        return

    if not check_plugin_path(path):
        logging.error("Plugin path check failed")
        return

    tmp_dir = os.path.join(CACHE_DIR, "plugin_install_tmp")
    if os.path.exists(tmp_dir):
        shutil.rmtree(tmp_dir)
    os.makedirs(tmp_dir, 0o750, exist_ok=False)

    plugin_name, frontend, backend = unzip_plugin(path, tmp_dir)
    if any(item is None for item in (plugin_name, frontend, backend)):
        logging.error("Unzip plugin failed")
        return
    logging.info(f"Unzip plugin success, plugin name:{plugin_name}")
    copy_file(plugin_name, frontend, backend)
    shutil.rmtree(tmp_dir)
    logging.info(f"Success install plugin {plugin_name}")
    logging.info(f'\t frontend path: {os.path.join(PROFILER_DIR, "frontend", "plugins", plugin_name)}')
    logging.info(f'\t backend path: {os.path.join(PROFILER_DIR, "server", "plugins", plugin_name)}')


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="This is a script to simply install/uninstall MindStudio Insight plugin")
    subparsers = parser.add_subparsers(dest='command', help="sub command help")
    # 安装命令
    parser_install = subparsers.add_parser('install', help='install plugin')
    parser_install.add_argument('--path', required=True, type=str, help='Path of plugin zip')
    # 卸载命令
    args = parser.parse_args()
    if args.command == 'install':
        install_plugin(args.path)
    logging.info("End")
