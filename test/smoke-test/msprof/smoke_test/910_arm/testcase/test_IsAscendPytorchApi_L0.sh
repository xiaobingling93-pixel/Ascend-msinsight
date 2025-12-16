#/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_profiling.py -m case_ascend_pytorch_api_scene -s AscendPytorchApi -p , --id AscendPytorchApi_L0
