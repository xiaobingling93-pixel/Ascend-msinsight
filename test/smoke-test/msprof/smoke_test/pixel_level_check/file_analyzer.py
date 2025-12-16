#!/usr/bin/python3
# -*- coding: utf-8 -*-
# Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.

from decimal import Decimal
import logging
import os
import sys
from typing import Dict
from typing import List
from common.file_manager import FileManager
from common.json_trace import TraceEvent
from common.structure import *
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from check_tools.db_check import DBManager, EmptyClass

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


class JsonAnalyzer:
    """
    分析json
    """

    def __init__(self, msprof_json_path=""):
        self.msprof_json_path = msprof_json_path
        # 下面是meta_data_events
        self.process_name_events: Dict[str, TraceEvent] = dict()
        self.hccl_group: Dict[str, TraceEvent] = dict()
        self.hccl_plane: Dict[str, TraceEvent] = dict()
        # 下面是duration_events
        self.python_dur_events: List[TraceEvent] = []
        self.cann_dur_events: List[TraceEvent] = []
        self.ascend_hardware_dur_events: List[TraceEvent] = []
        self.hccl_dur_events: List[TraceEvent] = []
        # 下面是flow_events
        self.flow_start: Dict[int, int] = dict()
        # 下面是counter_events
        self.freq_counter_event: List[TraceEvent] = []
        self.npu_mem_counter_event: List[TraceEvent] = []
        self.ddr_counter_event: List[TraceEvent] = []
        self.acc_pmu_counter_event: List[TraceEvent] = []
        self.hccs_counter_event: List[TraceEvent] = []
        self.nic_counter_event: List[TraceEvent] = []
        self.roce_counter_event: List[TraceEvent] = []
        self.pcie_counter_event: List[TraceEvent] = []
        self.hbm_counter_event: List[TraceEvent] = []
        self.stars_soc_info_counter_event: List[TraceEvent] = []
        self.llc_counter_event: List[TraceEvent] = []
        # 用于和统一db对比的dict
        self.ascend_task_data_obj: Dict[int, AscendTaskData] = dict()
        self.communication_op_obj: Dict[int, CommunicationOp] = dict()
        self.communication_task_info_obj: Dict[int, CommunicationTaskInfo] = dict()
        self.aicore_freq_obj: Dict[int, AicoreFreq] = dict()
        self.npu_mem_obj: Dict[str, NpuMem] = dict()
        self.ddr_obj: Dict[str, DDR] = dict()
        self.acc_pmu_obj: Dict[str, AccPmu] = dict()
        self.hccs_obj: Dict[int, Hccs] = dict()
        self.nic_obj: Dict[int, SysIO] = dict()
        self.roce_obj: Dict[int, SysIO] = dict()
        self.pcie_obj: Dict[int, Pcie] = dict()
        self.hbm_obj: Dict[str, HBM] = dict()
        self.stars_soc_info_obj: Dict[int, StarsSocInfo] = dict()
        self.llc_obj: Dict[str, LLc] = dict()
        self.wired_obj: Dict[int, Wired] = dict()
        self.api_obj: Dict[int, API] = dict()

    @staticmethod
    def check_duration_event(trace: Dict):
        """
        这一类的特点是多了ts dur, ph="X"，可能有cat args
        """
        return trace.get("ph") == "X" and trace.get("ts") is not None and trace.get("dur") is not None

    @staticmethod
    def check_counter_event(trace: Dict):
        """
        这一类的特点是多了ts, ph为"C", 一定有args
        """
        return trace.get("ph") == "C" and trace.get("ts") is not None and trace.get("args") is not None

    @staticmethod
    def check_flow_start_event(trace: Dict):
        """
        这一类的特点是多了ts, bp cat 连线起点ph为"s", 连线重点ph为"f", bp为"e",cat一般是MsTx和HostToDevice
        有个特殊的Data_aug Bound的，需要排除
        """
        return trace.get("ph") == "s" and trace.get("ts") is not None and "Data_aug Bound" not in trace.get("name")

    @staticmethod
    def check_flow_end_event(trace: Dict):
        """
        这一类的特点是多了ts, bp cat 连线起点ph为"s", 连线重点ph为"f", bp为"e",cat一般是MsTx和HostToDevice
        """
        return trace.get("ph") == "f" and trace.get("bp") == "e" and trace.get("ts") is not None

    def generate_data_instance(self):
        """
        生成可以与统一db进行对比的数据结构
        """
        self._generate_ascend_task_data()
        self._generate_communication_op_and_task_info()
        self._generate_aicore_freq()
        self._generate_npu_mem()
        self._generate_ddr()
        self._generate_acc_pmu()
        self._generate_hccs()
        self._generate_nic()
        self._generate_roce()
        self._generate_pcie()
        self._generate_hbm()
        self._generate_stars_soc_info()
        self._generate_llc()
        self._generate_qos()
        self._generate_cann_api()

    def load_json(self):
        traces = FileManager.read_json_file(self.msprof_json_path)
        if not traces:
            return
        self._load_meta_data_events(traces)
        self._load_flow_events(traces)
        self._load_duration_events(traces)
        self._load_counter_events(traces)
        self._generate_wired(traces)

    def _load_meta_data_events(self, traces: List[Dict]):
        """
        多了args，且ph="M"
        不同于thread_name, process_name的pid tid一定是唯一的，不会出现args_name相同，pid tid不同的情况
        为了后面区分不同层的数据做准备
        """
        for trace in traces:
            if not trace.get('args') or not trace.get('args').get('name'):
                continue
            args_name = trace.get('args').get('name')
            name = trace.get('name')
            if name == 'process_name':
                self.process_name_events[args_name] = TraceEvent(**trace)
            elif name == 'thread_name':
                key = str(trace.get("pid", "")) + "-" + str(trace.get("tid", ""))
                if "Group" in args_name:
                    self.hccl_group[key] = TraceEvent(**trace)
                elif "Plane" in args_name:
                    self.hccl_plane[key] = TraceEvent(**trace)

    def _load_flow_events(self, traces):
        for trace in traces:
            if self.check_flow_start_event(trace):
                cat = trace.get("cat")
                ts = int(Decimal(trace.get("ts")) * 1000)
                if cat is None:
                    logging.error("这种连线类型的起点的cat信息缺失，请补充！例如HostToDevice和MsTx")
                    continue
                if cat == "MsTx":
                    connection_id = int(trace.get("id"))
                elif cat == "HostToDevice":
                    # 这种类型的需要右移32位，因为现在的connection_id是左移32位之后得到的
                    connection_id = int(trace.get("id")) >> 32
                else:
                    logging.error(f"{ts}出现了新的连线的cat信息，请确认：{cat}")
                    continue
                if connection_id in self.flow_start and ts != self.flow_start[connection_id]:
                    logging.error(f"重复的connection:{connection_id}")
                    continue
                self.flow_start[connection_id] = ts

    def _load_duration_events(self, traces: List[Dict]):
        python_name_event = self.process_name_events.get("Python")
        cann_name_event = self.process_name_events.get("CANN")
        ascend_hardware_name_event = self.process_name_events.get("Ascend Hardware")
        hccl_name_event = self.process_name_events.get("Communication")
        for trace in traces:
            if not self.check_duration_event(trace):
                continue
            event = TraceEvent(**trace)
            pid = trace.get('pid')
            if python_name_event is not None and pid == python_name_event.pid:
                self.python_dur_events.append(event)
            elif cann_name_event is not None and pid == cann_name_event.pid:
                self.cann_dur_events.append(event)
            elif ascend_hardware_name_event is not None and pid == ascend_hardware_name_event.pid:
                if event.args.get("Subtask Id") is None:  # TODO 通信轮次
                    continue
                self.ascend_hardware_dur_events.append(event)
            elif hccl_name_event is not None and pid == hccl_name_event.pid:
                self.hccl_dur_events.append(event)

    def _load_counter_events(self, traces: List[Dict]):
        """
        qos sio stars_chip_trans以及 cpu memory disk network usage统一db里面没有
        """
        freq_name_event = self.process_name_events.get("AI Core Freq")
        npu_mem_name_event = self.process_name_events.get("NPU MEM")
        ddr_name_event = self.process_name_events.get("DDR")
        acc_pmu_name_event = self.process_name_events.get("Acc PMU")
        hccs_name_event = self.process_name_events.get("HCCS")
        nic_name_event = self.process_name_events.get("NIC")
        roce_name_event = self.process_name_events.get("RoCE")
        pcie_name_event = self.process_name_events.get("PCIe")
        hbm_name_event = self.process_name_events.get("HBM")
        stars_soc_info_name_event = self.process_name_events.get("Stars Soc Info")
        llc_name_event = self.process_name_events.get("LLC")
        for trace in traces:
            if not self.check_counter_event(trace):
                continue
            event = TraceEvent(**trace)
            pid = trace.get('pid')
            if freq_name_event is not None and pid == freq_name_event.pid:
                self.freq_counter_event.append(event)
            elif npu_mem_name_event is not None and pid == npu_mem_name_event.pid:
                self.npu_mem_counter_event.append(event)
            elif ddr_name_event is not None and pid == ddr_name_event.pid:
                self.ddr_counter_event.append(event)
            elif acc_pmu_name_event is not None and pid == acc_pmu_name_event.pid:
                self.acc_pmu_counter_event.append(event)
            elif hccs_name_event is not None and pid == hccs_name_event.pid:
                self.hccs_counter_event.append(event)
            elif nic_name_event is not None and pid == nic_name_event.pid:
                self.nic_counter_event.append(event)
            elif roce_name_event is not None and pid == roce_name_event.pid:
                self.roce_counter_event.append(event)
            elif pcie_name_event is not None and pid == pcie_name_event.pid:
                self.pcie_counter_event.append(event)
            elif hbm_name_event is not None and pid == hbm_name_event.pid:
                self.hbm_counter_event.append(event)
            elif stars_soc_info_name_event is not None and pid == stars_soc_info_name_event.pid:
                self.stars_soc_info_counter_event.append(event)
            elif llc_name_event is not None and pid == llc_name_event.pid:
                self.llc_counter_event.append(event)

    def _generate_ascend_task_data(self):
        for event in self.ascend_hardware_dur_events:
            start = int(Decimal(event.ts) * 1000)  # json里面的都是us
            end = start + int(Decimal(str(event.dur)) * 1000)
            self.ascend_task_data_obj[start] = AscendTaskData(start, end, event.args.get("Model Id"),
                                                              event.args.get("Physic Stream Id"),
                                                              event.args.get("Task Id"),
                                                              event.args.get("Subtask Id"),
                                                              event.args.get("Task Type"))

    def _generate_communication_op_and_task_info(self):
        """
        生成CommunicationOp和CommunicationTaskInfo
        """
        group_tids = [event.tid for event in self.hccl_group.values()]
        group_tids.sort()
        # 在self.hccl_plane的args参数里面加上这个plane所属group的tid
        for _, event in self.hccl_plane.items():
            for index, group_tid in enumerate(group_tids):
                if event.tid > group_tid:
                    event.args["group_tid"] = group_tid
                    break
        for event in self.hccl_dur_events:
            start = int(Decimal(event.ts) * 1000)
            end = start + int(Decimal(str(event.dur)) * 1000)
            key = str(event.pid) + "-" + str(event.tid)
            if event.args.get("alg_type") is not None:
                group_name = self.hccl_group.get(key, TraceEvent()).args.get("name")
                relay = 1 if event.args.get("relay") == "yes" else 0
                retry = 1 if event.args.get("retry") == "yes" else 0
                self.communication_op_obj[start] = CommunicationOp(event.name, start, end, group_name,
                                                                   event.args.get("data_type"),
                                                                   event.args.get("alg_type"),
                                                                   event.args.get("count"),
                                                                   relay,
                                                                   retry)
            else:
                args_name = self.hccl_plane.get(key, TraceEvent()).args.get("name")
                if args_name is None:
                    logging.error(f"The small communication operator does not belong to any plane.ts is {event.ts},"
                                  f"name is {event.name}, key is {key}")
                    continue
                plane_id = int(args_name.split()[-1])
                group_tid = self.hccl_plane.get(key, TraceEvent()).args.get("group_tid")
                if group_tid is None:
                    logging.error("The group_tid in args of the plane cannot be found.")
                    continue
                tmp_key = str(event.pid) + "-" + str(group_tid)
                group_name = self.hccl_group.get(tmp_key, TraceEvent()).args.get("name")
                self.communication_task_info_obj[start] = CommunicationTaskInfo(start, end, event.name, group_name,
                                                                                plane_id, event.args.get("notify_id"),
                                                                                event.args.get("src rank"),
                                                                                event.args.get("dst rank"),
                                                                                event.args.get("transport type"),
                                                                                event.args.get("data type"),
                                                                                event.args.get("link type"),
                                                                                event.args.get("size(Byte)"))

    def _generate_aicore_freq(self):
        for event in self.freq_counter_event:
            ts = int(Decimal(event.ts) * 1000)
            self.aicore_freq_obj[ts] = AicoreFreq(ts, int(event.args.get("MHz")))

    def _generate_npu_mem(self):
        for event in self.npu_mem_counter_event:
            ts = int(Decimal(event.ts) * 1000)
            tmp_list = event.name.split("/")
            if len(tmp_list) != 2:
                logging.error(f"The name of this NPU MEM data is invalid, is {event.name}")
            data_type = tmp_list[0]
            key = str(ts) + "-" + data_type
            size = int(event.args.get("KB") * 1024)
            if key not in self.npu_mem_obj:
                if tmp_list[1] == "DDR":
                    self.npu_mem_obj[key] = NpuMem(ts, ddr=size)
                elif tmp_list[1] == "HBM":
                    self.npu_mem_obj[key] = NpuMem(ts, hbm=size)
                elif tmp_list[1] == "Memory":
                    continue
                else:
                    logging.error(f"The name of this NPU MEM data is invalid, is {event.name}")
            else:
                if tmp_list[1] == "DDR":
                    self.npu_mem_obj[key].ddr = size
                elif tmp_list[1] == "HBM":
                    self.npu_mem_obj[key].hbm = size
                elif tmp_list[1] == "Memory":
                    continue
                else:
                    logging.error(f"The name of this NPU MEM data is invalid, is {event.name}")

    def _generate_ddr(self):
        pass

    def _generate_acc_pmu(self):
        for event in self.acc_pmu_counter_event:
            ts = int(Decimal(event.ts) * 1000)
            acc_id = event.args.get("acc_id")
            key = str(ts) + "-" + str(acc_id)
            value = event.args.get("value")
            name = event.name
            if key not in self.acc_pmu_obj:
                if name == "read_bandwidth":
                    self.acc_pmu_obj[key] = AccPmu(ts, acc_id, read_bw_level=value)
                elif name == "write_bandwidth":
                    self.acc_pmu_obj[key] = AccPmu(ts, acc_id, write_bw_level=value)
                elif name == "read_ost":
                    self.acc_pmu_obj[key] = AccPmu(ts, acc_id, read_ost_level=value)
                elif name == "write_ost":
                    self.acc_pmu_obj[key] = AccPmu(ts, acc_id, write_ost_level=value)
                else:
                    logging.error(f"The name of this Acc PMU data is invalid, is {name}")
            else:
                if name == "read_bandwidth":
                    self.acc_pmu_obj[key].read_bw_level = value
                elif name == "write_bandwidth":
                    self.acc_pmu_obj[key].write_bw_level = value
                elif name == "read_ost":
                    self.acc_pmu_obj[key].read_ost_level = value
                elif name == "write_ost":
                    self.acc_pmu_obj[key].write_ost_level = value

    def _generate_hccs(self):
        for event in self.hccs_counter_event:
            ts = int(Decimal(event.ts) * 1000)
            if event.name == "Tx":
                bw = int(event.args.get("Tx(MB/s)") * 1024 * 1024)
                if ts not in self.hccs_obj:
                    self.hccs_obj[ts] = Hccs(ts, tx=bw)
                else:
                    self.hccs_obj[ts].tx = bw
            elif event.name == "Rx":
                bw = int(event.args.get("Rx(MB/s)") * 1024 * 1024)
                if ts not in self.hccs_obj:
                    self.hccs_obj[ts] = Hccs(ts, rx=bw)
                else:
                    self.hccs_obj[ts].rx = bw

    def _generate_nic(self):
        for event in self.nic_counter_event:
            ts = int(Decimal(event.ts) * 1000)
            if event.name == "Port 0/Tx":
                tx_bandwidth_efficiency = event.args.get("Tx Bandwidth Efficiency")
                tx_packets = event.args.get("Tx Packets")
                tx_error_rate = event.args.get("Tx Error Rate")
                tx_dropped_rate = event.args.get("Tx Dropped Rate")
                if ts not in self.nic_obj:
                    self.nic_obj[ts] = SysIO(ts, tx_dropped_rate=tx_dropped_rate, tx_packets=tx_packets,
                                             tx_error_rate=tx_error_rate,
                                             tx_bandwidth_efficiency=tx_bandwidth_efficiency)
                else:
                    self.nic_obj[ts].tx_dropped_rate = tx_dropped_rate
                    self.nic_obj[ts].tx_packets = tx_packets
                    self.nic_obj[ts].tx_error_rate = tx_error_rate
                    self.nic_obj[ts].tx_bandwidth_efficiency = tx_bandwidth_efficiency
            elif event.name == "Port 0/Rx":
                rx_bandwidth_efficiency = event.args.get("Rx Bandwidth Efficiency")
                rx_packets = event.args.get("Rx Packets")
                rx_error_rate = event.args.get("Rx Error Rate")
                rx_dropped_rate = event.args.get("Rx Dropped Rate")
                if ts not in self.nic_obj:
                    self.nic_obj[ts] = SysIO(ts, rx_dropped_rate=rx_dropped_rate, rx_packets=rx_packets,
                                             rx_error_rate=rx_error_rate,
                                             rx_bandwidth_efficiency=rx_bandwidth_efficiency)
                else:
                    self.nic_obj[ts].rx_dropped_rate = rx_dropped_rate
                    self.nic_obj[ts].rx_packets = rx_packets
                    self.nic_obj[ts].rx_error_rate = rx_error_rate
                    self.nic_obj[ts].rx_bandwidth_efficiency = rx_bandwidth_efficiency
            else:
                logging.error(f"The name of this Nic data is invalid, is {event.name}")

    def _generate_roce(self):
        for event in self.roce_counter_event:
            ts = int(Decimal(event.ts) * 1000)
            if event.name == "Port 0/Tx":
                tx_bandwidth_efficiency = event.args.get("Tx Bandwidth Efficiency")
                tx_packets = event.args.get("Tx Packets")
                tx_error_rate = event.args.get("Tx Error Rate")
                tx_dropped_rate = event.args.get("Tx Dropped Rate")
                if ts not in self.roce_obj:
                    self.roce_obj[ts] = SysIO(ts, tx_dropped_rate=tx_dropped_rate, tx_packets=tx_packets,
                                              tx_error_rate=tx_error_rate,
                                              tx_bandwidth_efficiency=tx_bandwidth_efficiency)
                else:
                    self.roce_obj[ts].tx_dropped_rate = tx_dropped_rate
                    self.roce_obj[ts].tx_packets = tx_packets
                    self.roce_obj[ts].tx_error_rate = tx_error_rate
                    self.roce_obj[ts].tx_bandwidth_efficiency = tx_bandwidth_efficiency
            elif event.name == "Port 0/Rx":
                rx_bandwidth_efficiency = event.args.get("Rx Bandwidth Efficiency")
                rx_packets = event.args.get("Rx Packets")
                rx_error_rate = event.args.get("Rx Error Rate")
                rx_dropped_rate = event.args.get("Rx Dropped Rate")
                if ts not in self.roce_obj:
                    self.roce_obj[ts] = SysIO(ts, rx_dropped_rate=rx_dropped_rate, rx_packets=rx_packets,
                                              rx_error_rate=rx_error_rate,
                                              rx_bandwidth_efficiency=rx_bandwidth_efficiency)
                else:
                    self.roce_obj[ts].rx_dropped_rate = rx_dropped_rate
                    self.roce_obj[ts].rx_packets = rx_packets
                    self.roce_obj[ts].rx_error_rate = rx_error_rate
                    self.roce_obj[ts].rx_bandwidth_efficiency = rx_bandwidth_efficiency
            else:
                logging.error(f"The name of this RoCE data is invalid, is {event.name}")

    def _generate_pcie(self):
        for event in self.pcie_counter_event:
            ts = int(Decimal(event.ts) * 1000)
            if set(event.args.keys()) != {"Rx", "Tx"}:
                logging.error(f"ts: {event.ts}, {event.name} 的args发生了错误，不被期待的args参数：{event.args.keys()}")
                continue
            rx = int(event.args.get("Rx") * 1024 * 1024)
            tx = int(event.args.get("Tx") * 1024 * 1024)
            if ts not in self.pcie_obj:
                if event.name == "PCIe_cpl":
                    self.pcie_obj[ts] = Pcie(ts, pcie_cpl_tx=tx, pcie_cpl_rx=rx)
                elif event.name == "PCIe_nonpost":
                    self.pcie_obj[ts] = Pcie(ts, pcie_nonpost_tx=tx, pcie_nonpost_rx=rx)
                elif event.name == "PCIe_nonpost_latency":
                    rx = int(event.args.get("Rx") * 1000)
                    tx = int(event.args.get("Tx") * 1000)
                    self.pcie_obj[ts] = Pcie(ts, pcie_nonpost_latency_tx=tx, pcie_nonpost_latency_rx=rx)
                elif event.name == "PCIe_post":
                    self.pcie_obj[ts] = Pcie(ts, pcie_post_tx=tx, pcie_post_rx=rx)
                else:
                    logging.error(f"ts: {event.ts}, The name of this PCIE data is invalid, is {event.name}")
            else:
                if event.name == "PCIe_cpl":
                    self.pcie_obj[ts].pcie_cpl_tx = tx
                    self.pcie_obj[ts].pcie_cpl_rx = rx
                elif event.name == "PCIe_nonpost":
                    self.pcie_obj[ts].pcie_nonpost_tx = tx
                    self.pcie_obj[ts].pcie_nonpost_rx = rx
                elif event.name == "PCIe_nonpost_latency":
                    rx = int(event.args.get("Rx") * 1000)
                    tx = int(event.args.get("Tx") * 1000)
                    self.pcie_obj[ts].pcie_nonpost_latency_tx = tx
                    self.pcie_obj[ts].pcie_nonpost_latency_rx = rx
                elif event.name == "PCIe_post":
                    self.pcie_obj[ts].pcie_post_tx = tx
                    self.pcie_obj[ts].pcie_post_rx = rx
                else:
                    logging.error(f"ts: {event.ts}, The name of this PCIE data is invalid, is {event.name}")

    def _generate_hbm(self):
        for event in self.hbm_counter_event:
            ts = int(Decimal(event.ts) * 1000)
            key = str(ts) + event.name
            if "Read" in event.name:
                self.hbm_obj[key] = HBM(ts, int(event.args.get("Read(MB/s)") * 1024 * 1024))
            elif "Write" in event.name:
                self.hbm_obj[key] = HBM(ts, int(event.args.get("Write(MB/s)") * 1024 * 1024))
            else:
                logging.error(f"The name of this HBM data is invalid, is {event.name}")

    def _generate_stars_soc_info(self):
        for event in self.stars_soc_info_counter_event:
            ts = int(Decimal(event.ts) * 1000)
            if event.name == "Mata Bw Level":
                level = event.args.get("Mata Bw Level")
                if ts not in self.stars_soc_info_obj:
                    self.stars_soc_info_obj[ts] = StarsSocInfo(ts, mata_bw_level=level)
                else:
                    self.stars_soc_info_obj[ts].mata_bw_level = level
            elif event.name == "L2 Buffer Bw Level":
                level = event.args.get("L2 Buffer Bw Level")
                if ts not in self.stars_soc_info_obj:
                    self.stars_soc_info_obj[ts] = StarsSocInfo(ts, l2_buffer_level=level)
                else:
                    self.stars_soc_info_obj[ts].l2_buffer_level = level

    def _generate_llc(self):
        for event in self.llc_counter_event:
            ts = int(Decimal(event.ts) * 1000)
            key = str(ts) + event.name.split("/")[0]
            if "Throughput" in event.name:
                throughput = int(event.args.get("Throughput(MB/s)") * 1024 * 1024)
                if key not in self.llc_obj:
                    self.llc_obj[key] = LLc(ts, throughput=throughput)
                else:
                    self.llc_obj[key].throughput = throughput
            elif "Hit Rate" in event.name:
                hit_rate = event.args.get("Hit Rate(%)") * 100
                if key not in self.llc_obj:
                    self.llc_obj[key] = LLc(ts, hit_rate=hit_rate)
                else:
                    self.llc_obj[key].hit_rate = hit_rate
            else:
                logging.error(f"The name of this LLc data is invalid, is {event.name}")

    def _generate_qos(self):
        pass

    def _generate_wired(self, traces):
        for trace in traces:
            if self.check_flow_end_event(trace):
                ts = int(Decimal(trace.get("ts")) * 1000)
                cat = trace.get("cat")
                if cat is None:
                    logging.error("这种连线类型的终点的cat信息缺失，请补充！例如HostToDevice和MsTx")
                    continue
                if cat == "MsTx":
                    connection_id = int(trace.get("id"))
                elif cat == "HostToDevice":
                    # 这种类型的需要右移32位，因为现在的connection_id是左移32位之后得到的
                    connection_id = int(trace.get("id")) >> 32
                else:
                    logging.error(f"{ts}出现了新的连线的cat信息，请确认：{cat}")
                    continue
                if connection_id in self.flow_start:
                    self.wired_obj[ts] = Wired(self.flow_start[connection_id], ts, connection_id)

    def _generate_cann_api(self):
        for event in self.cann_dur_events:
            start = int(Decimal(event.ts) * 1000)
            end = start + int(Decimal(str(event.dur)) * 1000)
            self.api_obj[start] = API(start, end, event.name, event.args.get("level"))


