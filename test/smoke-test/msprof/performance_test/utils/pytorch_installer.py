#!/usr/bin/python
# -*- coding: UTF-8 -*-
"""PTA下载安装部署工具"""

import os
import sys
import argparse
import shutil
from pathlib import Path

from utils.date_utils import FormatDate
from utils.logger import logger
from utils.command_executor import CommandExecutor


class PTAInstaller:
    """PTA下载安装工具"""

    def __init__(self, params: dict):
        self.user = params.get("user_name", "open_mock")
        self.pwd = params.get("user_pwd", "open_mock")
        self.version = params["version"]
        self.py_version = "".join(params["py_version"].split("."))
        self.pta_path = Path(params["local_dir"]) / f"pta_{self.version}"
        self.cmd_executor = CommandExecutor()
        self.host_platform = self._get_platform()
        self.pta_package_date = ""
        self.pta_package = ""

    def _get_platform(self) -> str:
        """获取平台类型"""
        stdout, _, _ = self.cmd_executor.execute("uname -m")
        machine = stdout.strip().lower()
        if 'x86_64' in machine:
            return 'x86_64'
        elif 'aarch64' in machine:
            return 'aarch64'
        else:
            raise RuntimeError(f"Unsupported platform: {machine}")

    def _try_download_pta(self, date: str) -> str:
        """尝试下载指定日期的PTA包"""
        try:
            # 构建artget命令
            pkg_name = f"{date}-07/Pytorch/pytorch_{self.version}_py{self.py_version}.tar.gz"
            artget_cmd = [
                "artget", "pull", "FrameworkPTAdapter 7.2.0",
                "-ru", "software",
                "-user", self.user,
                "-pwd", self.pwd,
                "-rp", f"{date}-07/Pytorch/pytorch_{self.version}_py{self.py_version}.tar.gz",
                "-ap", str(self.pta_path)
            ]

            # 执行下载命令
            logger.info(f"尝试下载日期为 {date} 的PTA包...")
            self.cmd_executor.execute(artget_cmd)
            logger.info(f"日期为 {date} 的PTA包下载完成！")

            # 返回下载的包路径
            pkg_path = self.pta_path / f"pytorch_{self.version}_py{self.py_version}.tar.gz"
            if pkg_path:
                self.pta_package_date = date
                self.pta_package = pkg_name
            return str(pkg_path)

        except Exception as e:
            logger.warning(f"日期为 {date} 的PTA包下载失败: {str(e)}")
            return None

    def download_pta(self) -> str:
        """下载PTA包，尝试最近5天的日期"""
        # 创建目标目录
        self.pta_path.mkdir(parents=True, exist_ok=True)
        logger.info(f"创建目录: {self.pta_path}")

        # 获取最近三天的日期
        recent_dates = FormatDate.get_recent_days(5)
        for date in recent_dates:
            date_str = date.strftime("%Y%m%d")
            pkg_path = self._try_download_pta(date_str)
            if pkg_path:
                return pkg_path

        raise RuntimeError("最近5天均未找到可用的PTA包")

    def install_pta(self, pkg_path: str):
        """安装PTA包"""
        try:
            # 解压PTA包
            logger.info("开始解压PTA包...")
            self.cmd_executor.execute(["tar", "-xvf", pkg_path, "-C", self.pta_path])
            logger.info("PTA包解压完成！")

            # 安装whl文件
            whl_file = list(self.pta_path.glob(f"torch_npu-*{self.host_platform}.whl"))
            if not whl_file:
                raise FileNotFoundError(f"未找到whl文件: torch_npu-*{self.host_platform}.whl")
            logger.info(f"找到whl文件{whl_file}...")
            logger.info(f"开始安装whl文件...")
            self.cmd_executor.execute(["pip", "install", "--force-reinstall", str(whl_file[0])])
            logger.info("whl文件安装完成！")

        except Exception as e:
            logger.error(f"安装失败: {str(e)}")
            raise RuntimeError("PTA包安装失败")

    def clean(self):
        """清理安装目录"""
        if self.pta_path.exists():
            logger.info(f"清理目录: {self.pta_path}")
            try:
                shutil.rmtree(self.pta_path)
                logger.info(f"目录清理完成: {self.pta_path}")
            except Exception as e:
                raise RuntimeError(f"清理目录失败: {str(e)}")
        else:
            logger.info(f"目录不存在，无需清理: {self.pta_path}")

    def run_procedure(self):
        """执行完整的安装流程"""
        try:
            logger.info("=== 开始PTA下载安装流程 ===")

            logger.info("步骤1: 清理安装目录")
            self.clean()

            logger.info("步骤2: 下载PTA包")
            pkg_path = self.download_pta()

            logger.info("步骤3: 安装PTA包")
            self.install_pta(pkg_path)

            logger.info("=== PTA下载安装流程完成 ===")
            return True

        except Exception as e:
            logger.error(f"流程失败: {str(e)}")
            return False


def main():
    parser = argparse.ArgumentParser(description="PTA下载安装工具")
    parser.add_argument("--version",
                        default="v2.8.0",
                        help="PTA版本号,可选择[master、v2.1.0、v2.6.0、v2.7.1、v2.8.0]")
    parser.add_argument("--py_version",
                       default=f"{sys.version_info.major}.{sys.version_info.minor}",
                       help="Python version")
    parser.add_argument("--local_dir",
                        default="/tmp/pta_pkg",
                        help="存储包的本地目录")

    args = parser.parse_args()

    try:
        downloader = PTAInstaller(vars(args))
        success = downloader.run_procedure()
        return 0 if success else 1
    except Exception as e:
        logger.error(f"程序运行失败: {str(e)}")
        return 1


if __name__ == "__main__":
    exit(main())
