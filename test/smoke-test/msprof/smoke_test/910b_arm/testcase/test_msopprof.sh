source /usr/local/Ascend/ascend-toolkit/set_env.sh
HELP_INFO=$(msprof --help)
subcommand="This is subcommand for operator optimization situation"
result=$(echo $HELP_INFO | grep "${subcommand}")
if [[ $result != "" ]]; then
    relative_path=/tools/msopt/bin/msopprof
    msopprof_path=$ASCEND_TOOLKIT_HOME$relative_path
    if [ ! -f $msopprof_path ]; then
        echo "test_msopprof fail" >> result.txt
    else
        echo "test_msopprof pass" >> result.txt
    fi
fi