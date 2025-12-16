import os
import time
import glob
import json
import argparse
from collections import defaultdict
from datetime import datetime
from utils.plot_graph import plot_profiler_performance
from utils.logger import logger
from utils.send_mail import EmailSender

current_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(current_dir)
profiler_path = f"{current_dir}/output/profiler/dp_result/"
framework = "pytorch"
result_dir = f"{current_dir}/output/result"

def run_send_result():

    plot_profiler_performance(result_dir)

    # 发送邮件, 需手动配置发送邮箱配置
    email_config_path = os.path.join(root_dir, "test_config", "email_config.json")
    if os.path.exists(email_config_path):
        with open(email_config_path, 'r') as f:
            config = json.load(f)
        email_sender = EmailSender(config)
        daily_state = {
            'start_time': time.strftime("%Y-%m-%d %H:%M:%S"),
            'total_configs': 0,
            'success_configs': 0,
            'failed_configs': 0
        }
        with open(f"{current_dir}/output/daily_state.json", 'r') as f:
            pkg_state = json.load(f)
        email_sender.send_performance_report(daily_state, result_dir, pkg_state)


if __name__ == "__main__":
    result = run_send_result()