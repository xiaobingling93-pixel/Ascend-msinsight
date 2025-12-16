#!/usr/bin/python3
# -*- coding: utf-8 -*-
# Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
"""
这个文件用于对比统一db和csv、json的数据，做到像素级校验，因为统一db的数据更全，所以整体数据结构向db看齐
"""
import math


class AscendTaskData:
    """
    比较统一db的TASK表和json里面的Ascend Hardware层,通过ts字段(转成int)作为两者对比的依据，即两个ts完全一样才可以对比
    ts完全一样的概率极小（ns级别）
    比较范围：所有在device运行的任务，包括tx打点
    TODO:
    两者显示不同，统一db显示input和output信息、blockdim等csv里面的信息，不显示stream id等
    task_type在可视化的显示也不一样，KERNEL_AIVEC AI_CORE AI_VECTOR_CORE
    """

    def __init__(self, start: int, end: int, model_id: int, stream_id: int,
                 task_id: int, context_id: int, task_type: str):
        self.start = start
        self.end = end
        self.model_id = model_id
        self.stream_id = stream_id
        self.task_id = task_id
        self.context_id = context_id
        self.task_type = task_type

    def __ne__(self, other):
        if not isinstance(other, AscendTaskData):
            return True
        return not all([
            self.start == other.start,
            abs(self.end == other.end) <= 1,
            self.model_id == other.model_id,
            self.stream_id == other.stream_id,
            self.task_id == other.task_id,
            self.context_id == other.context_id,
            # self.task_type == other.task_type  # TODO:目前task_type, json和op_summary以及统一db的不一样
        ])


class ComputeTaskInfo:
    """
    对比ComputeTaskInfo表和op_summary里面计算类算子的数据，
    先通过globalTaskId将TASK和COMPUTE_TASK_INFO关联起来，然后理论上就可以根据start时间作为唯一对比的凭证
    理论上op_summary里面算子的数量等于COMMUNICATION_OP + COMPUTE_TASK_INFO
    优先以统一db里面的为基准，在op_summry里面找（包含计算、通信、mc2等）
    """

    def __init__(self, start, op_name: str, op_type: str, task_type: str, block_dim: int, mix_block_dim: int,
                 input_formats: str, input_data_types: str, input_shapes: str, output_formats: str,
                 output_data_types: str, output_shapes: str, op_state: str, hf32: str):
        self.start = start
        self.op_name = op_name
        self.op_type = op_type
        self.task_type = task_type
        self.block_dim = block_dim
        self.mix_block_dim = mix_block_dim
        self.input_formats = input_formats
        self.input_data_types = input_data_types
        self.input_shapes = input_shapes
        self.output_formats = output_formats
        self.output_data_types = output_data_types
        self.output_shapes = output_shapes
        self.op_state = op_state
        self.hf32 = hf32

    def __ne__(self, other):
        if not isinstance(other, ComputeTaskInfo):
            return True
        return not all([
            self.op_name == other.op_name,
            self.op_type == other.op_type,
            self.task_type == other.task_type,
            self.block_dim == other.block_dim,
            self.mix_block_dim == other.mix_block_dim,
            self.input_formats == other.input_formats,
            self.input_data_types == other.input_data_types,
            self.input_shapes == other.input_shapes,
            self.output_formats == other.output_formats,
            self.output_data_types == other.output_data_types,
            self.output_shapes == other.output_shapes,
            self.op_state == other.op_state,
            self.hf32 == other.hf32
        ])


class CommunicationOp:
    """
    和op_summary里面的通信算子的数量应该相等
    以ts为凭据,对比COMMUNICATION_OP表和HCCL层的通信大算子
    group_name需要从json里面筛选然后和json的hccl大算子关联组成该结构
    """

    def __init__(self, op_name: str, start: int, end: int, group_name: str, data_type: str, alg_type: str,
                 count: int, relay: int, retry: int):
        self.op_name = op_name
        self.start = start
        self.end = end
        self.group_name = group_name
        self.data_type = data_type
        self.alg_type = alg_type
        self.count = count
        self.relay = relay
        self.retry = retry

    def __ne__(self, other):
        if not isinstance(other, CommunicationOp):
            return True
        return not all([
            self.op_name == other.op_name,
            self.start == other.start,
            self.end == other.end,
            self.group_name == other.group_name,
            self.data_type == other.data_type,
            self.alg_type == other.alg_type,
            self.count == other.count,
            self.relay == other.relay,
            self.retry == other.retry
        ])


