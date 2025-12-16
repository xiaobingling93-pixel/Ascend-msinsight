import tempfile
import glob

import torch_npu

from check_tools.db_check import DBManager
from train import train
from check_tools.file_check import FileChecker

def test_profiler_with_opattr_gc_modules():
    epochs = 5
    with tempfile.TemporaryDirectory() as prof_dir:
        experimental_config = torch_npu.profiler._ExperimentalConfig(
            profiler_level=torch_npu.profiler.ProfilerLevel.Level0,
            op_attr=True,
            gc_detect_threshold=1,
            export_type=torch_npu.profiler.ExportType.Db
        )
        with torch_npu.profiler.profile(
                activities=[torch_npu.profiler.ProfilerActivity.NPU,
                            torch_npu.profiler.ProfilerActivity.CPU],
                on_trace_ready=torch_npu.profiler.tensorboard_trace_handler(prof_dir),
                schedule=torch_npu.profiler.schedule(
                    wait=0, warmup=0, active=2, repeat=1, skip_first=1),
                record_shapes=False,
                profile_memory=False,
                with_stack=False,
                with_flops=False,
                with_modules=True,
                experimental_config=experimental_config
        ) as prof:
            for epoch in range(epochs):
                train(epoch, epochs)
                prof.step()
            _check_output_db(prof_dir)



def _check_output_db(prof_dir: str):
    db_path = glob.glob(
        f"{prof_dir}/*_ascend_pt/ASCEND_PROFILER_OUTPUT/ascend_pytorch_profiler.db"
    )
    assert db_path, f"No db file found in {prof_dir}"
    FileChecker.check_file_exists(db_path[0])
    # gc_detect_threshold生成表GC_RECORD
    expect_tables = ["AICORE_FREQ", "CANN_API", "COMPUTE_TASK_INFO", "CONNECTION_IDS", "ENUM_API_TYPE",
                     "ENUM_HCCL_DATA_TYPE", "ENUM_HCCL_LINK_TYPE", "ENUM_HCCL_RDMA_TYPE", "ENUM_HCCL_TRANSPORT_TYPE",
                     "ENUM_MODULE", "ENUM_MSTX_EVENT_TYPE", "GC_RECORD", "HOST_INFO", "META_DATA", "NPU_INFO",
                     "PYTORCH_API", "SESSION_TIME_INFO", "STEP_TIME", "STRING_IDS", "TASK"]
    for table_name in expect_tables:
        FileChecker.check_db_table_exist(db_path[0], table_name)
    # 检查是存在调用栈API 50003 标识这条api是调用栈
    module_type = "50003"
    DBManager.check_item_in_table(db_path[0], "PYTORCH_API", "type", module_type)


if __name__ == "__main__":
    test_profiler_with_opattr_gc_modules()
