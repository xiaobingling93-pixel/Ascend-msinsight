# import seaborn as sns
import os
import json
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.font_manager import _get_font
from datetime import datetime
import io

def plot_profiler_performance(result_dir:str) -> dict:
    """
    绘制所有配置文件的性能趋势图
    """
    json_file_name = "results.json"
    json_file = os.path.join(result_dir, json_file_name)
    analysis_plot_path = f"{result_dir}/plot/analysis"
    collection_plot_path = f"{result_dir}/plot/collection"

    if not os.path.isfile(json_file):
        raise RuntimeError("测试结果json文件不存在")

    with open(json_file, 'r', encoding='utf-8') as f:
        historical_results = json.load(f)

    for config_name, daily_results in historical_results.items():
        print("config_name: ", config_name)
        if "analysis" in config_name:
            dates, msprof_times, total_times = extract_data_for_analysis_config(daily_results, config_name)
            generate_analysis_plot(config_name, dates, msprof_times, total_times, analysis_plot_path)
        elif "collection" in config_name:
            dates, normal_avg_step_time, profiler_avg_step_time, inflation_ratio = extract_data_for_collection_config(daily_results, config_name)
            generate_collection_plot(config_name, dates, normal_avg_step_time, profiler_avg_step_time, inflation_ratio, collection_plot_path)

def extract_data_for_analysis_config(results: list, config_name: str) -> tuple:
    """
    提取某个配置文件的性能数据
    """
    dates = []
    msprof_times = []
    total_times = []

    for day_result in results:
        # if day_result['config_file'].split('/')[-1] == config_name and day_result['status'] == 'success':
        if day_result['status'] == 'success':
            dates.append(day_result['date'])
            msprof_times.append(day_result['msprof_analysis_time'])
            total_times.append(day_result['total_analysis_time'])

    return dates, msprof_times, total_times

def extract_data_for_collection_config(results: list, config_name: str) -> tuple:
    """
    提取某个配置文件的性能数据
    """
    dates = []
    normal_avg_step_time = []
    profiler_avg_step_time = []
    inflation_ratio = []

    for day_result in results:
        # if day_result['config_file'].split('/')[-1] == config_name and day_result['status'] == 'success':
        if day_result['status'] == 'success':
            dates.append(day_result['date'])
            normal_avg_step_time.append(day_result['normal_avg_step_time'])
            profiler_avg_step_time.append(day_result['profiler_avg_step_time'])
            inflation_ratio.append(day_result['inflation_ratio'])

    return dates, normal_avg_step_time, profiler_avg_step_time, inflation_ratio

def generate_analysis_plot(config_name: str, dates: list, msprof_times: list, total_times: list, save_dir: str):
    """
    生成某个配置文件的解析耗时趋势图
    """
    if not dates:  # 如果没有数据，返回None
        return None

    if not os.path.exists(save_dir):
        os.makedirs(save_dir)
    # 创建图表
    plt.figure(figsize=(6, 3))
    # plt.style.use('seaborn')

    # 绘制折线图
    plt.plot(dates, msprof_times, 'o-', label='MsProf Parsing Times', linewidth=1, markersize=4)
    plt.plot(dates, total_times, 'o-', label='Total Parsing Times', linewidth=1, markersize=4)

    # 设置图表格式
    title_name = config_name.split('.')[0]
    plt.title(f'{title_name} Performance Trend', fontsize=6, pad=10)
    plt.xlabel('Date', fontsize=6)
    plt.ylabel('Parsing Times(s)', fontsize=6)
    # 调整x轴和y轴刻度字体大小
    plt.xticks(fontsize=6)
    plt.yticks(fontsize=6)
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.legend(fontsize=6, loc='upper right')

    # 设置x轴标签
    plt.xticks(rotation=45, ha='right')

    # 调整布局
    plt.tight_layout()

    # 将图片保存到内存
    save_png = f"{save_dir}/{title_name}.png"
    plt.savefig(save_png, format='png', dpi=170)
    plt.close()

