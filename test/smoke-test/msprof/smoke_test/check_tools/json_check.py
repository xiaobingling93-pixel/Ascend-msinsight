#!/usr/bin/python3
# -*- coding: utf-8 -*-
# Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
import pandas as pd
import numpy as np
import logging
import os
import glob

from check_tools.json_header_manager import JSONHeaderManager

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


def is_num(s):
    try:
        float(s)
        return True
    except ValueError:
        return False


def is_nan(s):
    try:
        np.isnan(s)
        return True
    except TypeError:
        return False


class MsprofJsonChecker:
    # chip trans type
    STARS_TID = ["111111", "100010"]
    # 纯解析用例不校验时间
    SCENE_WHITE_LIST = ["test_Analysis_ffts_on_hccl_l1_data", "test_Analysis_ffts_on_hccl_l0_data",
                        "test_Analysis_ffts_off_hccl_l0_data", "test_Analysis_ffts_off_hccl_l1_data",
                        "test_Analysis_hccl_graph", "Analysis_Helper_data"]

    MSPROF_JSON = "msprof*.json"
    MAX_JSON_SIZE = 1 * 1024 * 1024 * 1024
    KEY_LIST = ["name", "pid", "tid", "ts", "dur", "args", "ph", "cat", "id", "bp"]
    TYPE_LIST = [str, int, int, str, float, dict, str, str, str, str]
    MAX_STREAM_CNT = 1000
    MAX_THREAD_CNT = 1000

    def __init__(self, output: str, start, end, id: str, chip: str):
        self.output = output
        self.file_list = []
        self.logger = logging
        self.end_timestamp_us = end
        self.start_timestamp_us = start
        self.id = id
        self.current_time_max = 0.0
        self.last_time_max = 0.0
        self.expect_header = JSONHeaderManager.init_header_config(chip).get(id, [])

    def check(self):
        self._filter_json_file()
        if not self.file_list:
            self.logger.error("the path {} has no msprof*.json".format(self.output))
        self.file_list.sort()
        for file in self.file_list:
            df = self._read_json_file(file)
            if not self._check_timeline_data(df):
                return False
        self.logger.info("JSON check finished.")
        return True

    def _filter_json_file(self):
        """
        从mindstudio_profiler_output目录下筛选出msprof*.json文件
        """
        search_pattern = os.path.join(self.output, self.MSPROF_JSON)
        for file in glob.glob(search_pattern):
            self.file_list.append(file)

    def _read_json_file(self, file: str) -> any:
        if not os.path.isfile(file):
            self.logger.error("the file {} is not exist".format(file))
            return None
        if os.path.getsize(file) > self.MAX_JSON_SIZE:
            self.logger.error("the size of file {} is over 1 G")
            return None
        df = None
        try:
            df = pd.read_json(file, dtype=np.dtype)
        except Exception as e:
            self.logger.error("read {0} failed, errmsg is {1}".format(file, e.args))
        return df

    def _check_timeline_data(self, df: any):
        if df is None:
            self.logger.error("Json is None.")
            return False
        res_header = set()
        stream_set = set()
        thread_set = set()
        for _, row in df.iterrows():
            # 校验类型
            if not self._check_data_type(row):
                return False
            # 校验process_name类型是否齐全
            if not self._check_type_complete(row, res_header):
                return False
            # 校验时间戳
            if not self._check_timestamp(row):
                return
            # Stream数量校验
            self._check_stream_thread_cnt(row, stream_set, thread_set)
        if len(res_header) != len(self.expect_header):
            self.logger.error("Json check failed! The current json header is {0},"
                              " but expect json header is {1}".format(res_header, self.expect_header))
            return False
        thread_cnt = len(thread_set)
        stream_cnt = len(stream_set)
        if thread_cnt > self.MAX_THREAD_CNT:
            self.logger.error("Json check failed! The number of threads: {} "
                              "exceeds the maximum: {}.".format(thread_cnt, self.MAX_THREAD_CNT))
            return False
        if stream_cnt > self.MAX_STREAM_CNT:
            self.logger.error("Json check failed! The number of streams: {} "
                              "exceeds the maximum: {}.".format(stream_cnt, self.MAX_STREAM_CNT))
            return False
        return True

    def _check_stream_thread_cnt(self, row, stream_set: set, thread_set: set):
        if row["name"] != "thread_name":
            return True
        args_value = row["args"]["name"]
        if args_value.startswith("Stream"):
            stream_set.add(args_value)
        if args_value.startswith("Thread"):
            thread_set.add(args_value)

    def _check_timestamp(self, row):
        if self.id in self.SCENE_WHITE_LIST:
            return True
        timestamp = row.get("ts")
        timestamp = timestamp if not is_nan(timestamp) else None
        dur = row.get("dur")
        dur = dur if dur else 0.0
        # 校验时间是否都在采集的时间范围内
        if timestamp and not self.end_timestamp_us > float(timestamp) > self.start_timestamp_us:
            self.logger.error("Event {0} timestamp {1} is not between start time {2} and end"
                              " time {3}".format(row.get("name", "INVALID_NAME"), timestamp,
                                                 self.start_timestamp_us, self.end_timestamp_us))
            return False
        # 校验当期时间戳是否大于上一个切片最大时间
        if timestamp and float(timestamp) < self.last_time_max:
            self.logger.error("Event {0} timestamp {1} is earlier than last slice max time {2} ."
                              .format(row.get("name", "INVALID_NAME"), timestamp, self.last_time_max))
            return False
        if timestamp and float(timestamp) + dur > self.current_time_max:
            self.current_time_max = float(timestamp) + dur
        return True

    def _check_data_type(self, row):
        flag = True
        for index in range(len(self.KEY_LIST)):
            value = row.get(self.KEY_LIST[index], None)
            # pd读取json文件会取表头的最大集，因此不是所有的表头都有值，可能是NaN（业务值不会有NaN）
            key = self.KEY_LIST[index]
            if not value or is_nan(value):
                continue
            if value in self.STARS_TID:
                # PA Link 类型 tid是字符串，不校验类型
                continue
            if key in ["ts"] and not is_num(value):
                self.logger.error(
                    "The data {} value type of data {} must be number, invalid".format(value, key))
                flag = False
                break
            if isinstance(value, self.TYPE_LIST[index]):
                if (self.TYPE_LIST[index] == int or self.TYPE_LIST[index] == float) and value < 0:
                    self.logger.error(
                        "The data {} value type of data {} must be {}, "
                        "the value type is negative, invalid.".format(value, key, self.TYPE_LIST[index]))
                    flag = False
                    break
                continue
            self.logger.error("The data {} value {} type is {}, not expected {}"
                              .format(key, value, type(value), self.TYPE_LIST[index]))
            flag = False
            break
        return flag

    def _check_type_complete(self, row: any, res_header: set):
        if not self.expect_header:
            return True
        if row["name"] != "process_name":
            return True
        args_value = row["args"]["name"]
        if args_value in self.expect_header:
            res_header.add(args_value)
            return True
        else:
            self.logger.error("This Json has a header {}, which is not expected".format(args_value))
            return False
