#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/sample_hccl_test.py -m case_hccl_sence -s sampleSingleOpHcclFftsffOn -p , --id test_sample_single_op_ffts_on_hccl_data
