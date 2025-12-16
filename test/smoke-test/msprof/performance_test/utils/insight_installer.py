#!/usr/bin/python
# -*- coding: UTF-8 -*-
"""insight下载安装部署工具"""

import os
import sys
import argparse
import shutil
from pathlib import Path
import subprocess
import re

from utils.date_utils import FormatDate
from utils.logger import logger
from utils.command_executor import CommandExecutor


class InsightInstaller:
    """insight下载安装工具"""

    def __init__(self, params: dict):
        self.user = params.get("user_name", "open_mock")
        self.pwd = params.get("user_pwd", "open_mock")
        self.pta_path = Path("/tmp/insight_pkg")
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

    def _try_download_insight(self, version: list) -> str:
        """尝试下载指定日期的insight包"""
        for i in range(5):
            try:
                logger.info(f"开始下载{version[i]}，一共会尝试cmc上的5个最新包...")
                # 构建artget命令
                pkg_name = f"MindStudio-Insight_8.2.RC1_linux-{self.host_platform}.zip"
                artget_cmd = [
                    "artget", "pull", version[i],
                    "-ru", "software",
                    "-user", self.user,
                    "-pwd", self.pwd,
                    "-rp", pkg_name,
                    "-ap", str(self.pta_path)
                ]

                self.cmd_executor.execute(artget_cmd)


                # 返回下载的包路径
                pkg_path = self.pta_path / pkg_name
                if pkg_path:
                    self.pta_package_date = version[i]
                    self.pta_package = pkg_name
                    break

            except Exception as e:
                logger.warning(f"日期为 {date} 的insight包下载失败: {str(e)}")
                continue
        return str(pkg_path)

    def download_pta(self) -> str:
        """下载insight包"""
        # 创建目标目录
        self.pta_path.mkdir(parents=True, exist_ok=True)
        logger.info(f"创建目录: {self.pta_path}")

        pkg_path = self._try_download_insight(self.get_version())
        if pkg_path:
            return pkg_path

        raise RuntimeError("最近5天均未找到可用的insight包")

    def install_insight(self, pkg_path: str):
        """安装insight包"""
        try:
            # 解压insight包
            logger.info("开始解压insight包...")
            self.cmd_executor.execute(["unzip", "-o", pkg_path, "-d", "/home/profiler_performance/task/Insight"])
            logger.info("insight包解压完成！")

        except Exception as e:
            logger.error(f"安装失败: {str(e)}")
            raise RuntimeError("insight包安装失败")

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
            logger.info("=== 开始insight下载安装流程 ===")

            logger.info("步骤1: 清理安装目录")
            self.clean()

            logger.info("步骤2: 下载insight包")
            pkg_path = self.download_pta()

            logger.info("步骤3: 安装insight包")
            self.install_insight(pkg_path)

            logger.info("=== insight下载安装流程完成 ===")
            return True

        except Exception as e:
            logger.error(f"流程失败: {str(e)}")
            return False

    def get_version(self):
        try:
            result = subprocess.run(
                ["artget", "search", "MindStudio 8.2.RC1"],
                check=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True  # 输出为字符串（而非字节）
            )
            output = result.stdout
        except subprocess.CalledProcessError as e:
            print(f"命令执行失败：{e.stderr}")
            return []
        except FileNotFoundError:
            print("未找到 artget 命令，请检查是否安装或配置环境变量")
            return []

        version_lines = [line.strip() for line in output.splitlines() 
                        if "MindStudio 8.2.RC1.B" in line][0:5]

        versions = []
        for line in version_lines:
            # 第一步：匹配完整的 Version 部分（含开头的 MindStudio）
            # 规则：任意字符(Version) + 空格 + 纯数字(DeltaId) + 空格 + 分类(Category)
            match = re.match(r'^(.*?)\s+(\d+)\s+(\S+)$', line)
            if match:
                full_version = match.group(1).strip()  # 如 "MindStudio  MindStudio 8.2.RC1.B073-..."
                # 第二步：提取 "MindStudio" 后面的核心版本号（去掉所有开头的 "MindStudio" 及空格）
                # 用正则替换掉开头的 "MindStudio" 和可能的空格
                core_version = re.sub(r'^MindStudio\s+', '', full_version)
                versions.append(core_version)
        return versions


def main():
    parser = argparse.ArgumentParser(description="insight下载安装工具")
    parser.add_argument("--local_dir",
                        default="/tmp/insight_pkg",
                        help="存储包的本地目录")

    args = parser.parse_args()

    try:
        downloader = InsightInstaller(vars(args))
        success = downloader.run_procedure()
        return 0 if success else 1
    except Exception as e:
        logger.error(f"程序运行失败: {str(e)}")
        return 1


if __name__ == "__main__":
    exit(main())
