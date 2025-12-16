#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/mstx_test.py -m mstx -s msprof  -p , --id test_Mstx_msprof