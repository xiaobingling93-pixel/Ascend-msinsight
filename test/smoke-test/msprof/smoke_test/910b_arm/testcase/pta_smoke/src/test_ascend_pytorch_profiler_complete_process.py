import glob
import os.path
import sys
import pandas as pd
import logging

from check_tools.db_check import DBManager
from check_tools.file_check import FileChecker
from check_tools.table_fields import TableFields
from check_tools.path_manager import PathManager
from check_tools.command_executor import CommandExecutor

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')

def test_profiler_with_msmonitor_npu_trace(npu_trace_path):
    # 1. 校验text类型的npu_trace数据结果
    _check_text_files(npu_trace_path)
    # 2. 校验db类型的npu_trace数据结果
    _check_db_files(npu_trace_path)
    # 3. mstt analyse工具分析
    _mstt_advisor_text_analyse(npu_trace_path)

def _check_text_files(npu_trace_path):
    # 1. trace_view.json
    trace_view_path = glob.glob(f"{npu_trace_path}/*_ascend_pt/"
                                f"ASCEND_PROFILER_OUTPUT/trace_view.json")
    assert trace_view_path, f"No trace_view.json found in {npu_trace_path}"
    FileChecker.check_timeline_values(
        trace_view_path[0],
        "cat",
        [
            "async_npu",
            "async_task_queue",
            "fwdbwd",
            "HostToDevice",
        ],
        fuzzy_match=True
    )
    FileChecker.check_timeline_values(
        trace_view_path[0],
        "args",
        [
            {"name": "CPU Usage"},
            {"name": "Memory Usage"},
            {"name": "PCIe"},
            {"name": "HCCS"},
            {"name": "NIC"},
            {"name": "RoCE"}
        ],
        fuzzy_match=False
    )
    # 2. memory_record.csv
    memory_record_path = glob.glob(f"{npu_trace_path}/*_ascend_pt/"
                                   f"ASCEND_PROFILER_OUTPUT/memory_record.csv")
    assert memory_record_path, f"No memory_record.csv found in {npu_trace_path}"
    FileChecker.check_csv_items(memory_record_path[0], {"Component": ["APP", "PTA+GE", "PTA"]}, fuzzy_match=False)
    # 3. op_statistic.csv
    op_statistic_path = glob.glob(f"{npu_trace_path}/*_ascend_pt/"
                                  f"ASCEND_PROFILER_OUTPUT/op_statistic.csv")
    assert op_statistic_path, f"No op_statistic.csv found in {npu_trace_path}"
    FileChecker.check_csv_items(op_statistic_path[0], {"OP Type": ["MatMul", "Reduce"]}, fuzzy_match=True)
    # 4. operator_details.csv
    operator_details_path = glob.glob(f"{npu_trace_path}/*_ascend_pt/"
                                      f"ASCEND_PROFILER_OUTPUT/operator_details.csv")
    assert operator_details_path, f"No operator_details.csv found in {npu_trace_path}"
    FileChecker.check_csv_items(operator_details_path[0], {"Name": ["aten*"]}, fuzzy_match=True)
    # 5. operator_memory.csv
    operate_memory_path = glob.glob(f"{npu_trace_path}/*_ascend_pt/"
                                    f"ASCEND_PROFILER_OUTPUT/operator_memory.csv")
    assert operate_memory_path, f"No operator_memory.csv found in {npu_trace_path}"
    FileChecker.check_csv_items(operate_memory_path[0], {"Name": ["aten*"]}, fuzzy_match=True)
    # 6. kernel_details.csv
    kernel_details_path = glob.glob(f"{npu_trace_path}/*_ascend_pt/"
                                    f"ASCEND_PROFILER_OUTPUT/kernel_details.csv")
    assert kernel_details_path, f"No kernel_details.csv found in {npu_trace_path}"
    FileChecker.check_csv_items(kernel_details_path[0], {"Name": ["*allReduce*", "*MatMul*"]})
    # 7. l2_cache.csv
    l2_cache_path = glob.glob(f"{npu_trace_path}/*_ascend_pt/"
                              f"ASCEND_PROFILER_OUTPUT/l2_cache.csv")
    assert l2_cache_path, f"No l2_cache.csv found in {npu_trace_path}"
    FileChecker.check_csv_items(l2_cache_path[0], {"Op Name": ["*MatMul*", "*Reduce*"]})
    # 8. step_trace_time.csv
    step_trace_time_path = glob.glob(f"{npu_trace_path}/*_ascend_pt/"
                                     f"ASCEND_PROFILER_OUTPUT/step_trace_time.csv")
    assert step_trace_time_path, f"No step_trace_time.csv found in {npu_trace_path}"
    FileChecker.check_csv_items(step_trace_time_path[0], {"Step": ["10", "11"]})
    # 9. npu_module_mem.csv
    npu_module_mem_path = glob.glob(f"{npu_trace_path}/*_ascend_pt/"
                                    f"ASCEND_PROFILER_OUTPUT/npu_module_mem.csv")
    assert npu_module_mem_path, f"No npu_module_mem.csv found in {npu_trace_path}"
    FileChecker.check_csv_items(npu_module_mem_path[0], {"Component": ["SLOG"]})
    # 10. profiler.log
    profiler_log_paths = glob.glob(f"{npu_trace_path}/*_ascend_pt/"
                                   f"logs/*.log")
    assert profiler_log_paths, f"No profiler.log found in {npu_trace_path}"
    for profiler_log_path in profiler_log_paths:
        FileChecker.check_file_for_keyword(profiler_log_path, "error")


