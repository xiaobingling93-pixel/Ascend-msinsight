#!/bin/bash
clear;clear
rm -rf *.vcd *.dump *.log *.bin *.o *.so *pu build output/*.bin input/*.bin
export PRINT_TIK_MEM_ACCESS=FALSE

CURRENT_DIR=$(
    cd $(dirname ${BASH_SOURCE:-$0})
    pwd
); cd $CURRENT_DIR

declare -A VersionMap
VersionMap["Ascend910A"]="Ascend910A"
VersionMap["Ascend910B"]="Ascend910A"
VersionMap["Ascend910ProA"]="Ascend910A"
VersionMap["Ascend910ProB"]="Ascend910A"
VersionMap["Ascend910PremiumA"]="Ascend910A"
VersionMap["Ascend310B1"]="Ascend310B1"
VersionMap["Ascend310P1"]="Ascend310P1"
VersionMap["Ascend310P3"]="Ascend310P1"
VersionMap["Ascend910B1"]="Ascend910B1"
VersionMap["Ascend910B2"]="Ascend910B1"
VersionMap["Ascend910B3"]="Ascend910B1"
VersionMap["Ascend910B4"]="Ascend910B1"
# legacy
VersionMap["ascend910"]="Ascend910A"
VersionMap["ascend310p"]="Ascend310P1"
VersionMap["ascend310B1"]="Ascend310B1"
VersionMap["ascend910B1"]="Ascend910B1"

if [ ! $ASCEND_HOME_DIR ]; then
    if [ -d "$HOME/Ascend/ascend-toolkit/latest" ]; then
        export ASCEND_HOME_DIR=$HOME/Ascend/ascend-toolkit/latest
    else
        export ASCEND_HOME_DIR=/usr/local/Ascend/ascend-toolkit/latest
    fi
fi
source $ASCEND_HOME_DIR/bin/setenv.bash

FILE_NAME="add"

function useage() {
    echo "eg.:"
    echo "INFO: bash run.sh SOC_VERSION RUN_MODE"
    echo "INFO: SOC_VERSION should be in [Ascend910A, Ascend310P1 or Ascend910B1]"
    echo "INFO: RUN_MODE    should be in [cpu npu_simulator npu_onboard]"
}

if [ $# != 2 ]; then
    echo "ERROR: Nedd two params."
    useage
    exit
fi

SOC_VERSION=$1
if [ ${SOC_VERSION}"x" = "x" ]; then
    echo "ERROR: SOC_VERSION should be in [Ascend910A, Ascend310P1 or Ascend910B1]"
    exit -1
fi

if [ ${VersionMap[$SOC_VERSION]}"x" = "Ascend910Ax" ] || [ ${VersionMap[$SOC_VERSION]}"x" = "Ascend310P1x" ] || [ ${VersionMap[$SOC_VERSION]}"x" = "Ascend310B1x" ]; then
    CORE_TYPE="AiCore"
elif [ ${VersionMap[$SOC_VERSION]}"x" = "Ascend910B1x" ]; then
    CORE_TYPE="VectorCore"
else
    echo "ERROR: SOC_VERSION should be in [Ascend910A, Ascend310P1 or Ascend910B1]"
    exit -1
fi

RUN_MODE=$2
if [ ${RUN_MODE}"x" = "x" ]; then
    echo "WARNING: RUN_MODE is not specified, using cpu as default."
    RUN_TARGET="cpu"
    RUN_MODE="CPU"
elif [ ${RUN_MODE}"x" == "cpux" ]; then
    RUN_TARGET="cpu"
    RUN_MODE="CPU"
elif [ ${RUN_MODE}"x" == "npu_simulatorx" ]; then
    RUN_TARGET="npu"
    RUN_MODE="SIMULATOR"
elif [ ${RUN_MODE}"x" == "npu_onboardx" ]; then
    RUN_TARGET="npu"
    RUN_MODE="ONBOARD"
else
    echo "ERROR: RUN_MODE error, This sample only support specify cpu, npu_simulator or npu_onboard!"
    exit -1
fi

python3 scripts/gen_data.py

function main() {
    cd $CURRENT_DIR; rm -rf build; mkdir -p build; cd build
    cmake ..                                                \
        -Dsmoke_testcase=${FILE_NAME}                       \
        -DASCEND_PRODUCT_TYPE=${VersionMap[$SOC_VERSION]}   \
        -DASCEND_CORE_TYPE=${CORE_TYPE}                     \
        -DASCEND_RUN_MODE=${RUN_MODE}                       \
        -DASCEND_INSTALL_PATH=${ASCEND_HOME_DIR}
    cmake --build . --target ${FILE_NAME}_${RUN_TARGET}
    if [ $? -ne 0 ]; then
        echo "ERROR: compile op on failed!"
        return 1
    fi
    cd -
    echo "INFO: compile op on ${RUN_MODE} succeed!"

    (export LD_LIBRARY_PATH=`pwd`:${ASCEND_HOME_DIR}/tools/simulator/${VersionMap[$SOC_VERSION]}/lib:$LD_LIBRARY_PATH && ./${FILE_NAME}_${RUN_TARGET})
    if [ $? -ne 0 ]; then
        echo "ERROR: execute op on ${RUN_MODE} failed!"
        return 1
    fi
    echo "INFO: execute op on ${RUN_MODE} succeed!"
    python3 scripts/verify_result.py output/output_z.bin output/golden.bin 
}
main
