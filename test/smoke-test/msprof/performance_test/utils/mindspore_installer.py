#!/usr/bin/python
# -*- coding: UTF-8 -*-
"""MindSpore下载安装部署工具"""

import os
import re
import sys
import shutil

import argparse
import requests
import glob
from pathlib import Path
from lxml import etree
from utils.date_utils import FormatDate
from utils.logger import logger
from utils.command_executor import CommandExecutor
from tenacity import retry, stop_after_attempt, wait_fixed


class MindSporeInstaller:
    """MindSpore安装器"""

    def __init__(self, params: dict):
        self.params = params
        self.cmd_executor = CommandExecutor()
        self.host_platform = self._get_platform()
        self.ms_package_date = ""
        self.ms_package = ""

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

    def _build_branch_url(self) -> str:
        """构建分支URL"""
        base_url = self.params.get("base_url",
                                   "open_mock")
        build_date = self.params.get("build_date", FormatDate().today)
        input_branch = self.params.get("branch", "master")

        try:
            month = build_date[:6]
        except IndexError as e:
            raise RuntimeError("Failed to get month in build date") from e

        date_url = f"{base_url}/{month}/{build_date}"

        html = etree.HTML(requests.get(date_url).text)
        all_branch = html.xpath("/html/body/table/tr/td/a/text()")
        download_branchs = []
        for download_branch in all_branch:
            if input_branch in download_branch:
                download_branchs.append(download_branch)
        logger.info(f"{input_branch} branch in {build_date} is {all_branch}")
        if download_branchs:
            branch = max(download_branchs, key=lambda x: re.search(r"master_(\d+)", x).group(1))
            url = f"{date_url}/{branch}"
            response = requests.get(url)
            response.raise_for_status()
            self.ms_package_date = build_date
            return url

        # 如果没有指定日期和分支，查找最近三天的构建
        logger.info("未指定构建日期和分支，正在查找最近三天的构建包...")
        for date in FormatDate.get_recent_days(3):
            date = FormatDate(date)
            date_url = f"{base_url}/{date.month}/{date.today}"
            try:
                response = requests.get(date_url)
                response.raise_for_status()
                html = etree.HTML(response.text)
            except Exception as e:
                logger.warning(f"Failed to access {url}: {str(e)}")
                continue
            all_branch = html.xpath("/html/body/table/tr/td/a/text()")
            logger.info(f"All branches in {date.today}: {all_branch}")
            download_branchs = []
            for download_branch in all_branch:
                if input_branch in download_branch:
                    download_branchs.append(download_branch)
            if download_branchs:
                branch = max(download_branchs, key=lambda x: re.search(r"master_(\d+)", x).group(1))
                url = f"{date_url}/{branch}"
                response = requests.get(url)
                response.raise_for_status()
                self.ms_package_date = build_date
                return url
        raise RuntimeError(f"No available branch {input_branch} found in {base_url} for the last 3 days")

    def build_real_url(self) -> str:
        """构建下载URL"""
        branch = self._build_branch_url()
        py_version = "".join(self.params["py_version"].split("."))

        if py_version in ["37"]:
            py_suffix = f"cp{py_version}m"
        else:
            py_suffix = f"cp{py_version}"

        version = self.params["version"]
        platform = self.host_platform

        pkg_fpath = f"{branch}unified/{platform}"
        response = requests.get(pkg_fpath)
        response.raise_for_status()

        html = etree.HTML(response.text)
        all_pkg = html.xpath("/html/body/table/tr/td/a/text()")
        logger.info(f"All packages in {pkg_fpath}: {all_pkg}")

        pkg_name = f"mindspore-{version}-cp{py_version}-{py_suffix}-linux_{platform}.whl"
        if pkg_name not in all_pkg:
            raise RuntimeError(f"Package {pkg_name} not found in {pkg_fpath}")

        self.ms_package = pkg_name

        return f"{pkg_fpath}/{pkg_name}"

    @retry(stop=stop_after_attempt(3), wait=wait_fixed(2))
    def requests_url(self, url):
        logger.info(f"requests url from: {url}")
        response = requests.get(url, stream=False)
        response.raise_for_status()
        logger.info(f"sucess get requests from: {url}")
        return response

    def download_package(self) -> str:
        """下载安装包"""
        url = self.build_real_url()
        logger.info(f"Downloading from: {url}")

        local_dir = Path(self.params["local_dir"])
        local_dir.mkdir(parents=True, exist_ok=True)

        pkg_name = url.split("/")[-1]
        local_path = local_dir / pkg_name

        response = self.requests_url(url)

        with open(local_path, "wb") as f:
            for chunk in response.iter_content(chunk_size=8192):
                f.write(chunk)

        logger.info(f"Downloaded to: {local_path}")
        return str(local_path)

    def install(self, pkg_path: str):
        """安装MindSpore"""
        try:
            # 安装依赖包
            self.cmd_executor.execute("pip3 install sympy")
            te_files = glob.glob("/usr/local/Ascend/ascend-toolkit/latest/lib64/te-*-py3-none-any.whl")
            if not te_files:
                raise FileNotFoundError("No matching .whl file found")
            self.cmd_executor.execute(
                f"pip3 install {te_files[0]}"
            )
            hccl_files = glob.glob("/usr/local/Ascend/ascend-toolkit/latest/lib64/hccl-*-py3-none-any.whl")
            if not hccl_files:
                raise FileNotFoundError("No matching .whl file found")
            self.cmd_executor.execute(
                f"pip3 install {hccl_files[0]}"
            )

            # 卸载旧版本
            self.cmd_executor.execute("pip3 uninstall mindspore -y")

            # 安装新版本
            self.cmd_executor.execute(f"pip3 install {pkg_path}")

            # 验证安装
            stdout, _, _ = self.cmd_executor.execute(
                "python3 -c \"import mindspore;mindspore.set_context(device_target='Ascend');mindspore.run_check()\""
            )

            if self.params["version"] not in stdout or "successfully" not in stdout:
                raise RuntimeError("Installation verification failed")

            logger.info("MindSpore installed successfully!")

        except Exception as e:
            logger.error(f"Installation failed: {str(e)}")
            raise

    def clean(self):
        """清理安装目录"""
        local_dir = self.params["local_dir"]
        if os.path.exists(local_dir):
            logger.info(f"清理目录: {local_dir}")
            try:
                # 删除目录下的所有文件
                for item in os.listdir(local_dir):
                    item_path = os.path.join(local_dir, item)
                    if os.path.isfile(item_path):
                        os.remove(item_path)
                    else:
                        shutil.rmtree(item_path)
                logger.info(f"目录清理完成: {local_dir}")
            except Exception as e:
                raise RuntimeError(f"清理目录失败: {str(e)}")
        else:
            logger.info(f"目录不存在，无需清理: {local_dir}")

    def run_procedure(self):
        """执行完整的安装流程"""
        try:
            logger.info("=== 开始MindSpore安装流程 ===")

            logger.info("步骤1: 清理安装目录")
            self.clean()

            logger.info("步骤2: 下载MindSpore安装包")
            self.pkg_path = self.download_package()

            logger.info("步骤3: 安装MindSpore")
            self.install(self.pkg_path)

            logger.info("=== MindSpore安装流程完成 ===")
            return True

        except Exception as e:
            logger.error(f"安装流程失败: {str(e)}")
            return False


def main():
    parser = argparse.ArgumentParser(description="MindSpore Installer")
    parser.add_argument("--base_url",
                        default="open_mock",
                        help="Base URL for MindSpore packages")
    parser.add_argument("--build_date",
                        default=FormatDate().today,
                        help="Build date (YYYYMMDD)")
    parser.add_argument("--branch",
                        default="master",
                        help="Branch name")
    parser.add_argument("--py_version",
                        default=f"{sys.version_info.major}.{sys.version_info.minor}",
                        help="Python version")
    parser.add_argument("--version",
                        default="2.7.0",
                        help="MindSpore version")
    parser.add_argument("--local_dir",
                        default="/tmp/mindspore_pkg",
                        help="Directory to store packages")

    args = parser.parse_args()

    try:
        installer = MindSporeInstaller(vars(args))
        success = installer.run_procedure()
        return 0 if success else 1
    except Exception as e:
        logger.error(f"Installation failed: {str(e)}")
        return 1


if __name__ == "__main__":
    exit(main())
