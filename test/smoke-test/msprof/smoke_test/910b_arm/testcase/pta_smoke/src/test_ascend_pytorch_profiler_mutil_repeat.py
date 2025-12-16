import torch_npu
from check_tools.file_check import FileChecker
from train import train

import os
import glob
import tempfile


def test_profiler_with_mutil_repeat():
    epochs = 15
    with tempfile.TemporaryDirectory() as prof_dir:
        experimental_config = torch_npu.profiler._ExperimentalConfig(
            export_type=[
                torch_npu.profiler.ExportType.Text
            ],
            profiler_level=torch_npu.profiler.ProfilerLevel.Level0,
            aic_metrics=torch_npu.profiler.AiCMetrics.AiCoreNone)
        with torch_npu.profiler.profile(
                activities=[torch_npu.profiler.ProfilerActivity.CPU,
                            torch_npu.profiler.ProfilerActivity.NPU],
                on_trace_ready=torch_npu.profiler.tensorboard_trace_handler(prof_dir),
                schedule=torch_npu.profiler.schedule(
                    wait=1, warmup=1, active=2, repeat=3, skip_first=1),
                experimental_config=experimental_config
        ) as prof:
            for epoch in range(epochs):
                train(epoch, epochs)
                prof.step()
        _check_files(prof_dir)

def _check_files(prof_dir):
    # 1. trace_view.json
    trace_view_paths = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                f"ASCEND_PROFILER_OUTPUT/trace_view.json")
    assert trace_view_paths, f"No trace_view.json found in {prof_dir}"
    assert len(trace_view_paths) == 3, f"Expected 3 trace_view.json files, but found {len(trace_view_paths)} in {prof_dir}"
    trace_view_paths.sort(key=os.path.getmtime, reverse=False)
    FileChecker.check_timeline_values(
        trace_view_paths[0],
        "name",
        [
            "*ProfilerStep#3",  # check profiler step
            "*ProfilerStep#4"  # check profiler step
        ],
        fuzzy_match=True
    )
    FileChecker.check_timeline_values(
        trace_view_paths[1],
        "name",
        [
            "*ProfilerStep#7",  # check profiler step
            "*ProfilerStep#8"  # check profiler step
        ],
        fuzzy_match=True
    )
    FileChecker.check_timeline_values(
        trace_view_paths[2],
        "name",
        [
            "*ProfilerStep#11",  # check profiler step
            "*ProfilerStep#12"  # check profiler step
        ],
        fuzzy_match=True
    )
    # 2. kernel_details.csv
    kernel_details_paths = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                 f"ASCEND_PROFILER_OUTPUT/kernel_details.csv")
    assert kernel_details_paths, f"No kernel_details.csv found in {prof_dir}"
    assert len(kernel_details_paths) == 3, f"Expected 3 kernel_details.csv files, but found {len(kernel_details_paths)} in {prof_dir}"
    kernel_details_paths.sort(key=os.path.getmtime, reverse=False)
    FileChecker.check_csv_items(kernel_details_paths[0], {"Step Id": ["3", "4"]}, fuzzy_match=False)
    FileChecker.check_csv_items(kernel_details_paths[1], {"Step Id": ["7", "8"]}, fuzzy_match=False)
    FileChecker.check_csv_items(kernel_details_paths[2], {"Step Id": ["11", "12"]}, fuzzy_match=False)
    # 3. Check profiler.log
    profiler_log_paths = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                   f"logs/*.log")
    assert profiler_log_paths, f"No profiler.log found in {prof_dir}"
    for profiler_log_path in profiler_log_paths:
        FileChecker.check_file_for_keyword(profiler_log_path, "error")

if __name__ == '__main__':
    test_profiler_with_mutil_repeat()