def _check_db_files(npu_trace_path):
    db_path = glob.glob(
        f"{npu_trace_path}/*_ascend_pt/ASCEND_PROFILER_OUTPUT/ascend_pytorch_profiler_*.db"
    )
    assert db_path, f"No db file found in {npu_trace_path}"
    FileChecker.check_file_exists(db_path[0])
    expect_tables = ["ACC_PMU", "AICORE_FREQ", "CANN_API", "COMMUNICATION_OP", "COMMUNICATION_TASK_INFO", "COMPUTE_TASK_INFO",
                     "CONNECTION_IDS", "CPU_USAGE", "ENUM_API_TYPE", "ENUM_HCCL_DATA_TYPE", "ENUM_HCCL_LINK_TYPE", "ENUM_HCCL_RDMA_TYPE",
                     "ENUM_HCCL_TRANSPORT_TYPE", "ENUM_MEMCPY_OPERATION", "ENUM_MODULE", "ENUM_MSTX_EVENT_TYPE",
                     "HBM", "HCCS", "HOST_INFO", "HOST_MEM_USAGE", "LLC", "MEMCPY_INFO", "MEMORY_RECORD",
                     "META_DATA", "NETDEV_STATS", "NIC", "NPU_INFO", "NPU_MEM", "NPU_MODULE_MEM", "OP_MEMORY", "PCIE",
                     "PYTORCH_API", "PYTORCH_CALLCHAINS", "QOS", "RANK_DEVICE_MAP", "ROCE", "SESSION_TIME_INFO", "SOC_BANDWIDTH_LEVEL",
                     "STEP_TIME", "STRING_IDS", "TASK", "TASK_PMU_INFO"]
    for table_name in expect_tables:
        # 1. Check table exist
        FileChecker.check_db_table_exist(db_path[0], table_name)
        # 2. Check table fields
        res_table_fields = DBManager.fetch_all_field_name_in_table(db_path[0], table_name)
        assert res_table_fields == TableFields.get_fields(table_name), \
            f"Fields for table '{table_name}' do not match. Expected: {TableFields.get_fields(table_name)}, Actual: {res_table_fields}"


def _mstt_advisor_text_analyse(npu_trace_path):
    # 1. advisor db compare
    _advisor_db_compare(npu_trace_path)
    # 2. npu to npu compare
    _npu_to_npu_compare(npu_trace_path)
    # 3. cluster analyse
    _cluster_analyse(npu_trace_path)


def _advisor_db_compare(npu_trace_path):
    ascend_pt_path = glob.glob(f"{npu_trace_path}/*_ascend_pt/")
    mstt_analyse_path = os.path.join(npu_trace_path, "mstt_analyse_advisor")
    profiler_log_path = os.path.join(npu_trace_path, "mstt_advisor.log")
    advisor_cmd = ["msprof-analyze", "advisor", "all", "-d", ascend_pt_path[0], "-bp", ascend_pt_path[1],
                   "-o", mstt_analyse_path, "-l", "en", "--force"]
    CommandExecutor().execute(cmd=advisor_cmd, log_file=profiler_log_path)
    # 1. check advisor log
    FileChecker.check_file_for_keyword(profiler_log_path, "error")
    result_html, result_excel = PathManager.get_files(mstt_analyse_path)
    # 2. check excel
    _check_advisor_db_compare_excel(result_excel)
    # 3. check html
    _check_advisor_db_compare_html(result_html)

