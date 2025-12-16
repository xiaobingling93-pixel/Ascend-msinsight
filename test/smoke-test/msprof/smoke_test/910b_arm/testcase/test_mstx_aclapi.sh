#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/mstx_test.py -m mstx -s aclapi  -p , --id test_Mstx_aclapi