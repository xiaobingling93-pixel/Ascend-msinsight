import tempfile
import glob

import torch_npu
from train import train
from check_tools.file_check import FileChecker

def test_profiler_with_experimental_config_l1_default():
    epochs = 5
    with tempfile.TemporaryDirectory() as prof_dir:
        experimental_config = torch_npu.profiler._ExperimentalConfig(
            profiler_level=torch_npu.profiler.ProfilerLevel.Level1,
        )
        with torch_npu.profiler.profile(
                activities=[torch_npu.profiler.ProfilerActivity.CPU,
                            torch_npu.profiler.ProfilerActivity.NPU],
                on_trace_ready=torch_npu.profiler.tensorboard_trace_handler(prof_dir),
                schedule=torch_npu.profiler.schedule(
                    wait=0, warmup=0, active=2, repeat=1, skip_first=1),
                experimental_config=experimental_config
        ) as prof:
            for epoch in range(epochs):
                train(epoch, epochs)
                prof.step()
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
    # 3. kernel_details.csv
    kernel_details_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                    f"ASCEND_PROFILER_OUTPUT/kernel_details.csv")
    assert kernel_details_path, f"No kernel_details.csv found in {prof_dir}"
    FileChecker.check_csv_items(kernel_details_path[0], {"Name": ["Conv2D*", "*TransData*"]})
    # 4. step_trace_time.csv
    step_trace_time_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                    f"ASCEND_PROFILER_OUTPUT/step_trace_time.csv")
    assert step_trace_time_path, f"No step_trace_time.csv found in {prof_dir}"
    FileChecker.check_csv_items(step_trace_time_path[0], {"Step": ["1", "2"]})
    # 5. profiler.log
    profiler_log_paths = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                   f"logs/*.log")
    assert profiler_log_paths, f"No profiler.log found in {prof_dir}"
    for profiler_log_path in profiler_log_paths:
        FileChecker.check_file_for_keyword(profiler_log_path, "error")

if __name__ == "__main__":
    test_profiler_with_experimental_config_l1_default()
