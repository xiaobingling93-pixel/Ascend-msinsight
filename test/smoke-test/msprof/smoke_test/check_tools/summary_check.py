#!/usr/bin/python3
# -*- coding: utf-8 -*-

from pathlib import Path
import logging

import numpy
import pandas
import json


logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


# 所有PROF数据读取的总类
class FileManager:
    @staticmethod
    def getFilesWithPrefixRecursive(prof_path, file_name):
        # 没有就返回空
        matched_ir_files = list(Path(prof_path).rglob(file_name))
        return [str(item) for item in matched_ir_files]

    @staticmethod
    def readTimelineData(file_name):
        # 读取json数据存储在代码里
        pass

# 具体类型和开关要以及校验什么表格，都放在testcase自己的校验里，

# 没有特殊校验的，调用基类check，有特殊校验的，调用子类check

# csv表格的数据校验基类,芯片类型作为一个全局变量，考虑添加子类，区分910A\910B，因为有些函数需要重写
class CsvDataBaseCheck:
    INVALID_NA = "N/A"
    def __init__(self, prof_path, file_name, model: str = "", scene: str = ""):
        self.prof_path = prof_path
        self.file_name = file_name
        self.cur_file_path = None
        # 获取当前Prof里需要校验的文件,有文件才校验，没有就不校验
        self.files = []
        # 采集类型
        self.model = model
        # 采集开关
        self.scene = scene
        # csv 表的内容，可能当前结果里不止一条，存储在一个字典里
        self.contexts = {}
        self.err_cnt = 0

    def getErrCnt(self):
        return self.err_cnt

    def getFiles(self):
        return self.files

    def getContext(self):
        return self.contexts

    def getfunsToCheck(self):
        # 根据采集类型和采集开关，确定校验函数
        funs_to_check = [self.isTableHeaderCorreect]
        return funs_to_check

    def isTableHeaderCorreect(self):
        pass

    def checkAllPathSummary(self):
        # 获取需要校验的列
        self.files = FileManager.getFilesWithPrefixRecursive(self.prof_path, self.file_name)
        logging.info("------------------------------- "
                         "校验如下的路径的 {} 数据 -------------------------------".format(self.files))
        # 一个prof文件里，op_summary不止有一个，比如单进程多线程的情况下
        for file in self.files:
            # check的时候，记录基础数据、作为后续表格之间校验的基础
            self.checkDataByPath(file)
            
    # 对op_summary的基础数据校验,先校验最基本的后面根据芯片类型和开关不同，做不同的校验，表驱动或者工厂
    def checkDataByPath(self, filePath):
        # 获取需要校验的列
        funs_to_check = self.getfunsToCheck(filePath)
        summary_data = pandas.read_csv(filePath, na_values=[self.INVALID_NA])
        self.cur_file_path = filePath
        if summary_data.empty:
            logging.info("没有符合要求的csv表格数据或者表格为空，请排查您的PROFILING数据")
        # summary 在一个out里可能有多条，比如device0/summary device1/summary msprof/output 等等
        # 用一个字典去存,key是路径，例如 /home/result_dir/test_App_IsSwitchHost-sys/PROF_000001_20231101110753741_PHFECGHOEBBAMENA/device_0/summary/
        self.contexts[filePath] = summary_data
        for fun in funs_to_check:
            if not fun(summary_data):
                self.err_cnt = self.err_cnt + 1