def _npu_to_npu_compare(npu_trace_path):
    ascend_pt_path = glob.glob(f"{npu_trace_path}/*_ascend_pt/")
    mstt_compare_path = os.path.join(npu_trace_path, "mstt_compare_cluster")
    profiler_log_path = os.path.join(npu_trace_path, "mstt_compare.log")
    advisor_cmd = ["msprof-analyze", "compare", "-d", ascend_pt_path[0], "-bp", ascend_pt_path[1],
                   "-o", mstt_compare_path, "--force"]
    CommandExecutor().execute(cmd=advisor_cmd, log_file=profiler_log_path)
    # 1. check advisor log
    FileChecker.check_file_for_keyword(profiler_log_path, "error")
    # 2. check excel contents
    compare_xlsx_path = glob.glob(f"{mstt_compare_path}/*.xlsx")
    FileChecker.check_excel_row_fields(compare_xlsx_path[0], "OverallMetrics",
                                       1, ["Index", "Duration(ms)", "Duration Ratio",
                                           "Number", "Duration(ms)", "Duration Ratio", "Number", "Diff Duration(ms)",
                                           "Diff Ratio"])
    FileChecker.check_excel_row_fields(compare_xlsx_path[0], "CommunicationCompare",
                                       1, ["Order Id", "Communication OP Name", "Task Name",
                                           "Calls", "Total Duration(us)", "Avg Duration(us)", "Max Duration(us)", "Min Duration(us)",
                                           "Communication OP Name", "Task Name", "Calls", "Total Duration(us)", "Avg Duration(us)",
                                           "Max Duration(us)", "Min Duration(us)", "Diff Duration(us)", "Diff Ratio"])
    FileChecker.check_excel_row_fields(compare_xlsx_path[0], "ModuleCompareStatistic",
                                       1, ["Order Id", "Module Class", "Module Level", "Module Name",
                                           "Operator Name", "Kernel Details", "Device Self Time(ms)", "Number", "Device Total Time(ms)",
                                           "Kernel Details", "Device Self Time(ms)", "Number", "Device Total Time(ms)", "Device Total Time Diff(ms)",
                                           "Device Self Time Diff(ms)", "Diff Total Ratio", "Base Call Stack", "Comparison Call Stack"])
    FileChecker.check_excel_row_fields(compare_xlsx_path[0], "ModuleCompare",
                                       1, ["Order Id", "Module Class", "Module Level", "Module Name",
                                           "Operator Name", "Kernel Details", "Device Self Time(us)",
                                           "Device Total Time(us)", "Operator Name",
                                           "Kernel Details", "Device Self Time(us)", "Device Total Time(us)",
                                           "Device Total Time Diff(us)",
                                           "Device Self Time Diff(us)", "Diff Total Ratio", "Base Call Stack",
                                           "Comparison Call Stack"])
    FileChecker.check_excel_row_fields(compare_xlsx_path[0], "MemoryCompareStatistic",
                                       1, ["Top", "Operator Name", "Base Allocated Duration(ms)", "Base Allocated Memory(MB)",
                                           "Base Operator Number", "Comparison Allocated Duration(ms)", "Comparison Allocated Memory(MB)",
                                           "Comparison Operator Number", "Diff Memory(MB)", "Diff Ratio"])
    FileChecker.check_excel_row_fields(compare_xlsx_path[0], "MemoryCompare",
                                       1, ["Order Id", "Operator Name", "Input Shape", "Input Type",
                                           "Allocated Details", "Size(KB)", "Operator Name", "Input Shape", "Input Type",
                                           "Allocated Details", "Size(KB)", "Diff Size(KB)", "Diff Ratio"])
    FileChecker.check_excel_row_fields(compare_xlsx_path[0], "ApiCompare",
                                       1, ["Order Id", "api name", "Total Duration(ms)", "Self Time(ms)",
                                           "Avg Duration(ms)", "Calls", "Total Duration(ms)", "Self Time(ms)", "Avg Duration(ms)",
                                           "Calls", "Diff Total Ratio", "Diff Self Ratio", "Diff Avg Ratio", "Diff Calls Ratio"])
    FileChecker.check_excel_row_fields(compare_xlsx_path[0], "KernelCompare",
                                       1, ["Order Id", "Kernel", "Input Shape", "Total Duration(us)",
                                           "Avg Duration(us)", "Max Duration(us)", "Min Duration(us)", "Calls", "Total Duration(us)",
                                           "Avg Duration(us)", "Max Duration(us)", "Min Duration(us)", "Calls", "Diff Total Ratio",
                                           "Diff Avg Ratio"])
