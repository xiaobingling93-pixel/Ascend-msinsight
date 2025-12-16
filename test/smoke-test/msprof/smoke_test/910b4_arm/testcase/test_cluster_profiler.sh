#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../scripts/cluster_scense.py -m case_cluster_profiling -s llama2 -p , --id test_cluster_profiling