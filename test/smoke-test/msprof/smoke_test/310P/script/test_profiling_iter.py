#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
import re
import sys
import json
import time
import shutil
import logging
import argparse
import unittest
import threading
import subprocess
from collections import defaultdict
import pandas

from _cfg_iter import *

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


class TestProfiling(unittest.TestCase):
    def __init__(self, tc, scene, mode, params, model_id, iteration_id, timeout):
        self.__id = tc
        self.__scene = scene
        self.__mode = mode
        self.__params = params
        self.__model_id = model_id
        self.__iteration_id = iteration_id
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
        self.run_path = ""
        self.switch_values = []
        self.op_summary_op_num = 0
        self.op_statistic_op_num = 0
        self.hccl_statistic_op_num = 0
        self.op_summary_context = pandas.DataFrame()
        self.api_statistic_context = pandas.DataFrame()
        self.hccl_statistic_context = pandas.DataFrame()
        self.op_type_baseline = {
            "PromptFlashAttention": 216,
            "Axpy": 12,
            "BatchMatMulV2": 82,
            "Cast": 33,
            "GatherV2": 8238,
            "Gelu": 51,
            "LayerNormV3": 86,
            "Mul": 19,
            "SplitV": 13.5,
            "TopKV2": 483,
            "Conv2D": 14,
            "Pooling": 5,
            "FullyConnection": 8,
            "SoftmaxV2": 2
        }
        self.__logger.info("------------------------------- "
                           "Start execute case {} -------------------------------".format(self.__id))

    def write_res(self):
        with open('result.txt', 'a+') as f:
            f.write('%s %s\n' % (self.__id, self.__result))

    def execute(self):
        self.init()
        eval("self.{}()".format(self.__mode))
        self.write_res()
        self.assertTrue(self.cfg_value.pass_res == self.__result)

    def get_switch_value(self, switch_cmd):
        if "AclApi" == self.__scene or "AclApiStepInfo" == self.__scene:
            self.switch_values = self.cfg_value.aclapi_switch
        elif "AclJson" == self.__scene:
            self.switch_values = self.cfg_value.acljson_switch
        elif "AddOp" == self.__scene or "MsprofbinPyTorch" == self.__scene or "MsprofbinTensorFlow" == self.__scene \
                or "DynamicLaunch" == self.__scene or "DynamicAttach" == self.__scene \
                or "MsprofbinMindSpore" == self.__scene:
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
                    or "CannProfilingPyTorch" == self.__scene or "SingleOp65535PyTorch" == self.__scene \
                    or "SingleOpPyTorch" == self.__scene or "StepPyTorch" == self.__scene or "App" == self.__scene \
                    or "AclApi" == self.__scene or "AclJson" == self.__scene or "All" == self.__scene \
                    or "PyAclApi" == self.__scene or "GraphApi" == self.__scene or "DynamicLaunch" == self.__scene \
                    or "DynamicAttach" == self.__scene or "AclApiStepInfo" == self.__scene:
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
        if "AddOp" == self.__scene or "SingleOp" == self.__scene or "MsprofbinPyTorch" == self.__scene:
            dev_json_lst = [_data for _data in dev_json_lst if _data != "ge" if _data != "step_trace"]
            dev_csv_lst = [_data for _data in dev_csv_lst if _data != "ge" if _data != "aicpu"
                           if _data != "fusion_op" if _data != "step_trace"]
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
        except (Exception, TimeoutError) as err:
            res = err
            self.__logger.error(res)
        finally:
            pass
        return res

    def view_dev_host_timeline_file(self, prof_dir):
        self.__logger.info("view prof dir ...")
        dev_lst = []
        host_lst = []
        timeline_lst = []
        for prof in prof_dir:
            for dir_ in os.listdir(prof):
                if "dev" in dir_:
                    dev_lst.append(os.path.join(prof, dir_))
                elif "host" in dir_:
                    host_lst.append(os.path.join(prof, dir_))
                else:
                    timeline_lst.append(os.path.join(prof, dir_))
        self.__logger.warning("device_dir: {}".format(dev_lst))
        self.__logger.warning("host_dir: {}".format(host_lst))
        self.__logger.warning("timeline_dir: {}".format(timeline_lst))
        return dev_lst, host_lst, timeline_lst

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
        if os.path.exists(self.run_path + "/info_iter.txt"):
            cmd = "cat {} | grep op_num".format(self.run_path + "/info_iter.txt")
            last_op_num = self.subprocess_cmd(cmd).split("=")[1]
            if self.op_summary_op_num != int(last_op_num):
                self.__logger.error("current op summary num {} not equal to last op num {}".format(
                    self.op_summary_op_num, last_op_num))
                self.res += 1
            else:
                self.__logger.info("current op summary num {} equal to last op num {}".format(
                    self.op_summary_op_num, last_op_num))
        if self.hccl_statistic_op_num + self.op_statistic_op_num != self.op_summary_op_num:
            self.__logger.error("hccl statistic num {} and op statistic num {} not equal to op summary num {}".format(
                                self.hccl_statistic_op_num, self.op_statistic_op_num, self.op_summary_op_num))
            self.res += 1
        self.__logger.info("hccl statistic num {}; op statistic num {}; op summary num {}".format(
                           self.hccl_statistic_op_num, self.op_statistic_op_num, self.op_summary_op_num))

    def Traverse_op_summary(self):
        # Traverse op_summary.csv to obtain data required by testcase.
        # Verify the testcase of each operator at the same time.
        csv_statistic = {
            'op_summary_num': defaultdict(int),
            'op_type_cost': defaultdict(dict),
        }
        state_total = [0, 0]
        if os.path.exists('/home/zhaolei184/threshold/threshold.json'):
            with open('/home/zhaolei184/threshold/threshold.json', 'r') as f:
                csv_statistic['op_type_cost'] = json.load(f)

        for index, item in self.op_summary_context.iterrows():
            csv_statistic['op_summary_num'][item['OP Type']] += 1
            state = self.check_op_dimension(item)
            state_total[0] += state[0]
            state_total[1] += state[1]
            key = item['OP Type'] + '-' + str(item['Input Shapes'])[1:-1] + '-' + str(item['Output Shapes'])[1:-1]
            if item['OP Type'] in self.op_type_baseline and key not in self.op_type_baseline:
                self.op_type_baseline[key] = float(item['Task Duration(us)'])

            if key in self.op_type_baseline:
                if key in csv_statistic['op_type_cost']:
                    csv_statistic['op_type_cost'][key]['total'] += float(item['Task Duration(us)'])
                    csv_statistic['op_type_cost'][key]['count'] += 1
                else:
                    csv_statistic['op_type_cost'].update({key: {"total": float(item['Task Duration(us)']), "count": 1}})
        return csv_statistic, state_total

    def check_max_duration(self, op_type_cost):
        # Check the upper limit of the average fluctuation threshold of the operator task duration.
        for op_type in op_type_cost:
            op_cost = op_type_cost[op_type]['total'] / op_type_cost[op_type]['count']
            if not (self.op_type_baseline[op_type] * 0.8 < op_cost < self.op_type_baseline[op_type] * 1.2):
                self.__logger.warning(
                    f"task duration not match baseline, {op_type}\n{self.op_type_baseline[op_type]}\n{op_cost}")
            self.__logger.info(f"op type {op_type} check finished. Mean time: {op_cost}")
        with open('/home/zhaolei184/threshold/threshold.json', 'w') as f:
            threshold = json.dumps(op_type_cost)
            f.write(threshold)

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

    def check_aclnn_api(self, op_summary_num):
        pass

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
        pass

    def view_summary_content(self, summary_dir, summary_file):
        for index, csv in enumerate(sorted(os.listdir(summary_dir))):
            if csv.endswith("json"):
                continue
            self.__logger.info("start view {} content ...".format(csv))
            cmd = "cat {} | head -2".format(os.path.join(summary_dir, csv))
            res_content = self.subprocess_cmd(cmd)
            self.__logger.info("csv content: {}".format(res_content.split("\n")))
            header = res_content.split("\n")[0]
            under_header = res_content.split("\n")[1]
            # 判断表头数据和config.ini文件数据是否一�?
            if "ai_core_utilization" == summary_file[index] or "op_summary" == summary_file[index]:
                if "op_summary" == summary_file[index]:
                    cmd = "cat {} | wc -l".format(os.path.join(summary_dir, csv))
                    self.op_summary_op_num = int(self.subprocess_cmd(cmd)) - 1
                    self.op_summary_context = pandas.read_csv(os.path.join(summary_dir, csv))
                if not re.findall(r"aic-metrics\S+", str(self.switch_values)):
                    if "AclApi" == self.__scene or "AclJson" == self.__scene or "ExportTensorFlow" == self.__scene \
                            or "SessionRunTensorFlow" == self.__scene or "AclApiStepInfo" == self.__scene:
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
                    elif "SingleOp" == self.__scene or "InnerOneStepPyTorch" == self.__scene \
                            or "SingleOp65535PyTorch" == self.__scene or "SingleOpPyTorch" == self.__scene \
                            or "StepPyTorch" == self.__scene or "CannProfilingPyTorch" == self.__scene:
                        aic_metrics = "PipeUtilization_"
                        aic_mode = "task-based"
                    else:
                        aic_metrics = "PipeUtilization"
                        aic_mode = "sample-based"
                    except_res = self.cfg_header.table_header[summary_file[index]][aic_mode][aic_metrics]
                else:
                    aic_metrics = [_data.split("=")[1] for _data in self.switch_values if "aic-metrics" in _data]
                    aic_mode = [_data.split("=")[1] for _data in self.switch_values if "aic-mode" in _data]
                    if "AddOp" == self.__scene or "MsprofbinPyTorch" == self.__scene \
                            or "CannProfilingPyTorch" == self.__scene or "DynamicLaunch" == self.__scene \
                            or "DynamicAttach" == self.__scene or "MsprofbinMindSpore" == self.__scene:
                        except_res = self.cfg_header.table_header[summary_file[index]][aic_mode[0]][aic_metrics[0]+"_"]
                    elif "Custom" in aic_metrics[0]:
                        if "ai_core_utilization" == summary_file[index]:
                            except_res = ((aic_metrics[0]).replace("0x", "r")).replace("Custom:", "Core ID,")
                        else:
                            if aic_mode[0] == "sample-based":
                                except_res = self.cfg_header.table_header[summary_file[index]][aic_mode[0]]["MemoryUB"]
                            else:
                                except_res = self.cfg_header.table_header[summary_file[index]][aic_mode[0]]["MemoryUB"]
                                except_res = except_res.split("total_cycles,")
                                except_res = except_res[0] + "total_cycles," \
                                                           + aic_metrics[0].replace("0x", "r").replace("Custom:", "")
                    else:
                        except_res = self.cfg_header.table_header[summary_file[index]][aic_mode[0]][aic_metrics[0]]
            elif "op_statistic" == summary_file[index]:
                cmd = "cat {} | head -1".format(os.path.join(summary_dir, csv))
                op_statistic_header = self.subprocess_cmd(cmd)
                if "Model Name" in op_statistic_header:
                    cmd = "awk -F \',\' \'{{sum += $4}} END {{print sum}}\' {}".format(os.path.join(summary_dir, csv))
                else:
                    cmd = "awk -F \',\' \'{{sum += $3}} END {{print sum}}\' {}".format(os.path.join(summary_dir, csv))
                self.op_statistic_op_num = int(self.subprocess_cmd(cmd))
                if "AddOp" == self.__scene or "SingleOp" == self.__scene or "MsprofbinPyTorch" == self.__scene \
                        or "InnerOneStepPyTorch" == self.__scene or "SingleOp65535PyTorch" == self.__scene \
                        or "SingleOpPyTorch" == self.__scene or "StepPyTorch" == self.__scene \
                        or "CannProfilingPyTorch" == self.__scene or "DynamicLaunch" == self.__scene \
                        or "DynamicAttach" == self.__scene or "MsprofbinMindSpore" == self.__scene:
                    except_res = self.cfg_header.table_header[summary_file[index]]["SingleOp"]
                else:
                    except_res = self.cfg_header.table_header[summary_file[index]]["WholeNetwork"]
            elif "api_statistic" == summary_file[index]:
                self.api_statistic_context = pandas.read_csv(os.path.join(summary_dir, csv))
                except_res = self.cfg_header.table_header[summary_file[index]]
            else:
                except_res = self.cfg_header.table_header[summary_file[index]]
            if os.getenv("MSPROF_VERSION") == "tr5" or os.getenv("MSPROF_VERSION") == "TR5":
                except_res = except_res.replace("HF32 Eligible,", "")
            if header != except_res:
                self.__logger.error(except_res)
                self.res += 1
            # 判断表头下第一行数据是否为�?
            under_header_lst = under_header.split(", ")
            for value in under_header_lst:
                if not value:
                    self.__logger.error(value)
                    self.res += 1

        self.check_op_num_valid()

    def view_iteration_msg(self, argv_path):
        self.__logger.info("view prof iterations format ...")
        max_iter_format = ["0", "0", "1"]
        cmd = "{} {}".format(self.cfg_path.msprofinfo_path, argv_path)
        res = self.subprocess_cmd(cmd)
        try:
            json_dict = json.loads(res)
            if len(json_dict["data"]["model_info"]["iterations"]) == 0:
                max_iter_format = ["1"]
            else:
                for iteration in json_dict["data"]["model_info"]["iterations"]:
                    if str(iteration["Iteration Number"]).isdigit() and \
                                int(iteration["Iteration Number"]) >= int(max_iter_format[-1]):
                            max_iter_format[0] = str(iteration["Device Id"])
                            max_iter_format[1] = str(iteration["Model Id"])
                            max_iter_format[2] = str(iteration["Iteration Number"])
                    elif str(iteration["Iteration Number"]) == "N/A":
                        del max_iter_format[0]
                        max_iter_format[0] = str(iteration["Device Id"])
                    else:
                        self.__logger.warning(iteration)
                    max_iter_format[-1] = "1"
                if "4294967295" == max_iter_format[1]:
                    del max_iter_format[1]
                if "DockerExportTensorFlow" == self.__scene:
                    max_iter_format[1] = "1"
            self.__logger.warning("iterations format: {}".format(max_iter_format))
            return max_iter_format
        except Exception as error:
            self.__logger.error(error)
            return max_iter_format

    def view_timeline_summary_file(self, argv_path, switch_res, iter_format):
        self.__logger.info("start view {} ...".format(argv_path))
        argv_res = sorted(os.listdir(argv_path))
        self.__logger.info("file list: {}".format(argv_res))
        if argv_res:
            if "json" not in argv_res[0]:
                if "_".join(iter_format) not in argv_res[0]:
                    self.__logger.error(iter_format)
                    self.res += 1
        for index, item in enumerate(argv_res):
            # 获取 ai_stack_time_*.json 中ai_stack_time字符�?
            item = "_".join([k for k in (str(item.split(".")[0]).split("_")) if k.isalpha()])
            if "slice" in item:
                item.rstrip("_slice")
            if "cache" == item:
                item = "l2_cache"
            # 更新 timeline_res
            argv_res[index] = item
        if re.search(self.cfg_value.timeline, argv_path):
            if re.search(self.cfg_value.device, argv_path):
                if argv_res != switch_res["dev_json"]:
                    self.__logger.info("Current result: {}".format(argv_res))
                    self.__logger.info("Correct result: {}".format(switch_res["dev_json"]))
                    self.__logger.error("Different result: {}".format(set(switch_res["dev_json"]) ^ set(argv_res)))
                    self.res += 1
            elif re.search(self.cfg_value.host, argv_path):
                if argv_res != switch_res["host_json"]:
                    self.__logger.info("Current result: {}".format(argv_res))
                    self.__logger.info("Correct result: {}".format(switch_res["host_json"]))
                    self.__logger.error("Different result: {}".format(set(switch_res["host_json"]) ^ set(argv_res)))
                    self.res += 1
            elif re.search(self.cfg_value.timeline, argv_path):
                if argv_res != switch_res["timeline_json"]:
                    self.res += 1
        elif re.search(self.cfg_value.summary, argv_path):
            if re.search(self.cfg_value.device, argv_path):
                if argv_res != switch_res["dev_csv"]:
                    self.__logger.info("Current result: {}".format(argv_res))
                    self.__logger.info("Correct result: {}".format(switch_res["dev_csv"]))
                    self.__logger.error("Different result: {}".format(set(switch_res["dev_csv"]) ^ set(argv_res)))
                    self.res += 1
                self.view_summary_content(argv_path, argv_res)
            elif re.search(self.cfg_value.host, argv_path):
                if argv_res != switch_res["host_csv"]:
                    self.__logger.info("Current result: {}".format(argv_res))
                    self.__logger.info("Correct result: {}".format(switch_res["host_csv"]))
                    self.__logger.error("Different result: {}".format(set(switch_res["host_csv"]) ^ set(argv_res)))
                    self.res += 1
                self.view_summary_content(argv_path, argv_res)

    def view_dev_host_file(self, argv_path, switch_res):
        path_res = os.listdir(argv_path)
        path_res.sort()
        if re.search(self.cfg_value.device, argv_path):
            max_iter_msg = self.view_iteration_msg(argv_path)
            self.__logger.info("Current Device result: {}".format(path_res))
            dev_id = argv_path.split("_")[-1]
            prof_res = self.cfg_switch.prof_dev_dir
            if not switch_res["dev_json"]:
                prof_res.remove("timeline")
            if not switch_res["dev_csv"]:
                prof_res.remove("summary")
            prof_res = self.replace_dev_id(prof_res, dev_id)
        else:
            self.__logger.info("Current Host res: {}".format(path_res))
            max_iter_msg = ""
            prof_res = self.cfg_switch.prof_host_dir
            if not switch_res["host_json"]:
                prof_res.remove("timeline")
            if not switch_res["host_csv"]:
                prof_res.remove("summary")
        timeline_dir = os.path.join(argv_path, "timeline")
        summary_dir = os.path.join(argv_path, "summary")
        log_dir = os.path.join(os.path.dirname(argv_path), "mindstudio_profiler_log")
        if not prof_res == path_res:
            self.__logger.info("Correct result: {}".format(prof_res))
            self.__logger.error("Different result: {}".format(set(prof_res) - set(path_res)))
            self.res += 1
        if "timeline" not in self.cfg_switch.prof_dev_dir:
            self.cfg_switch.prof_dev_dir.append("timeline")
        if "summary" not in self.cfg_switch.prof_dev_dir:
            self.cfg_switch.prof_dev_dir.append("summary")
        self.view_data_file(argv_path)
        self.view_error_msg(log_dir, "collection")
        if os.path.exists(timeline_dir):
            self.view_timeline_summary_file(timeline_dir, switch_res, max_iter_msg)
        if os.path.exists(summary_dir):
            self.view_timeline_summary_file(summary_dir, switch_res, max_iter_msg)

    def view_timeline_file(self, argv_path):
        cmd = "du -b {}/msprof*.json".format(argv_path)
        res_du = self.subprocess_cmd(cmd)
        self.__logger.info(res_du)
        if not self.is_int(res_du.split()[0]):
            return False
        if int(res_du.split()[0]) <= 2:
            if not re.search(self.cfg_value.timeline, argv_path):
                return False
            self.__logger.error("timeline msprof*.json size is false.")
            self.res += 1
        return True

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
            dev_res, host_res, timeline_res = self.view_dev_host_timeline_file(prof_lst)
            try:
                for dev in dev_res:
                    self.view_dev_host_file(dev, switch_res)
                if "Sys" != self.__scene:
                    self.view_dev_host_file(host_res[0], switch_res)
                if switch_res["dev_json"] and switch_res["host_json"]:
                    if self.view_timeline_file(os.path.join(dev_res[0], self.cfg_value.timeline)) \
                            and self.view_timeline_file(os.path.join(host_res[0], self.cfg_value.timeline)):
                        self.view_timeline_file(timeline_res[0])
            except (IndexError, Exception) as err:
                self.__logger.error(err)
                self.res += 1

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
        self.subprocess_cmd(msprofbin_cmd)
        self.view_error_msg(self.res_dir, "plog")
        self.view_res_file(self.res_dir, switch_res)
        if self.res == 0:
            self.__result = self.cfg_value.pass_res
        else:
            self.__result = self.cfg_value.fail_res
        self.__logger.info("\n------------------------------- "
                           "End execute case {} -------------------------------".format(self.__id))

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
        if "App" == scene:
            msprofbin_cmd = ("{} --output={} --{} --{} --{} --model-id={} --iteration-id={} > {}"
                             .format(self.cfg_cmd.all_app_switch, self.res_dir,
                                     aic_mode, aic_metrics, llc_profiling, self.__model_id, self.__iteration_id,
                                     self.slog_stdout))
        else:
            msprofbin_cmd = ("{} --output={} --{} --{} --{} --model-id={} --iteration-id={} > {}"
                             .format(self.cfg_cmd.all_switch, self.res_dir,
                                     aic_mode, aic_metrics, llc_profiling, self.__model_id, self.__iteration_id,
                                     self.slog_stdout))
        self.msprofbin_and_parse(msprofbin_cmd)

    def case_api_scene(self, scene=None):
        scene = self.__scene if scene is None else scene
        if "AclApi" == scene:
            app_path = self.cfg_path.aclapi_path
        elif "AclApiStepInfo" == scene:
            app_path = self.cfg_path.aclapiStepInfo_path
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
        else:
            app_path = self.cfg_path.opst_path
        self.run_path = app_path
        cmd = "cd {}; {}; bash run_api.sh {} {} {} > {}".format(app_path, self.cfg_path.toolkit_env_path,
                                                                self.res_dir, self.__model_id, self.__iteration_id,
                                                                self.slog_stdout)
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
    parser.add_argument('--model-id', help='model id to export', default=1)
    parser.add_argument('--iteration-id', help='iteration id to export', default=1)
    args = parser.parse_args()
    suite = unittest.TestSuite()
    suite.addTest(
        TestProfiling(args.id, args.scene, args.mode, args.params,  args.model_id, args.iteration_id,
                      timeout=480 if not args.timeout else int(args.timeout)))
    unittest.TextTestRunner(verbosity=2).run(suite)