def _cluster_analyse(npu_trace_path):
    mstt_analyse_path = os.path.join(npu_trace_path, "mstt_analyse_cluster")
    profiler_log_path = os.path.join(npu_trace_path, "mstt_cluster.log")
    advisor_cmd = ["msprof-analyze", "-m", "all", "-d", npu_trace_path, "-o", mstt_analyse_path]
    CommandExecutor().execute(cmd=advisor_cmd, log_file=profiler_log_path)
    # 1. check advisor log
    FileChecker.check_file_for_keyword(profiler_log_path, "error")
    # 2. check cluster db
    _check_cluster_db(mstt_analyse_path)

def _check_cluster_db(mstt_analyse_path):
    db_path = glob.glob(
        f"{mstt_analyse_path}/cluster_analysis_output/cluster_analysis.db"
    )
    assert db_path, f"No db file found in {mstt_analyse_path}"
    FileChecker.check_file_exists(db_path[0])
    expect_tables = ["ClusterCommunicationBandwidth", "ClusterCommunicationMatrix", "ClusterCommunicationTime",
                     "ClusterStepTraceTime", "CommunicationGroupMapping", "HostInfo",
                     "RankDeviceMap"]
    for table_name in expect_tables:
        # 1. Check table exist
        FileChecker.check_db_table_exist(db_path[0], table_name)
        # 2. Check table fields
        res_table_fields = DBManager.fetch_all_field_name_in_table(db_path[0], table_name)
        assert res_table_fields == TableFields.get_fields(table_name), \
            f"Fields for table '{table_name}' do not match. Expected: {TableFields.get_fields(table_name)}, Actual: {res_table_fields}"

    # 1. check table contents
    table_field_content = {
        "ClusterCommunicationBandwidth": {
            "step": ["step10", "step11"],
            "hccl_op_name": ["Total Op Info"]
        },
        "ClusterCommunicationMatrix": {
            "step": ["step10", "step11"],
            "hccl_op_name": ["Total Op Info"]
        },
        "ClusterCommunicationTime": {
            "step": ["step10", "step11"],
            "hccl_op_name": ["Total Op Info"]
        },
        "ClusterStepTraceTime": {
            "step": ["10", "11"]
        },
        "CommunicationGroupMapping": {
            "rank_set": ["(0,1)"]
        },
        "RankDeviceMap": {
            "rankId": [0, 1],
            "deviceId": [0, 1]
        },
    }
    DBManager.check_table_field_content(db_path[0], table_field_content)
    # 2. check table contents with comparison func
    DBManager.check_table_field_values(db_path[0], "ClusterCommunicationTime",
                                       ["start_timestamp", "elapsed_time", "transit_time", "wait_time",
                                        "synchronization_time", "idle_time", "synchronization_time_ratio", "wait_time_ratio"],
                                       lambda x: float(x) >= 0)
    DBManager.check_table_field_values(db_path[0], "ClusterStepTraceTime",
                                       ["computing", "communication_not_overlapped", "overlapped", "communication",
                                        "free", "stage", "bubble", "communication_not_overlapped_and_exclude_receive", "preparing"],
                                       lambda x: float(x) >= 0)

