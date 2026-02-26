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
Get the key process / thread info to visualize the pstress
-------------------------------------------------------------------------
"""


import psutil
import subprocess
import re
import logging
from dataclasses import dataclass, field
from typing import List, Union
from cpu_binding_utils import LoggerUtils

@dataclass
class KeyProcNode:
    pid: Union[int, str]
    ppid: Union[int, str]
    name: str
    children: List["KeyProcNode"] = field(default_factory=list)

    def label(self):
        return f"{self.name} (PID={self.pid})"


class KeyPstreeVisualizer:
    def __init__(self):
        self.dev_sq_pattern = re.compile(r'^dev(\d+)_sq(?:_task)?$')
        self.logger = LoggerUtils.setup_logger(self.__class__.__name__, logging.INFO)

    def _safe_proc_info(self, pid: int):
        try:
            p = psutil.Process(pid)
            return {
                "pid": p.pid,
                "ppid": p.ppid(),
                "name": p.name()
            }
        except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
            return None
    
    # 获取当前系统中所有NPU进程的PID
    def get_npu_pids(self):
        pids = set()

        try:
            out = subprocess.check_output(
                ["npu-smi", "info"],
                text=True,
                stderr=subprocess.DEVNULL
            )
        except Exception as e:
            self.logger.warning("npu-smi info failed: %s", e)
            return pids

        # 匹配示例：
        # | 0       0                 | 1218144       | msmodelslim              | 1442 |
        proc_line = re.compile(
            r'^\|\s*\d+\s+\d+\s+\|\s*(\d+)\s+\|',
        )
        for line in out.splitlines():
            m = proc_line.match(line)
            if m:
                try:
                    pids.add(int(m.group(1)))
                except ValueError:
                    pass
        return pids

    # 获取系统中sq task的PID
    def get_sq_pattern_pids(self):
        pids = set()
        try:
            out = subprocess.check_output(
                ["ps", "-eL", "-o", "pid,comm"],
                text=True
            )
            for line in out.splitlines()[1:]:
                pid, comm = line.split(None, 1)
                if self.dev_sq_pattern.match(comm):
                    pids.add(int(pid))
        except Exception as e:
            self.logger.warning("get_sq_pattern_pids failed: %s", e)
        return pids

    # 获取用户输入的PID/进程名/正则匹配的PID
    def resolve_user_input(self, user_input) -> set[int]:
        pids = set()
        if not user_input:
            return pids
        inputs = user_input if isinstance(user_input, list) else [user_input]
        for item in inputs:
            if isinstance(item, int):
                try:
                    # 仅检查是否存在
                    psutil.Process(item)
                    pids.add(item)
                except (psutil.NoSuchProcess, psutil.AccessDenied):
                    pass
            elif isinstance(item, str):
                if item.isdigit():
                    pid = int(item)
                    try:
                        psutil.Process(pid)
                        pids.add(pid)
                    except (psutil.NoSuchProcess, psutil.AccessDenied):
                        pass
                else:
                    # 使用 pgrep 匹配名称/正则
                    try:
                        out = subprocess.check_output(["pgrep", "-f", item], text=True)
                        for line in out.splitlines():
                            try:
                                pids.add(int(line.strip()))
                            except ValueError:
                                continue
                    except subprocess.CalledProcessError:
                        pass
        return pids

    # ---------------- 构建进程树 ----------------
    def build_pstree(self, extra_input=None, max_depth=20) -> List[KeyProcNode]:
        root_pids = set()
        root_pids |= self.get_npu_pids()
        root_pids |= self.get_sq_pattern_pids()
        root_pids |= self.resolve_user_input(extra_input)

        nodes = {}

        def add_node(pid, ppid=None, name=None):
            if pid in nodes:
                return nodes[pid]
            # 仅当 pid 是 int 才用 psutil
            if isinstance(pid, int):
                try:
                    proc = psutil.Process(pid)
                    info = {"pid": pid, "ppid": ppid if ppid is not None else proc.ppid(),
                            "name": name or proc.name()}
                except (psutil.NoSuchProcess, psutil.AccessDenied):
                    info = {"pid": pid, "ppid": ppid, "name": name or "unknown"}
            else:
                # pid 是线程字符串
                info = {"pid": pid, "ppid": ppid, "name": name}
            node = KeyProcNode(pid=info["pid"], ppid=info["ppid"], name=info["name"])
            nodes[pid] = node
            return node

        roots = []
        for root_pid in root_pids:
            root_node = add_node(root_pid)
            roots.append(root_node)

            stack = [(root_pid, 0)]
            while stack:
                pid, depth = stack.pop()
                if depth >= max_depth:
                    continue
                try:
                    proc = psutil.Process(pid)
                except (psutil.NoSuchProcess, psutil.AccessDenied):
                    continue

                # 子进程
                for child in proc.children():
                    child_node = add_node(child.pid)
                    if child_node not in nodes[pid].children:
                        nodes[pid].children.append(child_node)
                        stack.append((child.pid, depth + 1))

                # 子线程
                for th in proc.threads():
                    tid = th.id
                    thread_pid = f"{pid}:{tid}"  # 字符串虚拟 PID
                    if thread_pid not in nodes:
                        tnode = add_node(thread_pid, ppid=pid, name=f"{proc.name()}-thread")
                        nodes[pid].children.append(tnode)

        return roots

    # ---------------- 打印树 ----------------
    def print_tree(self, nodes: List[KeyProcNode], indent: str = ""):
        for i, node in enumerate(nodes):
            is_last = i == len(nodes) - 1
            prefix = indent + ("└─ " if is_last else "├─ ")
            print(prefix + node.label())
            new_indent = indent + ("   " if is_last else "│  ")
            if node.children:
                self.print_tree(node.children, indent=new_indent)

    # ---------------- 搜索节点 ----------------
    def search_tree(self, nodes: List[KeyProcNode], keyword: str) -> List[KeyProcNode]:
        """返回匹配节点及其子树"""
        result = []

        def match(node: KeyProcNode):
            if keyword in str(node.pid) or keyword.lower() in node.name.lower():
                return True
            return False

        def dfs(node: KeyProcNode):
            matched_children = []
            for child in node.children:
                c = dfs(child)
                if c:
                    matched_children.append(c)
            if match(node) or matched_children:
                return KeyProcNode(pid=node.pid, ppid=node.ppid, name=node.name, children=matched_children)
            return None

        for root in nodes:
            n = dfs(root)
            if n:
                result.append(n)
        return result

    # ---------------- 动态交互搜索 ----------------
    def interactive_search(self, roots: List[KeyProcNode]):
        print("输入关键字搜索进程树，输入空回车退出。")
        while True:
            keyword = input("搜索关键字: ").strip()
            if not keyword:
                print("退出搜索。")
                break
            results = self.search_tree(roots, keyword)
            if not results:
                print(f"未找到匹配 '{keyword}' 的进程。")
            else:
                print(f"匹配 '{keyword}' 的进程树:")
                self.print_tree(results)
                print("="*50)
