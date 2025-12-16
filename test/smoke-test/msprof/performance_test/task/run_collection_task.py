import os
import yaml
import glob
import json
import argparse
from datetime import datetime
from utils.file_clear import remove_path_safety
from utils.logger import logger


def run_profiler_collection(config_path: str) -> dict:
    """
    运行profiler离线解析并统计性能指标
    """
    date = datetime.now().strftime('%Y-%m-%d-%H:%M:%S')
    config_name = os.path.basename(config_path)
    with open(config_path, 'r') as f:
        config = yaml.safe_load(f)
    profiler_config = config.get("profile")
    profiler_path = profiler_config.get("result_path")
    # try:
        # 采集数据
    if "_ms_" in config_name:
        from model.tiny_transformer.tiny_transformer_ms import run_ms_model
        # 采集数据（不开启 Profiler)
        step_times = run_ms_model(profiler_config, open_profiler=False)
        # 采集数据（开启 Profiler）
        _ = run_ms_model(profiler_config, open_profiler=True)
        profiler_output_path = glob.glob(f"{profiler_path}/*ascend_ms")[0]
    elif "_pt_" in config_name:
        from model.tiny_transformer.tiny_transformer_pt import run_pt_model
        # 采集数据（不开启 Profiler)
        step_times = run_pt_model(profiler_config, open_profiler=False)
        # 采集数据（开启 Profiler）
        _ = run_pt_model(profiler_config, open_profiler=True)
        profiler_output_path = glob.glob(f"{profiler_path}/*ascend_pt")[0]

    if not os.path.exists(profiler_output_path):
        error_msg = f"Profiler数据路径不存在: {profiler_output_path}"
        logger.error(error_msg)
        return {
            'status': 'failed',
            'date': date,
            'config_file': config_path,
            'profiler_path': profiler_path,
            'error': error_msg
        }

    framework = profiler_config.get("framework")
    logger.info(f"开始解析 {framework} profiler数据...")

    # 执行框架特定的离线解析
    logger.info(f"开始执行{framework}框架解析...")

    if framework == "pytorch":
        import torch_npu
        torch_npu.profiler.profiler.analyse(profiler_path=profiler_output_path)

    elif framework == "mindspore":
        import mindspore
        mindspore.profiler.profiler.analyse(profiler_path=profiler_output_path)

    # except Exception as e:
    #     error_msg = f"框架解析失败: {str(e)}"
    #     logger.error(error_msg)
    #     return {
    #         'status': 'failed',
    #         'date': date,
    #         'config_file': config_path,
    #         'profiler_path': profiler_path,
    #         'error': error_msg
    #     }

    # 读取 trace_view.json 文件
    trace_view_path = os.path.join(profiler_output_path, "ASCEND_PROFILER_OUTPUT", "trace_view.json")
    if not os.path.exists(trace_view_path):
        error_msg = f"trace_view.json 文件不存在: {trace_view_path}"
        logger.error(error_msg)
        return {
            'status': 'failed',
            'date': date,
            'config_file': config_path,
            'profiler_path': profiler_path,
            'error': error_msg
        }

    with open(trace_view_path, 'r') as f:
        trace_data = json.load(f)

    # 提取 ProfilerStep# 对应的时间
    profiler_step_durations = []
    for event in trace_data:
        if "ProfilerStep#" in event["name"]:
            profiler_step_durations.append(float(event["dur"]) / 1000)  # 转换为毫秒

    # 提取profiler的step
    real_step_times = []
    for i in range(len(step_times)):
        if i < profiler_config.get("skip_first", 0) + profiler_config.get("warmup", 0):
            continue
        real_step_times.append(step_times[i] * 1000)
    # 计算膨胀
    if len(real_step_times) != len(profiler_step_durations):
        error_msg = "real_step_times 和 profiler_step_durations 长度不一致"
        logger.error(error_msg)
        return {
            'status': 'failed',
            'date': date,
            'config_file': config_path,
            'profiler_path': profiler_path,
            'error': error_msg
        }

    normal_avg_step_time =  sum(real_step_times) / len(real_step_times)
    profiler_avg_step_time =  sum(profiler_step_durations) / len(profiler_step_durations)
    inflation_ratio = (sum(profiler_step_durations) - sum(real_step_times)) / sum(real_step_times) * 100
    remove_path_safety(profiler_output_path)

    return {
        'status': 'success',
        'date': date,
        'framework': framework,
        'normal_avg_step_time': normal_avg_step_time,
        'profiler_avg_step_time': profiler_avg_step_time,
        'inflation_ratio': inflation_ratio,
        'config_file': config_path,
        'profiler_path': profiler_output_path
    }


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Profiler性能分析工具')
    parser.add_argument('--config',
                        type=str,
                        required=True,
                        help='配置文件路径')

    args = parser.parse_args()
    result = run_profiler_collection(args.config)

    if result['status'] == 'success':
        logger.info("\n性能测试结果:")
        logger.info(f"框架: {result['framework']}")
        logger.info(f"配置文件: {result['config_file']}")
        logger.info(f"Profiler数据路径: {result['profiler_path']}")
        logger.info(f"平均膨胀比例: {result['inflation_ratio']:.4f}%")
    else:
        logger.error(f"测试失败:")
        logger.error(f"错误信息: {result['error']}")
