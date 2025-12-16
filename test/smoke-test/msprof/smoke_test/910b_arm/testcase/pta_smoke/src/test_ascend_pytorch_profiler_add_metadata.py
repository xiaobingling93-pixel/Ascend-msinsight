import torch_npu
from check_tools.file_check import FileChecker
from train import train

import glob
import tempfile

def test_profiler_with_metadata():
    epochs = 5
    with tempfile.TemporaryDirectory() as prof_dir:
        with torch_npu.profiler.profile(
                activities=[torch_npu.profiler.ProfilerActivity.CPU,
                            torch_npu.profiler.ProfilerActivity.NPU],
                on_trace_ready=torch_npu.profiler.tensorboard_trace_handler(prof_dir),
                schedule=torch_npu.profiler.schedule(
                    wait=0, warmup=0, active=1, repeat=1, skip_first=1),
        ) as prof:
            for epoch in range(epochs):
                train(epoch, epochs)
                if epoch == 1:
                    prof.add_metadata("test_key1", "test_val1")
                    prof.add_metadata_json("test_key2", "[1,2,3]")
                prof.step()
        _check_files(prof_dir)

def _check_files(prof_dir):
    # 1. profiler_metadata.json
    profiler_metadata_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                       f"profiler_metadata.json")
    assert profiler_metadata_path, f"No profiler_metadata.json found in {prof_dir}"
    FileChecker.check_json_keys(profiler_metadata_path[0], ["test_key1", "test_key2"])
    # 2. Check profiler.log
    profiler_log_paths = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                   f"logs/*.log")
    assert profiler_log_paths, f"No profiler.log found in {prof_dir}"
    for profiler_log_path in profiler_log_paths:
        FileChecker.check_file_for_keyword(profiler_log_path, "error")

if __name__ == "__main__":
    test_profiler_with_metadata()
