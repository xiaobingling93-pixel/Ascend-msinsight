import torch_npu
from torch_npu.profiler import dynamic_profile as dp
from check_tools.db_check import DBManager
from check_tools.file_check import FileChecker
from check_tools.table_fields import TableFields
from train import train

import glob
import tempfile
import json
import os
import time

def test_profiler_dynamic_all_params():
    epochs = 10
    with tempfile.TemporaryDirectory() as prof_dir:
        dp.init(prof_dir)
        data = None

        json_path = os.path.join(prof_dir, "profiler_config.json")


        for epoch in range(epochs):
            train(epoch, epochs)
            time.sleep(2)
            dp.step()
            if epoch == 1:
                with open(json_path, 'r', encoding='utf-8') as file:
                    data = json.load(file)

                data['prof_dir'] = prof_dir
                data['analyse'] = True
                data['record_shapes'] = True
                data['profile_memory'] = True
                data['record_shapes'] = True
                data['with_stack'] = True
                data['experimental_config']['profiler_level'] = "Level2"
                data['experimental_config']['aic_metrics'] = "ACL_AICORE_ARITHMETIC_UTILIZATION"
                data['experimental_config']['l2_cache'] = True
                data['experimental_config']['export_type'] = ["text", "db"]
                data['experimental_config']['host_sys'] = ["cpu", "mem"]
                data['experimental_config']['sys_io'] = True
                data['experimental_config']['sys_interconnection'] = True
                data['start_step'] = 3
                data['stop_step'] = 4

                with open(json_path, 'w', encoding='utf-8') as file:
                    json.dump(data, file, ensure_ascii=False, indent=4)

            if epoch == 6:
                with open(json_path, 'r', encoding='utf-8') as file:
                    data = json.load(file)

                data['start_step'] = 8
                data['stop_step'] = 9

                with open(json_path, 'w', encoding='utf-8') as file:
                    json.dump(data, file, ensure_ascii=False, indent=4)

        _check_text_files(prof_dir, 0)
        _check_db_files(prof_dir, 0)
        _check_text_files(prof_dir, 1)
        _check_db_files(prof_dir, 1)


def _check_text_files(prof_dir, num):
    # 1. trace_view.json
    trace_view_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                f"ASCEND_PROFILER_OUTPUT/trace_view.json")
    assert trace_view_path, f"No trace_view.json found in {prof_dir}"
    FileChecker.check_timeline_values(
        trace_view_path[num],
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
        trace_view_path[num],
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
    memory_record_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                   f"ASCEND_PROFILER_OUTPUT/memory_record.csv")
    assert memory_record_path, f"No memory_record.csv found in {prof_dir}"
    FileChecker.check_csv_items(memory_record_path[num], {"Component": ["APP", "PTA+GE", "PTA"]}, fuzzy_match=False)
    # 3. op_statistic.csv
    op_statistic_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                  f"ASCEND_PROFILER_OUTPUT/op_statistic.csv")
    assert op_statistic_path, f"No op_statistic.csv found in {prof_dir}"
    FileChecker.check_csv_items(op_statistic_path[num], {"OP Type": ["TransData", "Cast"]}, fuzzy_match=False)
    # 4. operator_details.csv
    operator_details_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                      f"ASCEND_PROFILER_OUTPUT/operator_details.csv")
    assert operator_details_path, f"No operator_details.csv found in {prof_dir}"
    FileChecker.check_csv_items(operator_details_path[num], {"Name": ["aten*"]}, fuzzy_match=True)
    # 5. operator_memory.csv
    operate_memory_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                    f"ASCEND_PROFILER_OUTPUT/operator_memory.csv")
    assert operate_memory_path, f"No operator_memory.csv found in {prof_dir}"
    FileChecker.check_csv_items(operate_memory_path[num], {"Name": ["aten*"]}, fuzzy_match=True)
    # 6. kernel_details.csv
    kernel_details_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                    f"ASCEND_PROFILER_OUTPUT/kernel_details.csv")
    assert kernel_details_path, f"No kernel_details.csv found in {prof_dir}"
    FileChecker.check_csv_items(kernel_details_path[num], {"Name": ["Conv2D*", "*TransData*"]})
    # 7. l2_cache.csv
    l2_cache_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                              f"ASCEND_PROFILER_OUTPUT/l2_cache.csv")
    assert l2_cache_path, f"No l2_cache.csv found in {prof_dir}"
    FileChecker.check_csv_items(l2_cache_path[num], {"Op Name": ["Conv2D*", "*TransData*"]})
    # 8. step_trace_time.csv
    step_trace_time_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                     f"ASCEND_PROFILER_OUTPUT/step_trace_time.csv")
    assert step_trace_time_path, f"No step_trace_time.csv found in {prof_dir}"
    step_trace_time_path = sorted(step_trace_time_path)
    if num == 0:
        FileChecker.check_csv_items(step_trace_time_path[num], {"Step": ["3"]})
    if num == 1:
        FileChecker.check_csv_items(step_trace_time_path[num], {"Step": ["8"]})
    # 9. npu_module_mem.csv
    npu_module_mem_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                    f"ASCEND_PROFILER_OUTPUT/npu_module_mem.csv")
    assert npu_module_mem_path, f"No npu_module_mem.csv found in {prof_dir}"
    FileChecker.check_csv_items(npu_module_mem_path[num], {"Component": ["SLOG"]})
    # 10. profiler.log
    profiler_log_paths = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                   f"logs/*.log")
    assert profiler_log_paths, f"No profiler.log found in {prof_dir}"
    for profiler_log_path in profiler_log_paths:
        FileChecker.check_file_for_keyword(profiler_log_path, "error")

def _check_db_files(prof_dir: str, num):
    db_path = glob.glob(
        f"{prof_dir}/*_ascend_pt/ASCEND_PROFILER_OUTPUT/ascend_pytorch_profiler.db"
    )
    assert db_path, f"No db file found in {prof_dir}"
    FileChecker.check_file_exists(db_path[num])
    expect_tables = ["ACC_PMU", "AICORE_FREQ", "CANN_API", "COMPUTE_TASK_INFO", "CONNECTION_IDS", "CPU_USAGE",
                     "ENUM_API_TYPE", "ENUM_HCCL_DATA_TYPE", "ENUM_HCCL_LINK_TYPE", "ENUM_HCCL_RDMA_TYPE",
                     "ENUM_HCCL_TRANSPORT_TYPE", "ENUM_MEMCPY_OPERATION", "ENUM_MODULE", "ENUM_MSTX_EVENT_TYPE",
                     "HBM", "HCCS", "HOST_INFO", "HOST_MEM_USAGE", "LLC", "MEMCPY_INFO", "MEMORY_RECORD", "META_DATA",
                     "NETDEV_STATS", "NIC", "NPU_INFO", "NPU_MEM", "NPU_MODULE_MEM", "NPU_OP_MEM", "OP_MEMORY", "PCIE",
                     "PYTORCH_API", "PYTORCH_CALLCHAINS", "QOS", "ROCE", "SESSION_TIME_INFO", "SOC_BANDWIDTH_LEVEL", "STEP_TIME",
                     "STRING_IDS", "TASK", "TASK_PMU_INFO"]
    for table_name in expect_tables:
        # 1. Check table exist
        FileChecker.check_db_table_exist(db_path[num], table_name)
        # 2. Check table fields
        res_table_fields = DBManager.fetch_all_field_name_in_table(db_path[num], table_name)
        assert res_table_fields == TableFields.get_fields(table_name)


if __name__ == '__main__':
    test_profiler_dynamic_all_params()
