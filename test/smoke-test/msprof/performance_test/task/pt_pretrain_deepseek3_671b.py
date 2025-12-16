import os
import yaml
import glob
import json
import argparse
from collections import defaultdict
from datetime import datetime
from task.run_deepseek_task import run_deepseek_collection
from task.run_deepseek_task import run_deepseek_analysis
from task.run_daily_performance_smoke import process_daily_result
from utils.logger import logger

current_dir = os.path.dirname(os.path.abspath(__file__))
profiler_path = f"{current_dir}/output/profiler/dp_result/"
framework = "pytorch"

def run_profiler_collection():
    config_name = "pt_pretrain_deepseek3_671b_MAX"
    collection_result = run_deepseek_collection(
        'examples/mcore/deepseek3/pretrain_deepseek3_671b_4k_ptd.sh',
        'examples/mcore/deepseek3/pretrain_deepseek3_671b_4k_ptd_max.sh',
        f'{current_dir}/MindSpeed-Core-MS/MindSpeed-LLM/logs/pretrain_deepseek3_671b_4k_ptd.log')
    no_profiler_cost = collection_result.get('normal_avg_step_time', None)
    analysis_result = run_deepseek_analysis("/pretrain_deepseek3_671b_4k_ptd", "pytorch", "max")
    result_all = defaultdict(list)
    result_all[config_name + "_collection"].append(collection_result)
    result_all[config_name + "_analysis"].append(analysis_result)

    config_name = "pt_pretrain_deepseek3_671b_MIN"
    collection_result = run_deepseek_collection(
        'examples/mcore/deepseek3/pretrain_deepseek3_671b_4k_ptd.sh',
        'examples/mcore/deepseek3/pretrain_deepseek3_671b_4k_ptd_min.sh',
        f'{current_dir}/MindSpeed-Core-MS/MindSpeed-LLM/logs/pretrain_deepseek3_671b_4k_ptd.log',
        no_profiler_cost)
    analysis_result = run_deepseek_analysis("/pretrain_deepseek3_671b_4k_ptd", "pytorch", "min")
    result_all[config_name + "_collection"].append(collection_result)
    result_all[config_name + "_analysis"].append(analysis_result)

    process_daily_result(result_all, f'{current_dir}/output/result')


if __name__ == "__main__":
    result = run_profiler_collection()
