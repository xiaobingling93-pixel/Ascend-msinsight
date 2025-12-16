import torch_npu
from check_tools.file_check import FileChecker
from train import train

import glob
import tempfile

def test_profiler_with_cpu_memory():
    epochs = 5
    with tempfile.TemporaryDirectory() as prof_dir:
        with torch_npu.profiler.profile(
                activities=[torch_npu.profiler.ProfilerActivity.CPU],
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
        _check_files(prof_dir)

def _check_files(prof_dir):
    # 1. trace_view.json
    trace_view_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                f"ASCEND_PROFILER_OUTPUT/trace_view.json")
    assert trace_view_path, f"No trace_view.json found in {prof_dir}"
    FileChecker.check_timeline_values(
        trace_view_path[0],
        "name",
        [
            "*ProfilerStep#1",
            "*ProfilerStep#2",
            "*aten::to*"
        ],
        fuzzy_match=True
    )
    # 2. memory_record.csv
    memory_record_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                   f"ASCEND_PROFILER_OUTPUT/memory_record.csv")
    assert memory_record_path, f"No memory_record.csv found in {prof_dir}"
    FileChecker.check_csv_items(memory_record_path[0], {"Component": ["PTA+GE", "PTA"]}, fuzzy_match=False)
    # 3. operator_memory.csv
    operate_memory_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                    f"ASCEND_PROFILER_OUTPUT/operator_memory.csv")
    assert operate_memory_path, f"No operator_memory.csv found in {prof_dir}"
    FileChecker.check_csv_items(operate_memory_path[0], {"Name": ["aten*"]}, fuzzy_match=True)
    # 4. operator_details.csv
    operator_details_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                      f"ASCEND_PROFILER_OUTPUT/operator_details.csv")
    assert operator_details_path, f"No operator_details.csv found in {prof_dir}"
    FileChecker.check_csv_items(operator_details_path[0], {"Name": ["aten*"]}, fuzzy_match=True)
    # 5. Check profiler.log
    profiler_log_paths = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                   f"logs/*.log")
    assert profiler_log_paths, f"No profiler.log found in {prof_dir}"
    for profiler_log_path in profiler_log_paths:
        FileChecker.check_file_for_keyword(profiler_log_path, "error")

if __name__ == "__main__":
    test_profiler_with_cpu_memory()
