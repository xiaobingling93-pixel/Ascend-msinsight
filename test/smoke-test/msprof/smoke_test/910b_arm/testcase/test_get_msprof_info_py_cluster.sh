#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/get_msprof_info_test.py -m cluster -s cluster -p 0, --id test_get_msprof_info_py_cluster