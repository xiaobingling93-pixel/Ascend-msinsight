import tempfile
import glob
import torch_npu

from train import train
from check_tools.file_check import FileChecker


def test_profiler_with_task_queue_0():
    epochs = 5
    with tempfile.TemporaryDirectory() as prof_dir:
        with torch_npu.profiler.profile(
                activities=[torch_npu.profiler.ProfilerActivity.CPU,
                            torch_npu.profiler.ProfilerActivity.NPU],
                on_trace_ready=torch_npu.profiler.tensorboard_trace_handler(prof_dir),
                schedule=torch_npu.profiler.schedule(
                    wait=0, warmup=0, active=2, repeat=1, skip_first=1),
        ) as prof:
            for epoch in range(epochs):
                train(epoch, epochs)
                prof.step()
        _check_text_files(prof_dir)

def _check_text_files(prof_dir):
    # trace_view.json
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

if __name__ == "__main__":
    test_profiler_with_task_queue_0()
