import os.path
import glob

import torch_npu
import tempfile
from train import train
from check_tools.file_check import FileChecker

common_header = [
    "Step Id", "Model ID", "Task ID", "Stream ID", "Name", "Type", "OP State", "Accelerator Core", "Start Time(us)",
    "Duration(us)", "Wait Time(us)", "Block Dim", "Mix Block Dim", "HF32 Eligible", "Input Shapes",
    "Input Data Types", "Input Formats", "Output Shapes", "Output Data Types", "Output Formats", "Context ID"
]

pmu_headers = {
    "PipeUtilization":
        ["aicore_time(us)", "aic_total_cycles", "aic_mac_time(us)", "aic_mac_ratio", "aic_scalar_time(us)",
         "aic_scalar_ratio", "aic_mte1_time(us)", "aic_mte1_ratio", "aic_mte2_time(us)", "aic_mte2_ratio",
         "aic_fixpipe_time(us)", "aic_fixpipe_ratio", "aic_icache_miss_rate", "aiv_time(us)", "aiv_total_cycles",
         "aiv_vec_time(us)", "aiv_vec_ratio", "aiv_scalar_time(us)", "aiv_scalar_ratio", "aiv_mte2_time(us)",
         "aiv_mte2_ratio", "aiv_mte3_time(us)", "aiv_mte3_ratio", "aiv_icache_miss_rate", "cube_utilization(%)"],
    "ArithmeticUtilization":
        ["aicore_time(us)", "aic_total_cycles", "aic_mac_fp16_ratio", "aic_mac_int8_ratio", "aic_cube_fops",
         "aiv_time(us)", "aiv_total_cycles", "aiv_vec_fp32_ratio", "aiv_vec_fp16_ratio",
         "aiv_vec_int32_ratio", "aiv_vec_misc_ratio", "aiv_vector_fops"],
    "Memory":
        ["aicore_time(us)", "aic_total_cycles", "aic_l1_read_bw(GB/s)", "aic_l1_write_bw(GB/s)",
         "aic_main_mem_read_bw(GB/s)", "aic_main_mem_write_bw(GB/s)", "aic_l2_read_bw(GB/s)", "aic_l2_write_bw(GB/s)",
         "aiv_time(us)", "aiv_total_cycles", "aiv_ub_read_bw(GB/s)", "aiv_ub_write_bw(GB/s)",
         "aiv_main_mem_read_bw(GB/s)", "aiv_main_mem_write_bw(GB/s)", "aiv_l2_read_bw(GB/s)", "aiv_l2_write_bw(GB/s)"],
    "MemoryL0":
        ["aicore_time(us)", "aic_total_cycles", "aic_l0a_read_bw(GB/s)", "aic_l0a_write_bw(GB/s)",
         "aic_l0b_read_bw(GB/s)", "aic_l0b_write_bw(GB/s)", "aic_l0c_read_bw_cube(GB/s)", "aic_l0c_write_bw_cube(GB/s)",
         "aiv_time(us)", "aiv_total_cycles", "aiv_l0c_read_bw(GB/s)", "aiv_l0c_write_bw(GB/s)"],
    "ResourceConflictRatio":
        ["aicore_time(us)", "aic_total_cycles", "aiv_time(us)", "aiv_total_cycles", "aiv_vec_bankgroup_cflt_ratio",
         "aiv_vec_bank_cflt_ratio", "aiv_vec_resc_cflt_ratio"],
    "MemoryUB":
        ["aicore_time(us)", "aic_total_cycles", "aic_ub_read_bw_scalar(GB/s)", "aic_ub_write_bw_scalar(GB/s)",
         "aiv_time(us)", "aiv_total_cycles", "aiv_ub_read_bw_vector(GB/s)", "aiv_ub_write_bw_vector(GB/s)",
         "aiv_ub_read_bw_scalar(GB/s)", "aiv_ub_write_bw_scalar(GB/s)"],
    "L2Cache":
        ["aicore_time(us)", "aic_total_cycles", "aic_write_cache_hit", "aic_write_cache_miss_allocate",
         "aic_r0_read_cache_hit", "aic_r0_read_cache_miss_allocate", "aic_r1_read_cache_hit",
         "aic_r1_read_cache_miss_allocate", "aiv_time(us)", "aiv_total_cycles", "aiv_write_cache_hit",
         "aiv_write_cache_miss_allocate", "aiv_r0_read_cache_hit", "aiv_r0_read_cache_miss_allocate",
         "aiv_r1_read_cache_hit", "aiv_r1_read_cache_miss_allocate"]
}

def test_profiler_with_experimental_aic_metrics():
    epochs = 5
    with tempfile.TemporaryDirectory() as prof_dir:
        aic_metrics_dict = {
            "PipeUtilization": torch_npu.profiler.AiCMetrics.PipeUtilization,
            "ArithmeticUtilization": torch_npu.profiler.AiCMetrics.ArithmeticUtilization,
            "Memory": torch_npu.profiler.AiCMetrics.Memory,
            "MemoryL0": torch_npu.profiler.AiCMetrics.MemoryL0,
            "ResourceConflictRatio": torch_npu.profiler.AiCMetrics.ResourceConflictRatio,
            "MemoryUB": torch_npu.profiler.AiCMetrics.MemoryUB,
            "L2Cache": torch_npu.profiler.AiCMetrics.L2Cache,
        }
        for key, aic_metrics in aic_metrics_dict.items():
            profiler_path = os.path.join(prof_dir, key)
            experimental_config = torch_npu.profiler._ExperimentalConfig(
                aic_metrics=aic_metrics,
                profiler_level=torch_npu.profiler.ProfilerLevel.Level1,
                l2_cache=False
            )
            with torch_npu.profiler.profile(
                    activities=[torch_npu.profiler.ProfilerActivity.CPU,
                                torch_npu.profiler.ProfilerActivity.NPU],
                    on_trace_ready=torch_npu.profiler.tensorboard_trace_handler(profiler_path),
                    schedule=torch_npu.profiler.schedule(wait=0, warmup=0, active=2, repeat=1, skip_first=1),
                    record_shapes=False,
                    profile_memory=False,
                    with_stack=False,
                    with_flops=False,
                    with_modules=False,
                    experimental_config=experimental_config
            ) as prof:
                for epoch in range(epochs):
                    train(epoch, epochs)
                    prof.step()
            # 1. Check kernel_details.csv
            ascend_profiler_output_path = glob.glob(f"{profiler_path}/*_ascend_pt/ASCEND_PROFILER_OUTPUT")[0]
            kernel_detail_path = os.path.join(ascend_profiler_output_path, "kernel_details.csv")
            pmu_header = pmu_headers.get(key, [])
            FileChecker.check_csv_headers(kernel_detail_path, common_header + pmu_header)
            # 2. Check profiler.log
            profiler_log_paths = glob.glob(f"{profiler_path}/*_ascend_pt/"
                                           f"logs/*.log")
            assert profiler_log_paths, f"No profiler.log found in {profiler_path}"
            for profiler_log_path in profiler_log_paths:
                FileChecker.check_file_for_keyword(profiler_log_path, "error")


if __name__ == "__main__":
    test_profiler_with_experimental_aic_metrics()
