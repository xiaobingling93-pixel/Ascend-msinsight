import os
import glob
import pandas as pd
import test_base
import argparse
import unittest
import logging
import re

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


def check_time(res, monitor_hccl_kernel=False, monitor_marker=False):
    """
    检查信息中end的时间大于start的时间
    """
    for msg in res:
        if monitor_hccl_kernel:
            time_list = re.findall(r"MsptiActivityKind.MSPTI_ACTIVITY_KIND_\S+, (\d{19}), (\d{19}),", msg)
        elif monitor_marker:
            time_list = re.findall(r"MsptiActivitySourceKind.MSPTI_ACTIVITY_SOURCE_KIND_\S+, \d+, \S+, \S+, "
                                   r"(\d{19}), (\d{19}),", msg)
        else:
            time_list = re.findall(r"start: (\d+), end: (\d+)", msg)
        for start, end in time_list:
            start, end = int(start), int(end)
            assert end > start, f"start时间({start})比end时间({end})晚，具体信息为：{msg}"

class CallbackDomainTest(test_base.TestProfiling):
    def getTestCmd(self, scene=None):
        script_path = os.path.join(self.cfg_path.mspti_sample_path, "callback_domain")
        self.msprofbin_cmd += ("cd {}; "
                               "source {}/set_env.sh; "
                               "bash sample_run.sh > {}").format(script_path,
                                                                 self.cfg_path.ascend_toolkit_path,
                                                                 self.slog_stdout)

    def mspti_test_case_domain_check(self, check_fields: list = None, res=None):
        """
        校验mspti用例集中的domain出现的信息
        """
        for field in check_fields:
            self.assertIn(f"{field} func enter", res, f"{field} enter 不存在")
            self.assertIn(f"{field} func exit", res, f"{field} exit 不存在")
            field_enter = re.findall(rf"{field} func enter", res)
            field_exit = re.findall(rf"{field} func exit", res)
            self.assertEqual(len(field_enter), len(field_exit), f"{field} 的 enter 和 exit 未成对出现")

    def checkResDir(self, scene=None):
        with open(self.slog_stdout, 'r', encoding='utf-8') as txtfile:
            content = txtfile.read()
            fields = ["aclrtMalloc", "aclrtMemcpy", "aclrtLaunchKernelWithHostArgs", "aclrtSynchronizeStream",
                      "aclrtFree", "aclrtDestroyStream", "aclrtResetDevice"]
            self.mspti_test_case_domain_check(check_fields=fields, res=content)

class CallbackMstxTest(test_base.TestProfiling):
    def getTestCmd(self, scene=None):
        script_path = os.path.join(self.cfg_path.mspti_sample_path, "callback_mstx")
        self.msprofbin_cmd += ("cd {}; "
                               "source {}/set_env.sh; "
                               "bash sample_run.sh > {}").format(script_path,
                                                                 self.cfg_path.ascend_toolkit_path,
                                                                 self.slog_stdout)

    def checkResDir(self, scene=None):
        with open(self.slog_stdout, 'r', encoding='utf-8') as txtfile:
            content = txtfile.read()
            pattern_marker_host = r"\[MARKER\] flag: \d+, sourceKind: HOST_DATA, timestamp: \d+, processId: \d+, " \
                                  r"threadId: \d+, name: aclrtLaunchKernelWithHostArgs, domain: default"
            pattern_marker_device = r"\[MARKER\] flag: \d+, sourceKind: DEVICE_DATA, timestamp: \d+, deviceId: \d+, " \
                                    r"streamId: \d+, name: , domain:"
            marker_host_count = len(re.findall(pattern_marker_host, content))
            marker_device_count = len(re.findall(pattern_marker_device, content))
            self.assertEqual(marker_host_count, 2, f"MARKER的host侧数据异常，数量为{marker_host_count}")
            self.assertEqual(marker_device_count, 2, f"MARKER的device侧数据异常，数量为{marker_device_count}")

