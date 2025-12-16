#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/mstx_test.py -m mstx -s acljson  -p , --id test_Mstx_acl_json