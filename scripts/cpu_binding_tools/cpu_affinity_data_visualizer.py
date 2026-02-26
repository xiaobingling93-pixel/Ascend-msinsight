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
Visualize the key process / thread cpu affinity data
-------------------------------------------------------------------------
"""
import os
import sys
import re
import logging
import traceback
import pandas as pd
import numpy as np
import plotly.graph_objects as go
import ipywidgets as widgets

from dataclasses import dataclass, field
from typing import Set, List, Dict, Any, Optional, Tuple
from IPython.display import display

if os.getcwd() not in sys.path:
    sys.path.append(os.getcwd())
from cpu_binding_utils.logger_utils import LoggerUtils

class WidgetLogger(logging.Handler):
    """将日志输出重定向到 Jupyter Widget 组件中显示。"""
    def __init__(self, output_widget):
        super().__init__()
        self.output = output_widget

    def emit(self, record):
        try:
            msg = self.format(record) 
            with self.output:
                print(f"\033[31;1m{msg}\033[0m" if record.levelno >= logging.WARNING else msg)
        except Exception:
            self.handleError(record)

logger = LoggerUtils.setup_logger("CpuAffinityVisualizer", level=logging.INFO)

@dataclass
class VisualConfig:
    """全局可视化配置：定义图表颜色、尺寸、布局和阈值参数。"""
    color_pass: str = '#1890ff'
    color_fail: str = '#ff4d4f'
    color_warn: str = '#faad14'
    color_numa: str = '#52c41a'
    color_exec: str = '#333333'

    color_bg: str = '#ffffff'
    color_grid: str = '#eeeeee'
    
    opacity_config: float = 1.0
    opacity_numa: float = 0.3
    
    heatmap_cols_per_row: int = 32
    heatmap_row_height_px: int = 40
    heatmap_min_height: int = 350
    heatmap_max_threads: int = 15
    min_display_cores: int = 32
    
    heatmap_margins: Dict[str, int] = field(
        default_factory=lambda: dict(l=180, r=50, t=60, b=20)
    )
    
    heatmap_colorscale: List[List[Any]] = field(
        default_factory=lambda: [
            [0.0, "#b6b6b6"], 
            [0.0001, "#2bff18"], 
            [0.5, "#eeff00"], 
            [1.0, '#f03b20']
        ]
    )

    row_height: int = 25
    affinity_min_height: int = 300
    affinity_left_margin: int = 300
    affinity_legend_gap: int = 120
    
    marker_size_pass: int = 7
    marker_size_fail: int = 12
    marker_size_warn: int = 10
    marker_symbol_pass: str = 'circle'
    marker_symbol_fail: str = 'x'
    marker_symbol_warn: str = 'triangle-up'
    
    bar_width_ratio_numa: float = 1.0
    bar_width_ratio_config: float = 0.6

    debounce_interval: float = 0.5
    
    filter_list_height: str = '30vh'
    checkbox_list_height: str = '25vh'

    max_display_limit: int = 1000

CONFIG = VisualConfig()

def create_error_figure(e: Exception, log_context: str) -> go.Figure:
    """生成包含错误堆栈信息的空白图表，用于在 UI 上提示渲染失败。"""
    logger.error(f"❌ {log_context}: {str(e)}")
    logger.debug(f"堆栈详情:\n{traceback.format_exc()}")

    err_fig = go.Figure()
    err_fig.add_annotation(
        text=f"⚠️ Visualization Failed:<br>{str(e)}", 
        xref="paper", yref="paper", 
        showarrow=False, 
        font=dict(color="red", size=16)
    )
    err_fig.update_layout(
        title="Rendering Error", 
        xaxis=dict(visible=False), 
        yaxis=dict(visible=False)
    )
    return err_fig

class LegendGuide(widgets.HTML):
    """显示图表图例说明的 HTML 组件。"""
    def __init__(self):
        tooltips = [
            ("Config (OK)", CONFIG.color_pass, "■", "正常绑定：线程被限定在推荐 NUMA 节点内，符合性能最佳实践。"),
            ("Config (Cross NUMA)", CONFIG.color_fail, "■", "跨 NUMA 警告：线程被绑在错误的 NUMA 节点内"),
            ("Status Unknown", CONFIG.color_warn, "▲", "状态未知：PSR 数据缺失或无效。"),
            ("NUMA Suggestion", CONFIG.color_numa, "■", "硬件拓扑建议：根据 NUMA 亲和性推荐的核心范围。"),
            ("Actual Execution", CONFIG.color_exec, "●", "实际运行点：采样时刻线程正在运行的 CPU 核。"),
            ("Error Execution", CONFIG.color_fail, "X", "错误实际运行点：线程没有运行在绑核范围内")
        ]
        
        items = "".join([
            f'<div title="{desc}" style="display:flex;align-items:center;cursor:help;padding:2px 6px;background:#fff;border:1px solid #eee;border-radius:3px;">'
            f'<span style="color:{col};font-size:18px;margin-right:6px;line-height:1;">{icon}</span>'
            f'<span style="font-size:13px;color:#333;font-weight:500;">{lbl}</span></div>' 
            for lbl, col, icon, desc in tooltips
        ])
        
        super().__init__(value=f'<div style="display:flex;flex-wrap:wrap;gap:15px;padding:8px;background:#f4f6f9;border-left:4px solid #1890ff;border-radius:4px;">'
                               f'<div style="font-weight:bold;color:#555;align-self:center;font-size:14px;">图例指南 <span style="font-size:10px;color:#999">(悬停查看)</span>:</div>{items}</div>')

class FilterGuide(widgets.HTML):
    """显示筛选功能简要说明的 HTML 组件。"""
    def __init__(self):
        content = """
        <div style="margin-bottom: 10px; padding: 8px; background: #e6f7ff; border: 1px solid #91d5ff; border-radius: 4px; font-size: 12px; line-height: 1.4;">
            <div style="font-weight: bold; color: #0050b3; margin-bottom: 5px; border-bottom: 1px dashed #91d5ff; padding-bottom: 3px;">💡 Filter Usage (筛选指南)</div>
            <ul style="padding-left: 16px; margin: 0; color: #555;">
                <li style="margin-bottom: 3px;"><b>Search:</b> 支持模糊搜索 NPU ID, PID, TID, Process Name 及 Thread Name。(如需清除搜索条件需清空搜索框后再次搜索)</li>
                <li><b>Select NPU:</b> 勾选列表可仅查看特定 NPU 的线程数据。</li>
            </ul>
        </div>
        """
        super().__init__(value=content)

class ScrollableCheckboxList(widgets.VBox):
    """带全选功能的滚动复选框列表组件。"""
    
    def __init__(self, options: List[str], height: str = CONFIG.checkbox_list_height, **kwargs: Any) -> None:
        self.checkbox_dict: Dict[str, widgets.Checkbox] = {}
        self.items: List[widgets.Checkbox] = []
        
        try:
            self.select_all_box = widgets.Checkbox(
                value=True, 
                description='(Select All)', 
                indent=False,
                layout=widgets.Layout(width='95%', margin='0px')
            )
            
            for opt in options:
                cb = widgets.Checkbox(
                    value=True, 
                    description=str(opt), 
                    indent=False,
                    layout=widgets.Layout(width='95%', margin='0px')
                )
                self.checkbox_dict[opt] = cb
                self.items.append(cb)
            
            self.select_all_box.observe(self._on_select_all, names='value')
            
            content_box = widgets.VBox(self.items, layout=widgets.Layout(width='98%'))
            divider = widgets.HTML('<hr style="margin: 4px 0; border:0; border-top:1px solid #ddd;">')
            
            super().__init__(
                children=[self.select_all_box, divider, content_box],
                layout=widgets.Layout(
                    height=height, 
                    overflow_y='scroll', 
                    border='1px solid #ccc', 
                    padding='5px', 
                    width='98%'
                )
            )
        except Exception as e:
            logger.warning(f"UI组件警告: 创建复选框列表失败 ({str(e)})")
        
    def _on_select_all(self, change: Dict[str, Any]) -> None:
        try:
            new_value = change['new']
            with self.hold_trait_notifications():
                for cb in self.items:
                    cb.value = new_value
        except Exception as e:
            logger.warning(f"Error: {e}")

    @property
    def value(self) -> List[str]:
        """返回当前选中的选项列表。"""
        return [opt for opt, cb in self.checkbox_dict.items() if cb.value]

    def observe_changes(self, callback: Any) -> None:
        """注册状态变更回调函数。"""
        try:
            self.select_all_box.observe(callback, names='value')
            for cb in self.items:
                cb.observe(callback, names='value')
        except Exception as e:
            logger.warning(f"Error: {e}")

class DataProcessor:
    """处理 CSV 数据的加载、清洗、解析和排序逻辑。"""
    CSV_COLUMNS = ['NPU_ID', 'NUMA', 'PID', 'PROCESS', 'TID', 'THREAD', 'PSR', 'CPU_AFFINITY']

    def __init__(self, filepath: str) -> None:
        self.filepath = filepath

    def validate_data_types(self, df: pd.DataFrame) -> None:
        """校验 DataFrame 的关键列存在性及数据类型。"""
        logger.info("正在进行数据类型校验...")
        
        missing = [col for col in self.CSV_COLUMNS if col not in df.columns]
        if missing:
            raise ValueError(f"数据结构错误：缺失以下关键列: {missing}。请检查 CSV 是否包含正确的列数。")

        for col in ['PID', 'TID']:
            numeric_check = pd.to_numeric(df[col].astype(str).replace('-', '0'), errors='coerce')
            if numeric_check.isna().any():
                bad_rows = df[numeric_check.isna()]
                example = bad_rows.iloc[0][col]
                raise TypeError(
                    f"列 '{col}' 数据类型错误：检测到非数字字符 (例如: '{example}')。\n"
                    "可能原因：CSV 文件包含了表头(Header)，但本工具默认按无表头读取。请删除第一行表头或检查数据格式。"
                )

        if df['CPU_AFFINITY'].isna().all():
             raise ValueError("列 'CPU_AFFINITY' 全部为空，无法进行分析。")

        logger.info("✅ 数据类型校验通过。")

    def process(self) -> pd.DataFrame:
        """执行完整的数据 ETL 流程并返回处理后的 DataFrame。"""
        logger.info(f"正在加载数据文件: {self.filepath}")

        if not os.path.exists(self.filepath):
            LoggerUtils.log_file_operation_error(logger, "加载数据", self.filepath, FileNotFoundError("File not found"))
            return pd.DataFrame()
            
        try:
            df = pd.read_csv(self.filepath, header=None, names=self.CSV_COLUMNS, dtype=str)
            logger.info(f"✅ CSV 读取成功，共 {len(df)} 行数据")
        except Exception as e:
            LoggerUtils.log_file_operation_error(logger, "读取/解析 CSV", self.filepath, e)
            return pd.DataFrame()

        if df.empty:
            logger.warning("⚠️ 警告: 数据文件为空。")
            return df

        try:
            self.validate_data_types(df)

            logger.info("正在清洗数据字段...")
            df['NPU'] = df['NPU_ID'].fillna('Unknown').astype(str).str.strip().replace(['-', '', 'nan', 'NaN'], 'Unknown')
            
            mask_proc = df['PROCESS'].isin(['', '-', 'nan', 'NaN', None])
            if mask_proc.any():
                logger.info(f"提示: 发现 {mask_proc.sum()} 行 PROCESS 为空，已使用 THREAD 字段自动填充。")
                  
            logger.info("正在清洗 PSR 数据...")
            df['PSR'] = pd.to_numeric(df['PSR'], errors='coerce')

            logger.info("正在解析 CPU 亲和性掩码...")
            df['Affinity_Set'] = df['CPU_AFFINITY'].apply(self._parse_affinity)
            df['Numa_Set'] = df['NUMA'].apply(self._parse_complex_numa)

            logger.info("正在计算绑定状态 (PASS/FAIL)...")
            df['Status'] = df.apply(self._check_psr_status, axis=1)
            df['Is_Cross_Numa'] = df.apply(self._check_numa_violation, axis=1)
            
            df['Label'] = (
                df['NPU'] + " | " + 
                df['PROCESS'] + " (PID:" + df['PID'] + ") | " + 
                df['THREAD'] + " (TID:" + df['TID'] + ")"
            )

            logger.info("正在建立搜索缓存索引...")
            df['Search_Index'] = (
                df['PROCESS'].fillna('').astype(str) + " " + 
                df['THREAD'].fillna('').astype(str) + " " + 
                df['NPU_ID'].fillna('').astype(str) + " " + 
                df['PID'].fillna('').astype(str) + " " + 
                df['TID'].fillna('').astype(str)
            ).str.lower()

            df['NPU_Sort_ID'] = df['NPU'].apply(self._extract_sort_key)
            sorted_df = df.sort_values(
                by=['NPU_Sort_ID', 'PID', 'TID'], 
                ascending=[True, True, True]
            )
            logger.info("✅ 数据预处理流程顺利完成。")
            return sorted_df

        except (ValueError, TypeError) as e:
            logger.error(f"❌ 数据校验失败: {str(e)}")
            return pd.DataFrame()
        except Exception as e:
            logger.error(f"❌ 数据处理过程中发生未知异常: {str(e)}")
            logger.debug(traceback.format_exc())
            return pd.DataFrame()

    @staticmethod
    def calculate_max_load(df: pd.DataFrame) -> int:
        """计算单核最大线程负载数，用于热力图归一化。"""
        if df.empty: return 1
        try:
            exploded = df.explode('Affinity_Set')
            counts = exploded['Affinity_Set'].value_counts()
            max_load = int(counts.max()) if not counts.empty else 1
            logger.info(f"统计信息: 单核最大负载线程数为 {max_load}")
            return max_load
        except Exception:
            return 1

    @staticmethod
    def format_set_to_range(s: Set[int]) -> str:
        """将整数集合格式化为字符串范围 (如 '0-3, 5')。"""
        try:
            if not s: return ""
            nums = sorted(list(s))
            ranges = []
            start = prev = nums[0]
            for x in nums[1:]:
                if x == prev + 1:
                    prev = x
                else:
                    ranges.append(f"{start}-{prev}" if start != prev else f"{start}")
                    start = prev = x
            ranges.append(f"{start}-{prev}" if start != prev else f"{start}")
            return ", ".join(ranges)
        except (TypeError, ValueError) as e:
            logger.debug(f"格式化集合异常 (输入: {s}): {e}")
            return str(s)
        except Exception as e:
            logger.debug(f"格式化集合发生未知异常: {e}")
            return str(s)

    def _check_psr_status(self, row: pd.Series) -> str:
        """判断 PSR 是否在亲和性集合内 (PASS/FAIL/WARN)。"""
        try:
            aff_set = row['Affinity_Set']
            if not aff_set: return 'PASS'
            psr_val = row['PSR']
            if pd.isna(psr_val): return 'WARN'
            return 'PASS' if int(psr_val) in aff_set else 'FAIL'
        except (ValueError, TypeError):
            return 'WARN'

    def _check_numa_violation(self, row: pd.Series) -> bool:
        """检查绑核范围是否超出 NUMA 建议范围。"""
        try:
            config_set = row['Affinity_Set']
            numa_set = row['Numa_Set']
            if not numa_set: return False 
            if not config_set: return True 
            return not config_set.issubset(numa_set)
        except Exception:
            return False

    @staticmethod
    def _extract_sort_key(val: str) -> float:
        """从字符串提取数字用于 NPU 排序。"""
        if val in ['Unknown', '-']: return float('inf')
        try:
            match = re.search(r'(\d+)', str(val))
            return float(match.group(1)) if match else float('inf')
        except Exception:
            return float('inf')

    @staticmethod
    def _parse_complex_numa(val: Any) -> Set[int]:
        """解析可能包含方括号的 NUMA 字符串。"""
        try:
            val_str = str(val).strip()
            if val_str in ['-', '', 'nan', 'None']: return frozenset()
            match = re.search(r'\[(.*?)\]', val_str)
            target = match.group(1) if match else val_str
            return DataProcessor._parse_affinity(target)
        except (TypeError, AttributeError) as e:
            logger.debug(f"解析 NUMA 节点异常 (输入: {val}): {e}")
            return frozenset()
        except Exception as e:
            logger.debug(f"解析 NUMA 节点发生未知异常: {e}")
            return frozenset()

    @staticmethod
    def _parse_affinity(x: Any) -> Set[int]:
        """将 CPU 范围解析为整数集合。"""
        if not x or pd.isna(x) or str(x).strip() in ['-', 'nan', 'None']:
            return frozenset()
        
        result = set()
        for part in str(x).split(','):
            part = part.strip()
            if not part: continue
            try:
                if '-' in part:
                    start, end = map(int, part.split('-'))
                    result.update(range(start, end + 1))
                else:
                    result.add(int(part))
            except (ValueError, TypeError) as e:
                logger.debug(f"解析掩码片段 '{part}' 失败: {e}")
        return frozenset(result)

class HeatmapBuilder:
    """构建 CPU 核心负载热力图。"""
    def __init__(self, df: pd.DataFrame, global_max: int, config: VisualConfig = CONFIG):
        self.df = df
        self.global_max = global_max
        self.config = config
        self.fig = go.Figure()

    def _prepare_matrix_data(self) -> Tuple[List[List[int]], List[List[str]], List[List[str]], List[str]]:
        process_df = self.df.copy()
        process_df['Hover_Label'] = (
            process_df['PROCESS'].astype(str) + "(" + process_df['PID'].astype(str) + ") | " +
            process_df['THREAD'].astype(str) + "(" + process_df['TID'].astype(str) + ")"
        )
        
        exploded = process_df.explode('Affinity_Set').dropna(subset=['Affinity_Set'])
        exploded['Core_ID'] = pd.to_numeric(exploded['Affinity_Set'], errors='coerce')
        exploded = exploded.dropna(subset=['Core_ID'])
        exploded['Core_ID'] = exploded['Core_ID'].astype(int)

        def format_thread_list(series):
            threads = [str(x) for x in series if pd.notna(x) and str(x).strip()]
            if len(threads) <= self.config.heatmap_max_threads:
                return "<br>• " + "<br>• ".join(threads)
            else:
                remain = len(threads) - self.config.heatmap_max_threads
                shown = threads[:self.config.heatmap_max_threads]
                return "<br>• " + "<br>• ".join(shown) + f"<br>... <i>({remain} more)</i>"

        core_stats = exploded.groupby('Core_ID').agg({
            'PID': 'count', 
            'Hover_Label': format_thread_list
        }).rename(columns={'PID': 'Load_Count', 'Hover_Label': 'Thread_List'})

        max_core = int(core_stats.index.max()) if not core_stats.empty else 0
        for nums in self.df['Numa_Set']:
            if nums and max(nums) > max_core:
                max_core = max(nums)
        if max_core < self.config.min_display_cores: max_core = self.config.min_display_cores

        cols_per_row = self.config.heatmap_cols_per_row
        z_data, text_data, custom_data, y_labels = [], [], [], []

        for start_id in range(0, max_core + 1, cols_per_row):
            end_id = min(start_id + cols_per_row, max_core + 1)
            row_core_ids = list(range(start_id, end_id))
            row_stats = core_stats.reindex(row_core_ids)
            
            row_z = row_stats['Load_Count'].fillna(0).astype(int).tolist()
            row_text = [str(i) for i in row_core_ids]
            row_custom = row_stats['Thread_List'].fillna("").tolist()

            pad_len = cols_per_row - len(row_z)
            if pad_len > 0:
                row_z.extend([None] * pad_len)
                row_text.extend([""] * pad_len)
                row_custom.extend([""] * pad_len)

            z_data.append(row_z)
            text_data.append(row_text)
            custom_data.append(row_custom)
            y_labels.append(f"Cores [{start_id}-{start_id + cols_per_row - 1}]")

        z_data.reverse()
        text_data.reverse()
        custom_data.reverse()
        y_labels.reverse()
        
        return z_data, text_data, custom_data, y_labels

    def build(self) -> go.Figure:
        """渲染并返回热力图 Figure 对象。"""
        logger.info(f"正在构建热力图 (数据量: {len(self.df)} 行)...")
        try:
            if self.df.empty:
                logger.warning("热力图数据为空，生成空白占位图。")
                self.fig.update_layout(title="No Data Available for Heatmap")
                return self.fig

            z_data, text_data, custom_data, y_labels = self._prepare_matrix_data()

            height = max(len(y_labels) * self.config.heatmap_row_height_px + 80, self.config.heatmap_min_height)
            cols_per_row = self.config.heatmap_cols_per_row
            
            self.fig.add_trace(go.Heatmap(
                z=z_data, x=list(range(cols_per_row)), text=text_data, customdata=custom_data,
                texttemplate="%{text}", textfont={"size": 11, "color": "black"},
                hovertemplate="Core: <b>%{text}</b><br>Load: <b>%{z}</b><br><b>Threads:</b>%{customdata}<extra></extra>",
                colorscale=self.config.heatmap_colorscale, zmin=0, zmax=self.global_max, 
                showscale=True, xgap=1, ygap=1
            ))

            self.fig.update_layout(
                title="Global CPU Load Heatmap", height=height,
                xaxis=dict(showticklabels=False, side='top', fixedrange=True),
                yaxis=dict(tickmode='linear', fixedrange=True),
                margin=self.config.heatmap_margins, plot_bgcolor=self.config.color_bg
            )
            return self.fig
            
        except Exception as e:
            return create_error_figure(e, "热力图渲染逻辑崩溃")

class AffinityChartBuilder:
    """构建线程绑核甘特图与实际运行点散点图。"""
    def __init__(self, df: pd.DataFrame, config: VisualConfig = CONFIG):
        self.df = df
        self.config = config
        self.fig = go.Figure()

    def build(self) -> go.Figure:
        """渲染并返回亲和性图表 Figure 对象。"""
        logger.info(f"正在构建亲和性图表 (数据量: {len(self.df)} 行)...")
        try:
            if self.df.empty: 
                logger.warning("亲和性图表数据为空，跳过绘制。")
                return self.fig.update_layout(title="No Data")

            self._add_bar('Numa_Set', 'NUMA Suggestion', self.config.color_numa, self.config.opacity_numa, self.config.bar_width_ratio_numa)
            
            pass_df = self.df[~self.df['Is_Cross_Numa']]
            if not pass_df.empty:
                self._add_bar('Affinity_Set', 'Config (OK)', self.config.color_pass, self.config.opacity_config, self.config.bar_width_ratio_config, pass_df)
                
            fail_df = self.df[self.df['Is_Cross_Numa']]
            if not fail_df.empty:
                self._add_bar('Affinity_Set', 'Config (Cross NUMA)', self.config.color_fail, self.config.opacity_config, self.config.bar_width_ratio_config, fail_df)

            self._add_markers()
            self._layout()
            return self.fig
        except Exception as e:
            return create_error_figure(e, "亲和性图表渲染逻辑崩溃")

    def _add_bar(self, col: str, name: str, color: str, opacity: float, width_adj: float, df: pd.DataFrame = None) -> None:
        try:
            data = self.df if df is None else df
            ranges = data[col].apply(lambda s: (min(s), max(s)) if s and len(s) > 0 else (None, None))            
            starts = ranges.apply(lambda x: x[0])
            ends = ranges.apply(lambda x: x[1])
            
            mask = (ends != 0) | (starts != 0)
            if not mask.any(): return

            widths = np.zeros(len(data))
            bases = np.zeros(len(data))
            
            widths[mask] = (ends[mask] - starts[mask]) + width_adj
            bases[mask] = starts[mask] - (width_adj / 2)

            self.fig.add_trace(go.Bar(
                x=widths, y=data['Label'], base=bases, orientation='h',
                marker=dict(color=color, opacity=opacity, line_width=0),
                hoverinfo='skip', showlegend=False, name=name
            ))
        except Exception as e:
            logger.warning(f"  -> 绘制条形图层 '{name}' 时遇到部分数据问题，已跳过该层: {e}")

    def _add_markers(self) -> None:
        try:
            status = self.df['Status']
            
            condlist = [status == 'FAIL', status == 'WARN']
            color_choices = [self.config.color_fail, self.config.color_warn]
            symbol_choices = [self.config.marker_symbol_fail, self.config.marker_symbol_warn]
            size_choices = [self.config.marker_size_fail, self.config.marker_size_warn]
            
            colors = np.select(condlist, color_choices, default=self.config.color_exec)
            symbols = np.select(condlist, symbol_choices, default=self.config.marker_symbol_pass)
            sizes = np.select(condlist, size_choices, default=self.config.marker_size_pass)

            hover_df = self.df[['Label', 'Status', 'PSR', 'PID', 'TID', 'NPU']].copy()
            hover_df['Config'] = self.df['Affinity_Set'].apply(DataProcessor.format_set_to_range)
            hover_df['Numa'] = self.df['Numa_Set'].apply(DataProcessor.format_set_to_range)

            self.fig.add_trace(go.Scatter(
                x=self.df['PSR'], y=self.df['Label'], mode='markers',
                marker=dict(size=sizes, color=colors, symbol=symbols, line=dict(width=1, color='#ffffff')),
                customdata=hover_df,
                hovertemplate=(
                    "<b>%{customdata[0]}</b><br>"
                    "Status: %{customdata[1]}<br>"
                    "NPU: %{customdata[5]}<br>"
                    "PID: <b>%{customdata[3]}</b> <br>"
                    "TID: <b>%{customdata[4]}</b><br>"
                    "Run Core: <b>%{customdata[2]}</b><br>"
                    "Config: %{customdata[6]}<br>"
                    "NUMA Ref: %{customdata[7]}<extra></extra>"
                ),
                showlegend=False, name='Actual Execution'
            ))
        except Exception as e:
             logger.warning(f"  -> 绘制标记层时遇到问题: {e}")

    def _layout(self) -> None:
        labels = self.df['Label'].unique().tolist()
        n = len(labels)

        height = max(n * self.config.row_height + 100, self.config.affinity_min_height)

        self.fig.update_layout(title="Thread Affinity Analysis",
            height=height,
            margin=dict(t=50, b=20, r=20, l=self.config.affinity_left_margin),
            barmode='overlay',
            plot_bgcolor=self.config.color_bg,
            xaxis=dict(title="CPU Core ID",side='top',dtick=16,showgrid=True,showspikes=True,spikemode='across'),
            yaxis=dict(type='category',categoryorder='array',categoryarray=labels,autorange=False,range=[n - 0.5, -0.5],showgrid=True,gridcolor=self.config.color_grid,),
        )


@dataclass
class FilterCriteria:
    """筛选条件数据类。"""
    search_text: str
    selected_npus: Optional[Set[str]] = None

class NotebookApp:
    """Jupyter Notebook 交互式可视化应用的主入口。"""
    def __init__(self, filepath: str) -> None:
        # 初始化 UI 组件
        # 使用 vh 单位实现响应式高度
        self.log_output = widgets.Output(layout={'height': '20vh', 'overflow_y': 'scroll', 'border': 'none', 'padding': '5px'})
        self.log_accordion = widgets.Accordion(children=[self.log_output])
        self.log_accordion.set_title(0, '📋 运行日志 (点击折叠/展开)')
        self.log_accordion.selected_index = 0
        
        self.legend_guide = LegendGuide()
        self.filter_guide = FilterGuide() 

        ui_handler = WidgetLogger(self.log_output)
        
        if logger.handlers:
            original_handler = logger.handlers[0]
            
            if original_handler.formatter:
                ui_handler.setFormatter(original_handler.formatter)
                
            logger.handlers.clear()
            
        logger.addHandler(ui_handler)
        
        logger.info("正在初始化应用程序...")
        self.processor = DataProcessor(filepath)
        
        self.full_df = self.processor.process()
        
        if self.full_df.empty:
            logger.error("❌ 初始化中断: 数据加载失败。")
            self.global_max_load = 1
        else:
            logger.info("数据加载成功，正在计算全局负载参数...")
            self.global_max_load = self.processor.calculate_max_load(self.full_df)

        self.heatmap_output = widgets.Output(layout=widgets.Layout(width='99%'))
        self.affinity_output = widgets.Output(layout=widgets.Layout(width='99%'))
        
        self.all_npus = []
        try:
            if not self.full_df.empty and 'NPU' in self.full_df.columns:
                unique_npus = self.full_df['NPU'].dropna().unique()
                self.all_npus = sorted([str(x) for x in unique_npus], key=DataProcessor._extract_sort_key)
        except Exception as e:
            logger.warning(f"Error: {e}")

        self._init_ui()
        
        if not self.full_df.empty:
            logger.info("UI 组件初始化完成，首次渲染视图...")
            self.refresh_view()
        else:
            with self.heatmap_output:
                print("等待有效数据...")

    def _init_ui(self) -> None:
        self.search_box = widgets.Text(
            placeholder='Search NPU/PID/Thread...', 
            layout=widgets.Layout(width='auto', flex='1 1 auto')
        )
        self.search_box.on_submit(lambda widget: self.refresh_view())
        self.search_btn = widgets.Button(icon='search', layout=widgets.Layout(width='40px'))
        self.search_btn.on_click(lambda b: self.refresh_view())
        
        self.npu_selector = ScrollableCheckboxList(options=self.all_npus, height=CONFIG.filter_list_height)
        self.npu_selector.observe_changes(self._on_npu_change)

    def _on_search(self, change):
        if not self.full_df.empty:
            self.refresh_view()
            
    def _on_npu_change(self, change):
        if not self.full_df.empty:
            self.refresh_view()

    def _get_filtered_df(self, criteria: FilterCriteria) -> pd.DataFrame:
        df = self.full_df 
        
        if df.empty: return df

        if criteria.search_text:
            txt = criteria.search_text.lower()
            df = df[df['Search_Index'].str.contains(txt, na=False)]
        
        if criteria.selected_npus is not None:
            if 'NPU' in df.columns:
                df = df[df['NPU'].isin(criteria.selected_npus)]
            elif 'NPU_ID' in df.columns:
                df = df[df['NPU_ID'].astype(str).isin(criteria.selected_npus)]
                
        return df

    def refresh_view(self) -> None:
        """根据 UI 筛选条件刷新热力图和亲和性图表。"""
        try:
            sel_npus = set(self.npu_selector.value)
            selected_npus_param = None if len(sel_npus) >= len(self.all_npus) else sel_npus
            criteria = FilterCriteria(search_text=self.search_box.value, selected_npus=selected_npus_param)
            
            logger.info(f"正在刷新视图...")
            df = self._get_filtered_df(criteria)
            
            # 截断逻辑
            total_rows = len(df)
            is_truncated = total_rows > CONFIG.max_display_limit
            
            if is_truncated:
                render_df = df.iloc[:CONFIG.max_display_limit].copy()
                logger.warning(f"数据过大 ({total_rows})，已截断显示前 {CONFIG.max_display_limit} 条")
            else:
                render_df = df

            self.heatmap_output.clear_output(wait=True)
            self.affinity_output.clear_output(wait=True)
            
            with self.heatmap_output:
                viz = HeatmapBuilder(df, self.global_max_load) 
                display(viz.build())
                
            with self.affinity_output:
                if is_truncated:
                    warning_html = f"""
                    <div style="background-color: #fff1f0; border: 1px solid #ffa39e; padding: 8px 12px; margin-bottom: 5px; border-radius: 4px; color: #cf1322; font-size: 13px;">
                        <b>⚠️ 数据量过大 ({total_rows} 条)，已截断显示前 {CONFIG.max_display_limit} 条。</b>
                        <span style="opacity: 0.8">图表可能未显示所有线程，请使用左侧 <b>Search</b> 或 <b>Select NPU</b> 缩小范围。</span>
                    </div>
                    """
                    display(widgets.HTML(warning_html))
                
                # 渲染甘特图
                viz = AffinityChartBuilder(render_df)
                display(viz.build())
                
            logger.info("✅ 视图刷新完毕。")
            
        except Exception as e:
            logger.error(f"❌ 视图刷新出错: {str(e)}")
            logger.debug(traceback.format_exc())

    def render(self) -> widgets.VBox:
        """组装并返回最终的 UI 布局。"""
        sidebar = widgets.VBox([
            widgets.HTML("<h3>Filters</h3>"),
            self.filter_guide, 
            widgets.HTML("<label>Search:</label>"),
            widgets.HBox([self.search_box, self.search_btn]),
            widgets.HTML("<br><label>Select NPU:</label>"),
            self.npu_selector
        ], layout=widgets.Layout(width='300px', padding='10px', border='1px solid #ddd'))

        main_area = widgets.VBox([
            self.legend_guide, 
            # 使用 vh 单位实现响应式高度
            widgets.VBox([self.affinity_output], layout=widgets.Layout(height='70vh', overflow_y='scroll', border='1px solid #eee')),
            widgets.HTML("<hr>"),
            self.heatmap_output
        ], layout=widgets.Layout(flex='1'))

        content = widgets.HBox([main_area, sidebar], layout=widgets.Layout(width='100%'))
        
        final_layout = widgets.VBox([
            content,
            widgets.HTML("<hr style='border-top: 2px dashed #bbb; margin: 10px 0;'>"),
            self.log_accordion
        ])

        return final_layout

def run_notebook_app(filepath: str) -> None:
    """启动 Notebook 可视化应用。"""
    if os.path.exists(filepath):
        app = NotebookApp(filepath)
        display(app.render())
    else:
        LoggerUtils.log_file_operation_error(logger, "启动应用", filepath, FileNotFoundError("输入文件不存在"))

if __name__ == "__main__":
        print("⚠️ 注意：此工具依赖 ipywidgets，专为 Jupyter Notebook 设计。\n请在 Jupyter Lab/Notebook 中打开此脚本以查看交互式图表。")
        print("使用方式:\nfrom cpu_affinity_data_visualizer import run_notebook_app\nrun_notebook_app(\"result.csv\")")