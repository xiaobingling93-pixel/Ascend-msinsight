#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_custom -s SingleOp -p ",--application=/home/msprof_smoke_test/model/VectorAndMatmul/every_thread_calls_aclInit_and_aclFinalize/bin/vectorAndMatmul," --id test_every_thread_calls_aclInit_and_aclFinalize