class MsptiActivityTest(test_base.TestProfiling):
    def getTestCmd(self, scene=None):
        script_path = os.path.join(self.cfg_path.mspti_sample_path, "mspti_activity")
        self.msprofbin_cmd += ("cd {}; "
                               "source {}/set_env.sh; "
                               "bash sample_run.sh > {}").format(script_path,
                                                                 self.cfg_path.ascend_toolkit_path,
                                                                 self.slog_stdout)

    def checkKernel(self, content):
        pattern_kernel = r"\[KERNEL\] type: KERNEL_AIVEC, name: " \
                         r"Axpy_233851a3505389e43928a8bba133a74d_high_performance_210000000, " \
                         r"start: \d+, end: \d+, deviceId: \d+, streamId: \d+, correlationId: \d+"
        kernel_msg = re.findall(pattern_kernel, content)
        pattern_kernel_count = len(kernel_msg)
        self.assertEqual(pattern_kernel_count, 1, f"KERNEL的数据异常，数量为{pattern_kernel_count}")
        check_time(kernel_msg)


    def checkApi(self, content):
        pattern_api = r"\[API\] name: aclnnAdd_AxpyAiCore_Axpy, start: \d+, end: \d+, processId: \d+, " \
                      r"threadId: \d+, correlationId: \d+"
        api_msg = re.findall(pattern_api, content)
        pattern_api_count = len(api_msg)
        self.assertEqual(pattern_api_count, 1, f"API的数据异常，数量为{pattern_api_count}")
        check_time(api_msg)


    def checkMemory(self, content):
        pattern_memory_allocation = r"\[MEMORY\] operationType: ALLOCATION, memoryKind: \w+, " \
                                    r"correlationId: \d+, start: \d+, end: \d+, address: \d+, bytes:\d+, " \
                                    r"processId: \d+, deviceId: \d+, streamId: \d+"
        pattern_memory_release = r"\[MEMORY\] operationType: RELEASE, memoryKind: \w+, correlationId: \d+, " \
                                 r"start: \d+, end: \d+, address: \d+, bytes:\d+, processId: \d+, deviceId: \d+, " \
                                 r"streamId: \d+"
        memory_allocation_msg = re.findall(pattern_memory_allocation, content)
        pattern_memory_allocation_count = len(memory_allocation_msg)
        memory_release_msg = re.findall(pattern_memory_release, content)
        pattern_memory_release_count = len(memory_release_msg)
        self.assertEqual(pattern_memory_allocation_count, 3,
                         f"MEMORY为ALLOCATION的数据异常，数量为{pattern_memory_allocation_count}")
        self.assertEqual(pattern_memory_release_count, 3,
                         f"MEMORY为RELEASE的数据异常，数量为{pattern_memory_release_count}")
        check_time(memory_allocation_msg)
        check_time(memory_release_msg)


    def checkMemory(self, content):
        pattern_memory_allocation = r"\[MEMORY\] operationType: ALLOCATION, memoryKind: \w+, " \
                                    r"correlationId: \d+, start: \d+, end: \d+, address: \d+, bytes:\d+, " \
                                    r"processId: \d+, deviceId: \d+, streamId: \d+"
        pattern_memory_release = r"\[MEMORY\] operationType: RELEASE, memoryKind: \w+, correlationId: \d+, " \
                                 r"start: \d+, end: \d+, address: \d+, bytes:\d+, processId: \d+, deviceId: \d+, " \
                                 r"streamId: \d+"
        memory_allocation_msg = re.findall(pattern_memory_allocation, content)
        pattern_memory_allocation_count = len(memory_allocation_msg)
        memory_release_msg = re.findall(pattern_memory_release, content)
        pattern_memory_release_count = len(memory_release_msg)
        self.assertEqual(pattern_memory_allocation_count, 3,
                         f"MEMORY为ALLOCATION的数据异常，数量为{pattern_memory_allocation_count}")
        self.assertEqual(pattern_memory_release_count, 3,
                         f"MEMORY为RELEASE的数据异常，数量为{pattern_memory_release_count}")
        check_time(memory_allocation_msg)
        check_time(memory_release_msg)


    def checkMemCpy(self, content):
        pattern_memcpy = r"\[MEMCPY\] copyKind: HTOD, bytes: \d+, start: \d+, end: \d+, deviceId: \d+, " \
                         r"streamId: \d+, correlationId: \d+, isAsync: \d+"
        memcpy_msg = re.findall(pattern_memcpy, content)
        pattern_memcpy_count = len(memcpy_msg)
        self.assertEqual(pattern_memcpy_count, 3, f"MEMCPY的数据异常，数量为{pattern_memcpy_count}")
        check_time(memcpy_msg)


    def checkResDir(self, scene=None):
        with open(self.slog_stdout, 'r', encoding='utf-8') as txtfile:
            content = txtfile.read()
            self.checkKernel(content)
            self.checkMemory(content)
            self.checkMemCpy(content)
            self.checkApi(content)


