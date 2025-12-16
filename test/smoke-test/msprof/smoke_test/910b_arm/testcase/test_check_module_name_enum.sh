#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_check_module_name_enum.py -m enum -s enum -p , --id test_check_module_name_enum
