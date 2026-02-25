#!/usr/bin/env python
# -*- coding: UTF-8 -*-

"""
-------------------------------------------------------------------------
This file is part of the MindStudio project.
Copyright (c) 2026 Huawei Technologies Co.,Ltd.

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
-------------------------------------------------------------------------
CPU Binding Tool General Logger Utility Class
-------------------------------------------------------------------------
"""

import logging
import os

class LoggerUtils:
    """General logger utility class"""
    
    @staticmethod
    def setup_logger(name: str, level: int = logging.INFO) -> logging.Logger:
        """setup and return a logger"""
        cur_logger = logging.getLogger(name)
        if not cur_logger.handlers:
            cur_handler = logging.StreamHandler()
            formatter = logging.Formatter(
                '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
            )
            cur_handler.setFormatter(formatter)
            cur_logger.addHandler(cur_handler)
            cur_logger.setLevel(level)
        return cur_logger

    @staticmethod
    def log_file_operation_error(
        logger: logging.Logger,
        operation: str,
        filepath: str,
        error: Exception
    ) -> None:
        """log file operation error details"""
        abs_path = os.path.abspath(filepath)
        
        if isinstance(error, FileNotFoundError):
            logger.error(f"❌ {operation}失败: 文件不存在 → {abs_path}")
        elif isinstance(error, PermissionError):
            logger.error(f"❌ {operation}失败: 权限不足 → {abs_path}")
        elif isinstance(error, UnicodeDecodeError):
            logger.error(f"❌ {operation}失败: 编码错误 (非 UTF-8) → {abs_path}")
        elif isinstance(error, OSError):
            logger.error(f"❌ {operation}失败: 系统错误 → {abs_path}\n   {error}")
        else:
            logger.error(f"❌ {operation}失败: 未知错误 → {abs_path}\n   {error}")
