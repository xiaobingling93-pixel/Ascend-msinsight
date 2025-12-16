import json
import os
import subprocess
import signal
import sys
import traceback
import time
from typing import Dict

import yaml


class RunCaseException(Exception):
    pass


class CleanCaseException(Exception):
    pass


class Config(dict):
    def __init__(self, **kwargs):
        super(Config, self).__init__(**kwargs)
        self.__dict__ = self


def read_yaml_config(test_case_yml: str, default_config: Dict):
    config = default_config
    with open(test_case_yml) as yaml_file:
        yaml_config = yaml.safe_load(yaml_file)
        config.update(yaml_config)
    return config


def exec_script_file(script_path: str):
    if script_path.endswith(".py"):
        python_command = sys.executable or "python3"
        return os.system(f"{python_command} {script_path}")
    elif script_path.endswith(".sh"):
        bash_command = "bash"
        return os.system(f"{bash_command} {script_path}")
    else:
        raise RunCaseException("unsupported script file")


def exec_script_file_with_timeout(script_path: str, timeout):
    if script_path.endswith(".py"):
        python_command = sys.executable or "python3"
        args = [python_command, script_path]
    elif script_path.endswith(".sh"):
        bash_command = "bash"
        args = [bash_command, script_path]
    else:
        raise RunCaseException("unsupported script file")

    try:
        # 使用 Popen 启动子进程，并创建新的进程组
        process = subprocess.Popen(args, start_new_session=True)

        def signal_handler(sig, frame):
            print("Received Ctrl+C, terminating the subprocess...")
            try:
                # 杀死整个进程组（包括子进程的子进程）
                os.killpg(os.getpgid(process.pid), signal.SIGKILL)
            except ProcessLookupError:
                pass
            # 等待子进程完全退出
            process.wait()
            raise RunCaseException(f"Process interrupted: {' '.join(args)}")

        # 注册 SIGINT 信号处理函数
        signal.signal(signal.SIGINT, signal_handler)

        try:
            # 等待子进程完成，超时后抛出 TimeoutExpired
            process.wait(timeout=timeout * 60)
        except subprocess.TimeoutExpired as ex:
            # 超时后，尝试杀死子进程 杀死整个进程组（包括子进程的子进程）
            os.killpg(os.getpgid(process.pid), signal.SIGKILL)
            # 等待子进程完全退出
            process.wait()
            # 抛出自定义异常
            raise RunCaseException(f"Time out: {' '.join(args)}") from ex

        # 如果正常完成，返回子进程的返回码
        return process.returncode
    except RunCaseException as e:
        raise e
    except Exception as e:
        print(f"An error occurred: {e}")
        return -1


def pre_processing(config, test_case_workspace_path, res_path):
    # 软连接
    if not os.path.exists(res_path) or not os.path.isdir(res_path):
        raise RunCaseException("资源文件夹不存在")
    for file_name_to_link in os.listdir(res_path):
        if file_name_to_link.endswith(".md"):
            continue
        file_path_to_link = os.path.join(res_path, file_name_to_link)
        file_path_link_to = os.path.join(test_case_workspace_path, file_name_to_link)
        if os.path.exists(file_path_link_to):
            os.unlink(file_path_link_to)
        os.symlink(file_path_to_link, file_path_link_to)

    # 配置环境变量
    os.putenv("CASE_WORKSPACE_PATH", test_case_workspace_path)


def post_processing(config, test_case_workspace_path):
    # 删除环境变量
    os.unsetenv("CASE_WORKSPACE_PATH")

    # 调用clean.sh
    clean_script = config.get("script", {}).get("clean", "clean.sh")
    clean_script_path = os.path.join(test_case_workspace_path, clean_script)
    if os.path.exists(clean_script_path):
        exec_script_file(clean_script_path)


