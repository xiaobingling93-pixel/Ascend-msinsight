#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_custom -s SingleOp -p ",--application=/home/msprof_smoke_test/model/VectorAndMatmul/constructor_aclInit_and_destructor_aclFinalize/bin/vectorAndMatmul," --id test_constructor_aclInit_and_destructor_aclFinalize
