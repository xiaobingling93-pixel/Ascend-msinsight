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
CPU Binding Tool General File Utility Class
-------
"""

from typing import Optional
import logging

from .logger_utils import LoggerUtils


class FileUtils:
    """General file utility class"""

    @staticmethod
    def safe_read_file(
        logger: logging.Logger,
        filepath: str,
        encoding: str = "utf-8",
        operation_name: str = "Read"
    ) -> Optional[str]:
        """read file content safely"""
        try:
            with open(filepath, "r", encoding=encoding) as f:
                return f.read()
        except Exception as e:
            LoggerUtils.log_file_operation_error(
                logger, operation_name, filepath, e
            )
            return None

    @staticmethod
    def safe_write_file(
        logger: logging.Logger,
        filepath: str,
        content: str,
        encoding: str = "utf-8"
    ) -> bool:
        """write file content safely"""
        try:
            with open(filepath, "w", encoding=encoding) as f:
                f.write(content)
            return True
        except Exception as e:
            LoggerUtils.log_file_operation_error(
                logger, "Write", filepath, e
            )
            return False