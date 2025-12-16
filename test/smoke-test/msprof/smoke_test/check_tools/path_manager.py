# Copyright 2023 Huawei Technologies Co., Ltd
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ============================================================================
"""Profiler path manager"""
import os
import re
import shutil
import logging

RE_EXCEL_MATCH_EXP = r"^mstt_advisor_\d{1,20}\.xlsx"
RE_HTML_MATCH_EXP = r"^mstt_advisor_\d{1,20}\.html"

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')
class PathManager:

    @classmethod
    def remove_path_safety(cls, path: str):
        """
        Function Description:
            remove path safety
        Parameter:
            path: the path to remove
        Exception Description:
            when invalid data throw exception
        """
        if not os.path.exists(path):
            return

        try:
            shutil.rmtree(path)
        except PermissionError as err:
            logging.error(f"Permission denied while removing path: {path}, err: {err}")
        except Exception as err:
            logging.error(f"Failed to remove path: {path}, err: {err}")

    @classmethod
    def get_files(cls, path: str):
        """
        Get the newest HTML and Excel files in the specified directory.

        Args:
            path (str): Path to the directory containing the files.

        Returns:
            tuple: Two dictionaries containing the paths to the newest HTML and Excel files.
        """
        result_html = {}
        result_excel = {}

        # Get all files and subdirectories in the directory
        items = os.listdir(path)

        # Find the latest HTML file
        newest_html_file = None
        for item in items:
            file_path = os.path.join(path, item)
            if os.path.isfile(file_path) and re.match(RE_HTML_MATCH_EXP, item):
                file_time = item.split(".")[0].split("_")[-1]
                if not newest_html_file or file_time > newest_html_file.split(".")[0].split("_")[-1]:
                    newest_html_file = item

        if not newest_html_file:
            logging.error("advisor result html is not found in path: %s", path)

        # Find the latest Excel file
        newest_excel_file = None
        log_dir = os.path.join(path, "log")
        if os.path.exists(log_dir):
            log_files = os.listdir(log_dir)
            for file_name in log_files:
                if re.match(RE_EXCEL_MATCH_EXP, file_name):
                    file_time = file_name.split(".")[0].split("_")[-1]
                    if not newest_excel_file or file_time > newest_excel_file.split(".")[0].split("_")[-1]:
                        newest_excel_file = file_name
        else:
            logging.error("Log directory not found in path: %s", path)

        if not newest_excel_file:
            logging.error("advisor result excel is not found in log directory: %s", log_dir)

        # Check whether the time in the HTML file matches that in the Excel file.
        if newest_html_file and newest_excel_file:
            if newest_html_file.split(".")[0].split("_")[-1] != newest_excel_file.split(".")[0].split("_")[-1]:
                logging.error("advisor html file and excel file do not match in path: %s", path)

        return (os.path.join(path, newest_html_file) if newest_html_file else None,
                os.path.join(log_dir, newest_excel_file) if newest_excel_file else None)