class CommunicationTaskInfo:
    """
    统一db里面COMMUNICATION_TASK_INFO表和json里面通信小算子的对比
    和json里面HCCL层通信小算子的数量应该相等
    统一db需要用glocalTaskID将TASK表和COMMUNICATION_TASK_INFO关联
    """

    def __init__(self, start: int, end: int, hccl_name: str, group_name: str, plane_id: int, notify_id: str,
                 src_rank: int, dst_rank: int, transport_type: str, data_type: str, link_type: str, size: int):
        self.start = start
        self.end = end
        self.hccl_name = hccl_name
        self.group_name = group_name
        self.plane_id = plane_id
        self.notify_id = notify_id
        self.src_rank = src_rank
        self.dst_rank = dst_rank
        self.transport_type = transport_type
        self.data_type = data_type
        self.link_type = link_type
        self.size = size

    def __ne__(self, other):
        if not isinstance(other, CommunicationTaskInfo):
            return True
        return not all([
            self.start == other.start,
            self.end == other.end,
            self.hccl_name == other.hccl_name,
            self.group_name == other.group_name,
            self.plane_id == other.plane_id,
            self.notify_id == other.notify_id,
            self.src_rank == other.src_rank,
            self.dst_rank == other.dst_rank,
            self.transport_type == other.transport_type,
            self.data_type == other.data_type,
            self.link_type == other.link_type,
            self.size == other.size
        ])


class AicoreFreq:
    """
    对比AI Core Freq层的数据和AICORE_FREQ表
    ts 作为对比凭据
    """

    def __init__(self, ts: int, freq):
        self.ts = ts
        self.freq = freq

    def __ne__(self, other):
        if not isinstance(other, AicoreFreq):
            return True
        return not all([
            self.freq == other.freq
        ])


class NpuMem:
    """
    对比NPU_MEM和json里面的NPU MEM层
    ts + data_type(APP和Device)作为对比凭据
    """

    def __init__(self, ts: int, hbm: int = None, ddr: int = None):
        self.ts = ts
        self.hbm = hbm
        self.ddr = ddr

    def __ne__(self, other):
        if not isinstance(other, NpuMem):
            return True
        return not all([
            self.hbm == other.hbm,
            self.ddr == other.ddr,
        ])


class DDR:
    """
    对比json的DDR层和统一db的DDR表, 310B
    ts
    """

    def __init__(self, ts: int, read_bw, write_bw):
        self.read_bw = read_bw
        self.write_bw = write_bw

    def __ne__(self, other):
        if not isinstance(other, DDR):
            return True
        return not all([
            self.read_bw == other.read_bw,
            self.write_bw == other.write_bw
        ])


class AccPmu:
    """
    对比json的 Acc PMU层和统一db的ACC_PMU表
    ts + accId作为对比凭据
    """

    def __init__(self, ts: int, acc_id: int, read_bw_level: int = None, write_bw_level: int = None,
                 read_ost_level: int = None, write_ost_level: int = None):
        self.ts = ts
        self.acc_id = acc_id
        self.read_bw_level = read_bw_level
        self.write_bw_level = write_bw_level
        self.read_ost_level = read_ost_level
        self.write_ost_level = write_ost_level

    def __ne__(self, other):
        if not isinstance(other, AccPmu):
            return True
        return not all([
            self.read_bw_level == other.read_bw_level,
            self.write_bw_level == other.write_bw_level,
            self.read_ost_level == other.read_ost_level,
            self.write_ost_level == other.write_ost_level
        ])


class Hccs:
    """
    对比json的HCCS层和统一db的HCCS表
    带宽用int对比，单位为B/s,是整数
    """

    def __init__(self, ts: int, tx: int = None, rx: int = None):
        self.ts = ts
        self.tx = tx
        self.rx = rx

    def __ne__(self, other):
        if not isinstance(other, Hccs):
            return True
        return not all([
            self.tx == other.tx,
            self.rx == other.rx
        ])


class SysIO:
    """
    对比json的NIC层和统一db的NIC表,RoCE层和ROCE表
    ts 作为对比凭据, port几乎是固定的0，所以暂时不考虑
    """

    def __init__(self, ts: int, port: int = 0, tx_dropped_rate: float = None, tx_packets: float = None,
                 tx_error_rate: float = None, tx_bandwidth_efficiency: float = None, rx_dropped_rate: float = None,
                 rx_packets: float = None, rx_error_rate: float = None, rx_bandwidth_efficiency: float = None):
        self.ts = ts
        self.port = port
        self.tx_dropped_rate = tx_dropped_rate
        self.tx_packets = tx_packets
        self.tx_error_rate = tx_error_rate
        self.tx_bandwidth_efficiency = tx_bandwidth_efficiency
        self.rx_dropped_rate = rx_dropped_rate
        self.rx_packets = rx_packets
        self.rx_error_rate = rx_error_rate
        self.rx_bandwidth_efficiency = rx_bandwidth_efficiency

    def __ne__(self, other):
        if not isinstance(other, SysIO):
            return True
        return not all([
            math.isclose(self.tx_dropped_rate, other.tx_dropped_rate),
            math.isclose(self.tx_packets, other.tx_packets),
            math.isclose(self.tx_error_rate, other.tx_error_rate),
            math.isclose(self.tx_bandwidth_efficiency, other.tx_bandwidth_efficiency),
            math.isclose(self.rx_dropped_rate, other.rx_dropped_rate),
            math.isclose(self.rx_packets, other.rx_packets),
            math.isclose(self.rx_error_rate, other.rx_error_rate),
            math.isclose(self.rx_bandwidth_efficiency, other.rx_bandwidth_efficiency),
        ])


