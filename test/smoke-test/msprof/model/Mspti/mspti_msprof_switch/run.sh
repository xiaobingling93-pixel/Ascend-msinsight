OUTPUT_PATH=$1

export ASCEND_GLOBAL_EVENT_ENABLE=1;
export ASCEND_SLOG_PRINT_TO_STDOUT=0;
export ASCEND_PROCESS_LOG_PATH=${OUTPUT_PATH}/plog;

export LD_PRELOAD=${ASCEND_HOME_PATH}/lib64/libmspti.so

export DB_LOGGER_ENABLE=1
# export OS_LOGGER_ENABLE=1

bash compile.sh
bash run_mspti.sh

mv ./activity_log_*.db ${OUTPUT_PATH}
mv ./result/* ${OUTPUT_PATH}
