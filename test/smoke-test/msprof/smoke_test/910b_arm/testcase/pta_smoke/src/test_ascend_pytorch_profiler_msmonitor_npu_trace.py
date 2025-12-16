import glob
import sys

import logging

from check_tools.db_check import DBManager
from check_tools.file_check import FileChecker
from check_tools.table_fields import TableFields

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


def test_profiler_with_msmonitor_npu_trace(npu_trace_path):
    # 1. 校验text类型的npu_trace数据结果
    _check_text_files(npu_trace_path)
    # 2. 校验db类型的npu_trace数据结果
    _check_db_files(npu_trace_path)


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
    expect_tables = ["ACC_PMU", "AICORE_FREQ", "CANN_API", "COMPUTE_TASK_INFO", "CONNECTION_IDS", "CPU_USAGE",
                     "ENUM_API_TYPE", "ENUM_HCCL_DATA_TYPE", "ENUM_HCCL_LINK_TYPE", "ENUM_HCCL_RDMA_TYPE",
                     "ENUM_HCCL_TRANSPORT_TYPE", "ENUM_MEMCPY_OPERATION", "ENUM_MODULE", "ENUM_MSTX_EVENT_TYPE",
                     "HBM", "HCCS", "HOST_INFO", "HOST_MEM_USAGE", "LLC", "MEMCPY_INFO", "MEMORY_RECORD",
                     "META_DATA",
                     "NETDEV_STATS", "NIC", "NPU_INFO", "NPU_MEM", "NPU_MODULE_MEM", "OP_MEMORY",
                     "PCIE",
                     "PYTORCH_API", "QOS", "ROCE", "SESSION_TIME_INFO", "SOC_BANDWIDTH_LEVEL",
                     "STEP_TIME",
                     "STRING_IDS", "TASK", "TASK_PMU_INFO"]
    for table_name in expect_tables:
        # 1. Check table exist
        FileChecker.check_db_table_exist(db_path[0], table_name)
        # 2. Check table fields
        res_table_fields = DBManager.fetch_all_field_name_in_table(db_path[0], table_name)
        assert res_table_fields == TableFields.get_fields(table_name), \
            f"Fields for table '{table_name}' do not match. Expected: {TableFields.get_fields(table_name)}, Actual: {res_table_fields}"


if __name__ == '__main__':
    prof_path = sys.argv[1]
    test_profiler_with_msmonitor_npu_trace(prof_path)