class OpSummaryDataCheck(CsvDataBaseCheck):
    def __init__(self, prof_path, model: str = "", scene: str = ""):
        super().__init__(prof_path, "op_summary*.csv", model, scene)

    # 表驱动，方便增删，if else太多的话，可读性比较差，维护起来麻烦
    def getfunsToCheck(self, file_path):
        check_funs_dict = {
            "ffts_off_hccl_l0": [self.isOpNameCorrect, ],
            "ffts_on_hccl_l0": [self.isOpNameCorrect, ],
            "dev_summary_check": [
                self.isOpNameCorrect,
                self.isOpTypeCorrect,
                self.check_task_duration,
                self.compareOpShapeTypeConsistent
            ],
            "mindstudio_profiler_output_function": [
                self.isOpNameCorrect,
                self.isOpTypeCorrect,
                self.compareOpShapeTypeConsistent
            ]
        }

        # 根据采集类型和采集开关，确定校验函数
        logging.info(self.scene)
        if self.scene in ("ffts_off_hccl_l0", "ffts_on_hccl_l0"):
            # 校验数据是不是N/A,比较特殊的情况，单独放在这里
            return check_funs_dict.get(self.scene, [])
        
        if "device" in file_path:
            # 只有device路径下的数据才需要校验 task_duration的有效性，其他的不需要
            return check_funs_dict["dev_summary_check"]
        else:
            return check_funs_dict["mindstudio_profiler_output_function"]

    # opName不能是数字、None、N/A
    def isOpNameCorrect(self, summary_data):
        for name in summary_data["Op Name"]:
            if name is None or pandas.isna(name) or name.isdigit():
                logging.error("OP name 存在数字或非法值{}，请检查表格".format(name))
                return False
        return True

    # opType不能是数字、None
    def isOpTypeCorrect(self, summary_data):
        for op_type in summary_data["OP Type"]:
            if pandas.isna(op_type):
                continue
            if op_type is None or op_type.isdigit():
                logging.error("OP Type 存在数字或非法值{}，请检查表格".format(op_type))
                return False
        return True

    def compareOpShapeTypeConsistent(self, summary_data):
        """ op_summary: shape数据和';'个数校验 """
        logging.info("start check Shapes in op_summary*.csv")
        column_list = [
            "Input Shapes", "Input Data Types", "Input Formats",
            "Output Shapes", "Output Data Types", "Output Formats"
        ]
        for column in column_list:
            if column not in summary_data.columns:
                logging.error("The column {} is not existed".format(column))
                return False
        if 'Input Shapes' in summary_data.columns:
            for input_shape, input_data_type in zip(summary_data['Input Shapes'],
                                                    summary_data['Input Data Types']):
                if pandas.isnull(input_shape):
                    continue
                if input_shape.count(";") != input_data_type.count(";"):
                    logging.error("'Input Shapes' not equal to 'Input Data Types'")
                    return False
        if 'Output Shapes' in summary_data.columns:
            for output_shape, output_data_type in zip(summary_data['Output Shapes'],
                                                      summary_data['Output Data Types']):
                if pandas.isnull(output_shape):
                    continue
                if output_data_type.count(";") != output_data_type.count(";"):
                    logging.error("'Onput Shapes' not equal to 'Output Data Types'")
                    return False
        logging.info("check Shapes in op_summary*.csv success!")
        return True

    def check_task_duration(self, summary_data):
        logging.info("start check 'Task Duration(us)' in op_summary*.csv")
        info_json_dir = self.cur_file_path.rsplit('/', 2)[0]
        info_json_path = list(Path(info_json_dir).rglob("info.json*[!done]"))[0]
        if "aicore_time(us)" not in summary_data.columns:
            logging.info("'aicore_time(us)' data is not existed!")
            return True
        df = summary_data[summary_data["aicore_time(us)"].notnull()]
        df_aic = df['Task Duration(us)'] > df['aicore_time(us)']
        if not df_aic.all():
            total_cycles_data = df["total_cycles"].values
            block_dim_data = df["Block Dim"].values
            aicore_time_data = df["aicore_time(us)"].values
            with open(info_json_path, 'r') as f:
                info_data = json.load(f)
            device_info = info_data.get("DeviceInfo")
            ai_core_num = device_info[0].get("ai_core_num")
            aic_frequency = device_info[0].get("aic_frequency")
            if not all([device_info, ai_core_num, aic_frequency]):
                logging.error("can not get 'DeviceInfo' or 'ai_core_num' or 'aic_frequency'!")
                return False
            freq = total_cycles_data / aicore_time_data / block_dim_data * \
                   ((block_dim_data + ai_core_num - 1) // ai_core_num)
            if not (abs(freq - int(aic_frequency)) < int(aic_frequency) * 0.1).all():
                return False
        if "aiv_time(us)" in df.columns:
            df_aiv = df['Task Duration(us)'] > df['aiv_time(us)']
            if not df_aiv.all():
                logging.error("invalid rows in table: {}".format(df_aiv.index[df_aiv == False].tolist()))
                return False
        logging.info("check 'Task Duration(us)' in op_summary*.csv success!")
        return True

    # op_type和算子类型应该对应
    def checkOptypeCorrespondToOp(self):
        pass


# 交互件之间关系校验
class DeliverablesCheck:
    def __init__(self, prof_path, model: str = "", scene: str = ""):
        self.prof_path = prof_path
        # 采集类型
        self.model = model
        # 采集开关
        self.scene = scene

        self.check = []

    # 判断需要校验哪些内容、校验
    def checkResult(self):
        pass

# 靠配置去新建csv