class MsptiCorrelationTest(test_base.TestProfiling):
    def getTestCmd(self, scene=None):
        script_path = os.path.join(self.cfg_path.mspti_sample_path, "mspti_correlation")
        self.msprofbin_cmd += ("cd {}; "
                               "source {}/set_env.sh; "
                               "bash sample_run.sh > {}").format(script_path,
                                                                 self.cfg_path.ascend_toolkit_path,
                                                                 self.slog_stdout)

    def checkResDir(self, scene=None):
        with open(self.slog_stdout, 'r', encoding='utf-8') as txtfile:
            content = txtfile.read()
            pattern_correlation = r"API and Activity correlation: correlation: \d+"
            correlation_msg = re.findall(pattern_correlation, content)
            pattern_correlation_count = len(correlation_msg)
            self.assertEqual(pattern_correlation_count, 1,
                             f"correlationId的数据异常，数量为{pattern_correlation_count}")


class MsptiExternalCorrelationTest(test_base.TestProfiling):
    def getTestCmd(self, scene=None):
        script_path = os.path.join(self.cfg_path.mspti_sample_path, "mspti_external_correlation")
        self.msprofbin_cmd += ("cd {}; "
                               "source {}/set_env.sh; "
                               "bash sample_run.sh > {}").format(script_path,
                                                                 self.cfg_path.ascend_toolkit_path,
                                                                 self.slog_stdout)

    def checkResDir(self, scene=None):
        with open(self.slog_stdout, 'r', encoding='utf-8') as txtfile:
            content = txtfile.read()
            for num in range(8):
                self.assertIn(f"result[{num}] is:", content, f"未发现第{num}个aclnn结果")
            self.assertIn("External id : 2: CLEANUP_EXTERNAL_ID \n"
                          "10, 11, 12, \n"
                          "External id : 1: EXECUTION_EXTERNAL_ID \n"
                          "7, 8, 8, 9, \n"
                          "External id : 0: INITIALIZATION_EXTERNAL_ID \n"
                          "1, 2, 3, 4, 5, 6,", content, "external_correlation的数据异常")

class MsptiHcclActivityTest(test_base.TestProfiling):
    def getTestCmd(self, scene=None):
        script_path = os.path.join(self.cfg_path.mspti_sample_path, "mspti_hccl_activity")
        self.msprofbin_cmd += ("cd {}; "
                               "source {}/set_env.sh; "
                               "bash sample_run.sh > {}").format(script_path,
                                                                 self.cfg_path.ascend_toolkit_path,
                                                                 self.slog_stdout)

    def checkResDir(self, scene=None):
        with open(self.slog_stdout, 'r', encoding='utf-8') as txtfile:
            content = txtfile.read()
            pattern_hccl = r"\[HCCL\] start: \d{19}, end: \d{19}, deviceId: \d+, streamId: \d+, bandWidth: \d+\.\d+, "\
                           r"name: HcclAllReduce, commName: \d+\.\d+\.\d+\.\d+%\S+_\d+_\d+_\d{16}"
            hccl_msg = re.findall(pattern_hccl, content)
            pattern_hccl_count = len(hccl_msg)
            self.assertEqual(pattern_hccl_count, 8, f"HCCL的数据异常，数量为{pattern_hccl_count}")
            check_time(hccl_msg)