def generate_collection_time_plot(title_name: str, dates: list, normal_avg_step_time: list, profiler_avg_step_time: list, save_dir: str):
    """
    生成某个配置文件的每个step的profiler耗时趋势图
    """
    # 创建图表
    plt.figure(figsize=(6, 3))

    # 绘制折线图
    plt.plot(dates, normal_avg_step_time, 'o-', label='Normal Avg Step Times', linewidth=1, markersize=4)
    for i, date in enumerate(dates):
        plt.text(date, normal_avg_step_time[i], f'{normal_avg_step_time[i]:.2f}', ha='center', va='bottom', fontsize=4)
    plt.plot(dates, profiler_avg_step_time, 'o-', label='Profiler Avg Step Times', linewidth=1, markersize=4)
    for i, date in enumerate(dates):
        plt.text(date, profiler_avg_step_time[i], f'{profiler_avg_step_time[i]:.2f}', ha='center', va='bottom', fontsize=4)

    # 设置图表格式
    plt.title(f'{title_name} Step Time Trend', fontsize=6, pad=10)
    plt.xlabel('Date', fontsize=6)
    plt.ylabel('Step Times(s)', fontsize=6)
    # 调整x轴和y轴刻度字体大小
    plt.xticks(fontsize=6)
    plt.yticks(fontsize=6)
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.legend(fontsize=6, loc='upper right')

    # 设置x轴标签
    plt.xticks(rotation=45, ha='right')

    # 设置y轴范围使其从0开始
    max_normal = max(normal_avg_step_time)
    max_profiler = max(profiler_avg_step_time)
    plt.ylim(ymin=0, ymax=max(max_normal, max_profiler) * 1.2)

    # 调整布局
    plt.tight_layout()

    # 将图片保存到内存
    save_png = f"{save_dir}/{title_name}_time.png"
    plt.savefig(save_png, format='png', dpi=170)
    plt.close()

def generate_collection_ratio_plot(title_name: str, dates: list, inflation_ratio: list, save_dir: str):
    """
    生成某个配置文件的膨胀系数趋势图
    """
    # 创建图表
    plt.figure(figsize=(6, 3))

    # 绘制折线图
    plt.plot(dates, inflation_ratio, 'o-', label='Inflation Ratio', linewidth=1, markersize=4)
    for i, date in enumerate(dates):
        plt.text(date, inflation_ratio[i], f'{inflation_ratio[i]:.2f}', ha='center', va='bottom', fontsize=4)

    # 设置图表格式
    plt.title(f'{title_name} Inflation Ratio Trend', fontsize=6, pad=10)
    plt.xlabel('Date', fontsize=6)
    plt.ylabel('Inflation Ratio(%)', fontsize=6)
    # 调整x轴和y轴刻度字体大小
    plt.xticks(fontsize=6)
    plt.yticks(fontsize=6)
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.legend(fontsize=6, loc='upper right')

    # 设置x轴标签
    plt.xticks(rotation=45, ha='right')

    plt.ylim(ymin=0, ymax=max(inflation_ratio) * 1.2)

    # 调整布局
    plt.tight_layout()

    # 将图片保存到内存
    save_png = f"{save_dir}/{title_name}_ratio.png"
    plt.savefig(save_png, format='png', dpi=170)
    plt.close()

def generate_collection_plot(config_name: str, dates: list, normal_avg_step_time: list, profiler_avg_step_time: list, inflation_ratio: list, save_dir: str):
    """
    生成某个配置文件的profiler性能趋势图
    """
    if not dates:  # 如果没有数据，返回None
        return None

    if not os.path.exists(save_dir):
        os.makedirs(save_dir)

    title_name = config_name.split('.')[0]
    generate_collection_time_plot(title_name, dates, normal_avg_step_time, profiler_avg_step_time, save_dir)
    generate_collection_ratio_plot(title_name, dates, inflation_ratio, save_dir)
