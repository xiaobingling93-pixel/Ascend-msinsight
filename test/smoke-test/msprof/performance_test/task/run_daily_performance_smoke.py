import os
import time
import json
import argparse
import traceback
# do not delete import torch below
import torch
from typing import List, Dict
from collections import defaultdict
from task.run_analysis_task import run_profiler_analysis
from task.run_collection_task import run_profiler_collection
from task.run_deepseek_task import run_deepseek_collection
from task.run_deepseek_task import run_deepseek_analysis
from task.run_env_prepare import daily_smoke_env_prepare
from utils.plot_graph import plot_profiler_performance
from utils.logger import logger
from utils.send_mail import EmailSender


current_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(current_dir)

performance_type = ['collection', 'analysis']
model_type = ['pangu', 'tiny_transformer']


def get_test_configs(test_dir) -> List[str]:
    """获取测试配置文件列表"""
    
    config_dir = os.path.join(root_dir, test_dir)
    if not os.path.exists(config_dir):
        logger.error(f"配置目录不存在：{config_dir}")
        return []

    config_files = []
    for file in os.listdir(config_dir):
        if file.endswith('.yaml'):
            config_files.append(os.path.join(config_dir, file))
    return sorted(config_files)


def run_daily_profiler_analysis(analysis_configs, daily_state, daily_result):
    # 执行解析性能测试
    logger.info("\n=== 开始执行解析性能测试 ===")
    for config in analysis_configs:
        logger.info(f"\n测试配置: {config}")
        result = run_profiler_analysis(config)
        config_name = config.split("/")[-1]
        daily_result[config_name].append(result)

        if result['status'] == 'success':
            daily_state['success_configs'] += 1
            logger.info("解析性能测试成功:")
            logger.info(f"msprof解析耗时: {result['msprof_analysis_time']:.2f} 秒")
            logger.info(f"框架解析耗时: {result['framework_analysis_time']:.2f} 秒")
            logger.info(f"总解析耗时: {result['total_analysis_time']:.2f} 秒")
        else:
            daily_state['failed_configs'] += 1
            logger.error(f"解析性能测试失败: {result['error']}")


def run_daily_profiler_collection(collection_configs, daily_state, daily_result):
    # 执行采集性能测试
    logger.info("\n=== 开始执行采集性能测试 ===")
    for config in collection_configs:
        logger.info(f"\n测试配置: {config}")
        result = run_profiler_collection(config)
        config_name = config.split("/")[-1]
        daily_result[config_name].append(result)

        if result['status'] == 'success':
            daily_state['success_configs'] += 1
            logger.info(f"采集性能测试{config}成功:")
            logger.info(f"普通训练平均步时: {result['normal_avg_step_time']:.4f} 毫秒")
            logger.info(f"Profiler训练平均步时: {result['profiler_avg_step_time']:.4f} 毫秒")
            logger.info(f"性能膨胀率: {result['inflation_ratio']:.2f}%")
        else:
            daily_state['failed_configs'] += 1
            logger.error(f"采集性能测试{config}失败: {result['error']}")

def run_daily_deepseek_collection(daily_state, daily_result):
    daily_state['total_configs'] += 1
    result = run_deepseek_collection()
    daily_result['ms_collection_pretrain_deepseekv3_671b_4k_ptd_8p'].append(result)
    if result['status'] == 'success':
        daily_state['success_configs'] += 1
    else:
        daily_state['failed_configs'] += 1

def run_daily_deepseek_analysis(daily_state, daily_result):
    daily_state['total_configs'] += 1
    result = run_deepseek_analysis()
    daily_result['ms_analysis_pretrain_deepseekv3_671b_4k_ptd_8p'].append(result)
    if result['status'] == 'success':
        daily_state['success_configs'] += 1
    else:
        daily_state['failed_configs'] += 1

def process_daily_result(daily_result, result_dir):
    # 处理并保存保存每日用例的数据
    result_json_dir = os.path.join(result_dir, 'results.json')

    historical_results = defaultdict(list)
    if os.path.exists(result_json_dir):
        with open(result_json_dir, 'r', encoding='utf-8') as f:
            historical_results = json.load(f)

    for config, result in daily_result.items():
        if config not in historical_results:
            historical_results[config] = result
        else:
            historical_results[config].extend(result)
        if len(historical_results[config]) > 30:  # 每个用例只保存前一个月的性能数据
            historical_results[config] = historical_results[config][-30:]

    with open(result_json_dir, 'w', encoding='utf-8') as f:
        json.dump(historical_results, f, ensure_ascii=False, indent=4)


