import os
import json
import time
from argparse import ArgumentParser
from collections import OrderedDict
from typing import List, Dict

from record import record, get_default_record_path, get_default_log_path, get_default_log_detail_path
from run_test_case import run_test_case, get_test_case_config
from utils import WorkspaceChange
from send_mail import send_mail
from create_excel import create_excel
from json_parser import get_latest_failed, filter_by_case_names


def parse_args():
    parser = ArgumentParser()
    parser.add_argument('--project_path', dest="project_path", help='project path', required=True)
    parser.add_argument('--test-case', dest="test_case_path", help='test case path', required=True)
    parser.add_argument('--workspace-path', dest="workspace_path", help='workspace path', required=True)
    parser.add_argument('--resource-path', dest="res_path", help='resource path', required=True)
    parser.add_argument('--case-tags', dest="case_tags", nargs='*',
                        help='case tags or type, split by [ ], run cases which contain all tags', default=None)
    parser.add_argument('--case-name', dest="case_name", help='case name', default=None)
    parser.add_argument('--dev-mode', type=int, dest="dev_mode", help='dev mode', default='0')
    parser.add_argument('--send-email', type=int, dest="send_email", help='send-email', default='1')
    parser.add_argument('--cann_version', type=str, dest="cann_version", help='cann_version', default='poc')
    parser.add_argument('--cc_email', type=str, dest="cc_email", help='cc_email', default=None)
    parser.add_argument('--execution_mode', type=str, dest="execution_mode", help='execution_mode', default=None)

    args = parser.parse_args()
    return args


def get_test_cases_from_path(test_case_path: str):
    if os.path.isfile(test_case_path):
        if test_case_path.endswith((".yml", ".yaml")):
            yield test_case_path
    elif os.path.isdir(test_case_path):
        files = list(os.listdir(test_case_path))
        # 优先检查 case.yml
        if "case.yml" in files:
            yield os.path.join(test_case_path, "case.yml")
            return
        # 其次检查 case.yaml
        elif "case.yaml" in files:
            yield os.path.join(test_case_path, "case.yaml")
            return

        for file_name in files:
            file_path = os.path.join(test_case_path, file_name)
            if not os.path.isdir(file_path):
                continue
            for path in get_test_cases_from_path(file_path):
                yield path


def print_sys_env():
    os.system("uname -a")
    os.system("pip list")
    os.system("env")
    os.system("npu-smi info")


def set_test_env(args):
    project_path = args.project_path
    os.environ["PROJECT_PATH"] = project_path

    workspace_path = args.workspace_path
    os.makedirs(workspace_path, exist_ok=True)
    os.environ["WORKSPACE_PATH"] = workspace_path

    res_path = args.res_path
    os.environ["RESOURCE_PATH"] = res_path


def execute_test_cases(execute_config_list: List[Dict], test_case_result=None):
    if test_case_result is None:
        print(f"\033[5;31m", "The test_case_result cannot be empty!", "\n\n\033[0m", flush=True)
        return

    res_path = os.getenv("RESOURCE_PATH", default=None)
    if res_path is None:
        print(f"\033[5;31m", "The args.res_path/'RESOURCE_PATH' cannot be empty!", "\n\n\033[0m", flush=True)
        return

    # 设置用例执行顺序
    # execute_test_cases_config_list = sorted(execute_config_list,
    #                                         key=lambda e: (e.get('exec_seq', 0), e.get('case_name', '')), reverse=True)
    execute_test_cases_config_list = execute_config_list

    for config in execute_test_cases_config_list:
        print(f"\033[34m", config.get("yml_path", ""), "\033[0m", flush=True)
        print(config.get("case_desc", ""), flush=True)
        test_case_workspace_path = os.path.dirname(config.get("yml_path", ""))
        if not test_case_workspace_path:
            print(
                f"\033[5;31m Wrong yml_path \033[0m:\n yml_path: {config.get('yml_path', '')}\n case_name: {config.get('case_name', '')}",
                flush=True)
            continue
        with WorkspaceChange(test_case_workspace_path):
            case_type, case_name, result = run_test_case(config, test_case_workspace_path, res_path)

        if result.get("success"):
            print(f"\033[32m", case_name, "succeed", "\n\n\033[0m", flush=True)
        else:
            print(f"\033[5;31m", case_name, "failed", "\n\n\033[0m", flush=True)
        test_case_result.setdefault(case_type, OrderedDict())
        test_case_result[case_type][case_name] = result
        time.sleep(3)


def main():
    # 获取入参
    args = parse_args()

    # 初始化一些环境变量
    set_test_env(args)

    # 记录当前运行环境变量
    if not args.dev_mode:
        print_sys_env()

    # 获取到 yaml 文件，并循环开始运行用例
    test_case_result = {}
    filter_case_tags = set([tag for tag in args.case_tags if tag])
    execute_test_cases_config_list = []

    for test_case_yml in get_test_cases_from_path(args.test_case_path):
        if args.dev_mode:
            print(test_case_yml, flush=True)
        test_case_workspace_path = os.path.dirname(test_case_yml)
        try:
            config = get_test_case_config(test_case_yml)
            if filter_case_tags and not config.get("case_tags") >= filter_case_tags:
                if args.dev_mode:
                    print("ignore")
                continue
            if args.case_name and config.get("case_name", "default") != args.case_name:
                if args.dev_mode:
                    print("ignore")
                continue
            if not config.get("enable", True):
                if args.dev_mode:
                    print("ignore")
                continue
            execute_test_cases_config_list.append(config)
        except Exception as ex:
            test_case_result.setdefault("unexpected", OrderedDict())
            test_case_result["unexpected"][test_case_yml] = dict(success=False, message=str(ex))
            continue

    if args.execution_mode == 'LAST_FAILED':
        print("模式选择：执行上一次失败用例，若获取不到则不执行")
        # 拼接文件名获取日志目录
        latest_failed_cases = get_latest_failed(os.path.join(args.project_path, "RunWorkspace"))
        # 取用例中上次失败的集合
        execute_test_cases_config_list = filter_by_case_names(execute_test_cases_config_list, latest_failed_cases)

    # 执行用例
    execute_test_cases(execute_test_cases_config_list, test_case_result)

    # 总结所有记录
    record(test_case_result=test_case_result)
    print("评估数值记录：", os.path.abspath(get_default_record_path()))

    if args.dev_mode:
        return
    # 整理为excel 并邮件发送
    if args.send_email:
        with open(get_default_record_path(), 'r') as f:
            test_case_result = json.load(f).get("test_case_result")
            print(test_case_result)
            case_types = test_case_result.keys()
            for case_type in case_types:
                case_result = test_case_result.get(case_type)
                # 整理为excel
                excel_name = "{}-{}-result.xls".format(time.strftime("%Y%m%d"), case_type)

                create_excel(case_result=case_result,
                             excel_name=excel_name,
                             workspace_path=args.workspace_path,
                             case_type=case_type)

                # 并邮件发送
                send_mail(case_result=case_result,
                          excel_name=excel_name,
                          workspace_path=args.workspace_path,
                          case_type=case_type,
                          version=args.cann_version,
                          cc_email=args.cc_email)


if __name__ == '__main__':
    main()
