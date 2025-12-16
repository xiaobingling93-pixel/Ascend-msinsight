export ASCEND_GLOBAL_EVENT_ENABLE=1
export ASCEND_GLOBAL_LOG_LEVEL=0
export ASCEND_SLOG_PRINT_TO_STDOUT=1
source /root/miniconda3/bin/activate /root/miniconda3/envs/smoke_test_env_bak
python3 train.py 
