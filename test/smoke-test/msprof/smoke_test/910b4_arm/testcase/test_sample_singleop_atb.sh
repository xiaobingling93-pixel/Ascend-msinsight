#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../scripts/sample_singleop_atb_test.py -m case_singleop_atb -s singleop_atb -p , --id test_sample_singleop_atb_profiling