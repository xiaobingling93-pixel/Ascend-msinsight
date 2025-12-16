#!/usr/bin/python3
# -*- coding: utf-8 -*-
# Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
class JSONHeaderManager:
    SAMPLE_HEADER_910B = ["Ascend Hardware", "HBM", "PCIe", "HCCS", "NIC", "RoCE", "LLC", "NPU MEM",
                          "AI Core Utilization",
                          "Stars Soc Info", "Stars Chip Trans", "Acc PMU", "CANN", "SIO", "QoS", "Overlap Analysis",
                          "AI Core Freq"]

    SAMPLE_HEADER_910A = ["Ascend Hardware", "HBM", "PCIe", "HCCS", "LLC", "NPU MEM",
                          "AI Core Utilization", "CANN", "DDR", "Overlap Analysis"]

    HCCL_HEADER = ["CANN", "Ascend Hardware", "AI Core Freq", "Communication", "Overlap Analysis"]

    TF_HEADER_910B = ["CANN", "Ascend Hardware", "AI Core Freq", "HBM", "PCIe", "HCCS", "NIC", "RoCE", "LLC", "NPU MEM",
                      "Stars Soc Info", "Stars Chip Trans", "Acc PMU", "SIO", "QoS", "Overlap Analysis"]

    TF_HEADER_910A = ["CANN", "Ascend Hardware", "HBM", "PCIe", "HCCS", "LLC", "NPU MEM",
                      "DDR", "Overlap Analysis"]

    SCENE_MAP = {
        "910A": {"test_IsAicModeIsSampleBased_LlcIsWrite_AicMetricsIsResourceConflictRatio": SAMPLE_HEADER_910A,
                 "test_Analysis_ffts_on_hccl_l1_data": HCCL_HEADER,
                 "test_IsSwitchMsprofbin-TensorFlow": TF_HEADER_910A},
        "910B": {"test_IsAicModeIsSampleBased_LlcIsWrite_AicMetricsIsResourceConflictRatio": SAMPLE_HEADER_910B,
                 "test_Analysis_ffts_on_hccl_l1_data": HCCL_HEADER,
                 "test_IsSwitchMsprofbin-TensorFlow": TF_HEADER_910B}
    }

    @classmethod
    def init_header_config(cls, chip):
        return cls.SCENE_MAP.get(chip)
