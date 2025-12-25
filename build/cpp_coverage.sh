#!/bin/bash
# This script is used to generate llt-cpp coverage.
# -------------------------------------------------------------------------
# This file is part of the MindStudio project.
# Copyright (c) 2025 Huawei Technologies Co.,Ltd.
#
# MindStudio is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#
#          http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.
# -------------------------------------------------------------------------

set -e
CUR_DIR=$(dirname $(readlink -f $0))
TOP_DIR=${CUR_DIR}/..
cd ${TOP_DIR}/server/build
export LD_LIBRARY_PATH=${TOP_DIR}'/server/output/linux-x86_64/bin':${LD_LIBRARY_PATH}
python3 preprocess_third_party.py
python3 build.py test

#
COV_DIR=${TOP_DIR}/build_llt/output/cpp_coverage
BUILD_DIR=${TOP_DIR}/server/build

if [ ! -d "${COV_DIR}" ] ; then
    mkdir -p ${COV_DIR}
fi

cd ../output/linux-x86_64/bin

./insight_test
# 平台暂时不支持覆盖率，这里先屏蔽
#cd ${BUILD_DIR}
#echo ${BUILD_DIR}
#test_dir=$(find ${BUILD_DIR} -name 'insight_test.dir')
#echo ${test_dir}
#echo "********************** Generate insight_test Coverage Start.************************"
#LCOV_RC="--rc lcov_branch_coverage=1 --rc geninfo_no_exception_branch=1"
#lcov -c -d ${test_dir} -o ${COV_DIR}/lcov_insight_test.info ${LCOV_RC}
#lcov -r ${COV_DIR}/lcov_insight_test.info '*include*' -o ${COV_DIR}/lcov_insight_test.info ${LCOV_RC}
#lcov -r ${COV_DIR}/lcov_insight_test.info '*test*' -o ${COV_DIR}/lcov_insight_test.info ${LCOV_RC}
#lcov -r ${COV_DIR}/lcov_insight_test.info '*third_party*' -o ${COV_DIR}/lcov_insight_test.info ${LCOV_RC}
#echo "********************** Generate insight_test Coverage Stop.*************************"
#
#genhtml ${COV_DIR}/lcov_insight_test.info -o ${COV_DIR}/result --branch-coverage
#echo "report: ${COV_DIR}"
