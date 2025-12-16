#!/bin/bash

# 脚本功能：
#   调用 core.sh 并传递参数
# 用法: 
#   1.填写参数(CANN路径、GIT仓路径、GIT仓分支、接收邮件的邮箱)
#   2.执行脚本

# 定义要传递的参数
# CANN路径，默认取一个新的，如果需要指定版本可自行更改
FOLDER=$(ls -r /path/smoke/CANN | head -1)
CANN_PATH="/path/smoke/CANN/${FOLDER}/ascend-toolkit/set_env.sh"
GIT_PATH="GIT仓路径"
GIT_BRANCH=="GIT仓分支"

# 接收邮件的邮箱 for example --> CC_EMAIL="example-mail"
CC_EMAIL="邮箱"
# （可选）指定可见显卡，不指定则将其注释，如开启该参数，必须指定两张显卡 例如： "0,1"（参数禁止存在空格）
ASCEND_RT_VISIBLE_DEVICES="0,1"
# 执行模式：
#    "ALL"值跑全量冒烟
#    "LAST_FAILED"为执行上一次成功全部执行完成后的失败用例（如上一次中断或其他意外情况，则不执行），注释或传其他值则默认前冒烟
#    "MS_V1"只执行modelslim_v1框架用例
EXECUTION_MODE="MS_V1"


#---------------------------------===-----------------------------------#
#-------------------------以下为系统处理，无需操作------------------------#
#---------------------------------===-----------------------------------#
# 要检测的文件列表（支持绝对路径/相对路径）
FILES=(
    "/path/before_smoke/core.sh"
    "/path/scripts/run-modelslim.sh"
)

# 定义错误处理函数
handle_error() {
    local file=$1
    local pids=$2
    echo -e "\033[31m错误：文件 \"$file\" 正在被以下进程占用：\033[0m"
    echo "  进程ID：$pids"
    echo "  进程详情：$(ps -p "$pids" -o pid,user,comm=)"
}

# 遍历文件列表进行检测
for file in "${FILES[@]}"; do
    # 检查文件是否存在
    if [[ ! -e "$file" ]]; then
        echo "警告：文件 \"$file\" 不存在，跳过检测。"
        continue
    fi
    
    # 获取占用进程ID（静默模式）
    pids=$(lsof -t "$file" 2>/dev/null)
    
    if [[ -n "$pids" ]]; then
        # 触发错误处理并退出脚本
        handle_error "$file" "$pids"
        echo -e "\n\033[33m提示：当前脚本已退出，请确认是否有其他人在进行冒烟\033[0m"
        exit 1  # 非零退出码表示检测到异常
    else
        echo "文件 \"$file\" 未被占用，可以继续操作。"
    fi
done

# 所有文件检测通过后的逻辑
echo -e "\n\033[32m所有文件均未被占用，程序继续执行...\033[0m"

# 当前脚本目录
SCRIPT_DIR=$(dirname "$(realpath "$0")")  # "

# 检查目标脚本是否存在
PROCESS_SCRIPT="$SCRIPT_DIR/core.sh"
if [ ! -f "$PROCESS_SCRIPT" ]; then
    echo "错误: 找不到脚本 $PROCESS_SCRIPT" >&2
    exit 1
fi

# 检查目标脚本是否可执行
if [ ! -x "$PROCESS_SCRIPT" ]; then
    echo "错误: 脚本 $PROCESS_SCRIPT 不可执行" >&2
    echo "请运行: chmod +x $PROCESS_SCRIPT" >&2
    exit 1
fi

# 调用脚本并传递参数
echo "正在调用脚本 $PROCESS_SCRIPT 并传递参数:"
echo

# 新的执行方式（参数顺序可任意调整）
"$PROCESS_SCRIPT" \
  --cann_path "$CANN_PATH" \
  --git_path "$GIT_PATH" \
  --git_branch "$GIT_BRANCH" \
  --cc_email "$CC_EMAIL" \
  --ascend_rt_visible_devices "$ASCEND_RT_VISIBLE_DEVICES" \
  --execution_mode "$EXECUTION_MODE"

# 检查执行状态
if [ $? -eq 0 ]; then
    echo -e "\n${GREEN}脚本执行成功${NC}"
else
    echo -e "\n${RED}脚本执行失败${NC}"
fi
