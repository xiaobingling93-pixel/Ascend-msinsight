#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_custom -s PyAclApi -p ",--application='python3 /home/msprof_smoke_test/model/PyAclNoFinalize/predict.py', --l2=on,--aic-metrics=ArithmeticUtilization, " --id test_pyacl_not_call_aclfinalize