def run_model_performance(params: dict, perf_type, model_type, daily_state):
    logger.info(f"=== 开始执行每日{model_type}模型{perf_type}性能测试 ===")

    daily_result = defaultdict(list)
    result_dir = params['result_dir']

    perf_type_name = f"{perf_type}_performance"
    configs = get_test_configs(f"test_config/{perf_type_name}/{model_type}")
    daily_state['total_configs'] += len(configs)
    logger.info(f"找到{model_type}模型{perf_type}测试配置 {len(configs)} 个")

    # 运行解析和采集用例
    if perf_type == 'analysis':
        run_daily_profiler_analysis(configs, daily_state, daily_result)
        run_daily_deepseek_analysis(daily_state, daily_result)
    elif perf_type == 'collection':
        run_daily_profiler_collection(configs, daily_state, daily_result)
        run_daily_deepseek_collection(daily_state, daily_result)

    # 保存测试数据
    process_daily_result(daily_result, result_dir)
    logger.info(f"每日采集性能监控数据结果已保存到 {result_dir}")


def run_performance_type(params: dict, perf_type, daily_state):
    if params['model_name'] == 'both':
        for mtype in model_type:
            run_model_performance(params, perf_type, mtype, daily_state)
    else:
        run_model_performance(params, perf_type, params['model_name'], daily_state)


def run_daily_profiler_performance(params: dict, pkg_state: dict) -> Dict:
    """执行每日性能测试"""
    daily_state = {
        'start_time': time.strftime("%Y-%m-%d %H:%M:%S"),
        'total_configs': 0,
        'success_configs': 0,
        'failed_configs': 0
    }
    result_dir = params['result_dir']

    logger.info("=== 开始执行每日性能测试 ===")
    logger.info(f"开始时间: {daily_state['start_time']}")

    try:
        if params['perf_type'] == 'both':
            for type in performance_type:
                run_performance_type(params, type, daily_state)
        else:
            run_performance_type(params, params['perf_type'], daily_state)
    except Exception as e:
        logger.info("开始打印调用栈:")
        traceback.print_exc()  # 打印完整的异常跟踪信息
        traceback.print_stack()  # 打印当前的调用栈

    daily_state['end_time'] = time.strftime("%Y-%m-%d %H:%M:%S")

    # 打印汇总信息
    logger.info("\n=== 测试汇总 ===")
    logger.info(f"minspore version: {pkg_state.get('ms_package_date', 'None')}-{pkg_state.get('ms_package', 'None')}")
    logger.info(f"pta version: {pkg_state.get('pta_package_date', 'None')}-{pkg_state.get('pta_package', 'None')}")
    logger.info(f"开始时间: {daily_state['start_time']}")
    logger.info(f"结束时间: {daily_state['end_time']}")
    logger.info(f"总配置数: {daily_state['total_configs']}")
    logger.info(f"成功数: {daily_state['success_configs']}")
    logger.info(f"失败数: {daily_state['failed_configs']}")

    # 画图
    plot_profiler_performance(result_dir)

    # 发送邮件, 需手动配置发送邮箱配置
    email_config_path = os.path.join(root_dir, "test_config", "email_config.json")
    if os.path.exists(email_config_path):
        with open(email_config_path, 'r') as f:
            config = json.load(f)
        email_sender = EmailSender(config)
        email_sender.send_performance_report(daily_state, result_dir, pkg_state)

    # 检查并清理不必要的文件

def main():
    parser = argparse.ArgumentParser(description="profiler performance")
    parser.add_argument("--model_name",
                       default="both",
                       help="select the model to run, (pangu, tiny_transformer, ...)")
    parser.add_argument("--perf_type",
                       default="both",
                       help="performance test type, (analysis, collection, ...)")
    parser.add_argument("--result_dir",
                       default="output/result",
                       help="performance test result save directory")

    args = parser.parse_args()

    # 校验入参
    if args.perf_type not in performance_type and args.perf_type != 'both':
        raise ValueError(f"--perf_type must be one of the following: {', '.join(performance_type), 'both'}")

    if args.model_name not in model_type and args.model_name != 'both':
        raise ValueError(f"--model_name must be one of the following: {', '.join(model_type), 'both'}")

    if not args.result_dir[0] == '/':
        result_d = os.path.join(root_dir, args.result_dir)
        args.result_dir = result_d
    if not os.path.exists(args.result_dir):
        os.mkdir(args.result_dir)

    # 准备环境
    pkg_state = daily_smoke_env_prepare()
    # 跑性能测试
    run_daily_profiler_performance(vars(args), pkg_state)

if __name__ == "__main__":
    main()
 