#!/bin/bash
currentDir=$(cd "$(dirname "$0")"; pwd)
python3 ${currentDir}/../script/test_big_data_export_db.py -m llama2 -s export_db -p 0, --id \
test_big_data_export_db