class CsvAnalyzer:
    """
    分析csv,暂时只有op_summary, 不同于JsonAnalyzer,外部调用staticmethod获取相应的数据结构
    """

    @staticmethod
    def generate_compute_task_info(csv_path: str) -> Dict[int, ComputeTaskInfo]:
        df = FileManager.read_csv_file(csv_path)
        compute_task_info_obj: Dict[int, ComputeTaskInfo] = dict()
        for index, row in df.iterrows():
            start = int(Decimal(str(row["Task Start Time(us)"])) * 1000)
            compute_task_info_obj[start] = ComputeTaskInfo(start, row["Op Name"], row["OP Type"], row["Task Type"],
                                                           row["Block Dim"], row["Mix Block Dim"],
                                                           row["Input Formats"], row["Input Data Types"],
                                                           row["Input Shapes"], row["Output Formats"],
                                                           row["Output Data Types"], row["Output Shapes"],
                                                           row["OP State"], row["HF32 Eligible"])
        return compute_task_info_obj


class DBAnalyzer:
    """
    分析db
    """

    def __init__(self, db_path=""):
        self.db_path = db_path
        self.conn = EmptyClass("empty conn")
        self.curs = EmptyClass("empty curs")
        # 存放统一db里面的映射
        self.tables: List[str] = []
        self.global_task_id: Dict[int, Dict[str, int]] = dict()
        self.string_ids: Dict[int, str] = dict()
        self.api_type: Dict[int, str] = dict()
        self.hccl_data_type: Dict[int, str] = dict()
        self.hccl_link_type: Dict[int, str] = dict()
        self.hccl_transport_type: Dict[int, str] = dict()
        # 用于和json、csv对比的dict
        self.ascend_task_data_obj: Dict[int, AscendTaskData] = dict()
        self.communication_op_obj: Dict[int, CommunicationOp] = dict()
        self.communication_task_info_obj: Dict[int, CommunicationTaskInfo] = dict()
        self.compute_task_info_obj: Dict[int, ComputeTaskInfo] = dict()
        self.aicore_freq_obj: Dict[int, AicoreFreq] = dict()
        self.npu_mem_obj: Dict[str, NpuMem] = dict()
        self.ddr_obj: Dict[str, DDR] = dict()
        self.acc_pmu_obj: Dict[str, AccPmu] = dict()
        self.hccs_obj: Dict[int, Hccs] = dict()
        self.nic_obj: Dict[int, SysIO] = dict()
        self.roce_obj: Dict[int, SysIO] = dict()
        self.pcie_obj: Dict[int, Pcie] = dict()
        self.hbm_obj: Dict[str, HBM] = dict()
        self.stars_soc_info_obj: Dict[int, StarsSocInfo] = dict()
        self.llc_obj: Dict[str, LLc] = dict()
        self.wired_obj: Dict[int, Wired] = dict()
        self.api_obj: Dict[int, API] = dict()

    def connect_db(self):
        self.conn, self.curs = DBManager.create_connect_db(self.db_path)

    def generate_data_instance(self):
        if not self.conn or not self.curs:
            logging.error("Failed to connect db")
            return
        self._load_tables()
        self._load_string_ids()
        self._load_enum_api_type()
        self._load_enum_hccl_data_type()
        self._load_enum_hccl_link_type()
        self._load_enum_hccl_transport_type()
        self._load_global_task_start_and_end()
        self._generate_ascend_task_data()
        self._generate_communication_op()
        self._generate_communication_task_info()
        self._generate_compute_task_info()
        self._generate_aicore_freq()
        self._generate_npu_mem()
        self._generate_ddr()
        self._generate_acc_pmu()
        self._generate_hccs()
        self._generate_nic()
        self._generate_roce()
        self._generate_pcie()
        self._generate_hbm()
        self._generate_stars_soc_info()
        self._generate_llc()
        self._generate_qos()
        self._generate_wired()
        self._generate_cann_api()

    def finalize(self):
        DBManager.destroy_db_connect(self.conn, self.curs)

    def _generate_ascend_task_data(self):
        """
        json里面过滤了ffts+，最后只呈现子任务
        """
        ffts_task = set()
        sql = f"select startNs, endNs, modelId, streamId, taskId, contextId, taskType from TASK;"
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            if row[5] != 4294967295:
                ffts_task.add((row[3], row[4], 4294967295))  # 缺少一个batchId,对该用例不影响
        for row in res:
            tmp = (row[3], row[4], row[5])
            if tmp in ffts_task:
                continue
            task_type = self.string_ids.get(row[6])
            self.ascend_task_data_obj[row[0]] = AscendTaskData(row[0], row[1], row[2], row[3], row[4], row[5],
                                                               task_type)

    def _generate_communication_op(self):
        if "COMMUNICATION_OP" not in self.tables:
            return
        sql = f"select opName, startNs, endNs, groupName, dataType, algType, count, relay, retry from COMMUNICATION_OP;"
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            op_name = self.string_ids.get(row[0])
            if op_name.startswith("hcomAicpuInit"):  # TODO: json里面过滤掉了，而且统一db虽然有这一块，但是无法正常显示
                continue
            group_name = "Group " + self.string_ids.get(row[3]) + " Communication"
            data_type = self.hccl_data_type.get(row[4])
            alg_type = self.string_ids.get(row[5])
            self.communication_op_obj[row[1]] = CommunicationOp(op_name, row[1], row[2], group_name, data_type,
                                                                alg_type, row[6], row[7], row[8])

    def _generate_communication_task_info(self):
        if "COMMUNICATION_TASK_INFO" not in self.tables:
            return
        sql = (f"select globalTaskId, taskType, groupName, planeId, notifyId, srcRank, dstRank, transportType, "
               f"dataType, linkType, size from COMMUNICATION_TASK_INFO;")
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            start = self.global_task_id.get(row[0], {}).get("start")
            end = self.global_task_id.get(row[0], {}).get("end")
            hccl_name = self.string_ids.get(row[1])
            group_name = self.string_ids.get(row[2])
            transport_type = self.hccl_transport_type.get(row[7])
            data_type = self.hccl_data_type.get(row[8])
            link_type = self.hccl_link_type.get(row[9])
            self.communication_task_info_obj[start] = CommunicationTaskInfo(start, end, hccl_name, group_name, row[3],
                                                                             row[4], row[5], row[6], transport_type,
                                                                             data_type, link_type, row[10])

    def _generate_compute_task_info(self):
        sql = (f"select globalTaskId, name, opType, taskType, blockDim, mixBlockDim, inputFormats, inputDataTypes, "
               f"inputShapes, outputFormats, outputDataTypes, outputShapes,"
               f"opState, hf32Eligible from COMPUTE_TASK_INFO;")
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            start = self.global_task_id.get(row[0], {}).get("start")
            op_name = self.string_ids.get(row[1])
            op_type = self.string_ids.get(row[2])
            task_type = self.string_ids.get(row[3])
            op_state = self.string_ids.get(row[12])
            hf32 = self.string_ids.get(row[13])
            self.compute_task_info_obj[start] = ComputeTaskInfo(start, op_name, op_type, task_type, row[4], row[5],
                                                                row[6], row[7], row[8], row[9], row[10], row[11],
                                                                op_state, hf32)

    def _generate_aicore_freq(self):
        sql = f"select timestampNs, freq from AICORE_FREQ;"
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            self.aicore_freq_obj[row[0]] = AicoreFreq(row[0], row[1])

    def _generate_npu_mem(self):
        if "NPU_MEM" not in self.tables:
            return
        sql = f"select timestampNs, hbm, ddr, type from NPU_MEM;"
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            data_type = self.string_ids.get(row[3])
            if data_type == "app":
                key = str(row[0]) + "-" + "APP"
            elif data_type == "device":
                key = str(row[0]) + "-" + "Device"
            else:
                logging.error(f"{row[0]}: Incorrect type in NPU_MEM")
                continue
            self.npu_mem_obj[key] = NpuMem(row[0], row[1], row[2])

    def _generate_ddr(self):
        pass

    def _generate_acc_pmu(self):
        if "ACC_PMU" not in self.tables:
            return
        sql = f"select timestampNs, accId, readBwLevel, writeBwLevel, readOstLevel, writeOstLevel from ACC_PMU;"
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            key = str(row[0]) + "-" + str(row[1])
            self.acc_pmu_obj[key] = AccPmu(row[0], row[1], row[2], row[3], row[4], row[5])

    def _generate_hccs(self):
        sql = f"select timestampNs, txThroughput, rxThroughput from HCCS;"
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            self.hccs_obj[row[0]] = Hccs(row[0], row[1], row[2])

    def _generate_nic(self):
        sql = (f"select timestampNs, bandwidth, rxPacketRate, rxByteRate, rxPackets, rxErrors, rxDropped, txPacketRate,"
               f"txByteRate, txPackets, txErrors, txDropped from NIC;")
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            rx_bandwidth_effciency = row[3] * 8 / row[1] if row[1] != 0 else 0.0
            rx_packets = row[2]
            rx_error_rate = row[5] / row[4] if row[4] != 0 else 0.0
            rx_dropped_rate = row[6] / row[4] if row[4] != 0 else 0.0
            tx_bandwidth_effciency = row[8] * 8 / row[1] if row[1] != 0 else 0.0
            tx_packets = row[7]
            tx_error_rate = row[10] / row[9] if row[9] != 0 else 0.0
            tx_dropped_rate = row[11] / row[9] if row[9] != 0 else 0.0
            self.nic_obj[row[0]] = SysIO(row[0], 0, tx_dropped_rate, tx_packets, tx_error_rate, tx_bandwidth_effciency,
                                         rx_dropped_rate, rx_packets, rx_error_rate, rx_bandwidth_effciency)


    def _generate_roce(self):
        sql = (f"select timestampNs, bandwidth, rxPacketRate, rxByteRate, rxPackets, rxErrors, rxDropped, txPacketRate,"
               f"txByteRate, txPackets, txErrors, txDropped from RoCE;")
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            rx_bandwidth_effciency = row[3] * 8 / row[1] if row[1] != 0 else 0.0
            rx_packets = row[2]
            rx_error_rate = row[5] / row[4] if row[4] != 0 else 0.0
            rx_dropped_rate = row[6] / row[4] if row[4] != 0 else 0.0
            tx_bandwidth_effciency = row[8] * 8 / row[1] if row[1] != 0 else 0.0
            tx_packets = row[7]
            tx_error_rate = row[10] / row[9] if row[9] != 0 else 0.0
            tx_dropped_rate = row[11] / row[9] if row[9] != 0 else 0.0
            self.roce_obj[row[0]] = SysIO(row[0], 0, tx_dropped_rate, tx_packets, tx_error_rate, tx_bandwidth_effciency,
                                         rx_dropped_rate, rx_packets, rx_error_rate, rx_bandwidth_effciency)

    def _generate_pcie(self):
        sql = (f"select timestampNs, txPostAvg, txNonpostAvg, txCplAvg, txNonpostLatencyAvg, rxPostAvg, rxNonpostAvg, "
               f"rxCplAvg from PCIE;")
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            self.pcie_obj[row[0]] = Pcie(row[0], row[3], row[2], row[4], row[1], row[7], row[6], row[5], 0)

    def _generate_hbm(self):
        if "HBM" not in self.tables:
            return
        sql = f"select timestampNs, hbmId, type, bandwidth from HBM;"
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            hbm_type = self.string_ids.get(row[2])
            if hbm_type == "read":
                key = str(row[0]) + f"HBM {row[1]}/" + "Read"
            elif hbm_type == "write":
                key = str(row[0]) + f"HBM {row[1]}/" + "Write"
            else:
                logging.error(f"{row[0]}, hbmId:{row[1]}: Incorrect type in HBM")
                continue
            self.hbm_obj[key] = HBM(row[0], row[3])

    def _generate_stars_soc_info(self):
        if "SOC_BANDWIDTH_LEVEL" not in self.tables:
            return
        sql = f"select timestampNs, l2BufferBwLevel, mataBwLevel from SOC_BANDWIDTH_LEVEL;"
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            self.stars_soc_info_obj[row[0]] = StarsSocInfo(row[0], row[1], row[2])

    def _generate_llc(self):
        if "LLC" not in self.tables:
            return
        sql = f"select timestampNs, llcId, mode, hitRate, throughput from LLC;"
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            mode = self.string_ids.get(row[2])
            if mode == "read":
                key = str(row[0]) + f"LLC {row[1]} Read"
            elif mode == "write":
                key = str(row[0]) + f"LLC {row[1]} Write"
            else:
                logging.error(f"{row[0]}, llcId:{row[1]}: Incorrect mode in LLC")
                continue
            self.llc_obj[key] = LLc(row[0], row[3], row[4])

    def _generate_qos(self):
        pass

    def _load_tables(self):
        sql = "SELECT name FROM sqlite_master WHERE type='table';"
        res = DBManager.fetch_all_data(self.curs, sql)
        self.tables = [table[0] for table in res]

    def _load_string_ids(self):
        sql = f"select * from STRING_IDS;"
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            self.string_ids[row[0]] = row[1]

    def _load_enum_api_type(self):
        sql = f"select * from ENUM_API_TYPE;"
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            self.api_type[row[0]] = row[1]

    def _load_enum_hccl_data_type(self):
        sql = f"select * from ENUM_HCCL_DATA_TYPE;"
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            self.hccl_data_type[row[0]] = row[1]

    def _load_enum_hccl_link_type(self):
        sql = f"select * from ENUM_HCCL_LINK_TYPE;"
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            self.hccl_link_type[row[0]] = row[1]

    def _load_enum_hccl_transport_type(self):
        sql = f"select * from ENUM_HCCL_TRANSPORT_TYPE;"
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            self.hccl_transport_type[row[0]] = row[1]

    def _load_global_task_start_and_end(self):
        sql = f"select globalTaskId, startNs, endNs from TASK;"
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            self.global_task_id[row[0]] = {"start": row[1], "end": row[2]}

    def _generate_wired(self):
        """
        可视化连线的逻辑是去COMMUNICATION_OP和COMPUTE_TASK_INFO里面根据connectionId找
        """
        sql = f"select connectionId, startNs from CANN_API;"
        res = DBManager.fetch_all_data(self.curs, sql)
        connection_start = {}
        for row in res:
            if row[0] in connection_start:
                logging.error(f"重复的connection:{row[0]}")
                continue
            connection_start[row[0]] = row[1]
        # 计算算子
        comute_task_ids = []
        sql = f"select globalTaskId from COMPUTE_TASK_INFO;"
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            comute_task_ids.append(row[0])
        # 去重可视化不会对这种进行连线
        ffts_task = set()
        sql = f"select connectionId, startNs, streamId, taskId, contextId, globalTaskId from TASK;"
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            if row[4] != 4294967295:
                ffts_task.add((row[2], row[3], 4294967295))  # 缺少一个batchId,对该用例不影响
        # 从TASK里面找
        for row in res:
            tmp_task = (row[2], row[3], row[4])
            if tmp_task in ffts_task or row[5] not in comute_task_ids:
                continue
            if row[0] in connection_start:
                self.wired_obj[row[1]] = Wired(connection_start[row[0]], row[1], row[0])
        # 从COMMUNICATION_OP里面找
        if "COMMUNICATION_OP" not in self.tables:
            return
        sql = f"select connectionId, startNs from COMMUNICATION_OP;"
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            if row[0] in connection_start:
                self.wired_obj[row[1]] = Wired(connection_start[row[0]], row[1], row[0])

    def _generate_cann_api(self):
        sql = f"select startNs, endNs, name, type from CANN_API;"
        res = DBManager.fetch_all_data(self.curs, sql)
        for row in res:
            self.api_obj[row[0]] = API(row[0], row[1], self.string_ids.get(row[2]), self.api_type.get(row[3]))
