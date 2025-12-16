# Copyright 2020-2024 Huawei Technologies Co., Ltd
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
"""Profiler command executor"""
import shlex
import subprocess
from typing import Union, Tuple, Optional
import multiprocessing
import logging

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


class CommandExecutor:
    """
    Execute command in subprocess.
    """

    @staticmethod
    def execute(
            cmd: Union[str, list],
            timeout: int = 10 * 60 * 60,
            check: bool = True,
            encoding: str = "utf-8",
            log_file: Optional[str] = None,  # 新增：日志文件路径
    ) -> Optional[subprocess.Popen]:
        """
        Execute a shell command and return the process object.

        Args:
            cmd (Union[str, list]): The command to execute, either as a string or list.
            timeout (int): Maximum execution time in seconds. Defaults to 10 hours.
            check (bool): If True, raise an exception for non-zero exit codes. Defaults to True.
            encoding (str): Specify the encoding for output. Defaults to 'utf-8'.
            log_file (str, optional): Path to log file for redirecting output.

        Returns:
            Optional[subprocess.Popen]: The process object or None if an error occurs.

        Raises:
            RuntimeError: If any error occurs during command execution.
        """
        if isinstance(cmd, str):
            cmd = shlex.split(cmd)

        try:
            # 设置输出文件
            stdout_dest = subprocess.PIPE
            stderr_dest = subprocess.PIPE

            if log_file:
                log_handle = open(log_file, 'w', encoding=encoding)
                stdout_dest = log_handle
                stderr_dest = subprocess.STDOUT  # 将stderr合并到stdout

            # 使用 subprocess.Popen 启动进程
            process = subprocess.Popen(
                cmd,
                stdout=stdout_dest,
                stderr=stderr_dest,
                encoding=encoding,
                text=True,
            )

            # 等待进程完成或超时
            try:
                if log_file:
                    # 如果重定向到文件，直接等待进程完成
                    process.wait(timeout=timeout)
                    stdout, stderr = "", ""
                else:
                    # 原有逻辑：捕获输出
                    stdout, stderr = process.communicate(timeout=timeout)

            except subprocess.TimeoutExpired:
                process.kill()
                if not log_file:
                    stdout, stderr = process.communicate()
                else:
                    process.wait()
                    stdout, stderr = "", ""
                error_msg = f"Command '{cmd}' timed out after {timeout} seconds"
                logging.error(error_msg)
                raise RuntimeError(error_msg)

            return_code = process.returncode

            if check and return_code != 0:
                error_msg = (
                    f"Command '{cmd}' failed with return code {return_code}"
                )
                if not log_file and stderr:
                    error_msg += f", Standard Error: {stderr}"
                elif log_file:
                    error_msg += f", Check log file: {log_file}"
                logging.error(error_msg)
                raise RuntimeError(error_msg)

            logging.info(
                f"Command '{cmd}' executed successfully. Return code: {return_code}, PID: {process.pid}"
            )

            # 关闭日志文件句柄
            if log_file:
                log_handle.close()

            return process

        except (FileNotFoundError, PermissionError) as e:
            error_msg = f"Error executing command '{cmd}': {str(e)}"
            logging.error(error_msg)
            raise RuntimeError(error_msg) from e

        except Exception as e:  # pylint: disable=broad-exception-caught
            error_msg = (
                f"Unexpected error occurred while executing command '{cmd}': {str(e)}"
            )
            logging.error(error_msg)
            raise RuntimeError(error_msg) from e