def record_and_analysis_metric(config, test_case_workspace_path, result_record):
    # record result
    metric_json_file = config.get("metric", "metric.json")
    result_file_path = os.path.join(test_case_workspace_path, metric_json_file)
    if not os.path.exists(result_file_path):
        return

    try:
        with open(result_file_path, "r") as json_file:
            metric_result = json.load(json_file)
    except Exception as ex:
        raise RunCaseException(ex) from ex

    result_record["metric_result"] = metric_result

    metric_check_rule: Dict = config.get("metric_check_rule", None)
    result_record["metric_check_rule"] = metric_check_rule
    if not metric_check_rule:
        return

    try:
        failed_message = ""
        metric_check_result = {}
        for metric_key, metric_rule in metric_check_rule.items():
            if metric_key not in metric_result:
                failed_message += f"metric result has no key {metric_key}."
                metric_check_result[metric_key] = False
                continue
            metric_value = float(metric_result[metric_key])
            if len(metric_rule) < 2:
                raise RunCaseException("The metric check rule is incorrectly configured. It must contain two elements.")
            min_metric_value = float(metric_rule[0])
            max_metric_value = float(metric_rule[1])

            if min_metric_value is not None and metric_value < min_metric_value:
                failed_message += f"{metric_key} value {metric_value} less than the minimum value {min_metric_value}."
                metric_check_result[metric_key] = False
                continue

            if max_metric_value is not None and metric_value > max_metric_value:
                failed_message += f"{metric_key} value {metric_value} more than the maximum  value {max_metric_value}."
                metric_check_result[metric_key] = False
                continue
            metric_check_result[metric_key] = True

        result_record["metric_check_result"] = metric_check_result
        if not all(metric_check_result.values()):
            raise RunCaseException(failed_message)
    except Exception as ex:
        raise RunCaseException(ex) from ex


def get_test_case_config(test_case_yml):
    config = Config(
        case_type="default",
        case_tags=[],
        case_name=test_case_yml,
        script=Config(
            start="run.sh",
            clean="clean.sh",
        ),
        enable=True,
        metric="metric.json",
        metric_check_rule=None,
        timeout=3,
        rank_size=1,
        yml_path=test_case_yml,
    )
    config = read_yaml_config(test_case_yml, config)

    config["case_tags"] = set(config.get("case_tags", []))
    config["case_tags"].add(config.get("case_type", "default"))
    config["case_tags"].add(config.get("case_name", test_case_yml))
    return config


def format_time(seconds):
    # 计算小时数，使用整除操作符得到整数小时
    hours = int(seconds // 3600)
    # 计算剩余的秒数用于后续计算分钟和秒
    remaining_seconds = seconds % 3600
    # 计算分钟数，使用整除操作符得到整数分钟
    minutes = int(remaining_seconds // 60)
    # 计算最终剩余的秒数
    secs = int(remaining_seconds % 60)
    # 使用字符串格式化输出为 hh:mm:ss 格式，不足两位的前面补 0
    return f"{hours:02d}:{minutes:02d}:{secs:02d}"


def run_test_case(config, test_case_workspace_path, res_path):
    result_record = dict(
        success=False, message="",
        metric_result={}, metric_check_rule=None, metric_check_result={}
    )
    try:
        # 记录开始时间
        start_time = time.time()

        try:
            # 运行前处理
            # pre_processing(config, test_case_workspace_path, res_path)

            # 运行
            run_script = config.get("script", {}).get("start", "run.sh")
            run_script_path = os.path.join(test_case_workspace_path, run_script)
            if not os.path.exists(run_script_path):
                raise RunCaseException(f"run script path not exists. {run_script_path}")
            if exec_script_file_with_timeout(run_script_path, config.get("timeout", 3)) != 0:
                raise RunCaseException(f"run script path failed. {run_script_path}")

            # 记录&分析结果
            record_and_analysis_metric(config, test_case_workspace_path, result_record)
        except RunCaseException as ex:
            result_record["success"] = False
            result_record["message"] = str(ex)
            print(f"\033[31m", f"[{config.get('case_name')}] has ERROR:", str(ex), "\033[0m")
        else:
            result_record["success"] = True

        # 记录结束时间
        end_time = time.time()

        # 计算执行时间
        execution_time = end_time - start_time
        result_record["execution_time"] = format_time(execution_time)

        # 后处理
        try:
            post_processing(config, test_case_workspace_path)
        except CleanCaseException as ex:
            print(f"\033[33m", f"[{config.get('case_name')}] has WARRING:", str(ex), "\033[0m")
    except Exception as ex:
        print(f"\033[31m", f"[{config.get('case_name')}] has ERROR:", str(ex), "\033[0m")
        traceback.print_stack()
        raise ex

    return config.get("case_type"), config.get("case_name"), result_record
