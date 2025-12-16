import torch_npu

import tempfile
import os
from train import train
from check_tools.file_check import FileChecker

def test_profiler_with_export_chrome_trace_and_stacks():
    epochs = 5
    with tempfile.TemporaryDirectory() as prof_dir:
        experimental_config = torch_npu.profiler._ExperimentalConfig(
            aic_metrics=torch_npu.profiler.AiCMetrics.ArithmeticUtilization,
            profiler_level=torch_npu.profiler.ProfilerLevel.Level2,
            l2_cache=True
        )
        with torch_npu.profiler.profile(
                activities=[torch_npu.profiler.ProfilerActivity.CPU,
                            torch_npu.profiler.ProfilerActivity.NPU],
                schedule=torch_npu.profiler.schedule(
                    wait=0, warmup=0, active=2, repeat=1, skip_first=1),
                record_shapes=False,
                profile_memory=False,
                with_stack=True,
                with_flops=False,
                with_modules=False,
                experimental_config=experimental_config
        ) as prof:
            for epoch in range(epochs):
                train(epoch, epochs)
                prof.step()
        prof.export_chrome_trace(os.path.join(prof_dir, "profiler_trace.json"))
        prof.export_stacks(os.path.join(prof_dir, "profiler_stacks.log"))
        _check_text_files(prof_dir)

def _check_text_files(prof_dir):
    # 1. profiler_trace.json
    chrome_trace_path = os.path.join(prof_dir, "profiler_trace.json")
    FileChecker.check_file_has_content(chrome_trace_path)
    # 2. profiler_stacks.log
    profiler_stacks_path = os.path.join(prof_dir, "profiler_stacks.log")
    assert profiler_stacks_path, f"No profiler_stacks.log found in {prof_dir}"
    FileChecker.check_file_has_content(profiler_stacks_path)

if __name__ == "__main__":
    test_profiler_with_export_chrome_trace_and_stacks()