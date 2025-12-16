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
import logging
import os
from logging.handlers import RotatingFileHandler
import common

DEFAULT_LOG_MODULE_NAME = 'PROFILER_PROXY_SERVER'


class LogUtil:
    _loggers = {}

    @classmethod
    def get_logger(cls, module_name=DEFAULT_LOG_MODULE_NAME):
        if module_name not in cls._loggers:
            cls._loggers[module_name] = cls._create_logger(module_name)
        return cls._loggers[module_name]

    @classmethod
    def _create_logger(cls, module_name=DEFAULT_LOG_MODULE_NAME):
        # 创建日志记录器
        new_logger = logging.getLogger(module_name)
        new_logger.setLevel(logging.DEBUG)  # 设置最低日志级别

        log_filer_conf = {}
        # 创建文件处理器，支持日志轮转
        if common.LOG_PATH:
            log_filer_conf['filename'] = os.path.join(common.LOG_PATH, f'{module_name.lower()}.log')
        else:
            log_filer_conf['filename'] = f'./{module_name.lower()}.log'
        if common.LOG_SIZE:
            log_filer_conf['maxBytes'] = common.LOG_SIZE
        if common.LOG_BACKUP_COUNT:
            log_filer_conf['backupCount'] = common.LOG_BACKUP_COUNT

        file_handler = RotatingFileHandler(**log_filer_conf)
        logging.info(log_filer_conf)
        file_handler.setLevel(logging.DEBUG)

        # 创建日志格式
        formatter = logging.Formatter('[%(asctime)s][%(name)s][%(levelname)s]: %(message)s')
        file_handler.setFormatter(formatter)

        # 将处理器添加到记录器
        new_logger.addHandler(file_handler)

        return new_logger


# 延迟初始化
logging.getLogger().setLevel(logging.DEBUG)
proxy_logger = logging
server_logger = logging
common_logger = logging
logger = logging
