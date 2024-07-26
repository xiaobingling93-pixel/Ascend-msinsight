#!/bin/bash
set -o errexit

dir_build_fuzz=$(pwd)
## need sudo
# Secodefuzz build only needed on first running
cd ${dir_build_fuzz}/../third_party/secodefuzz
bash build.sh gcc

cd ${dir_build_fuzz}
if [ -d ${dir_build_fuzz}/output ]; then
  rm -rf ${dir_build_fuzz}/output
fi
mkdir ${dir_build_fuzz}/output
cd ${dir_build_fuzz}/output
echo "*************** FUZZCODE  START *****************"
cmake -D_PROJECT_TYPE=fuzz ${dir_build_fuzz}/..
echo "*************** CMAKE  FINISH "
cpu_number=$(nproc)
make -j${cpu_number}
echo "*************** MAKE  FINISH "

cd ${dir_build_fuzz}
if [ -d ${dir_build_fuzz}/report ]; then
  rm -rf ${dir_build_fuzz}/report
fi
mkdir ${dir_build_fuzz}/report
cd ${dir_build_fuzz}/report
echo "*************** RUN insight_fuzz... "
system_arch=$(arch)
${dir_build_fuzz}/../output/linux-${system_arch}/bin/insight_fuzz > insight_fuzz_test_details.log
echo "**************** FUZZCODE  END ******************"
echo "Check more details in report/insight_fuzz_test_details.log"