import os
import yaml
import json
from collections import defaultdict
from utils.logger import logger
from utils.command_executor import CommandExecutor
from utils.mindspore_installer import MindSporeInstaller
from utils.pytorch_installer import PTAInstaller
from utils.insight_installer import InsightInstaller


current_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.dirname(current_dir)


def get_config_file(dir, config_name):
    """获取config目录路径"""
    config_dir = os.path.join(root_dir, dir)
    if not os.path.exists(config_dir):
        logger.error(f"{config_name}配置目录不存在：{config_dir}, 使用默认参数")
        return None

    # 检查配置文件是否存在
    config_path = os.path.join(config_dir, config_name)
    if not os.path.exists(config_path):
        logger.error(f"{config_name}配置文件不存在：{config_path}, 使用默认参数")
        return None
    
    return config_path


def activate_conda_env():
    """激活或创建 Conda 环境，并安装依赖"""
    config_file = get_config_file("test_config/env_packages", "conda_config.yaml")

    with open(config_file, 'r') as f:
        config = yaml.safe_load(f)

    conda_config = config.get("conda_env", {})
    env_name = conda_config.get("name")
    py_version = conda_config.get("python_version")

    if not env_name or not py_version:
        logger.error("Conda 配置文件中缺少必要的参数：name 或 python_version")

    # 检查 Conda 环境是否存在
    if not CommandExecutor.execute(f"conda activate {env_name}", shell=True):
        # 如果环境不存在，则创建
        logger.info(f"创建 Conda 环境 '{env_name}'，Python 版本为 {py_version}")
        CommandExecutor.execute(f"conda create -n {env_name} python={py_version} -y")
        CommandExecutor.execute(f"conda activate {env_name}")
        requirements = os.path.join(root_dir, "requirements.txt")
        # 安装依赖
        if requirements:
            logger.info(f"安装依赖: {requirements}")
            CommandExecutor.execute(f"pip install -r {requirements}")

    return py_version


def get_mindspore_configs(ms_dir):
    """
    获取mindspore配置文件
    """
    ms_config_name = "mindspore_config.yaml"
    config_file = get_config_file(ms_dir, ms_config_name)
    # 读取配置文件中的配置值
    with open(config_file, 'r') as f:
        config = yaml.safe_load(f)
    mindspore_config = config.get("mindspore")
    local_config = config.get("local")
    ascend_config = config.get("ascend")
    return {
        'base_url': mindspore_config.get("base_url"),
        'branch': mindspore_config.get("branch"),
        'version': mindspore_config.get("version"),
        'local_dir': local_config.get("local_dir"),
        'ascend_libs_dir': ascend_config.get("ascend_libs_dir")
    }

# 安装mindspore
def install_mindspore(py_version: str):
    args = get_mindspore_configs("test_config/env_packages")
    args['py_version'] = py_version
    try:
        installer = MindSporeInstaller(args)
        installer.run_procedure()
        return installer.ms_package_date, installer.ms_package
    except Exception as e:
        logger.error(f"Installation failed: {str(e)}")
        return None, None

def get_pta_configs(pta_dir):
    """获取pta配置文件"""

    pta_config_name = "pta_config.yaml"
    config_file = get_config_file(pta_dir, pta_config_name)
    # 读取配置文件中的配置值
    with open(config_file, 'r') as f:
        config = yaml.safe_load(f)
    pta_config = config.get("pta")
    local_config = config.get("local")
    return {
        'user_name': pta_config.get("user_name"),
        'user_pwd': pta_config.get("user_pwd"),
        'version': pta_config.get("version"),
        'local_dir': local_config.get("local_dir"),
    }

# 安装pytorch
def install_pytorch(py_version: str):
    args = get_pta_configs("test_config/env_packages")
    args['py_version'] = py_version
    try:
        installer = PTAInstaller(args)
        installer.run_procedure()
        return installer.pta_package_date, installer.pta_package
    except Exception as e:
        logger.error(f"Installation failed: {str(e)}")
        return None, None

def install_insight():
    try:
        args={}
        installer = InsightInstaller(args)
        installer.run_procedure()
    except Exception as e:
        logger.error(f"Install insight failed: {str(e)}")

def daily_smoke_env_prepare(daily_state = None):
    if not daily_state:
        daily_state = defaultdict(list)
    py_version = activate_conda_env()
    ms_pkg_data, ms_pkg_name = install_mindspore(py_version)
    pta_pkg_data, pta_pkg_name = install_pytorch(py_version)
    install_insight()
    daily_state.update(
        {
            "pta_package_date": pta_pkg_data,
            "pta_package": pta_pkg_name,
            "ms_package_date": ms_pkg_data,
            "ms_package": ms_pkg_name
        }
    )
    # todo CANN包安装脚本
    return daily_state

if __name__ == "__main__":
    pkg_state = daily_smoke_env_prepare()
    with open(f"{current_dir}/output/daily_state.json", 'w') as f:
        json.dump(pkg_state, f, indent=4)