class MsptiMstxActivityDomainTest(test_base.TestProfiling):
    def getTestCmd(self, scene=None):
        script_path = os.path.join(self.cfg_path.mspti_sample_path, "mspti_mstx_activity_domain")
        self.msprofbin_cmd += ("cd {}; "
                               "source {}/set_env.sh; "
                               
                               "bash sample_run.sh > {}").format(script_path,
                                                                 self.cfg_path.ascend_toolkit_path,
                                                                 self.slog_stdout)

    def checkResDir(self, scene=None):
        with open(self.slog_stdout, 'r', encoding='utf-8') as txtfile:
            content = txtfile.read()
            pattern_marker_host_domain = r"\[MARKER\] flag: \d+, sourceKind: HOST_DATA, timestamp: \d{19}, " \
                                         r"processId: \d+, threadId: \d+, name: \S+, domain: domainRange"
            pattern_marker_host = r"\[MARKER\] flag: \d+, sourceKind: HOST_DATA, timestamp: \d{19}, processId: \d+, " \
                                  r"threadId: \d+, name: , domain:"
            pattern_marker_device = r"\[MARKER\] flag: \d+, sourceKind: DEVICE_DATA, timestamp: \d{19}, deviceId: \d+, " \
                                    r"streamId: \d+, name: , domain:"
            marker_host_domain_count = len(re.findall(pattern_marker_host_domain, content))
            marker_host_count = len(re.findall(pattern_marker_host, content))
            marker_device_count = len(re.findall(pattern_marker_device, content))
            self.assertEqual(marker_host_count, 2, f"MARKER的host侧domain数据异常，数量为{marker_host_domain_count}")
            self.assertEqual(marker_host_count, 2, f"MARKER的host侧数据异常，数量为{marker_host_count}")
            self.assertEqual(marker_device_count, 2, f"MARKER的device侧数据异常，数量为{marker_device_count}")

class PythonMonitorTest(test_base.TestProfiling):
    def getTestCmd(self, scene=None):
        script_path = os.path.join(self.cfg_path.mspti_sample_path, "python_monitor")
        self.msprofbin_cmd += ("cd {}; "
                               "source {}/set_env.sh; "
                               "source /root/miniconda3/bin/activate /root/miniconda3/envs/smoke_test_env_pta;"
                               "bash sample_run.sh > {} 2>&1").format(script_path,
                                                                 self.cfg_path.ascend_toolkit_path,
                                                                 self.slog_stdout)

    def checkResDir(self, scene=None):
        with open(self.slog_stdout, 'r', encoding='utf-8') as txtfile:
            content = txtfile.read()
            pattern_kernel_aivec = r"INFO: MsptiActivityKind.MSPTI_ACTIVITY_KIND_KERNEL, \d{19}, \d{19}, \d+, \d+, " \
                                   r"\d+, KERNEL_AIVEC, \S+"
            pattern_kernel_aicore = r"INFO: MsptiActivityKind.MSPTI_ACTIVITY_KIND_KERNEL, \d{19}, \d{19}, \d+, \d+, " \
                                    r"\d+, KERNEL_AICORE, \S+"
            pattern_hccl = r"INFO: MsptiActivityKind.MSPTI_ACTIVITY_KIND_HCCL, \d{19}, \d{19}, \d+, \d+, \d+\.\d+, " \
                           r"HcclAllReduce, group_name_0"
            kernel_aivec_msg = re.findall(pattern_kernel_aivec, content)
            kernel_aicore_msg = re.findall(pattern_kernel_aicore, content)
            hccl_msg = re.findall(pattern_hccl, content)
            kernel_aivec_count = len(kernel_aivec_msg)
            kernel_aicore_count = len(kernel_aicore_msg)
            hccl_count = len(hccl_msg)
            self.assertEqual(kernel_aivec_count, 8, f"aivec数据异常，数量为{kernel_aivec_count}")
            self.assertEqual(kernel_aicore_count, 8, f"aicore数据异常，数量为{kernel_aicore_count}")
            self.assertEqual(hccl_count, 8, f"hccl数据异常，数量为{hccl_count}")
            check_time(kernel_aivec_msg, monitor_hccl_kernel=True)
            check_time(kernel_aicore_msg, monitor_hccl_kernel=True)
            check_time(hccl_msg, monitor_hccl_kernel=True)

