#!/usr/bin/env bash

declare -i ret_ok=0
declare -i ret_failed=1
run_ok=$ret_ok

docker start smoke_wan22

# 执行m3异常值抑制用例
docker exec -i smoke_wan22 bash -c "ASCEND_RT_VISIBLE_DEVICES='$ASCEND_RT_VISIBLE_DEVICES' CANN_PATH='$CANN_PATH' PROJECT_PATH='$PROJECT_PATH' MSMODELSLIM_SOURCE_DIR='$MSMODELSLIM_SOURCE_DIR' $PROJECT_PATH/test-case/multimodal_sd_modelslim_v1_wan2_2_w8a8_mxfp8/quant_wan2_2.sh"

if [ $? -eq 0 ]
then
    echo multimodal_sd_modelslim_v1_wan2_2_w8a8_mxfp8: Success
else
    echo multimodal_sd_modelslim_v1_wan2_2_w8a8_mxfp8: Failed
    run_ok=$ret_failed
fi

docker stop smoke_wan22

exit $run_ok