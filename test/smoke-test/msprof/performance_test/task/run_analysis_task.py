import os
import time
import yaml
import glob
import argparse
from datetime import datetime
from utils.command_executor import CommandExecutor
from utils.file_clear import simplify_profiler_data, remove_path_safety
from utils.logger import logger
from utils.result_file_check import directory_size


def run_profiler_analysis(config_path: str) -> dict:
    """
    运行profiler离线解析并统计性能指标
    """
    date = datetime.now().strftime('%Y-%m-%d-%H:%M:%S')
    config_name = os.path.basename(config_path)
    with open(config_path, 'r') as f:
        config = yaml.safe_load(f)
    profiler_config = config.get("profile")

    current_dir = os.path.dirname(os.path.abspath(__file__))
    parent_dir = os.path.dirname(current_dir)
    profiler_path = os.path.join(parent_dir, profiler_config['result_path'])
    profiler_config['result_path'] = profiler_path
    try:
        # 采集数据
        if "_ms_" in config_name:
            from model.tiny_transformer.tiny_transformer_ms import run_ms_model
            _ = run_ms_model(profiler_config, open_profiler = True)
            profiler_output_path = glob.glob(f"{profiler_path}/*ascend_ms")[0]
        elif "_pt_" in config_name:
            from model.tiny_transformer.tiny_transformer_pt import run_pt_model
            _ = run_pt_model(profiler_config, open_profiler = True)
            profiler_output_path = glob.glob(f"{profiler_path}/*ascend_pt")[0]

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
                'config_file': config_path,
                'profiler_path': profiler_output_path
            }

        framework = profiler_config.get("framework")
        logger.info(f"开始解析 {framework} profiler数据...")

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
            torch_npu.profiler.profiler.analyse(profiler_path=profiler_path)

        elif framework == "mindspore":
            import mindspore
            mindspore.profiler.profiler.analyse(profiler_path=profiler_path)

    except Exception as e:
        error_msg = f"框架解析失败: {str(e)}"
        logger.error(error_msg)
        return {
            'status': 'failed',
            'date': date,
            'error': error_msg,
            'config_file': config_path,
            'profiler_path': profiler_path
        }
    total_analysis_time = time.time() - start_time
    logger.info(f"框架解析完成，总耗时: {total_analysis_time:.2f}秒")

    framework_analysis_time = total_analysis_time - msprof_analysis_time
    remove_path_safety(profiler_path)

    return {
        'status': 'success',
        'date': date,
        'framework': framework,
        'msprof_analysis_time': msprof_analysis_time,
        'framework_analysis_time': framework_analysis_time,
        'total_analysis_time': total_analysis_time,
        'config_file': config_path,
        'profiler_path': profiler_path,
        'prof_size': prof_size,
        'framework_size': framework_size
    }


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Profiler性能分析工具')
    parser.add_argument('--config',
                        type=str,
                        required=True,
                        help='配置文件路径')

    args = parser.parse_args()
    result = run_profiler_analysis(args.config)

    if result['status'] == 'success':
        logger.info("\n性能测试结果:")
        logger.info(f"框架: {result['framework']}")
        logger.info(f"配置文件: {result['config_file']}")
        logger.info(f"Profiler数据路径: {result['profiler_path']}")
        logger.info(f"msprof解析耗时: {result['msprof_analysis_time']:.2f} 秒")
        logger.info(f"框架解析耗时: {result['framework_analysis_time']:.2f} 秒")
        logger.info(f"总解析耗时: {result['total_analysis_time']:.2f} 秒")
    else:
        logger.error(f"测试失败:")
        logger.error(f"错误信息: {result['error']}")
