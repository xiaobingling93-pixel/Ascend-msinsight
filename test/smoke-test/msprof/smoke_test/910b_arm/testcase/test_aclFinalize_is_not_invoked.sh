file_path="/home/msprof_smoke_test/model/VectorAndMatmul/aclFinalize_is_not_invoked/bin/vectorAndMatmul"
prof_path="/home/result_dir/test_aclFinalize_is_not_invoked"
rm -rf $prof_path
mkdir -p $prof_path
source /usr/local/Ascend/ascend-toolkit/set_env.sh
if [ -x $file_path ]; then
    msprof --output=$prof_path $file_path > $prof_path/plog.log
else
    echo "The binary file does not exist or cannot be executed."
    echo "test_aclFinalize_is_not_invoked fail" >> result.txt
    exit 0
fi

# 只校验是否存在profiling目录，不校验具体内容和解析日志;检查解析过程中打屏信息是否有ERROR
if find $prof_path -type d -name 'PROF_*' | grep -q .; then
    if grep -q -rn 'ERROR]' $prof_path/plog.log; then
        echo "test_aclFinalize_is_not_invoked fail" >> result.txt
    else
        echo "test_aclFinalize_is_not_invoked pass" >> result.txt
    fi
else
    echo "test_aclFinalize_is_not_invoked fail" >> result.txt
fi
