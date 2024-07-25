#!/bin/bash
set -o errexit

current_path=$(pwd)
## need sudo
# Secodefuzz build only needed on first running
cd ../third_party/secodefuzz
bash build.sh gcc

cd ${current_path}
echo "*************** FUZZCODE  START *****************"
cmake -D_PROJECT_TYPE=fuzz ..
echo "*************** CMAKE  FINISH "
cpu_number=$(nproc)
make -j${cpu_number}
echo "*************** MAKE  FINISH "
echo "*************** RUN insight_fuzz... "
system_arch=$(arch)
../output/linux-${system_arch}/bin/insight_fuzz > report/insight_fuzz_test_details.log
echo "**************** FUZZCODE  END ******************"
echo "Check more details in report/insight_fuzz_test_details.log"