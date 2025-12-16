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

OUTPUT_PATH = "./allreduce_profiler_data"
PathManager.remove_path_safety(OUTPUT_PATH)

def test_allreduce_profiler():

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
            export_type=[
                torch_npu.profiler.ExportType.Text,
                torch_npu.profiler.ExportType.Db
            ],
            host_sys=[
                torch_npu.profiler.HostSystem.CPU,
                torch_npu.profiler.HostSystem.MEM],
            sys_io=True,
            sys_interconnection=True
        )
        with torch_npu.profiler.profile(
                activities=[torch_npu.profiler.ProfilerActivity.CPU,
                            torch_npu.profiler.ProfilerActivity.NPU],
                on_trace_ready=torch_npu.profiler.tensorboard_trace_handler(OUTPUT_PATH),
                schedule=torch_npu.profiler.schedule(
                    wait=0, warmup=0, active=2, repeat=1, skip_first=1),
                record_shapes=True,
                profile_memory=True,
                with_stack=True,
                with_flops=False,
                with_modules=False,
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
        _check_text_files(ascend_pt_path)
        _check_db_files(ascend_pt_path)
    PathManager.remove_path_safety(OUTPUT_PATH)


def _check_text_files(prof_dir):
    # 1. trace_view.json
    trace_view_path = glob.glob(f"{prof_dir}/ASCEND_PROFILER_OUTPUT/trace_view.json")
    assert trace_view_path, f"No trace_view.json found in {prof_dir}"
    FileChecker.check_timeline_values(
        trace_view_path[0],
        "cat",
        [
            "async_npu",
            "async_task_queue",
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
    # 2. operator_details.csv
    operator_details_path = glob.glob(f"{prof_dir}/ASCEND_PROFILER_OUTPUT/operator_details.csv")
    assert operator_details_path, f"No operator_details.csv found in {prof_dir}"
    FileChecker.check_csv_items(operator_details_path[0], {"Name": ["allreduce*"]}, fuzzy_match=True)
    # 3. kernel_details.csv
    kernel_details_path = glob.glob(f"{prof_dir}/ASCEND_PROFILER_OUTPUT/kernel_details.csv")
    assert kernel_details_path, f"No kernel_details.csv found in {prof_dir}"
    FileChecker.check_csv_items(kernel_details_path[0], {"Name": ["*allReduce*"]})
    # 4. step_trace_time.csv
    step_trace_time_path = glob.glob(f"{prof_dir}/ASCEND_PROFILER_OUTPUT/step_trace_time.csv")
    assert step_trace_time_path, f"No step_trace_time.csv found in {prof_dir}"
    FileChecker.check_csv_items(step_trace_time_path[0], {"Step": ["1", "2"]})
    # 5. npu_module_mem.csv
    npu_module_mem_path = glob.glob(f"{prof_dir}/ASCEND_PROFILER_OUTPUT/npu_module_mem.csv")
    assert npu_module_mem_path, f"No npu_module_mem.csv found in {prof_dir}"
    FileChecker.check_csv_items(npu_module_mem_path[0], {"Component": ["HCCL", "HCCP"]})
    # 6. communication.json
    communication_path = glob.glob(f"{prof_dir}/ASCEND_PROFILER_OUTPUT/communication.json")
    assert communication_path, f"No communication.json found in {prof_dir}"
    FileChecker.check_communication_fields_from_communication_json(communication_path[0], "allReduce",
                                                                   ["Start Timestamp(us)", "Elapse Time(ms)",
                                                                    "Transit Time(ms)", "Wait Time(ms)",
                                                                    "Synchronization Time(ms)", "Idle Time(ms)",
                                                                    "Wait Time Ratio",
                                                                    "Synchronization Time Ratio"])
    # 7. from trace_view.json check communication fields
    FileChecker.check_communication_fields_from_trace_view(trace_view_path[0], "hcom_allReduce",
                                                           ["connection_id", "model id", "data_type", "alg_type",
                                                            "count", "relay", "retry"])
    FileChecker.check_communication_fields_from_trace_view(trace_view_path[0], "hcom_broadcast",
                                                           ["connection_id", "model id", "data_type", "alg_type",
                                                            "count", "relay", "retry"])
    # 8. profiler.log
    profiler_log_paths = glob.glob(f"{prof_dir}/logs/*.log")
    assert profiler_log_paths, f"No profiler.log found in {prof_dir}"
    for profiler_log_path in profiler_log_paths:
        FileChecker.check_file_for_keyword(profiler_log_path, "error")

def _check_db_files(prof_dir: str):
    db_path = glob.glob(
        f"{prof_dir}/ASCEND_PROFILER_OUTPUT/ascend_pytorch_profiler_*.db"
    )
    assert db_path, f"No db file found in {prof_dir}"
    FileChecker.check_file_exists(db_path[0])
    expect_tables = ["ACC_PMU", "AICORE_FREQ", "CANN_API", "COMMUNICATION_OP", "COMMUNICATION_TASK_INFO", "CONNECTION_IDS", "CPU_USAGE",
                     "ENUM_API_TYPE", "ENUM_HCCL_DATA_TYPE", "ENUM_HCCL_LINK_TYPE", "ENUM_HCCL_RDMA_TYPE",
                     "ENUM_HCCL_TRANSPORT_TYPE", "ENUM_MEMCPY_OPERATION", "ENUM_MODULE", "ENUM_MSTX_EVENT_TYPE",
                     "HCCS", "HOST_INFO", "HOST_MEM_USAGE", "META_DATA",
                     "NETDEV_STATS", "NIC", "NPU_INFO", "NPU_MEM", "NPU_MODULE_MEM", "PCIE",
                     "PYTORCH_API", "PYTORCH_CALLCHAINS", "QOS", "ROCE", "SESSION_TIME_INFO", "SOC_BANDWIDTH_LEVEL", "STEP_TIME",
                     "STRING_IDS", "TASK"]
    for table_name in expect_tables:
        # 1. Check table exist
        FileChecker.check_db_table_exist(db_path[0], table_name)
        # 2. Check table fields
        res_table_fields = DBManager.fetch_all_field_name_in_table(db_path[0], table_name)
        assert res_table_fields == TableFields.get_fields(table_name), \
            f"Fields for table '{table_name}' do not match. Expected: {TableFields.get_fields(table_name)}, Actual: {res_table_fields}"

if __name__ == '__main__':
    test_allreduce_profiler()
