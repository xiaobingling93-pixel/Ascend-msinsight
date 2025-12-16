#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/get_msprof_info_test.py -m Non-Cluster -s Non-Cluster -p 0, --id test_get_msprof_info_py_non_cluster