class Pcie:
    """
    对比json的PCIe层和统一db的PCIE表
    """

    def __init__(self, ts: int, pcie_cpl_tx: int = None, pcie_nonpost_tx: int = None,
                 pcie_nonpost_latency_tx: int = None, pcie_post_tx: int = None,
                 pcie_cpl_rx: int = None, pcie_nonpost_rx: int = None, pcie_nonpost_latency_rx: int = None,
                 pcie_post_rx: int = None):
        self.ts = ts
        self.pcie_cpl_tx = pcie_cpl_tx
        self.pcie_nonpost_tx = pcie_nonpost_tx
        self.pcie_nonpost_latency_tx = pcie_nonpost_latency_tx
        self.pcie_post_tx = pcie_post_tx
        self.pcie_cpl_rx = pcie_cpl_rx
        self.pcie_nonpost_rx = pcie_nonpost_rx
        self.pcie_nonpost_latency_rx = pcie_nonpost_latency_rx
        self.pcie_post_rx = pcie_post_rx

    def __ne__(self, other):
        if not isinstance(other, Pcie):
            return True
        return not all([
            self.pcie_cpl_tx == other.pcie_cpl_tx,
            self.pcie_post_tx == other.pcie_post_tx,
            self.pcie_nonpost_tx == other.pcie_nonpost_tx,
            self.pcie_nonpost_latency_tx == other.pcie_nonpost_latency_tx,
            self.pcie_cpl_rx == other.pcie_cpl_rx,
            self.pcie_post_rx == other.pcie_post_rx,
            self.pcie_nonpost_rx == other.pcie_nonpost_rx,
            self.pcie_nonpost_latency_rx == other.pcie_nonpost_latency_rx,
        ])


class HBM:
    """
    对比json的HBM层和统一db的HBM表
    ts + hbm_id + data_type作为唯一凭据
    """

    def __init__(self, ts: int, bandwidth: int):
        self.ts = ts
        self.bandwidth = bandwidth

    def __ne__(self, other):
        if not isinstance(other, HBM):
            return True
        return not all([
            self.bandwidth == other.bandwidth,
        ])


class StarsSocInfo:
    """
    对比json的Stars Soc Info和SOC_BANDWIDTH_LEVEL
    """

    def __init__(self, ts: int, l2_buffer_level: int = None, mata_bw_level: int = None):
        self.ts = ts
        self.l2_buffer_level = l2_buffer_level
        self.mata_bw_level = mata_bw_level

    def __ne__(self, other):
        if not isinstance(other, StarsSocInfo):
            return True
        return not all([
            self.l2_buffer_level == other.l2_buffer_level,
            self.mata_bw_level == other.mata_bw_level
        ])


class LLc:
    """
    对比json的LLC层和LLC表
    ts + llcid + mode(read, write)
    """

    def __init__(self, ts: int, hit_rate: float = None, throughput: int = None):
        self.ts = ts
        self.hit_rate = hit_rate
        self.throughput = throughput

    def __ne__(self, other):
        if not isinstance(other, LLc):
            return True
        return not all([
            math.isclose(self.hit_rate, other.hit_rate),
            self.throughput == other.throughput
        ])

class Wired:
    """
    连线数据
    连线的一个起点可能对应多个终点，所以以连线的终点的startNs作为对比凭据
    """
    def __init__(self, start: int = None, end: int = None, connection_id: int = None):
        self.start = start
        self.end = end
        self.connection_id = connection_id

    def __ne__(self, other):
        if not isinstance(other, Wired):
            return True
        return not all([
            self.start == other.start,
            self.connection_id == other.connection_id
        ])

class API:
    """
    对比db里面CANN_API和json里面CANN层API的数量、名字等
    以ts为唯一凭据
    """
    def __init__(self, start: int, end: int, name: str, level: str):
        self.start = start
        self.end = end
        self.name = name
        self.level = level

    def __ne__(self, other):
        if not isinstance(other, API):
            return True
        return not all([
            self.end == other.end,
            self.name in other.name,  # TODO
            self.level == other.level
        ])
