import tempfile
import glob
import torch_npu

from train import train
from check_tools.file_check import FileChecker


def test_profiler_with_start_stop():
    epochs = 5
    with tempfile.TemporaryDirectory() as prof_dir:
        experimental_config = torch_npu.profiler._ExperimentalConfig(
            profiler_level=torch_npu.profiler.ProfilerLevel.Level0)
        prof = torch_npu.profiler.profile(
            with_stack=True,
            record_shapes=True,
            profile_memory=True,
            experimental_config=experimental_config,
            on_trace_ready=torch_npu.profiler.tensorboard_trace_handler(prof_dir)
        )
        for epoch in range(epochs):
            if epoch == 0:
                prof.start()
            train(epoch, epochs)
            if epoch == 1:
                prof.stop()
        _check_text_files(prof_dir)

def _check_text_files(prof_dir):
    # 1. trace_view.json
    trace_view_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                f"ASCEND_PROFILER_OUTPUT/trace_view.json")
    assert trace_view_path, f"No trace_view.json found in {prof_dir}"
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
    # 2. operator_details.csv
    operator_details_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                      f"ASCEND_PROFILER_OUTPUT/operator_details.csv")
    assert operator_details_path, f"No operator_details.csv found in {prof_dir}"
    FileChecker.check_csv_items(operator_details_path[0], {"Name": ["aten*"]}, fuzzy_match=True)
    # 3. operator_memory.csv
    operate_memory_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                    f"ASCEND_PROFILER_OUTPUT/operator_memory.csv")
    assert operate_memory_path, f"No operator_memory.csv found in {prof_dir}"
    FileChecker.check_csv_items(operate_memory_path[0], {"Name": ["aten*"]}, fuzzy_match=True)
    # 4. kernel_details.csv
    kernel_details_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                    f"ASCEND_PROFILER_OUTPUT/kernel_details.csv")
    assert kernel_details_path, f"No kernel_details.csv found in {prof_dir}"
    FileChecker.check_csv_items(kernel_details_path[0], {"Name": ["Conv2D*", "*TransData*"]})
    # 5. memory_record.csv
    memory_record_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                   f"ASCEND_PROFILER_OUTPUT/memory_record.csv")
    assert memory_record_path, f"No memory_record.csv found in {prof_dir}"
    FileChecker.check_csv_items(memory_record_path[0], {"Component": ["APP", "PTA+GE", "PTA"]}, fuzzy_match=False)
    # 6. profiler.log
    profiler_log_paths = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                   f"logs/*.log")
    assert profiler_log_paths, f"No profiler.log found in {prof_dir}"
    for profiler_log_path in profiler_log_paths:
        FileChecker.check_file_for_keyword(profiler_log_path, "error")

if __name__ == "__main__":
    test_profiler_with_start_stop()
