#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ../scripts/sample_mc2_test.py -m case_gpt_profiling -s sample_mc2 -p , --id test_sample_mc2_profiling
