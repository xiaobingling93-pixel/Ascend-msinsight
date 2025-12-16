#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
import re
import json
import time
import shutil
import logging
import argparse
import unittest
import subprocess
from collections import defaultdict
from _datetime import datetime
import pandas
import sys
sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

from _cfg import *
from check_tools.summary_check import OpSummaryDataCheck
from check_tools.db_check import DBManager
from check_tools.json_check import MsprofJsonChecker
from check_tools.file_check import FileChecker

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


class TestProfiling(unittest.TestCase):
    S_TO_US = 1_000_000

    def __init__(self, tc, scene, mode, params, timeout):
        self.__id = tc
        self.__scene = scene
        self.__mode = mode
        self.__params = params
        self.__timeout = timeout
        self.__logger = logging
        super(TestProfiling, self).__init__("execute")

    def init(self):
        self.cfg_path = ConfigPaths()
        self.cfg_value = ConfigValues()
        self.cfg_cmd = ConfigCmd()
        self.cfg_switch = ConfigSwitchRes()
        self.cfg_header = ConfigTableHeaders()
        self.res_dir = os.path.join(self.cfg_path.result_path, self.__id)
        self.slog_stdout = "{}.log".format(os.path.join(self.res_dir, self.__id))
        self.res = 0
        self.switch_values = []
        self.run_path = ""
        self.data_platform = ''
        self.op_summary_op_num = 0
        self.op_summary_not_hccl_op_num = 0
        self.op_statistic_op_num = 0
        self.hccl_statistic_op_num = 0
        self.op_summary_context = pandas.DataFrame()
        self.api_statistic_context = pandas.DataFrame()
        self.hccl_statistic_context = pandas.DataFrame()
        self.op_type_baseline = {}
        self.op_statistic_context = pandas.DataFrame()
        self.op_summary_csv = OpSummaryDataCheck(self.res_dir, self.__mode, self.__scene)
        self.csv_cout_dict = dict()
        self.duration_time = 0
        self.start_timestamp_us = 0
        self.end_timestamp_us = 0
        self.__logger.info("------------------------------- "
                           "Start execute case {} -------------------------------".format(self.__id))

    def record_result(self):
        if self.res == 0:
            self.__result = self.cfg_value.pass_res
        else:
            self.__result = self.cfg_value.fail_res
        self.__logger.info("\n------------------------------- "
                           "End execute case {} -------------------------------".format(self.__id))

    def write_res(self):
        with open('result.txt', 'a+') as f:
            f.write('%s %s %s\n' % (self.__id, self.__result, self.duration_time))

    def execute(self):
        self.init()
        eval("self.{}()".format(self.__mode))
        self.record_result()
        self.write_res()
        self.assertTrue(self.cfg_value.pass_res == self.__result)

    def get_switch_value(self, switch_cmd):
        if "AclApi" == self.__scene:
            self.switch_values = self.cfg_value.aclapi_switch
        elif "AclJson" == self.__scene:
            self.switch_values = self.cfg_value.acljson_switch
        elif "AddOp" == self.__scene or "MsprofbinPyTorch" == self.__scene or "MsprofbinTensorFlow" == self.__scene \
                or "DynamicLaunch" == self.__scene or "DynamicAttach" == self.__scene \
                or "MsprofbinMindSpore" == self.__scene or self.__scene.startswith("AddKernelInvocation"):
            self.switch_values = self.cfg_value.msprofbin_switch
        elif "Msproftx" == self.__scene:
            self.switch_values = self.cfg_value.msproftx_switch
        elif "PyAclApi" == self.__scene:
            self.switch_values = self.cfg_value.pyaclapi_switch
        elif "SingleOp" == self.__scene:
            self.switch_values = self.cfg_value.opst_switch
        elif "GraphApi" == self.__scene:
            self.switch_values = self.cfg_value.graphapi_switch
        elif "CannProfilingPyTorch" == self.__scene or "InnerOneStepPyTorch" == self.__scene \
                or "SingleOp65535PyTorch" == self.__scene or "SingleOpPyTorch" == self.__scene \
                or "StepPyTorch" == self.__scene:
            self.switch_values = self.cfg_value.pytorch_switch
        elif "ExportTensorFlow" == self.__scene or "SessionRunTensorFlow" == self.__scene \
                or "LoopTensorFlow" == self.__scene or "NoLoopTensorFlow" == self.__scene \
                or "DockerExportTensorFlow" == self.__scene:
            self.switch_values = self.cfg_value.tensorflow_switch
        else:
            switch_lst = switch_cmd.split(" ")
            for switch in switch_lst:
                if switch.startswith("--"):
                    switch = switch.split("--")[1]
                    if "=on" in switch:
                        if "export" in switch:
                            continue
                        self.switch_values.append(switch)
                    elif "host-sys" in switch:
                        self.switch_values.append(switch)
                    elif "aic-mode" in switch or "llc-profiling" in switch or "aic-metrics" in switch:
                        self.switch_values.append(switch)
        self.__logger.info("switch on: {}".format(self.switch_values))
        return self.switch_values

    def get_device_msg(self):
        cmd_soc = "lspci"
        res = self.subprocess_cmd(cmd_soc)
        if "d100" in res:
            soc = "310"
        elif "d500" in res:
            soc = "310P"
        elif "d801" in res:
            soc = "910"
        elif "d802" in res:
            soc = "910B"
        elif "d107" in res:
            soc = "310B"
        else:
            soc = "invalid type"
        self.__logger.warning("current platform Ascend {}".format(soc))
        return soc

    def get_switch_res(self, switch_cmd):
        dev_json_lst = []
        dev_csv_lst = []
        host_json_lst = []
        host_csv_lst = []
        switch_res_dict = {}
        switch_values = self.get_switch_value(switch_cmd)
        try:
            for switch in switch_values:
                switch_ = switch.split("=")[0]
                res_ = switch.split("=")[1]
                if res_ == "on":
                    if "ACL_PROF_MSPROFTX" in switch_ or "TORCH_CALL_STACK" in switch_:
                        host_json_res = self.cfg_switch.switch_option[switch_]["timeline"]
                        host_json_lst.extend(host_json_res)
                        host_csv_res = self.cfg_switch.switch_option[switch_]["summary"]
                        host_csv_lst.extend(host_csv_res)
                    else:
                        dev_json_res = self.cfg_switch.switch_option[switch_]["timeline"]
                        dev_json_lst.extend(dev_json_res)
                        dev_csv_res = self.cfg_switch.switch_option[switch_]["summary"]
                        dev_csv_lst.extend(dev_csv_res)
                elif not re.findall(r"-pid", switch_) and "host-sys" in switch_ or "HOST_SYS" in switch_ \
                        or "host_sys" in switch_:
                    if "," not in res_:
                        res_ = "{},".format(res_)
                    sys_res = res_.split(",")
                    for res in sys_res:
                        if len(res) != 0:
                            host_json_res = self.cfg_switch.switch_option[switch_][res]["timeline"]
                            host_json_lst.extend(host_json_res)
                            host_csv_res = self.cfg_switch.switch_option[switch_][res]["summary"]
                            host_csv_lst.extend(host_csv_res)
                elif "llc-profiling" in switch_ or "llc_profiling" in switch_ or "ACL_PROF_LLC_MODE" in switch_:
                    dev_json_res = self.cfg_switch.switch_option[switch_][res_]["timeline"]
                    dev_json_lst.extend(dev_json_res)
                    dev_csv_res = self.cfg_switch.switch_option[switch_][res_]["summary"]
                    dev_csv_lst.extend(dev_csv_res)
        except KeyError as err:
            self.__logger.error(err)
        soc = self.get_device_msg()
        if "Sys" != self.__scene:
            dev_csv_lst = [_data for _data in dev_csv_lst if _data != "ai_cpu_pmu_events"
                           if _data != "ai_cpu_top_function" if _data != "cpu_usage"
                           if _data != "ctrl_cpu_pmu_events" if _data != "ctrl_cpu_top_function"
                           if _data != "process_cpu_usage" if _data != "process_mem" if _data != "sys_mem"]
            if soc == "310":
                dev_json_lst = [_data for _data in dev_json_lst if _data != "llc_bandwidth" if _data != "llc_aicpu"
                                if _data != "llc_ctrlcpu"]
                dev_csv_lst = [_data for _data in dev_csv_lst if _data != "llc_bandwidth" if _data != "llc_aicpu"
                               if _data != "llc_ctrlcpu"]
        if soc == "310":
            dev_json_lst = [_data for _data in dev_json_lst if _data != "roce" if _data != "nic"
                            if _data != "hbm" if _data != "hccs" if _data != "pcie"]
            dev_csv_lst = [_data for _data in dev_csv_lst if _data != "roce" if _data != "nic"
                           if _data != "hbm" if _data != "hccs" if _data != "pcie" if _data != "cache"]
        if soc == "310P":
            dev_json_lst = [_data for _data in dev_json_lst if _data != "hbm" if _data != "hccs"]
            dev_csv_lst = [_data for _data in dev_csv_lst if _data != "hbm" if _data != "hccs" if _data != "dvpp"]
        if soc == "910":
            if "MsprofbinPyTorch" == self.__scene or "InnerOneStepPyTorch" == self.__scene \
                    or "SingleOp65535PyTorch" == self.__scene \
                    or "SingleOpPyTorch" == self.__scene or "StepPyTorch" == self.__scene or "App" == self.__scene \
                    or "AclApi" == self.__scene or "AclJson" == self.__scene or "All" == self.__scene \
                    or "PyAclApi" == self.__scene or "GraphApi" == self.__scene or "DynamicLaunch" == self.__scene \
                    or "DynamicAttach" == self.__scene:
                dev_csv_lst = [_data for _data in dev_csv_lst if _data != "aicpu"]
            if "ExportTensorFlow" == self.__scene or "MsprofbinTensorFlow" == self.__scene \
                    or "SessionRunTensorFlow" == self.__scene or "LoopTensorFlow" == self.__scene \
                    or "NoLoopTensorFlow" == self.__scene:
                dev_json_lst = [_data for _data in dev_json_lst if _data != "acl"]
                dev_csv_lst = [_data for _data in dev_csv_lst if _data != "acl" if _data != "acl_statistic"
                               if _data != "aicpu" if _data != "fusion_op"]
            if "DockerExportTensorFlow" == self.__scene:
                dev_json_lst = [_data for _data in dev_json_lst if _data != "acl"]
                dev_csv_lst = [_data for _data in dev_csv_lst if _data != "acl" if _data != "acl_statistic"]
                dev_csv_lst.append("dp")
            if "CannProfilingPyTorch" == self.__scene or "InnerOneStepPyTorch" == self.__scene \
                    or "MsprofbinPyTorch" == self.__scene or "SingleOp65535PyTorch" == self.__scene \
                    or "SingleOpPyTorch" == self.__scene or "StepPyTorch" == self.__scene \
                    or "DynamicLaunch" == self.__scene or "DynamicAttach" == self.__scene:
                dev_json_lst = [_data for _data in dev_json_lst if _data != "ge"]
                dev_csv_lst = [_data for _data in dev_csv_lst if _data != "fusion_op"]
        if "aic-mode=task-based" in switch_values:
            dev_json_lst = [_data for _data in dev_json_lst if _data != "ai_core_utilization"]
            dev_csv_lst = [_data for _data in dev_csv_lst if _data != "ai_core_utilization"]
        if "AddOp" == self.__scene or "SingleOp" == self.__scene or "MsprofbinPyTorch" == self.__scene \
                or self.__scene.startswith("AddKernelInvocation") \
                or "DynamicLaunch" == self.__scene or "DynamicAttach" == self.__scene:
            dev_json_lst = [_data for _data in dev_json_lst if _data != "ge" if _data != "step_trace"]
            dev_csv_lst = [_data for _data in dev_csv_lst if _data != "ge" if _data != "aicpu"
                           if _data != "fusion_op" if _data != "step_trace"]
        if "AddKernelInvocation_L0" == self.__scene:
            dev_csv_lst = [_data for _data in dev_csv_lst if _data != "op_statistic"]
        if "UniqueIdTensorflow" == self.__scene:
            dev_json_lst = self.cfg_switch.switch_option["task-time"]["timeline"]
            dev_csv_lst = self.cfg_switch.switch_option["task-time"]["summary"]
        if "Sys" == self.__scene and (soc == "310P" or soc == "910"):
            dev_csv_lst = [_data for _data in dev_csv_lst if _data != "npu_module_mem"]
        switch_res_dict["dev_json"] = sorted(list(set(dev_json_lst)))
        switch_res_dict["dev_csv"] = sorted(list(set(dev_csv_lst)))
        switch_res_dict["host_json"] = sorted(list(set(host_json_lst)))
        switch_res_dict["host_csv"] = sorted(list(set(host_csv_lst)))
        self.__logger.info("switch res: {}".format(switch_res_dict))
        return switch_res_dict

    def clear_result_dir(self):
        if os.path.isdir(self.res_dir):
            shutil.rmtree(self.res_dir)
            self.__logger.info("clear output ok ...")
        os.makedirs(self.res_dir)

    def subprocess_cmd(self, cmd):
        self.__logger.info("host command: {}".format(cmd))
        try:
            status, res = subprocess.getstatusoutput(cmd)
            if re.search("Traceback", res):
                self.__logger.error(res)
                self.res += 1
        except (Exception, TimeoutError) as err:
            res = err
            self.__logger.error(res)
            self.res += 1
        finally:
            pass
        return res

    # 校验timeline目录下的json文件数，理论上json文件数是要小于等于device数的
    # 只有存在分片的情况下json文件数才会大于device数，这种情况就需要校验分片数的唯一性
    # 这里选择校验msprof_slice_0*.json的唯一性
    def timeline_json_count(self, argv_path, device_count: int = 1):
        if os.path.isdir(argv_path):
            cmd = "find {} -name msprof*.json | wc -l".format(argv_path)
            json_count = self.subprocess_cmd(cmd)
            cmd_slice = "find {} -name msprof_slice_0*.json | wc -l".format(argv_path)
            slice_0_json_count = self.subprocess_cmd(cmd_slice)
            if int(json_count) == 0 or int(json_count) > device_count and int(slice_0_json_count) != 1:
                    self.__logger.error("timeline msprof*.json count %d is incorrect.", int(json_count))
                    self.res += 1

    def get_dev_log_output_list(self, prof_dir):
        self.__logger.info("view prof dir {}".format(prof_dir))
        dev_lst = []
        log_list = []
        profiler_output_lst = []
        msprof_db_list = []
        for prof in prof_dir:
            device_count = 0
            for dir_ in os.listdir(prof):
                if "dev" in dir_:
                    device_count += 1
                    dev_lst.append(os.path.join(prof, dir_))
                elif "log" in dir_:
                    log_list.append(os.path.join(prof, dir_))
                elif "output" in dir_:
                    profiler_output_lst.append(os.path.join(prof, dir_))
                elif ".db" in dir_:
                    msprof_db_list.append(os.path.join(prof, dir_))
        self.__logger.warning("device_dir: {}".format(dev_lst))
        self.__logger.warning("host_dir: {}".format(log_list))
        self.__logger.warning("output_dir: {}".format(profiler_output_lst))
        self.__logger.warning("msprof_db_list: {}".format(msprof_db_list))
        return dev_lst, log_list, profiler_output_lst, msprof_db_list

    @staticmethod
    def replace_dev_id(dir_lst, dev_id):
        dir_lst_ = []
        for res_file in dir_lst:
            if ".0" not in res_file:
                pass
            else:
                res_file = res_file.replace(".0", ".{}".format(dev_id))
            dir_lst_.append(res_file)
        return dir_lst_

    def get_total_size_in_plog(self, size_type, _data, dev_id):
        cmd = "grep -rn '{}' {} | grep {} | grep '{}'".format(size_type, self.res_dir, _data, dev_id)
        res_size = self.subprocess_cmd(cmd)
        for res_ in res_size.split(","):
            if size_type in res_:
                self.__logger.info("{}: {}".format(size_type, int(res_.split(":")[1])))
                return int(res_.split(":")[1])
        return int(self.cfg_value.dev_id)

    def view_data_file(self, argv_path):
        if re.search(self.cfg_value.device, argv_path):
            dev_id = "device id {}".format(argv_path.split("_")[-1])
        else:
            dev_id = ""
        argv_path = os.path.join(argv_path, "data")
        self.__logger.info("start view {} ...".format(argv_path))
        data_lst = []
        for _data in os.listdir(argv_path):
            if _data.endswith("complete") or _data.endswith("done") or _data.startswith("Framework") \
                    or _data.startswith("unaging.additional") or _data.startswith("host") \
                    or (_data.split("-")[0]).isdigit():
                continue
            else:
                _data = ".".join(_data.split(".")[0:2])
                if _data in data_lst:
                    continue
                data_lst.append(_data)
                cmd = "find {} -name '{}*' | grep -v .done | grep -v .complete | xargs du -cb".format(argv_path, _data)
                res_size = int(self.subprocess_cmd(cmd).split()[-2])
                self.__logger.info("{}: {}".format(_data, res_size))
                res_channel = self.get_total_size_in_plog("total_size_channel", _data, dev_id)
                res_report = self.get_total_size_in_plog("totalDataLengthSuccess_", _data, dev_id)
                res_hash = self.get_total_size_in_plog("saveLen", _data, dev_id)
                if res_channel == res_report == res_hash == 0:
                    continue
                if res_size == res_channel or res_size == res_report + res_hash:
                    continue
                else:
                    self.res += 1
                    self.__logger.error("total_size ERROR: {} {}".format(_data, dev_id))

    def check_op_num_valid(self):
        self.__logger.info("start check if op num valid ...")
        if self.hccl_statistic_op_num == 0 and self.op_statistic_op_num == 0:
            self.__logger.info("current op summary num {}, in prof level0".format(self.op_summary_op_num))
            return
        if self.hccl_statistic_op_num + self.op_statistic_op_num != self.op_summary_op_num:
            self.__logger.error("hccl statistic num {} and op statistic num {} not equal to op summary num {}".format(
                self.hccl_statistic_op_num, self.op_statistic_op_num, self.op_summary_op_num))
            self.res += 1
        self.__logger.info("hccl statistic num {}; op statistic num {}; op summary num {}".format(
            self.hccl_statistic_op_num, self.op_statistic_op_num, self.op_summary_op_num))

    def check_flashattention_valid(self):
        # 检查FlashAttention算子耗时不能超过波动阈值上限（比如Task Duration < 200ms）
        if self.op_summary_context.empty or self.op_statistic_context.empty:
            return
        self.__logger.info(
            "Start check whether the duration of the FlashAttention operator exceeds the upper threshold.")
        df = self.op_summary_context[self.op_summary_context['OP Type'] == 'PromptFlashAttention']
        if not df.empty:
            if not (df['Task Duration(us)'] < 400).all():
                self.__logger.error("The duration of the FlashAttention operator exceeds the threshold.")
                self.res += 1
        # api_statistic表格中HOST侧FlashAttention算子下发的次数和op sumamry中Device侧算子执行个数一致
        self.__logger.info("Check whether the num of the FlashAttention operator valid.")
        host_num = len(self.op_statistic_context[self.op_statistic_context['OP Type'] == 'PromptFlashAttention'])
        if host_num != len(df):
            self.__logger.error("the num of the FlashAttention operator not valid.")
            self.res += 1
            return
        self.__logger.info(
            "Check whether the duration of the FlashAttention operator exceeds the upper threshold susccess!.")

    def check_op_statisic_avgtime(self):
        """ op_statistic算子平均耗时校验 """
        if self.op_statistic_context.empty:
            return
        self.__logger.info("start check avgtime in op_statistic*.csv")
        usecase_id = self.__id
        json_path = self.cfg_path.op_statistic_json_path
        with open(json_path, 'r') as f:
            avg_time_dict = json.load(f)
        avg_time_data = avg_time_dict.get(usecase_id)
        if avg_time_data is None:
            self.__logger.warning("op_statistic_avg_time.json has no the data of {}!".format(usecase_id))
            return
        avg_time_data_dev = avg_time_data.get(self.dev_id)
        if avg_time_data_dev is None:
            self.__logger.error("{} has no device {}".format(usecase_id, self.dev_id))
            self.res += 1
            return
        for op_type, avg_time in zip(self.op_statistic_context['OP Type'],
                                     self.op_statistic_context['Avg Time(us)']):
            standard_avg_time = avg_time_data_dev.get(op_type, 0)
            difference = abs(standard_avg_time - avg_time)
            threshold = standard_avg_time * 0.25
            if difference >= threshold:
                self.__logger.warning("{} avgtime in op_statistic*.csv is not valid!".format(op_type))
                return
        self.__logger.info("check avgtime in op_statistic*.csv success!")

    def check_overall_csv_valid(self):
        # Testcase entry function for checking CSV content
        csv_statistic, state = self.Traverse_op_summary()
        if "ffts_off_hccl_l1" == self.__scene or "ffts_on_hccl_l1" == self.__scene:
            self.check_hccl_api(csv_statistic['op_summary_num'])

        self.check_max_duration(csv_statistic['op_type_cost'])
        self.check_op_summary_dimension(state)

    def Traverse_op_summary(self):
        # Traverse op_summary.csv to obtain data required by testcase.
        # Verify the testcase of each operator at the same time.
        self.init_optype_baseline()
        csv_statistic = {
            'op_summary_num': defaultdict(int),
            'op_type_cost': defaultdict(dict),
        }
        state_total = [0, 0]

        for index, item in self.op_summary_context.iterrows():
            csv_statistic['op_summary_num'][item['OP Type']] += 1
            state = self.check_op_dimension(item)
            state_total[0] += state[0]
            state_total[1] += state[1]

            if pandas.isnull(item['OP Type']) or pandas.isna(item['OP Type']):
                continue
            key = str(item['OP Type']) + '-' + str(item['Input Shapes'])[1:-1] + '-' + str(item['Output Shapes'])[1:-1]

            if self.__id in self.op_type_baseline and key in self.op_type_baseline[self.__id]:
                if key in csv_statistic['op_type_cost']:
                    csv_statistic['op_type_cost'][key]['total'] += float(item['Task Duration(us)'])
                    csv_statistic['op_type_cost'][key]['count'] += 1
                else:
                    csv_statistic['op_type_cost'].update({key: {"total": float(item['Task Duration(us)']), "count": 1}})
        return csv_statistic, state_total

    def init_optype_baseline(self):
        with open(self.cfg_path.op_cost_baseline_path, 'r') as f:
            self.op_type_baseline = json.load(f)

    def check_max_duration(self, op_type_cost):
        # Check the upper limit of the average fluctuation threshold of the operator task duration.
        # 检查算子耗时是否超过基线1.5x
        if self.__id not in self.op_type_baseline:
            return
        for op_type in op_type_cost:
            op_cost = op_type_cost[op_type]['total'] / op_type_cost[op_type]['count']
            if op_type not in self.op_type_baseline[self.__id]:
                continue
            value = float(self.op_type_baseline[self.__id][op_type])
            if not op_cost < value * 2:
                self.res += 1
                self.__logger.error(
                    f"task duration does not match baseline(2x), {op_type}\n{self.op_type_baseline[self.__id][op_type]}\n{op_cost}")
            if not op_cost < value * 1.5:
                self.__logger.warning(
                    f"task duration does not match baseline(1.5x), {op_type}\n{self.op_type_baseline[self.__id][op_type]}\n{op_cost}")
            if not op_cost < value * 1.2:
                self.__logger.warning(
                    f"task duration does not match baseline(1.2x), {op_type}\n{self.op_type_baseline[self.__id][op_type]}\n{op_cost}")

    def check_op_dimension(self, item):
        return_val = [0, 0]
        input_shapes_num = str(item['Input Shapes']).count(';')
        input_types_num = str(item['Input Data Types']).count(';')
        input_formats_num = str(item['Input Formats']).count(';')
        output_shapes_num = str(item['Output Shapes']).count(';')
        output_types_num = str(item['Output Data Types']).count(';')
        output_formats_num = str(item['Output Formats']).count(';')
        num_input_list = [input_shapes_num, input_types_num, input_formats_num]
        num_output_list = [output_shapes_num, output_types_num, output_formats_num]
        if len(list(set(num_input_list))) != 1:
            self.__logger.error(f"Op Name: {item['Op Name']}, Input Shapes, Input Data Types and Input Formats "
                                f"mismatch in the op_summary.csv.")
            return_val[0] += 1
        if len(list(set(num_output_list))) != 1:
            self.__logger.error(f"Op Name: {item['Op Name']}, Output Shapes, Output Data Types and Output Formats "
                                f"mismatch in the op_summary.csv.")
            return_val[1] += 1

        return return_val

    def check_op_summary_dimension(self, state):
        if state[0]:
            self.__logger.error("Input Shapes, Input Data Types and Input Formats mismatch in the op_summary.csv.")
            self.res += 1
        if state[1]:
            self.__logger.error("Output Shapes, Output Data Types and Output Formats mismatch in the op_summary.csv.")
            self.res += 1
        if not state[0] and not state[1]:
            self.__logger.info('Dimension check in op_summary.csv success.')

    def check_hccl_api(self, op_summary_num):
        hccl_statistic_dict = {}
        api_statistic_dict = {}
        state = 0
        for index, item in self.hccl_statistic_context.iterrows():
            hccl_statistic_dict[item['OP Type']] = item['Count']
        for _, item in self.api_statistic_context.iterrows():
            api_statistic_dict[item['API Name']] = item['Count']
        for op_type in hccl_statistic_dict.keys():
            hccl_num = hccl_statistic_dict[op_type]
            api_num = api_statistic_dict[op_type]
            op_num = op_summary_num[op_type]
            num_list = [hccl_num, api_num, op_num]
            if len(set(num_list)) != 1:
                state += 1
        if state:
            self.__logger.error("The number of communication operators in hccl_statistic, api_statistic,"
                                " and op_summary is different.")
            self.res += 1
        else:
            self.__logger.info('hccl check success')

    def check_tensor_info_valid(self, op_summary_file):
        # cmd = "cat {} | head -1".format(op_summary_file)
        # op_summary_header = self.subprocess_cmd(cmd)
        # if 'Model Name' in op_summary_header:
        #     if 'Mix Block Dim' in op_summary_header:
        #         input_shapes = self.subprocess_cmd('cut -d \',\' -f 14 {}'.format(op_summary_file))
        #         input_data_types = self.subprocess_cmd('cut -d \',\' -f 15 {}'.format(op_summary_file))
        #         input_formats = self.subprocess_cmd('cut -d \',\' -f 16 {}'.format(op_summary_file))
        #         output_shapes = self.subprocess_cmd('cut -d \',\' -f 17 {}'.format(op_summary_file))
        #         output_data_types = self.subprocess_cmd('cut -d \',\' -f 18 {}'.format(op_summary_file))
        #         output_formats = self.subprocess_cmd('cut -d \',\' -f 19 {}'.format(op_summary_file))
        #     else:
        #         input_shapes = self.subprocess_cmd('cut -d \',\' -f 13 {}'.format(op_summary_file))
        #         input_data_types = self.subprocess_cmd('cut -d \',\' -f 14 {}'.format(op_summary_file))
        #         input_formats = self.subprocess_cmd('cut -d \',\' -f 15 {}'.format(op_summary_file))
        #         output_shapes = self.subprocess_cmd('cut -d \',\' -f 16 {}'.format(op_summary_file))
        #         output_data_types = self.subprocess_cmd('cut -d \',\' -f 17 {}'.format(op_summary_file))
        #         output_formats = self.subprocess_cmd('cut -d \',\' -f 18 {}'.format(op_summary_file))
        # else:
        #     if 'Mix Block Dim' in op_summary_header:
        #         input_shapes = self.subprocess_cmd('cut -d \',\' -f 13 {}'.format(op_summary_file))
        #         input_data_types = self.subprocess_cmd('cut -d \',\' -f 14 {}'.format(op_summary_file))
        #         input_formats = self.subprocess_cmd('cut -d \',\' -f 15 {}'.format(op_summary_file))
        #         output_shapes = self.subprocess_cmd('cut -d \',\' -f 16 {}'.format(op_summary_file))
        #         output_data_types = self.subprocess_cmd('cut -d \',\' -f 17 {}'.format(op_summary_file))
        #         output_formats = self.subprocess_cmd('cut -d \',\' -f 18 {}'.format(op_summary_file))
        #     else:
        #         input_shapes = self.subprocess_cmd('cut -d \',\' -f 12 {}'.format(op_summary_file))
        #         input_data_types = self.subprocess_cmd('cut -d \',\' -f 13 {}'.format(op_summary_file))
        #         input_formats = self.subprocess_cmd('cut -d \',\' -f 14 {}'.format(op_summary_file))
        #         output_shapes = self.subprocess_cmd('cut -d \',\' -f 15 {}'.format(op_summary_file))
        #         output_data_types = self.subprocess_cmd('cut -d \',\' -f 16 {}'.format(op_summary_file))
        #         output_formats = self.subprocess_cmd('cut -d \',\' -f 17 {}'.format(op_summary_file))

        pass

    @staticmethod
    def get_file_name(file_name: str) -> str:
        """
        get filemane like "op_summary"
        """
        match = re.search(r'(_\d)?(_slice_\d+)?_\d+', file_name)
        if match and match.start() > 0:
            return file_name[:match.start()]
        logging.warning("The file name  %s is invalid!", file_name)
        return "invalid"

    def count_csv_num(self, summary_dir: str, summary_name: str, is_plus=True) -> None:
        """
        对所选的文件进行读取，并记录csv中数据的条数。
        summary的条数增加，output中的减少，最后校验是否为零 即证明两者的条数是否一致
        """
        cmd = "wc -l {}".format(os.path.join(summary_dir, summary_name))
        res_content = self.subprocess_cmd(cmd).split()[0]
        ori_name = self.get_file_name(summary_name)
        if not self.is_int(res_content):
            return
        if is_plus:
            self.__logger.info("{0} PLUS:{1}".format(summary_name, res_content))
            # 减一是因为去掉表头
            self.csv_cout_dict.update({ori_name: self.csv_cout_dict.get(ori_name, 0) + int(res_content) - 1})
        else:
            self.__logger.info("{0} DESC:{1}".format(summary_name, res_content))
            self.csv_cout_dict.update({ori_name: self.csv_cout_dict.get(ori_name, 0) - int(res_content) + 1})

    def view_summary_content(self, summary_dir):
        for index, csv in enumerate(sorted(os.listdir(summary_dir))):
            if not csv.endswith(".csv"):
                continue
            self.__logger.info("start view {} content ...".format(csv))
            csv_name = self.get_file_name(csv)
            cmd = "cat {} | head -2".format(os.path.join(summary_dir, csv))
            res_content = self.subprocess_cmd(cmd)
            self.__logger.info("csv content: {}".format(res_content.split("\n")))
            header = res_content.split("\n")[0]
            under_header = res_content.split("\n")[1]

            # 对summary里的csv条数进行计数
            self.count_csv_num(summary_dir, csv)

            # 判断表头数据和config.ini文件数据是否一 ?
            if "ai_core_utilization" == csv_name or "op_summary" == csv_name:
                if "op_summary" == csv_name:
                    cmd = "cat {} | wc -l".format(os.path.join(summary_dir, csv))
                    self.op_summary_op_num = int(self.subprocess_cmd(cmd)) - 1
                    self.op_summary_context = pandas.read_csv(os.path.join(summary_dir, csv))
                    # cmd = "cat {} | grep -v 'HCCL' | wc -l".format(os.path.join(summary_dir, csv))
                    # self.op_summary_not_hccl_op_num = int(self.subprocess_cmd(cmd)) - 1
                    self.check_tensor_info_valid(os.path.join(summary_dir, csv))
                if not re.findall(r"aic-metrics\S+", str(self.switch_values)):
                    if "AclApi" == self.__scene or "AclJson" == self.__scene or "ExportTensorFlow" == self.__scene \
                            or "SessionRunTensorFlow" == self.__scene or "SingleOp" == self.__scene \
                            or "InnerOneStepPyTorch" == self.__scene or "SingleOp65535PyTorch" == self.__scene \
                            or "SingleOpPyTorch" == self.__scene or "UniqueIdTensorflow" == self.__scene \
                            or "StepPyTorch" == self.__scene or "CannProfilingPyTorch" == self.__scene:
                        aic_metrics = "PipeUtilization"
                        aic_mode = "task-based"
                    elif "LoopTensorFlow" == self.__scene or "DockerExportTensorFlow" == self.__scene:
                        aic_metrics = "ResourceConflictRatio"
                        aic_mode = "task-based"
                    elif "NoLoopTensorFlow" == self.__scene:
                        aic_metrics = "MemoryL0"
                        aic_mode = "task-based"
                    elif "PyAclApi" == self.__scene:
                        aic_metrics = "ArithmeticUtilization"
                        aic_mode = "task-based"
                    elif "GraphApi" == self.__scene:
                        aic_metrics = "MemoryUB"
                        aic_mode = "task-based"
                    elif "hccl_graph" == self.__scene:
                        aic_metrics = "PipeUtilization_not_compatible"
                        aic_mode = "task-based"
                    else:
                        aic_metrics = "PipeUtilization"
                        aic_mode = "sample-based"
                    except_res = self.cfg_header.table_header[csv_name][aic_mode][aic_metrics]
                else:
                    aic_metrics = [_data.split("=")[1] for _data in self.switch_values if "aic-metrics" in _data]
                    aic_mode = [_data.split("=")[1] for _data in self.switch_values if "aic-mode" in _data]
                    if "AddOp" == self.__scene or "MsprofbinPyTorch" == self.__scene \
                            or "CannProfilingPyTorch" == self.__scene or "DynamicLaunch" == self.__scene \
                            or "DynamicAttach" == self.__scene or "MsprofbinMindSpore" == self.__scene \
                            or self.__scene.startswith("AddKernelInvocation"):
                        except_res = self.cfg_header.table_header[csv_name][aic_mode[0]][aic_metrics[0]]
                    elif "Custom" in aic_metrics[0]:
                        if "ai_core_utilization" == csv_name:
                            except_res = ((aic_metrics[0]).replace("0x", "r")).replace("Custom:", "Core ID,")
                        else:
                            if aic_mode[0] == "sample-based":
                                except_res = self.cfg_header.table_header[csv_name][aic_mode[0]]["MemoryUB"]
                            else:
                                except_res = self.cfg_header.table_header[csv_name][aic_mode[0]]["MemoryUB"]
                                except_res = except_res.split("total_cycles,")
                                except_res = except_res[0] + "total_cycles," \
                                             + aic_metrics[0].replace("0x", "r").replace("Custom:", "")
                    else:
                        except_res = self.cfg_header.table_header[csv_name][aic_mode[0]][aic_metrics[0]]

                if self.data_platform == '5':
                    except_res += ',Context ID'
                    except_res = except_res.replace('Block Dim,', 'Block Dim,Mix Block Dim,')
                if 'ffts_off_hccl_l0' == self.__scene or 'ffts_on_hccl_l0' == self.__scene \
                        or 'analyze_communication' == self.__scene or 'ffts_off_hccl_l1' == self.__scene \
                        or 'ffts_on_hccl_l1' == self.__scene:
                    if 'ffts_off_hccl_l1' == self.__scene or 'ffts_on_hccl_l1' == self.__scene:
                        except_res += ',aicore_time(us),aic_total_cycles,aic_mac_time(us),' \
                                      'aic_mac_ratio,aic_scalar_time(us),aic_scalar_ratio,' \
                                      'aic_mte1_time(us),aic_mte1_ratio,aic_mte2_time(us),' \
                                      'aic_mte2_ratio,aic_fixpipe_time(us),aic_fixpipe_ratio,' \
                                      'aic_icache_miss_rate,aiv_time(us),aiv_total_cycles,' \
                                      'aiv_vec_time(us),aiv_vec_ratio,aiv_scalar_time(us),' \
                                      'aiv_scalar_ratio,aiv_mte2_time(us),aiv_mte2_ratio,' \
                                      'aiv_mte3_time(us),aiv_mte3_ratio,aiv_icache_miss_rate,cube_utilization(%)'

            elif "step_trace" == csv_name:
                except_res = self.cfg_header.table_header['step_trace']
                if 'hccl_graph' == self.__scene:
                    except_res += ',Reduce Start(us),Reduce Duration(us),Reduce Start(us),Reduce Duration(us)'
            elif "op_statistic" == csv_name:
                self.op_statistic_context = pandas.read_csv(os.path.join(summary_dir, csv))
                cmd = "cat {} | head -1".format(os.path.join(summary_dir, csv))
                op_statistic_header = self.subprocess_cmd(cmd)
                if "Model Name" in op_statistic_header:
                    cmd = "awk -F \',\' \'{{sum += $5}} END {{print sum}}\' {}".format(os.path.join(summary_dir, csv))
                else:
                    cmd = "awk -F \',\' \'{{sum += $4}} END {{print sum}}\' {}".format(os.path.join(summary_dir, csv))
                self.op_statistic_op_num = int(self.subprocess_cmd(cmd))
                if 'hccl_graph' == self.__scene:
                    except_res = self.cfg_header.table_header[csv_name]["WholeNetwork"]
                else:
                    except_res = self.cfg_header.table_header[csv_name]["AllData"]
            elif "api_statistic" == csv_name:
                self.api_statistic_context = pandas.read_csv(os.path.join(summary_dir, csv))
                except_res = self.cfg_header.table_header[csv_name]
            else:
                if "hccl_statistic" == csv_name:
                    cmd = "awk -F \',\' \'{{sum += $3}} END {{print sum}}\' {}".format(os.path.join(summary_dir, csv))
                    self.hccl_statistic_op_num = int(self.subprocess_cmd(cmd))
                    self.hccl_statistic_context = pandas.read_csv(os.path.join(summary_dir, csv))
                except_res = self.cfg_header.table_header[csv_name]
            if os.getenv("MSPROF_VERSION") == "tr5" or os.getenv("MSPROF_VERSION") == "TR5":
                except_res = except_res.replace("HF32 Eligible,", "")
            # 所有的表格都加了一列"Device_id"
            except_res = "Device_id," + except_res
            if header != except_res:
                self.__logger.error('current header: {}'.format(header))
                self.__logger.info('expect header: {}'.format(except_res))
                self.res += 1
            # 判断表头下第一行数据是否为 ?
            under_header_lst = under_header.split(", ")
            for value in under_header_lst:
                if not value:
                    self.__logger.error(value)
                    self.res += 1

        self.op_summary_csv.checkAllPathSummary()
        if self.op_summary_csv.getErrCnt() != 0:
            self.__logger.error("op_summary 基础校验共发现错误{}个，请检查修改".format(
                                self.op_summary_csv.getErrCnt()))
            self.res += 1
        self.check_op_num_valid()
        self.check_overall_csv_valid()
        self.check_flashattention_valid()
        # self.check_op_statisic_avgtime()

    def view_output_file(self, mindstudio_profiler_output, switch_res):
        """
        校验mindstudio_profiler_output目录里面的文件
        :param mindstudio_profiler_output: mindstudio_profiler_output路径
        :param switch_res:
        :return: None
        """
        self.__logger.info("start view {} ...".format(mindstudio_profiler_output))
        all_files = sorted(os.listdir(mindstudio_profiler_output))
        timeline_files = []
        summary_files = []
        self.__logger.info("file list: {}".format(all_files))
        for index, file in enumerate(all_files):
            # 获取一个文件不带时间戳的原始名字，例如从op_summary_20240304201531.csv得到op_summary
            file_name = "_".join([k for k in (str(file.split(".")[0]).split("_")) if k.isalpha()])
            # prof_rule*.json和begin文件不做处理
            if file_name in ["prof_rule", "begin", "README"]:
                continue
            if "slice" in file_name:
                file_name.rstrip("_slice")
            if "cache" == file_name:
                file_name = "l2_cache"
            if file.endswith(".csv"):
                summary_files.append(file_name)
            else:
                timeline_files.append(file_name)
        # 根据switch_res得到output目录下该有的文件（去重比较）,判断是否符合预期
        deduplicated_summary_files = set(summary_files)
        deduplicated_timeline_files = set(timeline_files)
        all_timeline_files = {"msprof", "step_trace", "msprof_tx"}
        expected_summary_files = set(switch_res.get("dev_csv", []) + switch_res.get("host_csv", [])) - {"prof_rule"}
        expected_timeline_files = set(switch_res.get("dev_json", []) + switch_res.get("host_json", [])) & all_timeline_files
        if expected_timeline_files != deduplicated_timeline_files:
            self.__logger.error("Current timeline files: {}".format(timeline_files))
            self.__logger.error("Expected timeline files: {}".format(expected_timeline_files))
            self.res += 1
        if expected_summary_files != deduplicated_summary_files:
            self.__logger.error("Current summary files: {}".format(summary_files))
            self.__logger.error("Expected summary files: {}".format(expected_summary_files))
            self.res += 1
        # 检查csv文件
        self.view_summary_content(mindstudio_profiler_output)
        # 检查json文件
        self.view_timeline_content(mindstudio_profiler_output)
        json_check = MsprofJsonChecker(mindstudio_profiler_output, self.start_timestamp_us,
                                       self.end_timestamp_us, self.__id, "910A")
        if not json_check.check():
            self.res += 1

    def view_timeline_content(self, mindstudio_profiler_output):
        """
        检查msprof.json文件的总大小
        :param output_dir: mindstudio_profiler_output目录路径
        :return: None
        """
        cmd = "du -b {}/msprof*.json".format(mindstudio_profiler_output)
        res_du = self.subprocess_cmd(cmd)
        self.__logger.info(res_du)
        file_size = res_du.split()[0]   # 字节数
        if not self.is_int(file_size):
            self.__logger.info("timeline msprof*.json is lost.")
            return
        if int(file_size) <= 2:
            self.__logger.error("timeline msprof*.json size is false.")
            self.res += 1

    def is_int(self, value: any):
        try:
            int(value)
            return True
        except ValueError:
            return False

    def view_res_file(self, output, switch_res):
        if "AclApiSubscription" == self.__scene:
            pass
        else:
            prof_lst = []
            for prof_ in os.listdir(output):
                if os.path.isfile(os.path.join(output, prof_)):
                    continue
                prof_lst.append(os.path.join(output, prof_))
            dev_dir_list, log_dir_list, output_dir_list, db_list = self.get_dev_log_output_list(prof_lst)
            try:
                for device_dir in dev_dir_list:
                    self.view_data_file(device_dir)
                for log_dir in log_dir_list:
                    self.view_error_msg(log_dir, "collection")
                for output_dir in output_dir_list:
                    self.view_output_file(output_dir, switch_res)
                for db_file in db_list:
                    self.view_output_db(db_file, switch_res)
            except (IndexError, Exception) as err:
                self.__logger.error(err)
                self.res += 1

    def view_output_db(self, db_path, switch_res):
        self.__logger.info("db_path is: {}".format(db_path))
        if self.__scene == 'msprof_db' and 'msprof_' in db_path :
            expect_tables = switch_res['output_db']["msprof_db"]
            for expect_table in expect_tables:
                FileChecker.check_db_table_exist(db_path, expect_table)

    def view_error_msg(self, argv_path, log_type):
        self.__logger.info("start view {} log ...".format(log_type))
        if log_type == "plog":
            cmd = r"grep -rn 'ERROR\] PROFILING' {0}; grep -rn '\[ERROR\] \[MSVP\]' {0}; " \
                  r"grep -rn 'ERROR\] Failed' {0}".format(argv_path)
        else:
            cmd = r"grep -rn 'ERROR\]' {0}".format(argv_path)
        res = self.subprocess_cmd(cmd)
        if re.search(r"ERROR", res):
            self.__logger.error(res)
            self.res += 1

    def msprofbin_and_parse(self, msprofbin_cmd):
        switch_res = self.get_switch_res(msprofbin_cmd)
        self.clear_result_dir()
        startTime = datetime.now()
        self.start_timestamp_us = datetime.now().timestamp() * self.S_TO_US
        self.subprocess_cmd(msprofbin_cmd)
        self.end_timestamp_us = datetime.now().timestamp() * self.S_TO_US
        diffTime = datetime.now() - startTime
        self.duration_time = diffTime.total_seconds()
        self.view_error_msg(self.res_dir, "plog")
        self.view_res_file(self.res_dir, switch_res)
        if self.res == 0:
            self.__result = self.cfg_value.pass_res
        else:
            self.__result = self.cfg_value.fail_res
        self.__logger.info("\n------------------------------- "
                           "End execute case {} -------------------------------".format(self.__id))

    def run_cmd(self, cmd):
        self.clear_result_dir()
        start_time = datetime.now()
        self.start_timestamp_us = start_time.timestamp() * self.S_TO_US
        if cmd:
            self.subprocess_cmd(cmd)
        end_time = datetime.now()
        self.end_timestamp_us = end_time.timestamp() * self.S_TO_US
        diff_time = end_time - start_time
        self.duration_time = diff_time.total_seconds()

    def case_single_switch(self, params=None, scene=None):
        """
        Switch case: App or Sys
        """
        params = self.__params if params is None else params
        scene = self.__scene if scene is None else scene
        any_switch = params.replace(",", " ")
        if "." in any_switch:
            any_switch = any_switch.replace(".", ",")
        if "Sys" == scene:
            msprofbin_cmd = "{0} --output={1} --{2} > {3}".format(self.cfg_cmd.single_sys_switch, self.res_dir,
                                                                  any_switch, self.slog_stdout)
        else:
            if "host-sys" in any_switch:
                perf_iotop_cmd = self.cfg_cmd.perf_iotop_cmd
                cmd = "{} {}".format(perf_iotop_cmd, self.cfg_cmd.single_app_switch)
            else:
                cmd = self.cfg_cmd.single_app_switch
            msprofbin_cmd = "{} --output={} --{} > {}".format(cmd, self.res_dir, any_switch, self.slog_stdout)

        self.msprofbin_and_parse(msprofbin_cmd)

    def case_app_end_switch(self, params=None, scene=None):
        """
        Switch case: App or Sys
        """
        params = self.__params if params is None else params
        scene = self.__scene if scene is None else scene
        if "single_switch" == scene:
            msprofbin_cmd = "cd {}/out/; {} --output={} {} > {}".format(self.cfg_path.infer_path,
                                                                  self.cfg_cmd.single_app_end_switch, self.res_dir,
                                                                  self.cfg_cmd.app_end_switch, self.slog_stdout)
        self.clear_result_dir()
        startTime = datetime.now()
        self.subprocess_cmd(msprofbin_cmd)
        diffTime = datetime.now() - startTime
        self.duration_time = diffTime.total_seconds()
        self.view_error_msg(self.res_dir, "plog")
        if self.res == 0:
            self.__result = self.cfg_value.pass_res
        else:
            self.__result = self.cfg_value.fail_res
        self.__logger.info("\n------------------------------- "
                           "End execute case {} -------------------------------".format(self.__id))

    def case_all_switch(self, params=None, scene=None):
        """
        Switch of App or Sys or All
        """
        params = self.__params if params is None else params
        scene = self.__scene if scene is None else scene
        aic_mode = params.split(",")[0]
        llc_profiling = params.split(",")[1]
        aic_metrics = params.split(",")[2]
        if "." in aic_metrics:
            aic_metrics = aic_metrics.replace(".", ",")
        if "Sys" == scene:
            msprofbin_cmd = "{} --output={} --{} --{} --{} > {}".format(self.cfg_cmd.all_sys_switch, self.res_dir,
                                                                        aic_mode, aic_metrics, llc_profiling,
                                                                        self.slog_stdout)
        elif "App" == scene:
            msprofbin_cmd = "{} --output={} --{} --{} --{} > {}".format(self.cfg_cmd.all_app_switch, self.res_dir,
                                                                        aic_mode, aic_metrics, llc_profiling,
                                                                        self.slog_stdout)
        else:
            msprofbin_cmd = "{} --output={} --{} --{} --{} > {}".format(self.cfg_cmd.all_switch, self.res_dir,
                                                                        aic_mode, aic_metrics, llc_profiling,
                                                                        self.slog_stdout)
        self.msprofbin_and_parse(msprofbin_cmd)

    def case_api_scene(self, scene=None):
        scene = self.__scene if scene is None else scene
        if "AclApi" == scene:
            app_path = self.cfg_path.aclapi_path
        elif "Msproftx" == scene:
            app_path = self.cfg_path.msproftx_path
        elif "AclApiSubscription" == scene:
            app_path = self.cfg_path.subscription_path
        elif "PyAclApi" == scene:
            app_path = self.cfg_path.pyaclapi_path
        elif "GraphApi" == scene:
            app_path = self.cfg_path.graphapi_path
        elif "AclJson" == scene:
            app_path = self.cfg_path.acljson_path
        elif "AddOp" == scene:
            app_path = self.cfg_path.addop_path
        elif "AddKernelInvocation_L1" == scene:
            app_path = self.cfg_path.AscendC_AddKernelInvocation_L1_path
        elif "AddKernelInvocation_L0" == scene:
            app_path = self.cfg_path.AscendC_AddKernelInvocation_L0_path
        else:
            app_path = self.cfg_path.opst_path

        self.run_path = app_path
        cmd = "cd {}; {}; bash run_api.sh {} > {}".format(app_path, self.cfg_path.toolkit_env_path,
                                                          self.res_dir, self.slog_stdout)
        self.msprofbin_and_parse(cmd)

    def case_train_scene(self, scene=None):
        scene = self.__scene if scene is None else scene
        if "CannProfilingPyTorch" == scene:
            train_path = self.cfg_path.cannProfiling_path
        elif "ExportMindSpore" == scene:
            train_path = ""
        elif "DockerExportTensorFlow" == scene:
            train_path = self.cfg_path.dockerexportTensorFlow_path
        elif "ExportTensorFlow" == scene:
            train_path = self.cfg_path.exportTensorFlow_path
        elif "MsprofbinMindSpore" == scene:
            train_path = self.cfg_path.msprofbinMindSpore_path
        elif "MsprofbinPyTorch" == scene:
            train_path = self.cfg_path.msprofbinPytorch_path
        elif "MsprofbinTensorFlow" == scene:
            train_path = self.cfg_path.msprofbinTensorFlow_path
        elif "ProfilerMindSpore" == scene:
            train_path = ""
        elif "SessionRunTensorFlow" == scene:
            train_path = self.cfg_path.sessionrunTensorFlow_path
        elif "LoopTensorFlow" == scene:
            train_path = self.cfg_path.loopTensorFlow_path
        elif "NoLoopTensorFlow" == scene:
            train_path = self.cfg_path.noLoopTensorFlow_path
        elif "InnerOneStepPyTorch" == scene:
            train_path = self.cfg_path.innerOneStepPytorch_path
        elif "SingleOp65535PyTorch" == scene:
            train_path = self.cfg_path.singleOp65535Pytorch_path
        elif "SingleOpPyTorch" == scene:
            train_path = self.cfg_path.singleOpPytorch_path
        elif "StepPyTorch" == scene:
            train_path = self.cfg_path.stepPytorch_path
        elif "DynamicLaunch" == scene:
            train_path = self.cfg_path.singleOp65535PytorchDynamicLauch_path
        elif "DynamicAttach" == scene:
            train_path = self.cfg_path.singleOp65535PytorchDynamicAttach_path
        elif "UniqueIdTensorflow" == scene:
            train_path = self.cfg_path.unique_id_tf_path
        else:
            train_path = ""
        self.run_path = train_path
        cmd = "cd {}; {}; bash run_train.sh {} > {}".format(train_path, self.cfg_path.toolkit_env_path,
                                                            self.res_dir, self.slog_stdout)
        self.msprofbin_and_parse(cmd)

    def case_docker_sys(self, params=None):
        """
        Docker Switch of Sys
        """
        params = self.__params if params is None else params
        aic_mode = params.split(",")[0]
        aic_metrics = params.split(",")[1]
        llc_profiling = params.split(",")[2]
        msprofbin_cmd = "{0} " \
                        "-v {1}:{1} -it ubuntu:18.04 /bin/bash " \
                        "-c \"{2} --output={1} --{3} --{4} --{5} > {6}\"; " \
                        "sleep 30; {7}{8} > {6}".format(self.cfg_cmd.docker_cmd, self.res_dir,
                                                        self.cfg_cmd.all_sys_switch,
                                                        aic_mode, aic_metrics, llc_profiling,
                                                        os.path.join("/tmp/profiling", self.slog_stdout),
                                                        self.cfg_cmd.msprof_parse, self.res_dir)
        self.msprofbin_and_parse(msprofbin_cmd)

    def get_analysis_switch_value(self, cmd, scene):
        dev_json_lst = []
        dev_csv_lst = []
        host_json_lst = []
        host_csv_lst = []
        output_db_tables = {}
        switch_res_dict = {}
        if "Helper" == scene:
            host_json_lst.extend(['msprof'])
            host_csv_lst.extend(['api_statistic'])
        elif "hccl_graph" == scene:
            dev_csv_lst.extend(['api_statistic', 'fusion_op', 'hccl_statistic', 'op_statistic', 'op_summary',
                                'step_trace', 'task_time'])
            dev_json_lst.extend(['hccl', 'msprof', 'step_trace', 'task_time'])
        elif "msprof_db" == scene:
            output_db_tables["msprof_db"] = ['CANN_API', 'COMMUNICATION_OP', 'COMMUNICATION_TASK_INFO',
                                             'COMPUTE_TASK_INFO', 'ENUM_API_TYPE', 'ENUM_HCCL_DATA_TYPE',
                                             'ENUM_HCCL_LINK_TYPE', 'ENUM_HCCL_RDMA_TYPE', 'ENUM_HCCL_TRANSPORT_TYPE',
                                             'ENUM_MODULE', 'ENUM_MSTX_EVENT_TYPE', 'HOST_INFO','META_DATA', 'NPU_INFO',
                                             'SESSION_TIME_INFO', 'STRING_IDS', 'TASK']
        else:
            dev_json_lst.extend(['msprof', 'task_time', 'hccl', 'ffts_sub_task_time'])
            dev_csv_lst.extend(['op_summary', 'task_time', 'api_statistic'])
            if "ffts_off_hccl_l1" == scene or "ffts_on_hccl_l1" == scene:
                dev_csv_lst.extend(['op_statistic', 'hccl_statistic'])

        switch_res_dict["dev_json"] = sorted(list(set(dev_json_lst)))
        switch_res_dict["dev_csv"] = sorted(list(set(dev_csv_lst)))
        switch_res_dict["host_json"] = sorted(list(set(host_json_lst)))
        switch_res_dict["host_csv"] = sorted(list(set(host_csv_lst)))
        switch_res_dict["output_db"] = output_db_tables
        self.__logger.info("switch res: {}".format(switch_res_dict))
        return switch_res_dict

    def run_analysis_cmd_and_parse(self, cmd, scene):
        switch_res = self.get_analysis_switch_value(cmd, scene)
        self.clear_result_dir()
        startTime = datetime.now()
        self.start_timestamp_us = datetime.now().timestamp() * self.S_TO_US
        self.subprocess_cmd(cmd)
        self.end_timestamp_us = datetime.now().timestamp() * self.S_TO_US
        diffTime = datetime.now() - startTime
        self.duration_time = diffTime.total_seconds()
        self.view_error_msg(self.res_dir, "plog")
        self.view_res_file(self.res_dir, switch_res)
        if self.res == 0:
            self.__result = self.cfg_value.pass_res
        else:
            self.__result = self.cfg_value.fail_res
        self.__logger.info("\n------------------------------- "
                           "End execute case {} -------------------------------".format(self.__id))

    def case_custom(self, params=None):
        """
        自定义的--application以及开关, 自定义校验内容
        """
        params = self.__params if params is None else params
        msprofbin_cmd = self.cfg_cmd.single_app_end_switch
        if params is not None:
            params = params.replace(",", " ")
            msprofbin_cmd = msprofbin_cmd + params + f"--output={self.res_dir} -> {self.slog_stdout}"
        self.run_cmd(msprofbin_cmd)
        self.view_error_msg(self.res_dir, "plog")
        switch_res = self.get_switch_res(msprofbin_cmd)
        self.view_res_file(self.res_dir, switch_res)

    def case_analysis_scene(self, scene=None):
        """
        analysis testcase
        :param params: none
        :return:  none
        """
        scene = self.__scene if scene is None else scene
        analysis_paths = {
            "ffts_off_hccl_l0": self.cfg_path.analysisFftsOffHcclL0_path,
            "ffts_off_hccl_l1": self.cfg_path.analysisFftsOffHcclL1_path,
            "ffts_on_hccl_l0": self.cfg_path.analysisFftsOnHcclL0_path,
            "ffts_on_hccl_l1": self.cfg_path.analysisFftsOnHcclL1_path,
            "analyze_communication": self.cfg_path.analysisAnalyzeCommunication_path,
            "Helper": self.cfg_path.analysisHelper_path,
            "hccl_graph": self.cfg_path.analysisHcclGraph_path,
            "msprof_db": self.cfg_path.msprof_db_path,
        }
        analysis_path = analysis_paths.get(scene, "")
        try:
            with open(analysis_path + '/prof_data/device_0/info.json.0', 'r') as f:
                info_data = json.load(f)
                self.data_platform = info_data['platform_version']
        except FileNotFoundError:
            with open(analysis_path + '/prof_data/host/info.json', 'r') as f:
                info_data = json.load(f)
                self.data_platform = info_data['platform_version']
        self.run_path = analysis_path
        self.__logger.info("switch res: {}".format(analysis_path))
        cmd = "cd {}; {}; bash run_analysis.sh {} > {}".format(analysis_path, self.cfg_path.toolkit_env_path,
                                                               self.res_dir, self.slog_stdout)

        self.run_analysis_cmd_and_parse(cmd, scene)

    def case_pytorch_profiling(self, scene=None):
        scene = self.__scene if scene is None else scene
        app_path = self.cfg_path.pytorch_lenet_path
        if "no_profiling" == scene:
            cmd = "cd {}; {}; bash run_lenet_no_profiling.sh {} > {}".format(app_path, self.cfg_path.toolkit_env_path,
                                                                             self.res_dir, self.slog_stdout)
        elif "cann_profiling" == scene:
            cmd = "cd {}; {}; bash run_lenet_cann_profiling.sh {} > {}".format(app_path, self.cfg_path.toolkit_env_path,
                                                                             self.res_dir, self.slog_stdout)
        elif "framework_profiling" == scene:
            cmd = "cd {}; {}; bash run_lenet_framework_profiling.sh {} > {}".format(app_path, self.cfg_path.toolkit_env_path,
                                                                             self.res_dir, self.slog_stdout)
        elif "all_profiling" == scene:
            cmd = "cd {}; {}; bash run_lenet_all_profiling.sh {} > {}".format(app_path, self.cfg_path.toolkit_env_path,
                                                                             self.res_dir, self.slog_stdout)

        self.clear_result_dir()
        startTime = datetime.now()
        self.subprocess_cmd(cmd)
        diffTime = datetime.now() - startTime
        self.duration_time = diffTime.total_seconds()
        self.view_error_msg(self.res_dir, "plog")
        if self.res == 0:
            self.__result = self.cfg_value.pass_res
        else:
            self.__result = self.cfg_value.fail_res
        self.__logger.info("\n------------------------------- "
                           "End execute case {} -------------------------------".format(self.__id))

    def case_parse_performance(self, scene=None):
        def remove_files(prof_file_path):
            cmd = f'rm -rf {prof_file_path}/*.json; \
                    rm -rf {prof_file_path}/device_*/log; \
                    rm -rf {prof_file_path}/device_*/sqlite; \
                    rm -rf {prof_file_path}/device_*/summary; \
                    rm -rf {prof_file_path}/device_*/timeline; \
                    rm {prof_file_path}/device_*/data/all_file.complete; \
                    rm -rf {prof_file_path}/mindstudio_profiler_log; \
                    rm -rf {prof_file_path}/mindstudio_profiler_output; \
                    rm -rf {prof_file_path}/*.json; \
                    rm -rf {prof_file_path}/host/sqlite; \
                    rm -rf {prof_file_path}/host/log; \
                    rm {prof_file_path}/host/data/all_file.complete;'
            self.subprocess_cmd(cmd)

        from os.path import join, getsize
        def get_dir_mb_size(dir):
            size = 0
            for root, dirs, files in os.walk(dir):
                size += sum([getsize(join(root, name)) for name in files])
            return round(size / 1024 / 1024, 2)

        remove_files(ConfigPaths.parse_performance_path)
        self.__logger.info("before parse remove files done")

        STD_RUNTIME = 1320 # 标准时间/s
        time_error = 0.07 # 最大可容忍误差 百分比
        MAX_RUN_TIME = int(STD_RUNTIME * (1 + time_error))

        STD_FILE_SIZE = 3964 # 标准解析后大小/MB
        file_error = 0.005 # 最大可容忍误差 百分比
        MAX_FILE_SIZE = int(STD_FILE_SIZE * (1 + file_error))
        MIN_FILE_SIZE = int(STD_FILE_SIZE * (1 - file_error))

        msprofbin_cmd = "{} > {}".format(self.cfg_cmd.perf_cmd, self.slog_stdout)
        t1 = time.time()
        self.run_analysis_cmd_and_parse(msprofbin_cmd, scene)
        run_t = time.time() - t1
        file_size = get_dir_mb_size(ConfigPaths.parse_performance_path)

        if run_t > MAX_RUN_TIME:
            self.res += 1
            self.__logger.error("Parse Performance check failed : %d > %d", run_t, MAX_RUN_TIME)
        else:
            self.__logger.info("Parse Performance check pass : %d <= %d", run_t, MAX_RUN_TIME)

        if not MIN_FILE_SIZE <= file_size <= MAX_FILE_SIZE:
            self.res += 1
            self.__logger.error("Paerse gen file size check failed : %d not in [%d, %d]",
                                file_size, MIN_FILE_SIZE, MAX_FILE_SIZE)
        else:
            self.__logger.info("Paerse gen file size check pass : %d in [%d, %d]",
                                file_size, MIN_FILE_SIZE, MAX_FILE_SIZE)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-id', '--id', help='test case')
    parser.add_argument('-s', '--scene', required=True,
                        help='run App or Sys or All')
    parser.add_argument('-m', '--mode', required=True,
                        help='mode in slogConfig')
    parser.add_argument('-p', '--params',
                        help='params of runtime test script')
    parser.add_argument('-t', '--timeout',
                        help='timeout of test case')
    args = parser.parse_args()
    suite = unittest.TestSuite()
    suite.addTest(
        TestProfiling(args.id, args.scene, args.mode, args.params,
                      timeout=480 if not args.timeout else int(args.timeout)))
    unittest.TextTestRunner(verbosity=2).run(suite)
