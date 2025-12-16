import json
import os
import subprocess
from typing import List, Dict


def get_latest_file_in_dir(dir_path):
    """
    获取传入目录下排序最后的文件名称，模拟ls -r $dir_path | head -2 命令

    返回:
        str: 最新文件的完整路径，如果目录不存在或为空则返回None
    """
    # 解析用户主目录
    test_dir = os.path.expanduser(dir_path)

    # 检查目录是否存在
    if not os.path.exists(test_dir):
        print(f"错误: 目录 {test_dir} 不存在")
        return None

    try:
        # 执行类似ls -r的命令，获取按反向排序的文件列表
        # -r: 反向排序，-1: 每行一个条目
        # 这里需要取第二个，因为倒数第一个是自己
        result = subprocess.run(
            ['ls', '-r1', test_dir],
            capture_output=True,
            text=True,
            check=True
        )

        # 获取输出并分割成列表
        files = result.stdout.strip().split('\n')

        if not files:
            print(f"警告: 目录 {test_dir} 为空")
            return None

        # 取第一个元素（即排序最后的文件）并拼接完整路径
        json_path = os.path.join(test_dir, files[1], "metric.json")

        # 检查test.json是否存在
        if not os.path.exists(json_path):
            print(f"错误: 文件 {json_path} 不存在")
            return None

        # 检查是否是文件而不是目录
        if not os.path.isfile(json_path):
            print(f"错误: {json_path} 不是一个文件")
            return None

        return json_path

    except subprocess.CalledProcessError as e:
        print(f"执行命令时出错: {e}")
        return None
    except Exception as e:
        print(f"处理文件路径时发生错误: {e}")
        return None


def find_test_cases(data, test_cases=None):
    """
    递归查找包含success字段的测试用例

    参数:
        data: 要解析的数据
        test_cases: 用于收集测试用例的字典

    返回:
        dict: 包含所有测试用例的字典
    """
    if test_cases is None:
        test_cases = {}

    # 如果是字典类型，递归检查
    if isinstance(data, dict):
        # 检查当前层级是否包含success字段
        if "success" in data:
            return data  # 这是一个测试用例

        # 否则递归检查子层级
        for key, value in data.items():
            result = find_test_cases(value, test_cases)
            # 如果找到测试用例，添加到集合中
            if isinstance(result, dict) and "success" in result:
                test_cases[key] = result

    return test_cases


def extract_failed_test_cases(json_file_path):
    """
    读取JSON文件，提取success为false的测试用例名称

    参数:
        json_file_path: JSON文件路径

    返回:
        list: 包含所有success为false的测试用例名称的列表
    """
    failed_tests = []

    # 读取JSON文件
    with open(json_file_path, 'r', encoding='utf-8') as f:
        data = json.load(f)

    # 动态查找所有测试用例，不依赖固定层级名称
    test_cases = find_test_cases(data)

    # 遍历所有测试用例
    for test_name, test_details in test_cases.items():
        # 检查success字段是否为false
        if not test_details.get('success', True):
            failed_tests.append(test_name)

    return failed_tests


def get_latest_failed(work_path):
    """
    获取上一次的失败用例

    参数:
        work_path: 工作日志的路径

    返回:
        list: 包含所有success为false的测试用例名称的列表
    """

    json_path = get_latest_file_in_dir(work_path)

    result = None
    if json_path:
        print(f"将处理文件: {json_path}")
        result = extract_failed_test_cases(json_path)
        print("上次失败的测试用例:", result)
    else:
        print("无法获取有效的JSON文件路径，默认全部执行")
    return result


def filter_by_case_names(execute_list: List[Dict], failed_list: List):
    """
    从 execute_list 中筛选出 case_name 存在于 failed_list 中的字典项

    参数:
        execute_list: 包含case_name键的字典列表，格式为List[Dict]
        failed_list: 包含case_name字符串的列表，格式为List[str]

    返回:
        List[Dict]: 筛选后的字典列表，仅包含那些case_name存在于 failed_list 中的项
                    若 failed_list 为空则返回[]
    """
    if not failed_list:
        print("获取的失败用例列表为空，将停止此次运行，如有单个用例执行需求，可参考WIKI执行单例冒烟")
        return []
    # 转换为集合以提高查找效率
    case_name_set = set(failed_list)

    # 筛选出 execute_list 中case_name存在于集合中的字典
    filtered_list = [
        item for item in execute_list
        # 确保字典包含case_name键，避免KeyError
        if 'case_name' in item and item['case_name'] in case_name_set
    ]

    return filtered_list

# # 示例用法
# if __name__ == "__main__":
#     # 获取~/test目录下排序最后的文件
#     dir_path = "/path/to/Smoke/RunWorkspace"
#     json_path = get_latest_file_in_dir(dir_path)

#     if json_path:
#         print(f"将处理文件: {json_path}")
#         result = extract_failed_test_cases(json_path)
#         print("success为false的测试用例:", result)
#     else:
#         print("无法获取有效的JSON文件路径，程序退出")