class PythonMstxMonitorTest(test_base.TestProfiling):
    def getTestCmd(self, scene=None):
        script_path = os.path.join(self.cfg_path.mspti_sample_path, "python_mstx_monitor")
        self.msprofbin_cmd += ("cd {}; "
                               "source {}/set_env.sh; "
                               "source /root/miniconda3/bin/activate /root/miniconda3/envs/smoke_test_env_pta;"
                               "bash sample_run.sh > {} 2>&1").format(script_path,
                                                                 self.cfg_path.ascend_toolkit_path,
                                                                 self.slog_stdout)

    def checkResDir(self, scene=None):
        with open(self.slog_stdout, 'r', encoding='utf-8') as txtfile:
            content = txtfile.read()
            pattern_marker_host = r"INFO: MsptiActivityKind.MSPTI_ACTIVITY_KIND_MARKER, " \
                                  r"MsptiActivitySourceKind.MSPTI_ACTIVITY_SOURCE_KIND_HOST, \d+, mstx_matmul, " \
                                  r"default, \d{19}, \d{19}, \d+, \d+, -1, -1"
            pattern_marker_device = r"INFO: MsptiActivityKind.MSPTI_ACTIVITY_KIND_MARKER, " \
                                    r"MsptiActivitySourceKind.MSPTI_ACTIVITY_SOURCE_KIND_DEVICE, \d+, , , " \
                                    r"\d{19}, \d{19}, -1, -1, \d+, \d+"
            marker_host_msg = re.findall(pattern_marker_host, content)
            marker_device_msg = re.findall(pattern_marker_device, content)
            marker_host_count = len(marker_host_msg)
            marker_device_count = len(marker_device_msg)
            self.assertEqual(marker_host_count, 8, f"HOST侧数据异常，数据为{marker_host_count}")
            self.assertEqual(marker_device_count, 8, f"DEVICE侧数据异常，数据为{marker_device_count}")

            pattern_rank_id = r"MsptiActivitySourceKind.MSPTI_ACTIVITY_SOURCE_KIND_DEVICE, \d+, , , \d{19}, " \
                              r"\d{19}, -1, -1, \d+, (\d+)"
            rank_id_list = sorted(re.findall(pattern_rank_id, content))
            rank_list = ["0", "1", "2", "3", "4", "5", "6", "7"]
            self.assertEqual(rank_id_list, rank_list, f"DEVICE侧数据的rank_id异常，{rank_id_list}")


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(
        MsptiCorrelationTest("test_mspti_correlation", "sample", "", "", timeout=480))
    suite.addTest(
        MsptiActivityTest("test_mspti_activity", "sample", "", "", timeout=480))
    suite.addTest(
        MsptiHcclActivityTest("test_hccl_mspti_correlation", "sample", "", "", timeout=480))
    suite.addTest(
        MsptiExternalCorrelationTest("test_mspti_external_correlation", "sample", "", "", timeout=480))
    suite.addTest(
        MsptiMstxActivityDomainTest("test_mstx_mspti", "sample", "", "", timeout=480))
    suite.addTest(
        CallbackMstxTest("test_mspti_callback", "sample", "", "", timeout=480))
    suite.addTest(
        CallbackDomainTest("test_mspti_callback_domain", "sample", "", "", timeout=480))
    suite.addTest(
        PythonMonitorTest("test_python_monitor", "sample", "", "", timeout=480))
    suite.addTest(
        PythonMstxMonitorTest("test_python_mstx_monitor", "sample", "", "", timeout=480))
    unittest.TextTestRunner(verbosity=2).run(suite)