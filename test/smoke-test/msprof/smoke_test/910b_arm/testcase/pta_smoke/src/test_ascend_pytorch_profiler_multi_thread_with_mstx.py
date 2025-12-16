import torch_npu
import torch
import torch.nn as nn

from check_tools.db_check import DBManager
from check_tools.file_check import FileChecker
from check_tools.table_fields import TableFields
from train import train

import glob
import random
import tempfile
import threading


class RMSNorm(torch.nn.Module):
    def __init__(self, dim: int, eps: float = 1e-6):
        super().__init__()
        self.eps = eps
        self.weight = nn.Parameter(torch.ones(dim))

    def _norm(self, x):
        return x * torch.rsqrt(x.pow(2).mean(-1, keepdim=True) + self.eps)

    def forward(self, x):
        output = self._norm(x.float()).type_as(x)
        return output * self.weight

class ToyModel(nn.Module):
    def __init__(self, D_in, H, D_out):
        super(ToyModel, self).__init__()
        self.input_linear = torch.nn.Linear(D_in, H)
        self.middle_linear = torch.nn.Linear(H, H)
        self.output_linear = torch.nn.Linear(H, D_out)
        self.rms_norm = RMSNorm(D_out)

    def forward(self, x):
        h_relu = self.input_linear(x).clamp(min=0)
        for i in range(3):
            h_relu = self.middle_linear(h_relu).clamp(min=random.random())
        y_pred = self.output_linear(h_relu)
        y_pred = self.rms_norm(y_pred)
        return y_pred

def test_profiler_multi_thread_mstx():
    with tempfile.TemporaryDirectory() as prof_dir:
        experimental_config = torch_npu.profiler._ExperimentalConfig(
            profiler_level=torch_npu.profiler.ProfilerLevel.Level1,
            mstx=True
        )
        with torch_npu.profiler.profile(
                activities=[torch_npu.profiler.ProfilerActivity.CPU,
                            torch_npu.profiler.ProfilerActivity.NPU],
                on_trace_ready=torch_npu.profiler.tensorboard_trace_handler(prof_dir),
                experimental_config=experimental_config
        ) as prof:
            threads = []
            for i in range(1, 3):
                t = threading.Thread(target=train_multi_thread, args=(i, True))
                t.start()
                threads.append(t)
            train_multi_thread(0, False)
            for t in threads:
                t.join()

        _check_text_files(prof_dir)
        _check_db_files(prof_dir)

def train_multi_thread(device, child_thread):
    torch.npu.set_device(device)

    N, D_in, H, D_out = 256, 1024, 4096, 64
    input_data = torch.randn(N, D_in).npu()
    model = ToyModel(D_in, H, D_out).npu()

    if child_thread:
        torch_npu.profiler.profile.enable_profiler_in_child_thread()

    outputs = []
    for _ in range(5):
        stream = torch_npu.npu.current_stream()
        range_id1 = torch_npu.npu.mstx.range_start("range_with_domain1", stream, domain="domain1")
        output = model(input_data)
        torch_npu.npu.mstx.mark("mark_id1")
        torch_npu.npu.mstx.range_end(range_id1, domain="domain1")
        outputs.append(output)

    if child_thread:
        torch_npu.profiler.profile.disable_profiler_in_child_thread()

def _check_text_files(prof_dir):
    # 1. trace_view.json
    trace_view_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                f"ASCEND_PROFILER_OUTPUT/trace_view.json")
    assert trace_view_path, f"No trace_view.json found in {prof_dir}"
    FileChecker.check_timeline_values(
        trace_view_path[0],
        "name",
        [
            "range_with_domain1",
            "mark_id1"
        ],
        fuzzy_match=True
    )
    # 2. operator_details.csv
    operator_details_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                      f"ASCEND_PROFILER_OUTPUT/operator_details.csv")
    assert operator_details_path, f"No operator_details.csv found in {prof_dir}"
    FileChecker.check_csv_items(operator_details_path[0], {"Name": ["aten*"]}, fuzzy_match=True)
    # 3. step_trace_time.csv
    step_trace_time_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                     f"ASCEND_PROFILER_OUTPUT/step_trace_time.csv")
    assert step_trace_time_path, f"No step_trace_time.csv found in {prof_dir}"
    FileChecker.check_csv_items(step_trace_time_path[0], {"Device_id": ["0", "1", "2"]})
    # 4. profiler.log
    profiler_log_paths = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                   f"logs/*.log")
    assert profiler_log_paths, f"No profiler.log found in {prof_dir}"
    for profiler_log_path in profiler_log_paths:
        FileChecker.check_file_for_keyword(profiler_log_path, "error")

def _check_db_files(prof_dir: str):
    db_path = glob.glob(
        f"{prof_dir}/*_ascend_pt/ASCEND_PROFILER_OUTPUT/ascend_pytorch_profiler.db"
    )
    assert db_path, f"No db file found in {prof_dir}"
    FileChecker.check_file_exists(db_path[0])
    expect_tables = ["AICORE_FREQ", "CANN_API", "COMPUTE_TASK_INFO", "ENUM_API_TYPE", "ENUM_HCCL_DATA_TYPE", "ENUM_HCCL_LINK_TYPE",
                     "ENUM_HCCL_RDMA_TYPE", "ENUM_HCCL_TRANSPORT_TYPE", "ENUM_MEMCPY_OPERATION", "ENUM_MODULE", "ENUM_MSTX_EVENT_TYPE",
                     "HOST_INFO", "META_DATA", "MSTX_EVENTS", "NPU_INFO", "PYTORCH_API", "SESSION_TIME_INFO", 
                     "STRING_IDS", "TASK", "TASK_PMU_INFO"]
    for table_name in expect_tables:
        FileChecker.check_db_table_exist(db_path[0], table_name)


if __name__ == '__main__':
    test_profiler_multi_thread_mstx()
