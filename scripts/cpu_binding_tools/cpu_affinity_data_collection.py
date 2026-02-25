#!/usr/bin/env python
# -*- coding: UTF-8 -*-

"""
-------------------------------------------------------------------------
This file is part of the MindStudio project.
Copyright (c) 2026 Huawei Technologies Co.,Ltd.

MindStudio is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:

        http://license.coscl.org.cn/MulanPSL2

THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details.
-------------------------------------------------------------------------
"""

"""
-------------------------------------------------------------------------
Collect the key process / thread cpu affinity data
-------------------------------------------------------------------------
"""

import os
import re
import subprocess
import argparse
import logging
from cpu_binding_utils import LoggerUtils, InputValidationUtils


class CpuAffinityCollector:
    # ==============================================================================
    # 正则 & 常量
    # ==============================================================================
    NPU_THREAD_FIXED_PATTERN = re.compile(
        r'^(release_thread|acl_thread|pt_(data_pin|autograd_\d+)|data_pin)$'
    )
    DEV_SQ_PATTERN = re.compile(r'^dev(\d+)_sq(?:_task)?$')
    BUS_ID_PATTERN = re.compile(
        r"^[0-9a-fA-F]{4}:[0-9a-fA-F]{2}:[0-9a-fA-F]{2}\.[0-9a-fA-F]$"
    )
    PROC_STAT_PROCESSOR_INDEX = 36  # processor 字段（0-based, after ')')

    # ==============================================================================
    # 初始化
    # ==============================================================================
    def __init__(self, csv_mode: bool = False):
        self.csv_mode = csv_mode
        self._npu_topology_cache = None
        self.logger = LoggerUtils.setup_logger(self.__class__.__name__, logging.INFO)

    # ==============================================================================
    # 输出
    # ==============================================================================
    def print_header(self):
        if not self.csv_mode:
            print(
                f"{'NPU_ID':<8} {'NUMA':<15} {'PID':<10} "
                f"{'PROCESS':<20} {'TID':<10} "
                f"{'THREAD':<20} {'PSR':<6} {'CPU_AFFINITY'}"
            )
            print("-" * 110)

    def print_row(self, npu_id, numa, pid, proc, tid, thread, psr, aff):
        if self.csv_mode:
            print(f"{npu_id},{numa},{pid},{proc},{tid},{thread},{psr},{aff}")
        else:
            print(
                f"{str(npu_id):<8} {str(numa):<15} {str(pid):<10} "
                f"{proc[:19]:<20} {str(tid):<10} "
                f"{thread[:19]:<20} {str(psr):<6} {aff}"
            )

    # ==============================================================================
    # 基础工具
    # ==============================================================================
    def _get_file_content(self, path):
        try:
            with open(path, "r") as f:
                return f.read().strip()
        except Exception as e:
            LoggerUtils.log_file_operation_error(
                self.logger,
                operation="读取",
                filepath=path,
                error=e)
            return None

    def _get_cpu_info(self, pid, tid):
        psr, aff = "-", "N/A"
        # PSR
        try:
            with open(f"/proc/{pid}/task/{tid}/stat", "r") as f:
                content = f.read()
                r_par = content.rfind(")")
                if r_par == -1:
                    raise ValueError("stat 格式异常，未找到 ')'")
                fields = content[r_par + 1:].split()

                if len(fields) > self.PROC_STAT_PROCESSOR_INDEX:
                    psr = fields[self.PROC_STAT_PROCESSOR_INDEX]
                else:
                    self.logger.warning(
                        f"/proc/{pid}/task/{tid}/stat 字段不足，无法解析 processor"
                    )
        except Exception as e:
            LoggerUtils.log_file_operation_error(
                self.logger,
                "读取线程 stat",
                f"/proc/{pid}/task/{tid}/stat",
                e
            )
        # Affinity
        try:
            with open(f"/proc/{pid}/task/{tid}/status", "r") as f:
                for line in f:
                    if line.startswith("Cpus_allowed_list:"):
                        aff = line.split(":", 1)[1].strip()
                        break
        except Exception as e:
            LoggerUtils.log_file_operation_error(
                self.logger,
                "读取线程 status",
                f"/proc/{pid}/task/{tid}/status",
                e
            )

        return psr, aff

    # ==============================================================================
    # NPU 拓扑
    # ==============================================================================
    def _parse_npu_topology(self, output: str):
        """
        npu-smi info 拓扑解析逻辑说明：

        表结构（| 分隔）：
        - NPU 行   : parts[0] = 物理 NPU ID
        - Chip 行  : parts[0] = Chip ID, parts[1] = PCI Bus ID
        - 遇到包含 'Process id' 行后，拓扑表结束

        pending_npu 用于记录最近解析到的 NPU 行，
        下一条合法的 chip 行会归属到该 NPU。
        """
        topo = {}
        pending_npu = None

        for line in output.splitlines():
            if "Process id" in line:
                break
            if not line.startswith("|"):
                continue

            parts = [p.strip() for p in line.split("|") if p.strip()]
            if not parts:
                continue

            if pending_npu is None:
                m = re.match(r"^(\d+)\b", parts[0]) # parts[0] = NPU ID（仅在 NPU 行）
                if m:
                    pending_npu = int(m.group(1))
                continue

            if len(parts) >= 2 and self.BUS_ID_PATTERN.match(parts[1]): # parts[1] = PCI Bus ID
                try:
                    chip = int(parts[0])
                except Exception:
                    chip = 0

                topo[(pending_npu, chip)] = {
                    "logical": pending_npu,
                    "bus_id": parts[1].lower(),
                }

            pending_npu = None

        return topo

    def _get_npu_topology(self):
        if self._npu_topology_cache is not None:
            return self._npu_topology_cache

        try:
            output = subprocess.check_output(
                ["npu-smi", "info"],
                text=True,
                timeout=5,
                stderr=subprocess.PIPE
            )
            self._npu_topology_cache = self._parse_npu_topology(output)
        except subprocess.TimeoutExpired:
            self.logger.error("扫描 NPU 拓扑超时")
        except subprocess.CalledProcessError as e:
            self.logger.error(
                f"❌ 执行 npu-smi info 失败，返回码={e.returncode}\n{e.stderr}"
            )
            self._npu_topology_cache = {}
        except FileNotFoundError as e:
            self.logger.error("❌ npu-smi 命令不存在，请确认环境变量")
            self._npu_topology_cache = {}
        except Exception:
            self.logger.exception("❌ 获取 NPU 拓扑时发生未知异常")
            self._npu_topology_cache = {}

        return self._npu_topology_cache

    def _get_cpus_for_numa(self, numa_node):
        cpulist = self._get_file_content(
            f"/sys/devices/system/node/node{numa_node}/cpulist"
        )
        return cpulist if cpulist else "-"

    def _get_npu_numa(self, npu_id):
        topo = self._get_npu_topology()
        bus_id = None

        for (phys, _), info in topo.items():
            if phys == npu_id:
                bus_id = info.get("bus_id")
                break

        if not bus_id:
            return "-"

        numa_id = self._get_file_content(
            f"/sys/bus/pci/devices/{bus_id}/numa_node"
        )

        if not numa_id or numa_id == "-1":
            return "-"

        cpu_range = self._get_cpus_for_numa(numa_id)
        return f"{numa_id}[{cpu_range}]"

    # ==============================================================================
    # 扫描逻辑
    # ==============================================================================
    def scan_npu_process(self, extra_keywords):
        safe_extra_keywords = InputValidationUtils.sanitize_keywords(extra_keywords, self.logger)
        if extra_keywords and not safe_extra_keywords:
            self.logger.warning("提供的额外 NPU 线程关键词全部非法，将被忽略")

        try:
            output = subprocess.check_output(
                ["npu-smi", "info"], text=True, timeout=5, stderr=subprocess.DEVNULL
            )
            topo = self._parse_npu_topology(output)
            if self._npu_topology_cache is None:
                self._npu_topology_cache = topo

            in_proc = False
            for line in output.splitlines():
                if "Process id" in line:
                    in_proc = True
                    continue
                if not in_proc or not line.startswith("|"):
                    continue
                if "No running processes" in line:
                    continue

                parts = [p.strip() for p in line.split("|")]
                if len(parts) < 5:
                    continue

                ids = re.findall(r"\d+", parts[1])
                if len(ids) < 2:
                    continue

                phys = int(ids[0])
                chip = int(ids[1])
                pid = parts[2]
                pname = parts[3]

                if not pid.isdigit():
                    continue

                logical = topo.get((phys, chip), {}).get("logical")
                if logical is None:
                    continue

                numa = self._get_npu_numa(phys)
                task_dir = f"/proc/{pid}/task"
                if not os.path.isdir(task_dir):
                    continue

                for tid in os.listdir(task_dir):
                    tname = self._get_file_content(
                        f"{task_dir}/{tid}/comm"
                    ) or "unknown"

                    if (
                        tid == pid
                        or self.NPU_THREAD_FIXED_PATTERN.match(tname)
                        or any(kw in tname.lower() for kw in safe_extra_keywords)
                    ):
                        psr, aff = self._get_cpu_info(pid, tid)
                        self.print_row(
                            logical, numa, pid, pname, tid, tname, psr, aff
                        )
        except subprocess.TimeoutExpired:
            self.logger.error("扫描 NPU 进程/线程超时")
        except Exception:
            self.logger.exception("扫描 NPU 进程时发生异常")

    def scan_sq_task(self):
        try:
            output = subprocess.check_output(
                ["ps", "-eL", "-o", "pid,tid,comm"],
                text=True,
                timeout=5,
            )

            found = False

            for line in output.splitlines():
                parts = line.strip().split(None, 2)
                if len(parts) != 3:
                    continue

                pid, tid, tname = parts
                m = self.DEV_SQ_PATTERN.match(tname)
                if not m:
                    continue

                found = True

                nid = int(m.group(1))
                numa = self._get_npu_numa(nid)
                pname = self._get_file_content(f"/proc/{pid}/comm") or "kernel"
                psr, aff = self._get_cpu_info(pid, tid)

                self.print_row(nid, numa, pid, pname, tid, tname, psr, aff)

            if not found:
                self.logger.debug("未发现 dev*_sq 线程")

        except subprocess.TimeoutExpired:
            self.logger.error("扫描 SQ Task 线程超时")
        except FileNotFoundError:
            self.logger.error("ps 命令不存在，无法扫描 SQ Task 线程")
        except Exception:
            self.logger.exception("扫描 SQ Task 线程时发生异常")

    def scan_datawork_process(self, keywords):
        if not keywords:
            return

        kw_set = InputValidationUtils.sanitize_keywords(keywords, self.logger)
        if keywords and not kw_set:
            self.logger.warning("提供的 datawork 关键词全部非法，将被忽略")
            return
        
        try:
            output = subprocess.check_output(
                ["ps", "-eL", "-o", "pid,tid,comm"], text=True, timeout=5
            )
            for line in output.splitlines()[1:]:
                pid, tid, comm = line.strip().split(None, 2)
                if any(kw in comm.lower() for kw in kw_set):
                    pname = self._get_file_content(f"/proc/{pid}/comm") or "unknown"
                    psr, aff = self._get_cpu_info(pid, tid)
                    self.print_row("-", "-", pid, pname, tid, comm, psr, aff)
        except subprocess.TimeoutExpired:
            self.logger.error("扫描 Datawork 进程/线程超时")
        except Exception:
            self.logger.exception("扫描 Datawork 进程/线程时发生异常")


# ==============================================================================
# CLI
# ==============================================================================
def parse_args():
    parser = argparse.ArgumentParser(description="CPU Binding Validation")
    parser.add_argument("--csv", action="store_true", help="输出 CSV 格式")
    parser.add_argument("--npu-process", nargs="*", default=[], help="额外关注的 NPU 线程名")
    parser.add_argument(
        "--datawork-process", nargs="*", default=[], help="datawork 扫描关键词"
    )
    return parser.parse_args()


def main():
    args = parse_args()
    csv_mode = args.csv or os.environ.get("CSV") == "1"

    collector = CpuAffinityCollector(csv_mode)
    print("🔍 开始扫描 CPU 绑定相关信息...\n")
    collector.print_header()

    collector.scan_npu_process(args.npu_process)
    collector.scan_sq_task()
    collector.scan_datawork_process(args.datawork_process)
    print("\n✅ 扫描完成！")


if __name__ == "__main__":
    main()
