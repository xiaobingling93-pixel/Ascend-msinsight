import glob
import os

import multiprocessing as mp
import torch
import torch_npu
import torch.distributed as dist
from check_tools.db_check import DBManager
from check_tools.file_check import FileChecker
from check_tools.table_fields import TableFields
from check_tools.path_manager import PathManager

OUTPUT_PATH = "./sys_io_profiler_data"
PathManager.remove_path_safety(OUTPUT_PATH)

def run():

    def worker(rank, world_size=2):
        # Initialization
        os.environ['MASTER_ADDR'] = '10.174.216.241'
        os.environ['MASTER_PORT'] = '55234'
        torch_npu.npu.set_device(rank)
        dist.init_process_group(backend='hccl', world_size=world_size, rank=rank)

        # Create data (each rank has different data)
        data = torch.ones(5) * (rank + 1)  # rank0: [1,1,1,1,1], rank1: [2,2,2,2,2]
        data = data.npu()

        print(f"Rank {rank} before allreduce: {data.cpu()}")

        # Perform allreduce (SUM)
        epochs = 5
        experimental_config = torch_npu.profiler._ExperimentalConfig(
            aic_metrics=torch_npu.profiler.AiCMetrics.ArithmeticUtilization,
            profiler_level=torch_npu.profiler.ProfilerLevel.Level2,
            l2_cache=True,
            sys_io=True,
            data_simplification=False,
            export_type=[
                torch_npu.profiler.ExportType.Db
            ]
        )
        with torch_npu.profiler.profile(
                activities=[torch_npu.profiler.ProfilerActivity.CPU,
                            torch_npu.profiler.ProfilerActivity.NPU],
                on_trace_ready=torch_npu.profiler.tensorboard_trace_handler(OUTPUT_PATH),
                schedule=torch_npu.profiler.schedule(
                    wait=0, warmup=0, active=2, repeat=1, skip_first=1),
                record_shapes=True,
                with_stack=False,
                with_flops=False,
                with_modules=True,
                experimental_config=experimental_config
        ) as prof:
            for _ in range(epochs):
                dist.all_reduce(data, op=dist.ReduceOp.SUM)
                prof.step()
            print(f"Profiling completed for rank {rank}")
        dist.destroy_process_group()


    # Start multiple processes
    processes = []
    for i in range(2):
        p = mp.Process(target=worker, args=(i,))
        p.start()
        processes.append(p)

    for p in processes:
        p.join()

    _check()

def _check():
    ascend_pt_paths = glob.glob(f"{OUTPUT_PATH}/*_ascend_pt")
    assert ascend_pt_paths, f"No ascend_pt found in {OUTPUT_PATH}"
    for ascend_pt_path in ascend_pt_paths:
        _check_db_files(ascend_pt_path)
    PathManager.remove_path_safety(OUTPUT_PATH)

def _check_db_files(prof_dir: str):
    db_path = glob.glob(
        f"{prof_dir}/ASCEND_PROFILER_OUTPUT/ascend_pytorch_profiler_*.db"
    )
    assert db_path, f"No db file found in {prof_dir}"
    FileChecker.check_file_exists(db_path[0])
    expect_tables = ["AICORE_FREQ", "CANN_API", "COMMUNICATION_OP", "COMMUNICATION_TASK_INFO", "CONNECTION_IDS",
                     "ENUM_API_TYPE",
                     "ENUM_HCCL_DATA_TYPE", "ENUM_HCCL_LINK_TYPE", "ENUM_HCCL_RDMA_TYPE", "ENUM_HCCL_TRANSPORT_TYPE",
                     "ENUM_MEMCPY_OPERATION", "ENUM_MODULE", "ENUM_MSTX_EVENT_TYPE", "HOST_INFO", "META_DATA",
                     "NPU_INFO", "PYTORCH_API",
                     "SESSION_TIME_INFO", "STEP_TIME", "STRING_IDS", "TASK", "NETDEV_STATS", "HOST_INFO", "NIC",
                     "RANK_DEVICE_MAP"]
    for table_name in expect_tables:
        # 1. Check table exist
        FileChecker.check_db_table_exist(db_path[0], table_name)
        # 2. Check table fields
        res_table_fields = DBManager.fetch_all_field_name_in_table(db_path[0], table_name)
        assert res_table_fields == TableFields.get_fields(table_name), \
            f"Fields for table '{table_name}' do not match. Expected: {TableFields.get_fields(table_name)}, Actual: {res_table_fields}"

if __name__ == "__main__":
    run()
