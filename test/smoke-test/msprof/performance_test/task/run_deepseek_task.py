import re
import os
import time
import glob
import subprocess
import argparse
from statistics import mean
from datetime import datetime
from utils.command_executor import CommandExecutor
from utils.file_clear import simplify_profiler_data, remove_path_safety
from utils.logger import logger
from utils.result_file_check import directory_size
from task.check_tools import check_analyse_result

def extract_iteration_times(file_path=None, decimal_places=None):
    """
    从日志文件中提取所有 iteration (ms): 后的数字
    
    参数:
        file_path (str): 日志文件路径
        decimal_places (int, 可选): 保留的小数位数，默认不限制
    
    返回:
        list: 提取的时间列表（浮点数）
    """
    times = []
    pattern = r'iteration \(ms\):\s*(\d+\.?\d*)'

    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            for line in file:
                match = re.search(pattern, line)
                if match:
                    time = float(match.group(1))
                    if decimal_places is not None:
                        time = round(time, decimal_places)
                    times.append(time)
    except FileNotFoundError:
        print(f"错误：文件 '{file_path}' 不存在")
        return []
    except Exception as e:
        print(f"错误：读取文件时发生异常: {e}")
        return []
    print(times[3:-2])
    if (len(times) > 1):
        return times[3:-2]
    return []

def run_deepseek_collection(bash_path, bash_path_profiler, log_path, no_profiler_cost = None):
    current_dir = os.path.dirname(os.path.abspath(__file__))
    working_directory = os.path.join(current_dir, 'MindSpeed-Core-MS/MindSpeed-LLM')
    script_path = os.path.join(working_directory, bash_path)
    script_profiler_path = os.path.join(working_directory, bash_path_profiler)
    date = datetime.now().strftime('%Y-%m-%d-%H:%M:%S')
    config_path = bash_path
    parent_dir = os.path.dirname(current_dir)
    profiler_path = os.path.join(parent_dir, 'output/profiler/dp_result/')

    try:
        cost = no_profiler_cost
        if not cost:
            result = subprocess.run(
                ['bash', script_path],
                cwd=working_directory,
                capture_output=True,
                text=True,
                check=False
            )

            if result.returncode != 0:
                return {
                    'status': 'failed',
                    'date': datetime.now().strftime('%Y-%m-%d-%H:%M:%S'),
                    'config_file': config_path,
                    'profiler_path': profiler_path,
                    'error': ''
                }

            cost = mean(extract_iteration_times(log_path))

        result = subprocess.run(
            ['bash', script_profiler_path],
            cwd=working_directory,
            capture_output=True,
            text=True,
            check=False
        )

        if result.returncode != 0:
            return {
                'status': 'failed',
                'date': datetime.now().strftime('%Y-%m-%d-%H:%M:%S'),
                'config_file': config_path,
                'profiler_path': profiler_path,
                'error': ''
            }

        cost_p = mean(extract_iteration_times(log_path))
        expansion = (cost_p - cost) / cost * 100
        # print(cost, cost_p, expansion)
        return {
            'status': 'success',
            'date': date,
            'framework': 'mindspore',
            'normal_avg_step_time': cost,
            'profiler_avg_step_time': cost_p,
            'inflation_ratio': expansion,
            'config_file': config_path,
            'profiler_path': profiler_path
        }

    except Exception as e:
        return {
            'status': 'failed',
            'date': date,
            'config_file': config_path,
            'profiler_path': profiler_path,
            'error': ''
        }

def run_deepseek_analysis(config_name, framework, level):
    date = datetime.now().strftime('%Y-%m-%d-%H:%M:%S')
    current_dir = os.path.dirname(os.path.abspath(__file__))
    profiler_path = os.path.join(current_dir, 'output/profiler/dp_result')
    print(profiler_path)
    profiler_output_paths = glob.glob(f"{profiler_path}/*ascend_*")
    for path in profiler_output_paths:
        if glob.glob(f"{path}/PROF_*/device_7"):
            profiler_output_path = path
    try:
        # 获取采集后的原始数据文件夹大小
        prof_dir = glob.glob(f"{profiler_output_path}/PROF_*")[0]
        prof_size = directory_size(prof_dir)
        framework_dir = f"{profiler_output_path}/FRAMEWORK"
        framework_size = directory_size(framework_dir)

        if not os.path.exists(profiler_output_path):
            error_msg = f"Profiler数据路径不存在: {profiler_output_path}"
            logger.error(error_msg)
            return {
                'status': 'failed',
                'date': date,
                'error': error_msg,
                'config_file': config_name,
                'profiler_path': profiler_output_path
            }

        logger.info(f"开始解析 deepseek profiler数据...")

        # 执行msprof命令解析
        msprof_profiler_path = glob.glob(f"{profiler_output_path}/PROF_*")[0]
        logger.info("开始执行msprof导出...")
        msprof_start_time = time.time()
        CommandExecutor.execute(["msprof", "--export=on", f"--output={msprof_profiler_path}"])
        logger.info("开始执行msprof解析...")
        CommandExecutor.execute(["msprof", "--analyze=on", "--rule=communication,communication_matrix",
                                f"--output={msprof_profiler_path}"])
        msprof_analysis_time = time.time() - msprof_start_time
        logger.info(f"msprof解析完成，耗时: {msprof_analysis_time:.2f}秒")

        # 清理中间文件
        simplify_profiler_data(msprof_profiler_path)

        # 执行框架特定的离线解析
        logger.info(f"开始执行{framework}框架解析...")
        start_time = time.time()

        if framework == "pytorch":
            import torch_npu
            torch_npu.profiler.profiler.analyse(profiler_path=profiler_output_path)

        elif framework == "mindspore":
            import mindspore
            mindspore.profiler.profiler.analyse(profiler_path=profiler_output_path)

    except Exception as e:
        remove_path_safety(profiler_path)
        error_msg = f"框架解析失败: {str(e)}"
        logger.error(error_msg)
        return {
            'status': 'failed',
            'date': date,
            'error': error_msg,
            'config_file': config_name,
            'profiler_path': profiler_path
        }
    total_analysis_time = time.time() - start_time
    logger.info(f"框架解析完成，总耗时: {total_analysis_time:.2f}秒")

    framework_analysis_time = total_analysis_time - msprof_analysis_time

    insight_result = check_analyse_result(profiler_path, framework, level)
    check_pass = 'success'
    # if not check_analyse_result(profiler_path, framework):
    #     check_pass = 'failed'

    remove_path_safety(profiler_path)

    return {
        'status': check_pass,
        'date': date,
        'framework': framework,
        'msprof_analysis_time': msprof_analysis_time,
        'framework_analysis_time': framework_analysis_time,
        'total_analysis_time': total_analysis_time,
        'config_file': config_name,
        'profiler_path': profiler_path,
        'prof_size': prof_size,
        'framework_size': framework_size,
        'insight_result': insight_result
    }
