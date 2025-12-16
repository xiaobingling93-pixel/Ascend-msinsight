import os
import sys
import time
import glob
import json
from vllm import LLM, SamplingParams

sys.path.append(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
from check_tools.db_check import DBManager
from check_tools.file_check import FileChecker
from check_tools.table_fields import TableFields


def test_pytorch_vllm_profiling():
    prof_dir = "/home/result_dir/test_pytorch_vllm_profiling"
    os.environ["VLLM_TORCH_PROFILER_DIR"] = prof_dir

    prompts = [
        "Hello, my name is",
        "The president of the United States is",
        "The capital of France is",
        "The future of AI is",
    ]

    sampling_params = SamplingParams(temperature=0.8, top_p=0.95)
    llm = LLM(model="/home/msprof_smoke_test/model/Pytorch_vllm/Qwen2.5-0.5B-Instruct")

    llm.start_profile()
    outputs = llm.generate(prompts, sampling_params)
    llm.stop_profile()

    for output in outputs:
        prompt = output.prompt
        generated_text = output.outputs[0].text
        print(f"Prompt: {prompt!r}, Generated text: {generated_text!r}")

    _check_text_files(prof_dir)
    _check_db_files(prof_dir)

def _check_text_files(prof_dir):
    # 1. trace_view.json
    trace_view_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                f"ASCEND_PROFILER_OUTPUT/trace_view.json")
    assert trace_view_path, f"No trace_view.json found in {prof_dir}"
    # 2. op_statistic.csv
    op_statistic_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                  f"ASCEND_PROFILER_OUTPUT/op_statistic.csv")
    assert op_statistic_path, f"No op_statistic.csv found in {prof_dir}"
    FileChecker.check_csv_items(op_statistic_path[0], {"OP Type": ["ScatterElementsV2", "Cast"]}, fuzzy_match=False)
    # 3. operator_details.csv
    operator_details_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                      f"ASCEND_PROFILER_OUTPUT/operator_details.csv")
    assert operator_details_path, f"No operator_details.csv found in {prof_dir}"
    FileChecker.check_csv_items(operator_details_path[0], {"Name": ["aten*"]}, fuzzy_match=True)
    # 4. kernel_details.csv
    kernel_details_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                    f"ASCEND_PROFILER_OUTPUT/kernel_details.csv")
    assert kernel_details_path, f"No kernel_details.csv found in {prof_dir}"
    FileChecker.check_csv_items(kernel_details_path[0], {"Name": ["aclnn*"]})
    # 5. step_trace_time.csv
    step_trace_time_path = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                     f"ASCEND_PROFILER_OUTPUT/step_trace_time.csv")
    assert step_trace_time_path, f"No step_trace_time.csv found in {prof_dir}"
    # 6. profiler.log
    profiler_log_paths = glob.glob(f"{prof_dir}/*_ascend_pt/"
                                   f"logs/*.log")
    assert profiler_log_paths, f"No profiler.log found in {prof_dir}"
    for profiler_log_path in profiler_log_paths:
        FileChecker.check_file_for_keyword(profiler_log_path, "error")

def _check_db_files(prof_dir: str):
    db_path = glob.glob(
        f"{prof_dir}/*_ascend_pt/ASCEND_PROFILER_OUTPUT/ascend_pytorch_profiler*.db"
    )
    assert db_path, f"No db file found in {prof_dir}"
    FileChecker.check_file_exists(db_path[0])
    expect_tables = ["AICORE_FREQ", "CANN_API", "COMPUTE_TASK_INFO", "CONNECTION_IDS",
                     "ENUM_API_TYPE", "ENUM_HCCL_DATA_TYPE", "ENUM_HCCL_LINK_TYPE", "ENUM_HCCL_RDMA_TYPE",
                     "ENUM_HCCL_TRANSPORT_TYPE", "ENUM_MEMCPY_OPERATION", "ENUM_MODULE", "ENUM_MSTX_EVENT_TYPE",
                     "HOST_INFO", "META_DATA", "NPU_INFO", "PYTORCH_API", "RANK_DEVICE_MAP", "SESSION_TIME_INFO",
                     "STRING_IDS", "TASK", "TASK_PMU_INFO"]
    for table_name in expect_tables:
        FileChecker.check_db_table_exist(db_path[0], table_name)


if __name__ == '__main__':
    test_pytorch_vllm_profiling()
