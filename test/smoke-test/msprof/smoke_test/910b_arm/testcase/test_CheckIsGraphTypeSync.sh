#!/bin/bash
#
# Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
#

currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_check_graph_type.py -m checkResDir -s All --id test_check_python_ge_constant_is_sync
