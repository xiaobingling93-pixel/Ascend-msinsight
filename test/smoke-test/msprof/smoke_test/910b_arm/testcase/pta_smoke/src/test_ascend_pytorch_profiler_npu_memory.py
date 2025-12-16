import tempfile
import glob

import torch_npu

from check_tools.db_check import DBManager
from check_tools.table_fields import TableFields
from train import train
from check_tools.file_check import FileChecker


def test_profiler_with_npu_memory():
    epochs = 5
    with tempfile.TemporaryDirectory() as prof_dir:
        with torch_npu.profiler.profile(
                activities=[torch_npu.profiler.ProfilerActivity.NPU],
                on_trace_ready=torch_npu.profiler.tensorboard_trace_handler(prof_dir),
                schedule=torch_npu.profiler.schedule(
                    wait=0, warmup=0, active=2, repeat=1, skip_first=1),
                record_shapes=False,
                profile_memory=True,
                with_stack=False,
                with_flops=False,
                with_modules=False
        ) as prof:
            for epoch in range(epochs):
                train(epoch, epochs)
                prof.step()
        _check_text_files(prof_dir)
        _check_db_files(prof_dir)

def _check_text_files(prof_dir):
    # 1. trace_view.json
    trace_view_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                f"ASCEND_PROFILER_OUTPUT/trace_view.json")
    assert trace_view_path, f"No trace_view.json found in {prof_dir}"
    FileChecker.check_timeline_values(
        trace_view_path[0],
        "name",
        [
            "*TransData*"
        ],
        fuzzy_match=True
    )
    # 2. memory_record.csv
    memory_record_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                   f"ASCEND_PROFILER_OUTPUT/memory_record.csv")
    assert memory_record_path, f"No memory_record.csv found in {prof_dir}"
    FileChecker.check_csv_items(memory_record_path[0], {"Component": ["APP"]}, fuzzy_match=False)
    # 3. operator_memory.csv
    operate_memory_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                    f"ASCEND_PROFILER_OUTPUT/operator_memory.csv")
    assert operate_memory_path, f"No operator_memory.csv found in {prof_dir}"
    FileChecker.check_csv_items(operate_memory_path[0], {"Name": ["*TransData*"]}, fuzzy_match=True)
    # 4. profiler.log
    profiler_log_paths = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                   f"logs/*.log")
    assert profiler_log_paths, f"No profiler.log found in {prof_dir}"
    for profiler_log_path in profiler_log_paths:
        FileChecker.check_file_for_keyword(profiler_log_path, "error")

def _check_db_files(prof_dir: str):
    db_path = glob.glob(
        f"{prof_dir}/*_ascend_pt/ASCEND_PROFILER_OUTPUT/ascend_pytorch_profiler.db"
    )
    assert db_path, f"No db file found in {prof_dir}"
    FileChecker.check_file_exists(db_path[0])
    expect_tables = ["ACC_PMU", "AICORE_FREQ", "CANN_API", "COMPUTE_TASK_INFO",
                     "ENUM_API_TYPE", "ENUM_HCCL_DATA_TYPE", "ENUM_HCCL_LINK_TYPE", "ENUM_HCCL_RDMA_TYPE",
                     "ENUM_HCCL_TRANSPORT_TYPE", "ENUM_MEMCPY_OPERATION", "ENUM_MODULE", "ENUM_MSTX_EVENT_TYPE",
                     "HBM", "HOST_INFO", "LLC", "MEMORY_RECORD", "META_DATA",
                     "NPU_INFO", "NPU_MEM", "NPU_MODULE_MEM", "NPU_OP_MEM", "OP_MEMORY",
                     "QOS", "SESSION_TIME_INFO", "SOC_BANDWIDTH_LEVEL",
                     "STRING_IDS", "TASK"]
    for table_name in expect_tables:
        # 1. Check table exist
        FileChecker.check_db_table_exist(db_path[0], table_name)
        # 2. Check table fields
        res_table_fields = DBManager.fetch_all_field_name_in_table(db_path[0], table_name)
        assert res_table_fields == TableFields.get_fields(table_name)

if __name__ == "__main__":
    test_profiler_with_npu_memory()
