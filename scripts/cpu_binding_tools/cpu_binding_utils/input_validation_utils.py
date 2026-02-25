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
CPU Binding Tool General Input Validation Utility Class
-------
"""

import re
from typing import List, Optional
import logging


class InputValidationUtils:
    """
    外部输入安全校验工具类
    用于过滤 CLI / 配置文件 / API 输入的非法参数
    """

    # 允许的普通进程关键字字符
    SAFE_KEYWORD_PATTERN = re.compile(r"^[a-zA-Z0-9_\-\.]{1,64}$")

    # 最大关键字数量限制（防止滥用）
    MAX_KEYWORD_COUNT = 100

    @staticmethod
    def validate_keyword(
        keyword: str,
        logger: Optional[logging.Logger] = None
    ) -> Optional[str]:
        """
        校验单个关键字是否合法
        返回合法 keyword（原样），否则返回 None
        """

        if not isinstance(keyword, str):
            if logger:
                logger.warning(f"非法 keyword 类型: {keyword}")
            return None

        keyword = keyword.strip()

        if not keyword:
            return None

        if not InputValidationUtils.SAFE_KEYWORD_PATTERN.match(keyword):
            if logger:
                logger.warning(
                    f"忽略非法 keyword（包含危险字符）: {keyword}"
                )
            return None

        return keyword

    @staticmethod
    def sanitize_keywords(
        keywords: List[str],
        logger: Optional[logging.Logger] = None
    ) -> List[str]:
        """
        批量校验 keyword 列表
        自动过滤非法值
        """

        if not keywords:
            return []

        if len(keywords) > InputValidationUtils.MAX_KEYWORD_COUNT:
            if logger:
                logger.warning(
                    f"keyword 数量超过限制 {InputValidationUtils.MAX_KEYWORD_COUNT}，将截断"
                )
            keywords = keywords[:InputValidationUtils.MAX_KEYWORD_COUNT]

        safe_list = []

        for kw in keywords:
            safe_kw = InputValidationUtils.validate_keyword(kw, logger)
            if safe_kw:
                safe_list.append(safe_kw.lower())

        return safe_list

    @staticmethod
    def safe_compile_regex(
        pattern: str,
        logger: Optional[logging.Logger] = None
    ) -> Optional[re.Pattern]:
        """
        安全编译正则表达式
        - 捕获异常
        - 限制长度
        """

        if not pattern or len(pattern) > 128:
            if logger:
                logger.warning("非法或过长的正则表达式")
            return None

        try:
            return re.compile(pattern)
        except re.error as e:
            if logger:
                logger.warning(f"正则表达式编译失败: {e}")
            return None
