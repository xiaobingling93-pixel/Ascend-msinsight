#!/usr/bin/python
# -*- coding: UTF-8 -*-
"""Model下载安装部署工具"""

import os
import re
import sys

import argparse
import requests
import glob
import tarfile
from lxml import etree
from utils.date_utils import FormatDate
from utils.logger import logger


class ModelInstaller:
    """Model安装器"""
    
    def __init__(self, params: dict):
        super().__init__(params)

    def build_real_url(self) -> str:
        """构建下载URL"""
        base_url = self.params["base_url"]
        model = self.params["model"]

        html = etree.HTML(requests.get(base_url).text)
        all_branch = html.xpath("/html/body/table/tr/td/a/text()")
        if model not in all_branch:
            raise RuntimeError(f"{model} is not in {all_branch}")

        return f"{base_url}/{model}"

    def download_model(self) -> str:
        """下载model工程包"""
        url = self.build_real_url()
        local_dir = self.params["local_dir"]
        return self.download_package(url, local_dir)

    def deploy(self, pkg_path: str):
        """部署Model"""
        try:
            with tarfile.open(pkg_path, 'r') as tar:
                # 解压所有文件
                parent_name = os.path.dirname(pkg_path)
                tar.extractall(path=parent_name)
            logger.info("Model deploy successfully!")

        except Exception as e:
            logger.error(f"Installation failed: {str(e)}")
            raise

    def run_procedure(self):
        """执行完整的安装流程"""
        try:
            logger.info("=== 开始Model部署流程 ===")

            logger.info("步骤1: 清理安装目录")
            self.clean()

            logger.info("步骤2: 下载Model工程包")
            self.pkg_path = self.download_model()

            logger.info("步骤3: 部署Model")
            self.deploy(self.pkg_path)

            logger.info("=== Model部署流程完成 ===")
            return True

        except Exception as e:
            logger.error(f"安装流程失败: {str(e)}")
            return False


def main():
    parser = argparse.ArgumentParser(description="Model Installer")
    parser.add_argument("--base_url",
                       default="open_mock",
                       help="Base URL for Model packages")
    parser.add_argument("--model",
                       default="pangu_deploy.tar",
                       help="Model packages")
    parser.add_argument("--local_dir",
                       default="/home/yanx/PanGu/Auto_Task/profiler_performance/model/pangu/infer/38Bv3",
                       help="Directory to store packages")

    args = parser.parse_args()

    try:
        installer = ModelInstaller(vars(args))
        success = installer.run_procedure()
        return 0 if success else 1
    except Exception as e:
        logger.error(f"Installation failed: {str(e)}")
        return 1

if __name__ == "__main__":
    exit(main())
