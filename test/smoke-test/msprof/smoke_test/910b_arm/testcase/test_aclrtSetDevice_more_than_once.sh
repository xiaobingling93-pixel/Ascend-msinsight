#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_custom -s SingleOp -p ",--application=/home/msprof_smoke_test/model/VectorAndMatmul/aclrtSetDevice_more_than_once/bin/vectorAndMatmul," --id test_aclrtSetDevice_more_than_once
