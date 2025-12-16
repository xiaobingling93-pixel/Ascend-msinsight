#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
import re
import subprocess
import time
import unittest
import logging
from datetime import datetime
from file_analyzer import JsonAnalyzer
from file_analyzer import DBAnalyzer
from file_analyzer import CsvAnalyzer

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')

OUTPUT_PATH1 = "/home/result_dir/pixel_level_check/test1"
PLOG1 = f"{OUTPUT_PATH1}/pixel_level_check.log"
CMD1 = (f"cd /home/msprof_smoke_test/model/Pytorch_cover_more_scenarios;"
        f"source /root/miniconda3/bin/activate smoke_test_env_pta;"
        f"export ASCEND_GLOBAL_EVENT_ENABLE=1;"
        f"export ASCEND_SLOG_PRINT_TO_STDOUT=1;"
        f"source /usr/local/Ascend/ascend-toolkit/set_env.sh;"
        f"msprof --application=\"python3 demo.py\" --output={OUTPUT_PATH1} --ascendcl=on --runtime-api=on "
        f"--task-time=on --aicpu=on --ai-core=on --l2=on "
        f"--llc-profiling=read --sys-cpu-profiling=on --sys-profiling=on --sys-io-profiling=on "
        f"--sys-interconnection-profiling=on > {PLOG1};"
        f"msprof --export=on --type=db --output={OUTPUT_PATH1}")

OUTPUT_PATH2 = "/home/result_dir/pixel_level_check/test2"
PLOG2 = f"{OUTPUT_PATH2}/pixel_level_check.log"
CMD2 = (f"cd /home/msprof_smoke_test/model/Pytorch_cover_more_scenarios;"
        f"source /root/miniconda3/bin/activate smoke_test_env_pta;"
        f"export ASCEND_GLOBAL_EVENT_ENABLE=1;"
        f"export ASCEND_SLOG_PRINT_TO_STDOUT=1;"
        f"source /usr/local/Ascend/ascend-toolkit/set_env.sh;"
        f"msprof --application=\"python3 single_op.py\" --output={OUTPUT_PATH2} --ascendcl=on --runtime-api=on "
        f"--task-time=on --aicpu=on --ai-core=on --l2=on --sys-hardware-mem=on "
        f"--llc-profiling=read --sys-cpu-profiling=on --sys-profiling=on --sys-io-profiling=on "
        f"--sys-interconnection-profiling=on > {PLOG2};"
        f"msprof --export=on --type=db --output={OUTPUT_PATH2}")


