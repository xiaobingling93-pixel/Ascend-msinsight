#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/mstx_test.py -m mstx -s pytorch_domain_exclude  -p , --id test_Mstx_pytorch_domain_exclude
