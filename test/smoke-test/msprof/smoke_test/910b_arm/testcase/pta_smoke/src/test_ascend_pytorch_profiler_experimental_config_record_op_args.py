import glob
import tempfile
import os

import torch_npu
from train import train


def test_profiler_with_experimental_config_record_op_args():
    epochs = 5
    with tempfile.TemporaryDirectory() as prof_dir:
        experimental_config = torch_npu.profiler._ExperimentalConfig(
            record_op_args=True
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
    # ascend_pt_op_args
    ascend_pt_folders = glob.glob(os.path.join(prof_dir, "*_ascend_pt_op_args"))
    ascend_pt_folders = [f for f in ascend_pt_folders if os.path.isdir(f)]
    assert len(ascend_pt_folders) == 1

if __name__ == "__main__":
    test_profiler_with_experimental_config_record_op_args()
