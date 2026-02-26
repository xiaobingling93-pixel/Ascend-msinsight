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
Suggestions for binding process / thread on cpu core
-------------------------------------------------------------------------
"""

from typing import Optional
from datetime import datetime
import inspect

class CpuBindingSuggestion:
   """
   生成 CPU 绑核建议说明（Markdown 格式）
   """

   @staticmethod
   def generate_markdown(
      numa_hint: Optional[str] = None,
      example_main_pid: Optional[int] = None,
      example_sq_pid: Optional[int] = None,
      example_acl_pid: Optional[int] = None,
   ) -> str:
      
      main_pid = example_main_pid or 12345
      sq_pid = example_sq_pid or 22345
      acl_pid = example_acl_pid or 32345
      now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

      # 使用 cleandoc 包装多行字符串
      md_content = inspect.cleandoc(f"""
         # CPU Binding Suggestion

         **生成时间**: {now}

         ---

         ## 一、总体设计原则

         模型运行时的 CPU 绑核目标：

         1. 保证主调度线程稳定
         2. 防止 sq_task 抢占
         3. 避免 NUMA 跨节点访问
         4. IO 与计算分离
         5. 降低上下文切换

         ---

         ## 二、关键线程绑核建议

         ### 1️⃣ 主调度线程（Main Thread）

         - 必须独占物理核
         - 禁止与 sq_task / DataLoader 共核
         - 推荐固定在单独核心
         - 推荐与 NPU 同 NUMA

         示例命令：
         `taskset -cp 0 {main_pid}`

         ---

         ### 2️⃣ sq_task / device queue 线程

         - 建议绑定连续核心
         - 不与主线程共核
         - 与对应 NPU NUMA 保持一致

         示例命令：
         `taskset -cp 1-7 {sq_pid}`

         ---

         ### 3️⃣ ACL / runtime 线程

         - 可绑定一小组核心
         - 避免与 IO 线程混合

         示例命令：
         `taskset -cp 8-11 {acl_pid}`

         ---

         ### 4️⃣ DataLoader / 预处理线程

         - 单独分配核心区间
         - 可按 worker 数量分配
         - 不影响前向主线程

         示例命令：
         `taskset -cp 12-19 <dataloader_pid>`

         ---

         ## 三、NUMA 绑定建议
         """)

      if numa_hint:
         md_content += f"\n{numa_hint}\n"
      else:
         md_content += inspect.cleandoc("""
               - 主线程与 sq_task 必须在同一 NUMA
               - 避免跨 NUMA 访问内存
               - 使用 numactl 控制内存绑定

               示例命令：
               `numactl --cpunodebind=0 --membind=0 python train.py`
               """)

      # 拼接后续部分
      footer = inspect.cleandoc(f"""
         ---

         ## 四、推荐核心分区示例（32 核机器）

         | 类型 | 核心范围 |
         | :--- | :--- |
         | 主线程 | 0 |
         | sq_task | 1-7 |
         | ACL | 8-11 |
         | DataLoader | 12-19 |
         | 系统线程 | 20-31 |

         ---

         ## 五、不绑核的风险

         1. 主线程被抢占 → step 抖动
         2. sq_task 抢核 → 队列堆积
         3. NUMA 远程访问 → 带宽下降
         4. IO 与计算混合 → Cache 污染
         5. 频繁上下文切换 → CPU 利用率虚高

         ---

         ## 六、建议优先级

         1. **主线程独占核**
         2. **sq_task 连续独立核**
         3. NUMA 一致性
         4. IO 与计算分离

         ---

         ## 七、最终建议总结

         - [x] 主线程独占物理核
         - [x] sq_task 连续分配
         - [x] ACL 归组
         - [x] DataLoader 隔离
         - [x] NUMA 内存绑定
         """)
      
      return md_content + "\n\n" + footer
