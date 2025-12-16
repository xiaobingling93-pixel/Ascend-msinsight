#!/usr/bin/env bash

func() {
    echo "Usage:"
    echo "run.sh [-p TEST_CASE_PATH] [-f TEST_CASE_FILE_PATH] [-t TEST_CASE_TAGS] [-n TEST_CASE_NAME] [-d]"
    echo "Description:"
    echo "-p TEST_CASE_PATH,the path of test cases."
    echo "-f TEST_CASE_FILE_PATH,the path of test case file.It's got to be a case.yml file"
    echo "-t TEST_CASE_TAGS,type or tags of the test case to be run"
    echo "-n TEST_CASE_NAME,name of the test case to be run"
    echo "-d enable debug mode"
    exit 0
}

export PROJECT_PATH=$(realpath `dirname $0`/..)                               # 工程路径
PROJECT_RESOURCE_PATH=$PROJECT_PATH/resource                                  # 资源路径
PROJECT_TEST_CASE_PATH=$PROJECT_PATH/test-case                                # 默认用例
PROJECT_TEST_PY_SCRIPT=$PROJECT_PATH/smoke_modelslim/run.py                   # 运行python脚本
PROJECT_WORKSPACE_PATH=RunWorkspace/`date "+%Y-%m-%d_%H:%M:%S"`               # 运行工作路径(临时文件夹
PROJECT_LOG_PATH=$PROJECT_WORKSPACE_PATH/run.log                              # 运行日志文件


CANN_VERSION='poc'
TEST_CASE_PATH=$PROJECT_TEST_CASE_PATH
TEST_CASE_TYPE=
TEST_CASE_NAME=
DEV_MODE=0
while getopts 'p:t:n:f:hdv:m:' OPT; do
    case $OPT in
        p) TEST_CASE_PATH=`realpath "$OPTARG"`;;
        f) TEST_CASE_FILE_PATH=`realpath "$OPTARG"`;;
        t) TEST_CASE_TYPE="$OPTARG";;
        n) TEST_CASE_NAME="$OPTARG";;
        d) DEV_MODE=1;;
        h) func;;
        v) CANN_VERSION="$OPTARG";;
        m) CC_EMAIL="$OPTARG";;
    esac
done
echo 运行用例路径： $TEST_CASE_PATH
echo 运行用例： [$TEST_CASE_TYPE][$TEST_CASE_NAME]

# 创建工作路径
mkdir -p $PROJECT_WORKSPACE_PATH
echo 运行工作路径： $PROJECT_WORKSPACE_PATH
echo 运行工程路径：  $PROJECT_PATH
echo 运行模式：  $MSMODELSLIM_SMOKE_EXECUTION_MODE

# 设置环境变量
PYTHONPATH=$PROJECT_PATH:$PYTHONPATH
WORKSPACE_PATH=$PROJECT_WORKSPACE_PATH
python $PROJECT_TEST_PY_SCRIPT \
          --project_path=$PROJECT_PATH \
          --test-case=$TEST_CASE_PATH \
          --case-tags=$TEST_CASE_TYPE \
          --case-name=$TEST_CASE_NAME \
          --dev-mode=$DEV_MODE \
          --workspace-path=$PROJECT_WORKSPACE_PATH \
          --cann_version=$CANN_VERSION \
          --cc_email=$CC_EMAIL \
          --execution_mode=$MSMODELSLIM_SMOKE_EXECUTION_MODE \
          --resource-path=$PROJECT_RESOURCE_PATH 2>&1 |
tee $PROJECT_LOG_PATH

echo 运行工作路径： `realpath $PROJECT_WORKSPACE_PATH `
echo 运行日志文件： `realpath $PROJECT_LOG_PATH `