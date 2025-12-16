import torch_npu

from check_tools.db_check import DBManager
from check_tools.file_check import FileChecker
from check_tools.table_fields import TableFields
from train import train

import glob
import tempfile


def test_profiler_with_type_db():
    epochs = 5
    with tempfile.TemporaryDirectory() as prof_dir:
        experimental_config = torch_npu.profiler._ExperimentalConfig(
            export_type=torch_npu.profiler.ExportType.Db,
            profiler_level=torch_npu.profiler.ProfilerLevel.Level1,
        )
        with torch_npu.profiler.profile(
                activities=[
                    torch_npu.profiler.ProfilerActivity.CPU,
                    torch_npu.profiler.ProfilerActivity.NPU,
                ],
                on_trace_ready=torch_npu.profiler.tensorboard_trace_handler(prof_dir),
                schedule=torch_npu.profiler.schedule(
                    wait=0, warmup=0, active=2, repeat=1, skip_first=1
                ),
                experimental_config=experimental_config,
        ) as prof:
            for epoch in range(epochs):
                train(epoch, epochs)
                prof.step()
            _check_files(prof_dir)

def _check_files(prof_dir):
    db_path = glob.glob(
        f"{prof_dir}/*_ascend_pt/ASCEND_PROFILER_OUTPUT/ascend_pytorch_profiler.db"
    )
    assert db_path, f"No db found in {prof_dir}"
    FileChecker.check_file_exists(db_path[0])
    expect_tables = ["AICORE_FREQ", "CANN_API", "COMPUTE_TASK_INFO", "CONNECTION_IDS", "ENUM_API_TYPE",
                     "ENUM_HCCL_DATA_TYPE", "ENUM_HCCL_LINK_TYPE", "ENUM_HCCL_RDMA_TYPE", "ENUM_HCCL_TRANSPORT_TYPE",
                     "ENUM_MODULE", "ENUM_MSTX_EVENT_TYPE", "HOST_INFO", "META_DATA", "NPU_INFO", "PYTORCH_API",
                     "SESSION_TIME_INFO", "STEP_TIME", "STRING_IDS", "TASK", "TASK_PMU_INFO"]
    for table_name in expect_tables:
        # 1. Check table exist
        FileChecker.check_db_table_exist(db_path[0], table_name)
        # 2. Check table fields
        res_table_fields = DBManager.fetch_all_field_name_in_table(db_path[0], table_name)
        assert res_table_fields == TableFields.get_fields(table_name)
    # 3. Check profiler.log
    profiler_log_paths = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                   f"logs/*.log")
    assert profiler_log_paths, f"No profiler.log found in {prof_dir}"
    for profiler_log_path in profiler_log_paths:
        FileChecker.check_file_for_keyword(profiler_log_path, "error")

if __name__ == "__main__":
    test_profiler_with_type_db()