def _check_advisor_db_compare_excel(result_excel):
    # 1. check 'problems' sheet
    headers = ["category", "description", "suggestion", "problem count", "total_time(us)", "time ratio", "income(us)", "income ratio"]
    category = [
        "Kernel compare of Target and Benchmark",
        "Api compare of Target and Benchmark",
        "Operator Dynamic Shape Issues",
        "Conjectured Gc",
        "Affinity API Issues"
    ]
    # read all category
    FileChecker.check_excel_headers(result_excel, "problems", headers)
    df = pd.read_excel(result_excel, sheet_name="problems", nrows=0)
    category_s = df["category"].dropna().tolist()
    for category in category_s:
        # 1. check 'Kernel compare of Target and Benchmark' sheet
        if category == "Kernel compare of Target and Benchmark":
            category = "Kernel compare of Target and Be"
            headers = ["Order Id", "Kernel Type", "Core Type", "Total Duration(us)", "Avg Duration(us)",
                       "Max Duration(us)",
                       "Min Duration(us)",
                       "Calls", "Benchmark  Total Duration(us)", "Benchmark  Avg Duration(us)",
                       "Benchmark  Max Duration(us)",
                       "Benchmark  Min Duration(us)",
                       "Benchmark  Calls", "Diff Total Ratio", "Diff Avg Ratio"]
            kernel_type = [
                "ForeachAddList", "ReduceSum", "MSELossV2", "Mul", "MemSet", "MatMulV2", "Fill", "Relu", "MseLossGrad",
                "ReluGrad"
            ]
            FileChecker.check_excel_headers(result_excel, category, headers)
            FileChecker.check_excel_items(result_excel, category, {"Kernel Type": kernel_type})
        # 2. check 'Api compare of Target and Benchmark' sheet
        elif category == "Api compare of Target and Benchmark":
            category = "Api compare of Target and Bench"
            headers = ["Order Id", "api name", "Total Duration(ms)", "Self Time(ms)", "Avg Duration(ms)", "Calls",
                       "Benchmark  Total Duration(ms)",
                       "Benchmark  Self Time(ms)", "Benchmark  Avg Duration(ms)", "Benchmark  Calls",
                       "Diff Total Ratio", "Diff Self Ratio",
                       "Diff Avg Ratio", "Diff Calls Ratio"]
            api_name = ["*Relu*", "*aten*", "*allreduce*"]
            FileChecker.check_excel_headers(result_excel, category, headers)
            FileChecker.check_excel_items(result_excel, category, {"api name": api_name})
        # 3. check 'ConjecturedGcAnalysis' sheet
        elif category == "ConjecturedGcAnalysis":
            headers = ["timestamp", "duration(us)"]
            FileChecker.check_excel_headers(result_excel, category, headers)
        # 4. check 'Affinity API Issues' sheet
        elif category == "Affinity API Issues":
            headers = ["Affinity API", "Code stacks", "Stack called counts"]
            FileChecker.check_excel_headers(result_excel, category, headers)
        # 4. check 'Packet Analysis' sheet
        elif category == "Packet Analysis":
            headers = ["SDMA total count", "Small SDMA count", "Small SDMA ratio", "Small SDMA duration(ms)"]
            FileChecker.check_excel_headers(result_excel, category, headers)

def _check_advisor_db_compare_html(result_html):
    # 1. check 'Kernel compare of Target and Benchmark' sheet
    headers = ["Order Id", "Kernel Type", "Core Type", "Total Duration(us)", "Avg Duration(us)", "Max Duration(us)",
               "Min Duration(us)",
               "Calls", "Benchmark  Total Duration(us)", "Benchmark  Avg Duration(us)", "Benchmark  Max Duration(us)",
               "Benchmark  Min Duration(us)",
               "Benchmark  Calls", "Diff Total Ratio", "Diff Avg Ratio"]
    FileChecker.check_html_table_headers(result_html, "Kernel compare of Target and Benchmark", headers)

    # 2. check 'Api compare of Target and Benchmark' sheet
    headers = ["Order Id", "api name", "Total Duration(ms)", "Self Time(ms)", "Avg Duration(ms)", "Calls",
               "Benchmark  Total Duration(ms)",
               "Benchmark  Self Time(ms)", "Benchmark  Avg Duration(ms)", "Benchmark  Calls", "Diff Total Ratio",
               "Diff Self Ratio",
               "Diff Avg Ratio", "Diff Calls Ratio"]
    FileChecker.check_html_table_headers(result_html, "Api compare of Target and Benchmark", headers)

    # 3. check 'Operator Dynamic Shape Issues' sheet
    headers = ["Description", "Suggestion"]
    FileChecker.check_html_table_headers(result_html, "Operator Dynamic Shape Issues", headers)


if __name__ == '__main__':
    prof_path = sys.argv[1]
    test_profiler_with_msmonitor_npu_trace(prof_path)