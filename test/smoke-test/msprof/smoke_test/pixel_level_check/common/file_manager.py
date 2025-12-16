#!/usr/bin/python3
# -*- coding: utf-8 -*-
# Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.

import json
import pandas as pd
from pandas import DataFrame
from typing import List, Dict


class FileManager:
    """
    读取json和db等
    """

    @classmethod
    def read_json_file(cls, file_path: str) -> List[Dict]:
        """
        读取json
        """
        try:
            with open(file_path, "r") as json_file:
                result_data = json.loads(json_file.read())
        except Exception as e:
            raise RuntimeError(f"Failed to read the file: {file_path}") from e
        return result_data

    @classmethod
    def read_csv_file(cls, file_path: str) -> DataFrame:
        """
        读取json
        """
        try:
            summary_data = pd.read_csv(file_path)
        except Exception as e:
            raise RuntimeError(f"Failed to read the file: {file_path}") from e
        return summary_data