class PixelLevelCheck(unittest.TestCase):
    """
    像素级校验，主要用于对比统一db和json、csv的内容是否一致
    """

    def __init__(self, output_path, plog_path, cmd):
        super().__init__("execute")
        self.output_path = output_path
        self.plog_path = plog_path
        self.cmd = cmd
        self.prof_path = ""
        self.db_path = ""  # 统一db的路径
        self.msprof_json_path = ""  # msprof_xxx.json的路径
        self.op_summary_path = ""  # op_summary_xxx.csv的路径
        self.logger = logging
        self.id = "test_pixel_level_check"  # 用例名字
        self.duration_time = 0  # 对比的时间
        self.res = 0  # 只要不为0，说明这个用例失败
        self.result = ""  # 根据self.res的结果，0的话是pass, 非0为fail，然后写到result.txt里面
        self.db_analyzer = DBAnalyzer()
        self.msprof_json_analyzer = JsonAnalyzer()

    @staticmethod
    def compare_length(obj1, obj2):
        if len(obj1) != len(obj2):
            return False
        return True

    @staticmethod
    def get_db_path(path):
        # path为PROF目录的路径
        for file in os.listdir(path):
            if file.startswith("msprof") and file.endswith(".db"):
                return os.path.join(path, file)
        return ""

    @staticmethod
    def get_text_path(prof_path):
        mindstudio_profiler_output = os.path.join(prof_path, "mindstudio_profiler_output")
        op_summary_path = ""
        msprof_json_path = ""
        for file in os.listdir(mindstudio_profiler_output):
            if file.startswith("op_summary") and file.endswith(".csv"):
                op_summary_path = os.path.join(mindstudio_profiler_output, file)
            elif file.startswith("msprof") and file.endswith(".json"):
                msprof_json_path = os.path.join(mindstudio_profiler_output, file)
        return op_summary_path, msprof_json_path

    def init(self):
        self.logger.info("------------------------------- "
                         "Start execute case {} -------------------------------".format(self.id))
        self.get_all_path()
        self.db_analyzer = DBAnalyzer(self.db_path)
        self.msprof_json_analyzer = JsonAnalyzer(self.msprof_json_path)
        flag = True
        if not self.db_path:
            self.logger.error("msprof_xxx.db is not existed")
            flag = False
        if not self.op_summary_path:
            self.logger.error("op_summary_xxx.csv is not existed")
            flag = False
        if not self.msprof_json_path:
            self.logger.error("msprof_xxx.json is not existed")
            flag = False
        return flag

    def finalize(self):
        self.db_analyzer.finalize()
        self.check_result()
        self.logger.info("------------------------------- "
                         "End execute case {} -------------------------------".format(self.id))

    def check_result(self):
        if self.res == 0:
            self.result = "pass"
        else:
            self.result = "fail"
        # 记录执行结果到result.txt
        self.write_res()
        self.assertTrue("pass" == self.result)

    def pixel_level_check(self):
        # 首先要组装好各种数据的一个公共的数据结构
        self.db_analyzer.connect_db()
        self.msprof_json_analyzer.load_json()
        self.db_analyzer.generate_data_instance()
        self.msprof_json_analyzer.generate_data_instance()
        # 开始对比每一项
        self.compare_ascend_task_data()
        self.compare_communication_op()
        self.compare_communication_task_info()
        # self.compare_compute_task_info()  # TODO: op_summary还没有C化，暂时不做对比
        self.compare_aicore_freq()
        self.compare_npu_mem()
        # self.compare_acc_pmu() # text现在是柱状图，db和json里面确实不一样，可视化说先不改db
        self.compare_hccs()
        self.compare_nic()
        self.compare_roce()
        self.compare_pcie()
        self.compare_hbm()
        self.compare_stars_soc_info()
        self.compare_llc()
        self.compare_qos()
        self.compare_wired()
        self.compare_cann_api()

    def write_res(self):
        with open('result.txt', 'a+') as f:
            f.write('%s %s %s\n' % (self.id, self.result, self.duration_time))

    def subprocess_cmd(self, cmd):
        self.logger.info("host command: {}".format(cmd))
        try:
            result = subprocess.run(['bash', '-c', cmd], capture_output=True, text=True)
            if result.returncode != 0 and len(result.stderr) != 0:
                self.logger.error(result.stderr)
                self.res += 1
                return result.stderr
        except (Exception, TimeoutError) as err:
            self.logger.error(err)
            self.res += 1
            return err
        finally:
            pass
        return result.stdout

    def view_error_msg(self, log_path, log_type=""):
        self.logger.info("start view {} log ...".format(log_type))
        if log_type == "plog":
            cmd = r"grep -rn 'ERROR\] PROFILING' {0}; grep -rn '\[ERROR\] \[MSVP\]' {0}; " \
                  r"grep -rn 'ERROR\] Failed' {0}".format(log_path)
        else:
            cmd = r"grep -rn 'ERROR\]' {0}".format(log_path)
        res = self.subprocess_cmd(cmd)
        if re.search(r"ERROR", res):
            self.logger.error(res)
            self.res += 1

    def view_log(self):
        log = os.path.join(self.prof_path, "mindstudio_profiler_log")
        self.view_error_msg(self.plog_path, "plog")
        self.view_error_msg(log)

    def get_all_path(self):
        for file in os.listdir(self.output_path):
            prof_path = os.path.join(self.output_path, file)
            device_0_path = os.path.join(prof_path, "device_0")
            if os.path.exists(device_0_path):
                self.prof_path = prof_path
                self.db_path = self.get_db_path(prof_path)
                self.op_summary_path, self.msprof_json_path = self.get_text_path(prof_path)

    def execute(self):
        self.start_time = time.time_ns()
        start_time = datetime.now()
        self.subprocess_cmd(self.cmd)
        try:
            if self.init():
                # log校验
                self.view_log()
                self.pixel_level_check()
            else:
                self.logger.error("Init failed, please check the msprof analysis result")
                self.res += 1
        except Exception as err:
            self.logger.error(f"Execute {self.id} failed: {err}")
            self.res += 1
        time_diff = datetime.now() - start_time
        self.duration_time = time_diff.total_seconds()
        self.finalize()

    def compare_ascend_task_data(self):
        json_obj = self.msprof_json_analyzer.ascend_task_data_obj
        db_obj = self.db_analyzer.ascend_task_data_obj
        if not self.compare_length(db_obj, json_obj):
            self.logger.error(f"DB的TASK表和json的Ascend Hardware层的数据量不一致，分别是{len(db_obj)},"
                              f"{len(json_obj)}")
            self.res += 1
        for key, value in db_obj.items():
            if key not in json_obj:
                self.logger.error(f"[关键词：{key}]: json的Ascend Hardware层没有这一条数据，可能是两边时间不一致造成的")
                continue
            else:
                if value != json_obj[key]:
                    self.logger.error(f"[关键词：{key}]: DB的TASK表和json的Ascend Hardware层的数据不一致")
                    self.res += 1

    def compare_communication_op(self):
        json_obj = self.msprof_json_analyzer.communication_op_obj
        db_obj = self.db_analyzer.communication_op_obj
        if not self.compare_length(db_obj, json_obj):
            self.logger.error(f"DB的COMMUNICATION_OP表和json的HCCL层的通信大算子的数据量不一致，分别是{len(db_obj)},"
                              f"{len(json_obj)}")
            self.res += 1
        # for key, value in db_obj.items():
        #     if key not in json_obj:
        #         self.logger.error(f"[关键词：{key}]: json的HCCL层没有这个通信大算子，可能是两边时间不一致造成的")
        #         continue
        #     else:
        #         if value != json_obj[key]:
        #             self.logger.error(f"[关键词：{key}]: DB的COMMUNICATION_OP表和json的HCCL层的通信大算子的数据不一致")
        #             self.res += 1

    def compare_communication_task_info(self):
        json_obj = self.msprof_json_analyzer.communication_task_info_obj
        db_obj = self.db_analyzer.communication_task_info_obj
        if not self.compare_length(db_obj, json_obj):
            self.logger.error(
                f"DB的COMMUNICATION_TASK_INFO表和json的HCCL层的通信小算子的数据量不一致，分别是{len(db_obj)},"
                f"{len(json_obj)}")
            self.res += 1
        # for key, value in db_obj.items():
        #     if key not in json_obj:
        #         self.logger.error(f"[关键词：{key}]: json的HCCL层没有这个通信小算子，可能是两边时间不一致造成的")
        #         continue
        #     else:
        #         if value != json_obj[key]:
        #             self.logger.error(
        #                 f"[关键词：{key}]: DB的COMMUNICATION_TASK_INFO表和json的HCCL层的通信小算子的数据不一致")
        #             self.res += 1

    def compare_compute_task_info(self):
        csv_obj = CsvAnalyzer.generate_compute_task_info(self.op_summary_path)
        db_obj = self.db_analyzer.compute_task_info_obj
        if not self.compare_length(db_obj, csv_obj):
            self.logger.error(f"DB的COMMUNICATION_TASK_INFO表和op_summary的计算算子的数据量不一致，分别是{len(db_obj)},"
                              f"{len(csv_obj)}")
            self.res += 1
        for key, value in db_obj.items():
            if key not in csv_obj:
                self.logger.error(f"[关键词：{key}]: op_summary没有这个计算算子，可能是两边时间不一致造成的")
                continue
            else:
                if value != csv_obj[key]:
                    self.logger.error(f"[关键词：{key}]: DB的COMPUTE_TASK_INFO表和op_summary的计算算子的数据不一致")
                    self.res += 1

    def compare_aicore_freq(self):
        json_obj = self.msprof_json_analyzer.aicore_freq_obj
        db_obj = self.db_analyzer.aicore_freq_obj
        if not self.compare_length(db_obj, json_obj):
            self.logger.error(f"DB的AICORE_FREQ表和json的AI Core Freq层的数据量不一致，分别是{len(db_obj)},"
                              f"{len(json_obj)}")
            self.res += 1
        for key, value in db_obj.items():
            if key not in json_obj:
                self.logger.error(f"[关键词：{key}]: json的AI Core Freq层没有这个频率，可能是两边时间不一致造成的")
                continue
            else:
                if value != json_obj[key]:
                    self.logger.error(f"[关键词：{key}]: DB的AICORE_FREQ表和json的AI Core Freq层的数据不一致")
                    self.res += 1

    def compare_npu_mem(self):
        json_obj = self.msprof_json_analyzer.npu_mem_obj
        db_obj = self.db_analyzer.npu_mem_obj
        if not self.compare_length(db_obj, json_obj):
            self.logger.error(f"DB的NPU_MEM表和json的NPU MEM层的数据量不一致，分别是{len(db_obj)},"
                              f"{len(json_obj)}")
            self.res += 1
        for key, value in db_obj.items():
            if key not in json_obj:
                self.logger.error(f"[关键词：{key}]: json的NPU MEM层没有这一条数据，可能是两边时间不一致造成的")
                continue
            else:
                if value != json_obj[key]:
                    self.logger.error(f"[关键词：{key}]: DB的NPU_MEM表和json的NPU MEM层的数据不一致")
                    self.res += 1

    def compare_acc_pmu(self):
        json_obj = self.msprof_json_analyzer.acc_pmu_obj
        db_obj = self.db_analyzer.acc_pmu_obj
        if not self.compare_length(db_obj, json_obj):
            self.logger.error(f"DB的ACC_PMU表和json的Acc PMU层的数据量不一致，分别是{len(db_obj)},"
                              f"{len(json_obj)}")
            self.res += 1
        for key, value in db_obj.items():
            if key not in json_obj:
                self.logger.error(f"[关键词：{key}]: json的Acc PMU层没有这一条数据，可能是两边时间不一致造成的")
                continue
            else:
                if value != json_obj[key]:
                    self.logger.error(f"[关键词：{key}]: DB的ACC_PMU表和json的Acc PMU层的数据不一致")
                    self.res += 1

    def compare_hccs(self):
        json_obj = self.msprof_json_analyzer.hccs_obj
        db_obj = self.db_analyzer.hccs_obj
        if not self.compare_length(db_obj, json_obj):
            self.logger.error(f"DB的HCCS表和json的HCCS层的数据量不一致，分别是{len(db_obj)},"
                              f"{len(json_obj)}")
            self.res += 1
        for key, value in db_obj.items():
            if key not in json_obj:
                self.logger.error(f"[关键词：{key}]: json的HCCS层没有这一条数据，可能是两边时间不一致造成的")
                continue
            else:
                if value != json_obj[key]:
                    if json_obj[key].tx < 0 or json_obj[key].rx < 0:    # TODO
                        continue
                    self.logger.error(f"[关键词：{key}]: DB的HCCS表和json的HCCS层的数据不一致")
                    self.res += 1

    def compare_nic(self):
        json_obj = self.msprof_json_analyzer.nic_obj
        db_obj = self.db_analyzer.nic_obj
        if not self.compare_length(db_obj, json_obj):
            self.logger.error(f"DB的NIC表和json的NIC层的数据量不一致，分别是{len(db_obj)},"
                              f"{len(json_obj)}")
            self.res += 1
        for key, value in db_obj.items():
            if key not in json_obj:
                self.logger.error(f"[关键词：{key}]: json的NIC层没有这一条数据，可能是两边时间不一致造成的")
                continue
            else:
                if value != json_obj[key]:
                    self.logger.error(f"[关键词：{key}]: DB的NIC表和json的NIC层的数据不一致")
                    self.res += 1

    def compare_roce(self):
        json_obj = self.msprof_json_analyzer.roce_obj
        db_obj = self.db_analyzer.roce_obj
        if not self.compare_length(db_obj, json_obj):
            self.logger.error(f"DB的ROCE表和json的RoCE层的数据量不一致，分别是{len(db_obj)},"
                              f"{len(json_obj)}")
            self.res += 1
        for key, value in db_obj.items():
            if key not in json_obj:
                self.logger.error(f"[关键词：{key}]: json的RoCE层没有这一条数据，可能是两边时间不一致造成的")
                continue
            else:
                if value != json_obj[key]:
                    self.logger.error(f"[关键词：{key}]: DB的ROCE表和json的RoCE层的数据不一致")
                    self.res += 1

    def compare_pcie(self):
        json_obj = self.msprof_json_analyzer.pcie_obj
        db_obj = self.db_analyzer.pcie_obj
        if not self.compare_length(db_obj, json_obj):
            self.logger.error(f"DB的PCIE表和json的PCIe层的数据量不一致，分别是{len(db_obj)},"
                              f"{len(json_obj)}")
            self.res += 1
        for key, value in db_obj.items():
            if key not in json_obj:
                self.logger.error(f"[关键词：{key}]: json的PCIe层没有这一条数据，可能是两边时间不一致造成的")
                continue
            else:
                if value != json_obj[key]:
                    self.logger.error(f"[关键词：{key}]: DB的PCIE表和json的PCIe层的数据不一致")
                    self.res += 1

    def compare_hbm(self):
        json_obj = self.msprof_json_analyzer.hbm_obj
        db_obj = self.db_analyzer.hbm_obj
        if not self.compare_length(db_obj, json_obj):
            self.logger.error(f"DB的HBM表和json的HBM层的数据量不一致，分别是{len(db_obj)},"
                              f"{len(json_obj)}")
            self.res += 1
        for key, value in db_obj.items():
            if key not in json_obj:
                self.logger.error(f"[关键词：{key}]: json的HBM层没有这一条数据，可能是两边时间不一致造成的")
                continue
            else:
                if value != json_obj[key]:
                    self.logger.error(f"[关键词：{key}]: DB的HBM表和json的HBM层的数据不一致")
                    self.res += 1

    def compare_stars_soc_info(self):
        json_obj = self.msprof_json_analyzer.stars_soc_info_obj
        db_obj = self.db_analyzer.stars_soc_info_obj
        if not self.compare_length(db_obj, json_obj):
            self.logger.error(f"DB的SOC_BANDWIDTH_LEVEL表和json的Stars Soc Info层的数据量不一致，分别是{len(db_obj)},"
                              f"{len(json_obj)}")
            self.res += 1
        for key, value in db_obj.items():
            if key not in json_obj:
                self.logger.error(f"[关键词：{key}]: json的Stars Soc Info层没有这一条数据，可能是两边时间不一致造成的")
                continue
            else:
                if value != json_obj[key]:
                    self.logger.error(f"[关键词：{key}]: DB的SOC_BANDWIDTH_LEVEL表和json的Stars Soc Info层的数据不一致")
                    self.res += 1

    def compare_llc(self):
        json_obj = self.msprof_json_analyzer.llc_obj
        db_obj = self.db_analyzer.llc_obj
        if not self.compare_length(db_obj, json_obj):
            self.logger.error(f"DB的LLC表和json的LLC层的数据量不一致，分别是{len(db_obj)},"
                              f"{len(json_obj)}")
            self.res += 1
        for key, value in db_obj.items():
            if key not in json_obj:
                self.logger.error(f"[关键词：{key}]: json的LLC层没有这一条数据，可能是两边时间不一致造成的")
                continue
            else:
                if value != json_obj[key]:
                    self.logger.error(f"[关键词：{key}]: DB的LLC表和json的LLC层的数据不一致")
                    self.res += 1

    def compare_qos(self):
        pass

    def compare_wired(self):
        json_obj = self.msprof_json_analyzer.wired_obj
        db_obj = self.db_analyzer.wired_obj
        # if not self.compare_length(db_obj, json_obj):
        #     self.logger.error(f"DB和json的连线的数据量不一致，分别是{len(db_obj)},"
        #                       f"{len(json_obj)}")
        #     self.res += 1
        for key, value in db_obj.items():
            if key not in json_obj:
                self.logger.error(f"[关键词：{key}]: json里面没有这一条连线，请检查")  # TODO:目前缺少EVENT_RECORD和mc2 aicpu算子
                self.res += 1
                continue
            else:
                if value != json_obj[key]:
                    self.logger.error(f"[关键词：{key}]: DB的和json的这条连线的起点或者connection_id不一致,"
                                      f"可能是connection_id生成的规则改变了")
                    self.res += 1

    def compare_cann_api(self):
        json_obj = self.msprof_json_analyzer.api_obj
        db_obj = self.db_analyzer.api_obj
        if not self.compare_length(db_obj, json_obj):
            self.logger.error(f"DB和json的api的数据量不一致，分别是{len(db_obj)},"
                              f"{len(json_obj)}")
            self.res += 1
        for key, value in db_obj.items():
            if key not in json_obj:
                self.logger.error(f"[关键词：{key}]: json里面没有这一个api，请检查: name is {value.name}")
                continue
            else:
                if value != json_obj[key]:
                    self.logger.error(f"[关键词：{key}]: DB的和json的开始时间相同的api的name、end或者level不一致")
                    self.res += 1


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(PixelLevelCheck(OUTPUT_PATH1, PLOG1, CMD1))  # TODO
    suite.addTest(PixelLevelCheck(OUTPUT_PATH2, PLOG2, CMD2))
    unittest.TextTestRunner(verbosity=2).run(suite)
