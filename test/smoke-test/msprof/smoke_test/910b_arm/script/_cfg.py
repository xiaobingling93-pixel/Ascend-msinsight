# -*- coding: utf-8 -*-
import os.path


class ConfigPaths:
    # environment and common path
    log_level = "export ASCEND_GLOBAL_EVENT_ENABLE=1; " \
                "export ASCEND_GLOBAL_LOG_LEVEL=0; " \
                "export ASCEND_SLOG_PRINT_TO_STDOUT=1"
    python_env = "source /root/miniconda3/bin/activate /root/miniconda3/envs/smoke_test_env_bak"
    ascend_path = "/usr/local/Ascend"
    ascend_toolkit_path = "{}/ascend-toolkit".format(ascend_path)
    toolkit_env_path = "{}; source {}/set_env.sh".format(python_env, ascend_toolkit_path)
    # toolkit_env_path = "{}; source /usr/local/Ascend/CANN-6.4/bin/setenv.bash".format(python_env)
    latest_path = "{}/latest".format(ascend_toolkit_path)
    msprofbin_path = "{}/tools/profiler/bin/msprof".format(latest_path)
    analysis_path = "{}/tools/profiler/profiler_tool/analysis".format(latest_path)
    # msprofbin_path = "/usr/local/Ascend/CANN-6.4/tools/profiler/bin/msprof".format(latest_path)
    # analysis_path = "/usr/local/Ascend/CANN-6.4/tools/profiler/profiler_tool/analysis".format(latest_path)
    msprofpy_path = "{}/msprof/msprof.py".format(analysis_path)
    msprofinfo_path = "python3 {}/interface/get_msprof_info.py -dir".format(analysis_path)
    result_path = "/home/result_dir"
    model_path = "/home/msprof_smoke_test/model/"
    pytorch_lenet_path = os.path.join(model_path, "pytorch_lenet/")
    add_main_path = os.path.join(model_path, "resnet50_main/out/")
    aclapi_path = os.path.join(model_path, "AclApi")
    acljson_path = os.path.join(model_path, "AclJson")
    msproftx_path = os.path.join(model_path, "Msproftx")
    opst_path = os.path.join(model_path, "OpSt")
    graphapi_path = os.path.join(model_path, "AclGraphApi")
    pyaclapi_path = os.path.join(model_path, "PyAclApi")
    subscription_path = os.path.join(model_path, "AclApiSubscription")
    infer_path = os.path.join(model_path, "RunDevId0")
    addop_path = os.path.join(model_path, "AddOp/run")
    cannProfiling_path = os.path.join(model_path, "CannProfiling-Pytorch")
    exportTensorFlow_path = os.path.join(model_path, "Export-TensorFlow")
    dockerexportTensorFlow_path = os.path.join(model_path, "DockerExport-TensorFlow")
    msprofbinPytorch_path = os.path.join(model_path, "MsprofbinPyTorch")
    msprofbinTensorFlow_path = os.path.join(model_path, "MsprofbinTensorFlow")
    msprofbinMindSpore_path = os.path.join(model_path, "MsprofbinMindSpore")
    sessionrunTensorFlow_path = os.path.join(model_path, "SessionRun-TensorFlow")
    bertSquadPytorch_path = ""
    cpmFinetunePytorch_path = ""
    tacotron2Pytorch_path = ""
    autoAugmentTensorFlow_path = ""
    dcganTensorFlow_path = ""
    gcMcTensorFlow_path = ""
    dockerTensorFlow_path = ""
    innerOneStepPytorch_path = os.path.join(model_path, "Inner-One-Step_for_PyTorch")
    loopTensorFlow_path = os.path.join(model_path, "Loop_for_TensorFlow")
    noLoopTensorFlow_path = os.path.join(model_path, "No-Loop_for_TensorFlow")
    netMindSpore_path = ""
    resNet50MindSpore_path = ""
    singleOp65535Pytorch_path = os.path.join(model_path, "Single-Op-65535_for_PyTorch")
    singleOp65535PytorchDynamicLauch_path = os.path.join(model_path, "Single-Op-65535_for_PyTorch_Dynamic_Launch")
    singleOp65535PytorchDynamicAttach_path = os.path.join(model_path, "Single-Op-65535_for_PyTorch_Dynamic_Attach")
    unique_id_tf_path = os.path.join(model_path, "UniqueId-Tensorflow")
    singleOpPytorch_path = os.path.join(model_path, "Single-Op_for_PyTorch")
    stepPytorch_path = os.path.join(model_path, "Step_for_PyTorch")
    singleOpTensorFlow_path = ""
    yolov5MindSpore_path = ""
    fpBpTensorFlow_path = ""
    analysisFftsOffHcclL0_path = os.path.join(model_path, "Hccl_l0_ffts_off")
    analysisFftsOffHcclL1_path = os.path.join(model_path, "Hccl_l1_ffts_off")
    analysisFftsOnHcclL0_path = os.path.join(model_path, "Hccl_l0_ffts_on")
    analysisFftsOnHcclL1_path = os.path.join(model_path, "Hccl_l1_ffts_on")
    analysisHelper_path = os.path.join(model_path, "Helper")
    analysisAnalyzeCommunication_path = os.path.join(model_path, "Analyze_Communication")
    nanoPipeUtilization_path = os.path.join(model_path, "Analysis_NANO_PipeUtilization")
    nanoMemory_path = os.path.join(model_path, "Analysis_NANO_Memory")
    nanoMemoryUB_path = os.path.join(model_path, "Analysis_NANO_MemoryUB")
    nanoPipeStallCycle_path = os.path.join(model_path, "Analysis_NANO_PipeStallCycle")
    nanoScalarRatio_path = os.path.join(model_path, "Analysis_NANO_ScalarRatio")
    AscendC_AddKernelInvocation_L1_path = os.path.join(model_path, "AscendC_OP", "AddKernelInvocationL1")
    AscendC_AddKernelInvocation_L0_path = os.path.join(model_path, "AscendC_OP", "AddKernelInvocationL0")
    parse_performance_path = os.path.join(model_path,
                                          "ParsePerformance/PROF_000001_20231010105807679_GRRHDCGNICIIQCPA")
    analysisHcclGraph_path = os.path.join(model_path, "Hccl_graph")
    sampleSingleOpHcclFftsffOff_path = os.path.join(model_path, "sampleSingleOpHcclFftsffOff")
    sampleSingleOpHcclFftsffOn_path = os.path.join(model_path, "sampleSingleOpHcclFftsffOn")
    sampleGraphHcclFftsOff_path = os.path.join(model_path, "sampleGraphHcclFftsoff")
    sampleGraphHcclFftsOn_path = os.path.join(model_path, "sampleGraphHcclFftsOn")
    ImageNetTrain = os.path.join(model_path, "ModelZoo-PyTorch/PyTorch/contrib/cv/classification/ResNeXt-50-32x4d_ID1624_for_PyTorch/test/")
    MindSporeLlama2 = os.path.join(model_path,
                                 "/home/msprof_smoke_test/model/mindformers/scripts/")
    PytorchLlama2Single = os.path.join(model_path, "/home/msprof_smoke_test/model/op_test/01-Inference/LLAMA2/LLAMA2_ID4322_for_PyTorch/")
    PytorchLlama2Graph = os.path.join(model_path,
                                       "/home/msprof_smoke_test/model/op_test/01-Inference/LLAMA2/LLAMA2_ID4322_for_PyTorch_graph/")
    op_cost_baseline_path = "/home/msprof_smoke_test/smoke_test/910b_arm/baseline/op_meantime_baseline_910b.json"
    op_statistic_json_path = "/home/msprof_smoke_test/compare_profiling/op_statistic_avg_time.json"

    mstx_profiling_msprof_path = os.path.join(model_path, "Mstx_Profiling", "msprofbin")
    mstx_profiling_acljson_path = os.path.join(model_path, "Mstx_Profiling", "acljson")
    mstx_profiling_aclapi_path = os.path.join(model_path, "Mstx_Profiling", "aclapi")
    mstx_profiling_pytorch_domain_include_path = os.path.join(model_path, "Mstx_Profiling", "pytorch_domain_include")
    mstx_profiling_pytorch_domain_exclude_path = os.path.join(model_path, "Mstx_Profiling", "pytorch_domain_exclude")
    mspti_base_testcase_path = os.path.join(model_path, "Mspti")

    msprof_info_cluster_path = os.path.join(model_path, "msprofInfoCluster")
    msprof_info_non_cluster_path = os.path.join(model_path, "msprofInfoNonCluster/device_0")
    ModellinkLlama2 = os.path.join(model_path, "ModelLink")

    multi_thread_use_npu_mem_path = os.path.join(model_path, "MulThreadUseNpuMem")
    multi_thread_setdevice_in_dynmode = os.path.join(model_path, "MulThreadUseSetDeviceInDynMode")

    analysis_a3_path = os.path.join(model_path, "Analysis_A3")

    mspti_domain_test_path = os.path.join(model_path, "mspti_domain")
    mspti_domain_c_path = os.path.join(mspti_domain_test_path, "cpp")
    mspti_domain_pta_path = os.path.join(mspti_domain_test_path, "pta")
    mspti_domain_ms_path = os.path.join(mspti_domain_test_path, "ms")

    mspti_sample_path = os.path.join(ascend_toolkit_path, "latest/tools/mspti/samples")
    mspti_communication_path = os.path.join(mspti_base_testcase_path, "mspti_communication")
    mspti_graph_path = os.path.join(mspti_base_testcase_path, "mspti_graph")

class ConfigValues:
    # common value
    dev_id = 0
    pass_res = "pass"
    fail_res = "fail"
    timeline = "timeline"
    summary = "summary"
    data = "data"
    host = "host"
    device = "device"
    aclapi_switch = ["ACL_PROF_ACL_API=on", "ACL_PROF_TASK_TIME=on", "ACL_PROF_AICPU=on", "ACL_PROF_L2CACHE=on",
                     "ACL_PROF_RUNTIME_API=on", "ACL_PROF_SYS_HARDWARE_MEM_FREQ=on", "ACL_PROF_LLC_MODE=read",
                     "ACL_PROF_SYS_IO_FREQ=on", "ACL_PROF_SYS_INTERCONNECTION_FREQ=on", "ACL_PROF_DVPP_FREQ=on",
                     "ACL_PROF_HOST_SYS=cpu,mem", "ACL_PROF_HOST_SYS_USAGE=cpu,mem",
                     "ACL_PROF_HOST_SYS_USAGE_FREQ=cpu,mem", "ACL_PROF_HCCL_TRACE=off", "ACL_PROF_MSPROFTX=off"]
    acljson_switch = ["aicpu=on", "l2=on", "hccl=off", "task_time=on", "ascendcl=on", "runtime_api=on",
                      "sys_hardware_mem_freq=on", "llc_profiling=write", "sys_io_sampling_freq=on",
                      "sys_interconnection_freq=on", "dvpp_freq=on", "host_sys=cpu,mem", "host_sys_usage=cpu,mem",
                      "host_sys_usage_freq=cpu,mem", "msproftx=off"]
    msproftx_switch = ["ACL_PROF_TASK_TIME=on", "ACL_PROF_MSPROFTX=on"]
    opst_switch = ["ascendcl=on", "task_time=on", "runtime_api=on"]
    pyaclapi_switch = ["ACL_PROF_ACL_API=on", "ACL_PROF_TASK_TIME=on", "ACL_PROF_AICPU=on", "ACL_PROF_L2CACHE=on",
                       "ACL_PROF_RUNTIME_API=on", "ACL_PROF_HCCL_TRACE=off", "ACL_PROF_MSPROFTX=off"]
    tensorflow_switch = ["hccl=off", "aicpu=on", "l2=on", "msproftx=off", "task_time=on", "runtime_api=on",
                         "sys_hardware_mem_freq=on", "llc_profiling=write", "sys_io_sampling_freq=on",
                         "sys_interconnection_freq=on", "dvpp_freq=on", "host_sys=cpu,mem", "host_sys_usage=cpu,mem",
                         "host_sys_usage_freq=cpu,mem", "training_trace=on", "task_trace=on"]
    pytorch_switch = ["ACL_PROF_ACL_API=on", "ACL_PROF_TASK_TIME=on", "ACL_PROF_AICPU=on", "ACL_PROF_L2CACHE=on",
                      "ACL_PROF_HCCL_TRACE=off", "ACL_PROF_TRAINING_TRACE=on", "ACL_PROF_MSPROFTX=on",
                      "TORCH_CALL_STACK=on", "ACL_PROF_RUNTIME_API=on"]
    mindspore_switch = []
    msprofbin_switch = ["ascendcl=on", "model-execution=on", "runtime-api=on", "task-time=on", "aicpu=on",
                        "sys-hardware-mem=on", "sys-cpu-profiling=on", "sys-profiling=on", "sys-pid-profiling=on",
                        "sys-io-profiling=on", "dvpp-profiling=on", "llc-profiling=read", "aic-mode=task-based",
                        "aic-metrics=PipeUtilization", "sys_interconnection_freq=on", "l2=on"]
    graphapi_switch = ["ACL_PROF_TRAINING_TRACE=on", "ACL_PROF_AICPU=on", "ACL_PROF_L2CACHE=on",
                       "ACL_PROF_HCCL_TRACE=off", "ACL_PROF_TASK_TIME=on"]


class ConfigCmd:
    app_switch = "--application='{}/model/main --model {}/model/tf_resnet50.om --loop 1'".format(ConfigPaths.infer_path, ConfigPaths.infer_path)
    app_end_switch = "./main"
    perf_iotop_cmd = "sudo msprof_data_collection.sh get-version perf; " \
                     "sudo msprof_data_collection.sh get-version iotop;"
    msprof_parse = "{}; {}; msprof --export=on --output=".format(ConfigPaths.log_level, ConfigPaths.toolkit_env_path)
    # common cmd
    all_switch = "{}; {}; " \
                 "{} --ascendcl=on --model-execution=on --runtime-api=on --task-time=on --ai-core=on --l2=on " \
                 "--aic-freq=100 --aicpu=on --sys-hardware-mem=on --sys-hardware-mem-freq=50 --sys-cpu-profiling=on " \
                 "--sys-cpu-freq=50 --sys-profiling=on --sys-sampling-freq=10 --sys-pid-profiling=on " \
                 "--sys-pid-sampling-freq=10 --sys-io-profiling=on --sys-io-sampling-freq=100 " \
                 "--sys-interconnection-profiling=on --sys-interconnection-freq=50 --dvpp-profiling=on " \
                 "--dvpp-freq=50 {}".format(ConfigPaths.log_level, ConfigPaths.toolkit_env_path,
                                            ConfigPaths.msprofbin_path, app_switch)
    all_sys_switch = "{}; {}; " \
                     "{} --sys-period=10 --sys-devices=all --ai-core=on --aic-freq=100 --sys-hardware-mem=on " \
                     "--sys-hardware-mem-freq=50 --sys-cpu-profiling=on --sys-cpu-freq=50 --sys-profiling=on " \
                     "--sys-sampling-freq=10 --sys-pid-profiling=on --sys-pid-sampling-freq=10 --sys-io-profiling=on " \
                     "--sys-io-sampling-freq=100 --sys-interconnection-profiling=on --sys-interconnection-freq=50 " \
                     "--dvpp-profiling=on --dvpp-freq=50".format(ConfigPaths.log_level, ConfigPaths.toolkit_env_path,
                                                                 ConfigPaths.msprofbin_path)
    all_app_switch = "{}; {}; " \
                     "{} --ascendcl=on --model-execution=on --runtime-api=on --task-time=on --ai-core=on --l2=on " \
                     "--aic-freq=100 --aicpu=on --sys-hardware-mem=on {}".format(ConfigPaths.log_level,
                                                                                 ConfigPaths.toolkit_env_path,
                                                                                 ConfigPaths.msprofbin_path, app_switch)
    single_sys_switch = "{}; {}; " \
                        "{} --sys-period=10 --sys-devices=all --ai-core=off --sys-hardware-mem=off " \
                        "--sys-cpu-profiling=off --sys-profiling=off --sys-pid-profiling=off --sys-io-profiling=off " \
                        "--dvpp-profiling=off --sys-interconnection-profiling=off ".format(ConfigPaths.log_level,
                                                                                           ConfigPaths.toolkit_env_path,
                                                                                           ConfigPaths.msprofbin_path,
                                                                                           ConfigValues.dev_id)
    single_app_end_switch = "{}; {}; " \
                     "{}".format(ConfigPaths.log_level, ConfigPaths.toolkit_env_path,
                                     ConfigPaths.msprofbin_path)
    single_app_switch = "{}; {}; " \
                        "{} --ascendcl=off --model-execution=off --runtime-api=off --task-time=off --ai-core=off " \
                        "--aicpu=off --dvpp-profiling=off --l2=off {}".format(ConfigPaths.log_level,
                                                                              ConfigPaths.toolkit_env_path,
                                                                              ConfigPaths.msprofbin_path, app_switch)

    single_analyze_switch = "{}; {}" \
                            "{} --analyze=on".format(ConfigPaths.log_level, ConfigPaths.toolkit_env_path,
                                                     ConfigPaths.msprofbin_path)

    docker_cmd = "docker run --shm-size 8g -d --device=/dev/davinci_manager " \
                 "--device=/dev/davinci0 --device=/dev/devmm_svm --device=/dev/hisi_hdc " \
                 "-v {0}/driver/lib64/:{0}/driver/lib64/ " \
                 "-v {0}/driver/tools/:{0}/driver/tools/ " \
                 "-v /usr/local/python3.7.5:/usr/local/python3.7.5 " \
                 "-v {0}:{0}".format(ConfigPaths.ascend_path)

    perf_cmd = "{}; {}; " \
                 "{} --export=on --output={}".format(ConfigPaths.log_level, ConfigPaths.toolkit_env_path,
                                                             ConfigPaths.msprofbin_path, ConfigPaths.parse_performance_path)


class ConfigTableHeaders:
    table_header = {
        "acl": "Name,Type,Start Time,Duration(us),Process ID,Thread ID",
        "api_statistic": "Level,API Name,Time(us),Count,Avg(us),Min(us),Max(us),Variance",
        "acl_statistic": "Name,Type,Time(%),Time(us),Count,Avg(us),Min(us),Max(us),Process ID,Thread ID",
        "aicpu": "Timestamp(us),Node,Compute_time(us),Memcpy_time(us),Task_time(us),Dispatch_time(us),Total_time(us),"
                 "Stream ID,Task ID",
        "ai_core_utilization": {
            "sample-based": {
                "ArithmeticUtilization": "Core ID,mac_fp16_ratio,mac_int8_ratio,vec_fp32_ratio,vec_fp16_ratio,"
                                         "vec_int32_ratio,vec_misc_ratio,cube_fops,vector_fops",
                "L2Cache": "Core ID,write_cache_hit,write_cache_miss_allocate,r0_read_cache_hit,"
                           "r0_read_cache_miss_allocate,r1_read_cache_hit,r1_read_cache_miss_allocate",
                "Memory": "Core ID,ub_read_bw(GB/s),ub_write_bw(GB/s),l1_read_bw(GB/s),l1_write_bw(GB/s),"
                          "l2_read_bw(GB/s),l2_write_bw(GB/s),main_mem_read_bw(GB/s),main_mem_write_bw(GB/s)",
                "MemoryL0": "Core ID,l0a_read_bw(GB/s),l0a_write_bw(GB/s),l0b_read_bw(GB/s),l0b_write_bw(GB/s),"
                            "l0c_read_bw(GB/s),l0c_write_bw(GB/s),l0c_read_bw_cube(GB/s),l0c_write_bw_cube(GB/s)",
                "MemoryUB": "Core ID,ub_read_bw_vector(GB/s),ub_write_bw_vector(GB/s),ub_read_bw_scalar(GB/s),"
                            "ub_write_bw_scalar(GB/s)",
                "MemoryUB_": "Core ID,vec_ratio,mac_ratio,scalar_ratio,mte1_ratio,mte2_ratio,mte3_ratio,"
                             "icache_miss_rate,memory_bound",
                "PipeUtilization": "Core ID,mac_ratio,scalar_ratio,mte1_ratio,mte2_ratio,fixpipe_ratio,icache_miss_rate",
                "ResourceConflictRatio": "Core ID,vec_bankgroup_cflt_ratio,vec_bank_cflt_ratio,vec_resc_cflt_ratio",
            },
            "task-based": {
                "ArithmeticUtilization": "Core ID,mac_fp16_ratio,mac_int8_ratio,vec_fp32_ratio,vec_fp16_ratio,"
                                         "vec_int32_ratio,vec_misc_ratio,cube_fops,vector_fops",
                "L2Cache": "Core ID,write_cache_hit,write_cache_miss_allocate,r0_read_cache_hit,"
                           "r0_read_cache_miss_allocate,r1_read_cache_hit,r1_read_cache_miss_allocate",
                "Memory": "Core ID,ub_read_bw(GB/s),ub_write_bw(GB/s),l1_read_bw(GB/s),l1_write_bw(GB/s),"
                          "main_mem_read_bw(GB/s),main_mem_write_bw(GB/s)",
                "MemoryL0": "Core ID,l0a_read_bw(GB/s),l0a_write_bw(GB/s),l0b_read_bw(GB/s),l0b_write_bw(GB/s),"
                            "l0c_read_bw(GB/s),l0c_write_bw(GB/s),l0c_read_bw_cube(GB/s),l0c_write_bw_cube(GB/s)",
                "MemoryUB": "Core ID,aiv_ub_read_bw_vector(GB/s),aiv_ub_write_bw_vector(GB/s),aiv_ub_read_bw_scalar(GB/s),"
                            "aiv_ub_write_bw_scalar(GB/s)",
                "MemoryUB_": "Core ID,vec_ratio,mac_ratio,scalar_ratio,mte1_ratio,mte2_ratio,mte3_ratio,"
                             "icache_miss_rate,memory_bound",
                "PipeUtilization": "Core ID,vec_ratio,mac_ratio,scalar_ratio,mte1_ratio,mte2_ratio,mte3_ratio,"
                                   "icache_miss_rate,memory_bound",
                "ResourceConflictRatio": "Core ID,vec_bankgroup_cflt_ratio,vec_bank_cflt_ratio,vec_resc_cflt_ratio",
            },
        },
        "ai_vector_core_utilization": {
            "sample-based": {
                "ArithmeticUtilization": "Core ID,mac_fp16_ratio,mac_int8_ratio,vec_fp32_ratio,vec_fp16_ratio,"
                                         "vec_int32_ratio,vec_misc_ratio,cube_fops,vector_fops",
                "L2Cache": "Core ID,write_cache_hit,write_cache_miss_allocate,r0_read_cache_hit,"
                           "r0_read_cache_miss_allocate,r1_read_cache_hit,r1_read_cache_miss_allocate",
                "Memory": "Core ID,ub_read_bw(GB/s),ub_write_bw(GB/s),l1_read_bw(GB/s),l1_write_bw(GB/s),"
                          "l2_read_bw(GB/s),l2_write_bw(GB/s),main_mem_read_bw(GB/s),main_mem_write_bw(GB/s)",
                "MemoryL0": "Core ID,l0a_read_bw(GB/s),l0a_write_bw(GB/s),l0b_read_bw(GB/s),l0b_write_bw(GB/s),"
                            "l0c_read_bw(GB/s),l0c_write_bw(GB/s),l0c_read_bw_cube(GB/s),l0c_write_bw_cube(GB/s)",
                "MemoryUB_": "Core ID,vec_ratio,mac_ratio,scalar_ratio,mte1_ratio,mte2_ratio,mte3_ratio,"
                             "icache_miss_rate,memory_bound",
                "MemoryUB": "Core ID,ub_read_bw_vector(GB/s),ub_write_bw_vector(GB/s),ub_read_bw_scalar(GB/s),"
                            "ub_write_bw_scalar(GB/s)",
                "PipeUtilization": "Core ID,vec_ratio,mac_ratio,scalar_ratio,mte1_ratio,mte2_ratio,mte3_ratio,"
                                   "icache_miss_rate,memory_bound",
                "ResourceConflictRatio": "Core ID,vec_bankgroup_cflt_ratio,vec_bank_cflt_ratio,vec_resc_cflt_ratio",
            },
            "task-based": {
                "ArithmeticUtilization": "Core ID,mac_fp16_ratio,mac_int8_ratio,vec_fp32_ratio,vec_fp16_ratio,"
                                         "vec_int32_ratio,vec_misc_ratio,cube_fops,vector_fops",
                "Memory": "Core ID,ub_read_bw(GB/s),ub_write_bw(GB/s),l1_read_bw(GB/s),l1_write_bw(GB/s),"
                          "main_mem_read_bw(GB/s),main_mem_write_bw(GB/s)",
                "MemoryL0": "Core ID,l0a_read_bw(GB/s),l0a_write_bw(GB/s),l0b_read_bw(GB/s),l0b_write_bw(GB/s),"
                            "l0c_read_bw(GB/s),l0c_write_bw(GB/s),l0c_read_bw_cube(GB/s),l0c_write_bw_cube(GB/s)",
                "MemoryUB_": "Core ID,vec_ratio,mac_ratio,scalar_ratio,mte1_ratio,mte2_ratio,mte3_ratio,"
                             "icache_miss_rate,memory_bound",
                "MemoryUB": "Core ID,ub_read_bw_vector(GB/s),ub_write_bw_vector(GB/s),ub_read_bw_scalar(GB/s),"
                            "ub_write_bw_scalar(GB/s)",
                "PipeUtilization": "Core ID,vec_ratio,mac_ratio,scalar_ratio,mte1_ratio,mte2_ratio,mte3_ratio,"
                                   "icache_miss_rate,memory_bound",
                "ResourceConflictRatio": "Core ID,vec_bankgroup_cflt_ratio,vec_bank_cflt_ratio,vec_resc_cflt_ratio",
            },
        },
        "ai_cpu_pmu_events": "Event,Name,Count",
        "ai_cpu_top_function": "Function,Module,Cycles,Cycles(%)",
        "ai_stack_time": "Infer ID,Module,API,Start Time,Duration(ns)",
        "cpu_usage": "Cpu Type,User(%),Sys(%),IoWait(%),Irq(%),Soft(%),Idle(%)",
        "ctrl_cpu_pmu_events": "Event,Name,Count",
        "ctrl_cpu_top_function": "Function,Module,Cycles,Cycles(%)",
        "ddr": "Metric,Read(MB/s),Write(MB/s)",
        "dp": "Timestamp(us),Action,Source,Cached Buffer Size",
        "dvpp": "Dvpp Id,Engine Type,Engine ID,All Time(us),All Frame,All Utilization(%)",
        "fusion_op": "Model Name,Model ID,Fusion Op,Original Ops,Memory Input(KB),Memory Output(KB),"
                     "Memory Weight(KB),Memory Workspace(KB),Memory Total(KB)",
        "ge_op_execute": "Thread ID,OP Name,OP Type,Event Type,Start Time,Duration(us)",
        "hbm": "Metric,Read(MB/s),Write(MB/s)",
        "hccs": "Mode,Max,Min,Average",
        "host_cpu_usage": "Total Cpu Numbers,Occupied Cpu Numbers,Recommend Cpu Numbers",
        "host_disk_usage": "Peak Disk Read(KB/s),Recommend Disk Read(KB/s),Peak Disk Write(KB/s),"
                           "Recommend Disk Write(KB/s)",
        "host_mem_usage": "Total Memory(KB),Peak Used Memory(KB),Recommend Memory(KB)",
        "host_network_usage": "Netcard Speed(KB/s),Peak Used Speed(KB/s),Recommend Speed(KB/s)",
        "llc_aicpu": "Metric,CPU0(KB),CPU1(KB),CPU2(KB),CPU3(KB),Total(KB)",
        "llc_bandwidth": "Metric,l3c_rd,l3c_wr",
        "llc_ctrlcpu": "Metric,CPU0(KB),CPU1(KB),CPU2(KB),CPU3(KB),Total(KB)",
        "llc_read_write": "Mode,Task,Hit Rate(%),Throughput(MB/s)",
        "l2_cache": "Stream Id,Task Id,Hit Rate,Victim Rate,Op Name",
        "msprof_tx": "pid,tid,category,event_type,payload_type,payload_value,Start_time(us),End_time(us),"
                     "message_type,message,domain,Device Start_time(us),Device End_time(us)",
        "nic": "Timestamp(us),Bandwidth(MB/s),Rx Bandwidth efficiency(%),rxPacket/s,rxError rate(%),rxDropped rate(%),"
               "Tx Bandwidth efficiency(%),txPacket/s,txError rate(%),txDropped rate(%),funcId",
        "npu_mem": "event,ddr(KB),hbm(KB),memory(KB),timestamp(us)",
        "npu_module_mem": "Component,Timestamp(us),Total Reserved(KB),Device",
        "op_statistic": {"WholeNetwork": "Model Name,OP Type,Core Type,Count,Total Time(us),Min Time(us),Avg Time(us),"
                                         "Max Time(us),Ratio(%)",
                         "AllData": "OP Type,Core Type,Count,Total Time(us),Min Time(us),Avg Time(us),Max Time(us),"
                                    "Ratio(%)"},
        "op_summary": {
            "sample-based": {
                "ArithmeticUtilization": "Model ID,Task ID,Stream ID,Op Name,OP Type,OP State,"
                                         "Task Type,Task Start Time(us),Task Duration(us),Task Wait Time(us),Block Dim,Mix Block Dim,"
                                         "HF32 Eligible,Input Shapes,Input Data Types,Input Formats,Output Shapes,Output Data Types,"
                                         "Output Formats,Context ID",
                "L2Cache": "Model ID,Task ID,Stream ID,Op Name,OP Type,OP State,Task Type,"
                           "Task Start Time(us),Task Duration(us),Task Wait Time(us),Block Dim,Mix Block Dim,"
                           "HF32 Eligible,Input Shapes,Input Data Types,Input Formats,Output Shapes,Output Data Types,Output Formats,Context ID",
                "Memory": "Model ID,Task ID,Stream ID,Op Name,OP Type,OP State,Task Type,"
                          "Task Start Time(us),Task Duration(us),Task Wait Time(us),Block Dim,Mix Block Dim,HF32 Eligible,Input Shapes,"
                          "Input Data Types,Input Formats,Output Shapes,Output Data Types,Output Formats,Context ID",
                "MemoryL0": "Model ID,Task ID,Stream ID,Op Name,OP Type,OP State,Task Type,"
                            "Task Start Time(us),Task Duration(us),Task Wait Time(us),Block Dim,Mix Block Dim,HF32 Eligible,Input Shapes,"
                            "Input Data Types,Input Formats,Output Shapes,Output Data Types,Output Formats,Context ID",
                "MemoryUB": "Model ID,Task ID,Stream ID,Op Name,OP Type,OP State,Task Type,"
                            "Task Start Time(us),Task Duration(us),Task Wait Time(us),Block Dim,Mix Block Dim,HF32 Eligible,Input Shapes,"
                            "Input Data Types,Input Formats,Output Shapes,Output Data Types,Output Formats,Context ID",
                "PipeUtilization": "Model ID,Task ID,Stream ID,Op Name,OP Type,OP State,Task Type,"
                                   "Task Start Time(us),Task Duration(us),Task Wait Time(us),Block Dim,Mix Block Dim,HF32 Eligible,Input Shapes,"
                                   "Input Data Types,Input Formats,Output Shapes,Output Data Types,Output Formats,Context ID",
                "ResourceConflictRatio": "Model ID,Task ID,Stream ID,Op Name,OP Type,OP State,Task Type,"
                                         "Task Start Time(us),Task Duration(us),Task Wait Time(us),Block Dim,Mix Block Dim,"
                                         "HF32 Eligible,Input Shapes,Input Data Types,Input Formats,Output Shapes,Output Data Types,"
                                         "Output Formats,Context ID",
            },
            "task-based": {
                "ArithmeticUtilization": "Model ID,Task ID,Stream ID,Op Name,OP Type,OP State,Task Type"
                                         ",Task Start Time(us),Task Duration(us),Task Wait Time(us),Block Dim,Mix Block Dim,"
                                         "HF32 Eligible,Input Shapes,Input Data Types,Input Formats,Output Shapes,Output Data Types,"
                                         "Output Formats,Context ID,aicore_time(us),aic_total_cycles,aic_mac_fp16_ratio,aic_mac_int8_ratio,"
                                         "aic_cube_fops,aiv_time(us),aiv_total_cycles,aiv_vec_fp32_ratio,"
                                         "aiv_vec_fp16_ratio,aiv_vec_int32_ratio,aiv_vec_misc_ratio,aiv_vector_fops",
                "L2Cache": "Model ID,Task ID,Stream ID,Op Name,OP Type,OP State,Task Type,"
                           "Task Start Time(us),Task Duration(us),Task Wait Time(us),Block Dim,Mix Block Dim,"
                           "HF32 Eligible,Input Shapes,Input Data Types,Input Formats,Output Shapes,Output Data Types,Output Formats,Context ID,"
                           "aicore_time(us),aic_total_cycles,aic_write_cache_hit,"
                           "aic_write_cache_miss_allocate,aic_r0_read_cache_hit,"
                           "aic_r0_read_cache_miss_allocate,aic_r1_read_cache_hit,aic_r1_read_cache_miss_allocate,"
                           "aiv_time(us),aiv_total_cycles,aiv_write_cache_hit,aiv_write_cache_miss_allocate,"
                           "aiv_r0_read_cache_hit,aiv_r0_read_cache_miss_allocate,aiv_r1_read_cache_hit,"
                           "aiv_r1_read_cache_miss_allocate",
                "Memory": "Model ID,Task ID,Stream ID,Op Name,OP Type,OP State,Task Type,"
                          "Task Start Time(us),Task Duration(us),Task Wait Time(us),Block Dim,Mix Block Dim,HF32 Eligible,Input Shapes,"
                          "Input Data Types,Input Formats,Output Shapes,Output Data Types,"
                          "Output Formats,Context ID,aicore_time(us),"
                          "aic_total_cycles,aic_l1_read_bw(GB/s),aic_l1_write_bw(GB/s),aic_main_mem_read_bw(GB/s),aic_main_mem_write_bw(GB/s),"
                          "aic_l2_read_bw(GB/s),aic_l2_write_bw(GB/s),aiv_time(us),aiv_total_cycles,aiv_ub_read_bw(GB/s),aiv_ub_write_bw(GB/s),aiv_main_mem_read_bw(GB/s),"
                          "aiv_main_mem_write_bw(GB/s),aiv_l2_read_bw(GB/s),aiv_l2_write_bw(GB/s)",
                "MemoryL0": "Model ID,Task ID,Stream ID,Op Name,OP Type,OP State,Task Type,"
                            "Task Start Time(us),Task Duration(us),Task Wait Time(us),Block Dim,Mix Block Dim,HF32 Eligible,Input Shapes,"
                            "Input Data Types,Input Formats,Output Shapes,Output Data Types,Output Formats,Context ID,"
                            "aicore_time(us),aic_total_cycles,aic_l0a_read_bw(GB/s),aic_l0a_write_bw(GB/s),aic_l0b_read_bw(GB/s),"
                            "aic_l0b_write_bw(GB/s),aic_l0c_read_bw_cube(GB/s),aic_l0c_write_bw_cube(GB/s),aiv_time(us),"
                            "aiv_total_cycles,aiv_l0c_read_bw(GB/s),aiv_l0c_write_bw(GB/s)",
                "MemoryUB": "Model ID,Task ID,Stream ID,Op Name,OP Type,OP State,Task Type,"
                            "Task Start Time(us),Task Duration(us),Task Wait Time(us),Block Dim,Mix Block Dim,HF32 Eligible,Input Shapes,"
                            "Input Data Types,Input Formats,Output Shapes,Output Data Types,Output Formats,Context ID,"
                            "aicore_time(us),aic_total_cycles,aic_ub_read_bw_scalar(GB/s),aic_ub_write_bw_scalar(GB/s),"
                            "aiv_time(us),aiv_total_cycles,aiv_ub_read_bw_vector(GB/s),aiv_ub_write_bw_vector(GB/s),"
                            "aiv_ub_read_bw_scalar(GB/s),aiv_ub_write_bw_scalar(GB/s)",
                "PipeUtilization": "Model ID,Task ID,Stream ID,Op Name,OP Type,OP State,Task Type,"
                                   "Task Start Time(us),Task Duration(us),Task Wait Time(us),Block Dim,Mix Block Dim,HF32 Eligible,Input Shapes,"
                                   "Input Data Types,Input Formats,Output Shapes,Output Data Types,Output Formats,Context ID,"
                                   "aicore_time(us),aic_total_cycles,aic_mac_time(us),aic_mac_ratio,aic_scalar_time(us),aic_scalar_ratio,"
                                  "aic_mte1_time(us),aic_mte1_ratio,aic_mte2_time(us),aic_mte2_ratio,aic_fixpipe_time(us),"
                                  "aic_fixpipe_ratio,aic_icache_miss_rate,aiv_time(us),aiv_total_cycles,aiv_vec_time(us),"
                                  "aiv_vec_ratio,aiv_scalar_time(us),aiv_scalar_ratio,aiv_mte2_time(us),aiv_mte2_ratio,aiv_mte3_time(us),"
                                  "aiv_mte3_ratio,aiv_icache_miss_rate,cube_utilization(%)",
                "PipeUtilization_not_compatible": "Model Name,Model ID,Task ID,Stream ID,Infer ID,Op Name,OP Type,OP State,Task Type,"
                                   "Task Start Time(us),Task Duration(us),Task Wait Time(us),Block Dim,Mix Block Dim,HF32 Eligible,Input Shapes,"
                                   "Input Data Types,Input Formats,Output Shapes,Output Data Types,Output Formats,Context ID,"
                                   "aicore_time(us),aic_total_cycles,aic_mac_time(us),aic_mac_ratio,aic_scalar_time(us),aic_scalar_ratio,"
                                  "aic_mte1_time(us),aic_mte1_ratio,aic_mte2_time(us),aic_mte2_ratio,aic_fixpipe_time(us),"
                                  "aic_fixpipe_ratio,aic_icache_miss_rate,aiv_time(us),aiv_total_cycles,aiv_vec_time(us),"
                                  "aiv_vec_ratio,aiv_scalar_time(us),aiv_scalar_ratio,aiv_mte2_time(us),aiv_mte2_ratio,aiv_mte3_time(us),"
                                  "aiv_mte3_ratio,aiv_icache_miss_rate,cube_utilization(%)",
                "ResourceConflictRatio": "Model ID,Task ID,Stream ID,Op Name,OP Type,OP State,Task Type,"
                                         "Task Start Time(us),Task Duration(us),Task Wait Time(us),Block Dim,Mix Block Dim,"
                                         "HF32 Eligible,Input Shapes,Input Data Types,Input Formats,Output Shapes,Output Data Types,"
                                         "Output Formats,Context ID,aicore_time(us),aic_total_cycles,aiv_time(us),"
                                         "aiv_total_cycles,aiv_vec_bankgroup_cflt_ratio,aiv_vec_bank_cflt_ratio,aiv_vec_resc_cflt_ratio",
            },
        },
        "os_runtime_statistic": "Process ID,Thread ID,Name,Time(%),Time(us),Count,Avg(us),Max(us),Min(us)",
        "pcie": "Mode,Min,Max,Avg",
        "process_cpu_usage": "PID,Name,CPU(%)",
        "process_mem": "PID,Name,Size(pages),Resident(pages),Shared(pages)",
        "roce": "Timestamp(us),Bandwidth(MB/s),Rx Bandwidth efficiency(%),rxPacket/s,rxError rate(%),rxDropped rate(%),"
                "Tx Bandwidth efficiency(%),txPacket/s,txError rate(%),txDropped rate(%),funcId",
        "runtime_api": "Name,Stream ID,Time(%),Time(ns),Calls,Avg(ns),Min(ns),Max(ns),Process ID,Thread ID",
        "step_trace": "Iteration ID,FP Start(us),BP End(us),Iteration End(us),Iteration Time(us),FP to BP Time(us),"
                      "Iteration Refresh(us),Data Aug Bound(us),Model ID",
        "sys_mem": "Memory Total(kB),Memory Free(kB),Buffers(kB),Cached(kB),Share Memory(kB),Commit Limit(kB),"
                   "Committed AS(kB),Huge Pages Total(pages),Huge Pages Free(pages)",
        "task_time": "kernel_name,kernel_type,stream_id,task_id,task_time(us),task_start(us),task_stop(us)",
        "ts_cpu_pmu_events": "Event,Name,Count",
        "ts_cpu_top_function": "Function,Cycles,Cycles(%)",
        "communication_statistic": "OP Type,Count,Total Time(us),Min Time(us),Avg Time(us),Max Time(us),Ratio(%)",
        "soc_pmu": "Stream Id,Task Id,TLB Hit Rate,TLB Miss Rate,Op Name"
    }


class ConfigSwitchRes:
    # default result
    prof_dev_dir = ["data", "dev_start.log.0", "dev_start.log.0.done", "end_info.0", "end_info.0.done",
                    "host_start.log.0", "host_start.log.0.done", "info.json.0", "info.json.0.done",
                    "sample.json", "sample.json.done", "sqlite", "start_info.0", "start_info.0.done", "summary",
                    "timeline"]
    prof_host_dir = ['data', 'end_info', 'end_info.done', 'host_start.log', 'host_start.log.done', 'info.json', 'info.json.done', 'sample.json',
                     'sample.json.done', 'sqlite', 'start_info', 'start_info.done', "summary", "timeline"]
    #prof_host_dir = ['data', 'end_info', 'end_info.done', 'info.json', 'info.json.done', 'log', 'sample.json',
    #		     'sample.json.done', 'sqlite', 'start_info', 'start_info.done', "summary", "timeline"]
    # switch result
    switch_option = {
        "ai-core": {
            "timeline": ["ai_core_utilization", "ai_vector_core_utilization"],
            "summary": ["ai_core_utilization", "ai_vector_core_utilization"],
        },
        "ascendcl": {
            "timeline": ["msprof"],
            "summary": ["api_statistic", "prof_rule"],
        },
        "ACL_PROF_ACL_API": {
            "timeline": ["msprof"],
            "summary": ["api_statistic", "prof_rule"],
        },
        "model-execution": {
            "timeline": ['msprof'],
            "summary": ['fusion_op', 'api_statistic', 'prof_rule'],
        },
        "runtime-api": {
            "timeline": ['msprof'],
            "summary": ['prof_rule', 'api_statistic'],
        },
        "runtime_api": {
            "timeline": ['msprof'],
            "summary": ['prof_rule', 'api_statistic'],
        },
        "ACL_PROF_RUNTIME_API": {
            "timeline": ['msprof'],
            "summary": ['prof_rule', 'api_statistic'],
        },
        "task-time": {
            "timeline": ["step_trace", "msprof", "task_time"],
            "summary": ["fusion_op", "api_statistic", "prof_rule", 'op_statistic', 'op_summary',
                        'step_trace', 'task_time'],
        },
        "task_time": {
            "timeline": ["step_trace", "msprof", "task_time"],
            "summary": ["fusion_op", "api_statistic", "prof_rule", 'op_statistic', 'op_summary',
                        'step_trace', 'task_time'],
        },
        "ge-api": {
            "timeline": ["msprof"],
            "summary": ["api_statistic"],
        },
        "ACL_PROF_TASK_TIME": {
            "timeline": ["step_trace", "msprof", "task_time"],
            "summary": ["fusion_op", "api_statistic", "prof_rule", 'op_statistic', 'op_summary',
                        'step_trace', 'task_time'],
        },
        "hccl": {
            "timeline": ["hccl"],
            "summary": [],
        },
        "ACL_PROF_HCCL_TRACE": {
            "timeline": ["hccl"],
            "summary": [],
        },
        "aicpu": {
            "timeline": ["msprof"],
            "summary": ["prof_rule"],
        },
        "ACL_PROF_AICPU": {
            "timeline": [],
            "summary": ["prof_rule"],
        },
        "l2": {
            "timeline": [],
            "summary": ["l2_cache"],
        },
        "ACL_PROF_L2CACHE": {
            "timeline": [],
            "summary": ["l2_cache"],
        },
        "task_trace": {
            "timeline": ["task_time"],
            "summary": ["task_time"],
        },
        "training_trace": {
            "timeline": ["task_time"],
            "summary": ["task_time"],
        },
        "ACL_PROF_TRAINING_TRACE": {
            "timeline": ["task_time"],
            "summary": ["task_time"],
        },
        "sys-hardware-mem": {
            "timeline": ["hbm", "npu_mem", "msprof"],
            "summary": ["hbm", "npu_mem", "npu_module_mem", "prof_rule"],
        },
        "sys_hardware_mem_freq": {
            "timeline": ["hbm", "npu_mem", "msprof"],
            "summary": ["hbm", "npu_mem", "npu_module_mem", "prof_rule"],
        },
        "ACL_PROF_SYS_HARDWARE_MEM_FREQ": {
            "timeline": ["hbm", "npu_mem", "msprof"],
            "summary": ["hbm", "npu_mem", "npu_module_mem", "prof_rule"],
        },
        "sys-cpu-profiling": {
            "timeline": ["msprof"],
            "summary": ["ai_cpu_top_function", "ai_cpu_pmu_events", "ctrl_cpu_top_function", "ctrl_cpu_pmu_events",
                        "ts_cpu_top_function", "ts_cpu_pmu_events", "prof_rule"],
        },
        "sys-profiling": {
            "timeline": ["msprof"],
            "summary": ["cpu_usage", "sys_mem", "prof_rule"],
        },
        "sys-pid-profiling": {
            "timeline": ["msprof"],
            "summary": ["process_cpu_usage", "process_mem", "prof_rule"],
        },
        "sys-io-profiling": {
            "timeline": ["nic", "roce", "msprof"],
            "summary": ["nic", "roce", "prof_rule"],
        },
        "sys_io_sampling_freq": {
            "timeline": ["nic", "roce", "msprof"],
            "summary": ["nic", "roce", "prof_rule"],
        },
        "ACL_PROF_SYS_IO_FREQ": {
            "timeline": ["nic", "roce", "msprof"],
            "summary": ["nic", "roce", "prof_rule"],
        },
        "sys-interconnection-profiling": {
            "timeline": ["hccs", "pcie", "msprof"],
            "summary": ["hccs", "pcie", "prof_rule"],
        },
        "sys_interconnection_freq": {
            "timeline": ["hccs", "pcie", "msprof"],
            "summary": ["hccs", "pcie", "prof_rule"],
        },
        "ACL_PROF_SYS_INTERCONNECTION_FREQ": {
            "timeline": ["hccs", "pcie", "msprof"],
            "summary": ["hccs", "pcie", "prof_rule"],
        },
        "dvpp-profiling": {
            "timeline": ["msprof"],
            "summary": ["dvpp", "prof_rule"],
        },
        "dvpp_freq": {
            "timeline": [],
            "summary": ["dvpp", "prof_rule"],
        },
        "ACL_PROF_DVPP_FREQ": {
            "timeline": [],
            "summary": ["dvpp", "prof_rule"],
        },
        "msproftx": {
            "timeline": ["msprof", "msprof_tx"],
            "summary": ["msprof_tx", ],
        },
        "ACL_PROF_MSPROFTX": {
            "timeline": ["msprof", "msprof_tx"],
            "summary": ["msprof_tx"],
        },
        "TORCH_CALL_STACK": {
            "timeline": [],
            "summary": [],
        },
        "llc-profiling": {
            "read": {
                "timeline": ["llc_read_write"],
                "summary": ["llc_read_write"],
            },
            "write": {
                "timeline": ["llc_read_write"],
                "summary": ["llc_read_write"],
            },
            "bandwidth": {
                "timeline": ["llc_bandwidth"],
                "summary": ["llc_bandwidth"],
            },
            "capacity": {
                "timeline": ["llc_aicpu", "llc_ctrlcpu"],
                "summary": ["llc_aicpu", "llc_ctrlcpu"],
            },
        },
        "llc_profiling": {
            "read": {
                "timeline": ["llc_read_write"],
                "summary": ["llc_read_write"],
            },
            "write": {
                "timeline": ["llc_read_write"],
                "summary": ["llc_read_write"],
            },
            "bandwidth": {
                "timeline": ["llc_bandwidth"],
                "summary": ["llc_bandwidth"],
            },
            "capacity": {
                "timeline": ["llc_aicpu", "llc_ctrlcpu"],
                "summary": ["llc_aicpu", "llc_ctrlcpu"],
            },
        },
        "ACL_PROF_LLC_MODE": {
            "read": {
                "timeline": ["llc_read_write"],
                "summary": ["llc_read_write"],
            },
            "write": {
                "timeline": ["llc_read_write"],
                "summary": ["llc_read_write"],
            },
            "bandwidth": {
                "timeline": ["llc_bandwidth"],
                "summary": ["llc_bandwidth"],
            },
            "capacity": {
                "timeline": ["llc_aicpu", "llc_ctrlcpu"],
                "summary": ["llc_aicpu", "llc_ctrlcpu"],
            },
        },
        "host-sys": {
            "cpu": {
                "timeline": ["host_cpu_usage", "msprof"],
                "summary": ["host_cpu_usage"],
            },
            "mem": {
                "timeline": ["host_mem_usage", "msprof"],
                "summary": ["host_mem_usage"],
            },
            "disk": {
                "timeline": ["host_disk_usage", "msprof"],
                "summary": ["host_disk_usage"],
            },
            "network": {
                "timeline": ["host_network_usage", "msprof"],
                "summary": ["host_network_usage"],
            },
            "osrt": {
                "timeline": ["os_runtime_api", "msprof"],
                "summary": ["os_runtime_statistic"],
            },
        },
        "host_sys": {
            "cpu": {
                "timeline": ["host_cpu_usage", "msprof"],
                "summary": ["host_cpu_usage"],
            },
            "mem": {
                "timeline": ["host_mem_usage", "msprof"],
                "summary": ["host_mem_usage"],
            },
        },
        "ACL_PROF_HOST_SYS": {
            "cpu": {
                "timeline": ["host_cpu_usage", "msprof"],
                "summary": ["host_cpu_usage"],
            },
            "mem": {
                "timeline": ["host_mem_usage", "msprof"],
                "summary": ["host_mem_usage"],
            },
        },
        "host-sys-usage": {
            "cpu": {
                "timeline": [],
                "summary": ["cpu_usage", "process_cpu_usage"],
            },
            "mem": {
                "timeline": [],
                "summary": ["sys_mem", "process_mem"],
            },
        },
        "host_sys_usage": {
            "cpu": {
                "timeline": [],
                "summary": ["sys_mem", "process_cpu_usage"],
            },
            "mem": {
                "timeline": [],
                "summary": ["cpu_usage", "process_mem"],
            },
        },
        "ACL_PROF_HOST_SYS_USAGE": {
            "cpu": {
                "timeline": [],
                "summary": ["cpu_usage", "process_cpu_usage"],
            },
            "mem": {
                "timeline": [],
                "summary": ["sys_mem", "process_mem"],
            },
        },
        "ACL_PROF_HOST_SYS_USAGE_FREQ": {
            "cpu": {
                "timeline": [],
                "summary": ["cpu_usage", "process_cpu_usage"],
            },
            "mem": {
                "timeline": [],
                "summary": ["sys_mem", "process_mem"],
            }
        },
        "host_sys_usage_freq": {
            "cpu": {
                "timeline": [],
                "summary": ["cpu_usage", "process_cpu_usage"],
            },
            "mem": {
                "timeline": [],
                "summary": ["sys_mem", "process_mem"],
            }
        },
    }


class ConfigExportDbTableHeaders:
    table_header = {
        "ACC_PMU": ["accId", "readBwLevel", "writeBwLevel", "readOstLevel", "writeOstLevel", "timestampNs", "deviceId"],
        "AICORE_FREQ": ["deviceId", "timestampNs", "freq"],
        "CANN_API": ["startNs", "endNs", "type", "globalTid", "connectionId", "name"],
        "COMMUNICATION_OP": ["opName", "startNs", "endNs", "connectionId", "groupName", "opId", "relay", "retry",
                             "dataType", "algType", "count", "opType", "deviceId"],
        "COMMUNICATION_SCHEDULE_TASK_INFO": ['name','globalTaskId','taskType','opType'],
        "COMMUNICATION_TASK_INFO": [ "name", "globalTaskId", "taskType", "planeId", "groupName", "notifyId",
                                     "rdmaType", "srcRank", "dstRank", "transportType", "size", "dataType",
                                     "linkType",  "opId", "isMaster"],
        "COMPUTE_TASK_INFO": ["name", "globalTaskId", "blockDim", "mixBlockDim", "taskType", "opType", "inputFormats",
                              "inputDataTypes", "inputShapes", "outputFormats", "outputDataTypes", "outputShapes",
                              "attrInfo", "opState", "hf32Eligible"],
        "CONNECTION_IDS": ["id", "connectionId"],
        "ENUM_API_TYPE": ["id", "name"],
        "ENUM_HCCL_DATA_TYPE": ["id", "name"],
        "ENUM_HCCL_LINK_TYPE": ["id", "name",],
        "ENUM_HCCL_RDMA_TYPE": ["id", "name",],
        "ENUM_HCCL_TRANSPORT_TYPE": ["id", "name",],
        "ENUM_MEMCPY_OPERATION": ["id", "name",],
        "ENUM_MODULE": ["id", "name",],
        "ENUM_MSTX_EVENT_TYPE": ["id", "name",],
        "HBM": ["deviceId", "timestampNs", "bandwidth", "hbmId", "type",],
        "HOST_INFO": ["hostUid", "hostName"],
        "LLC": ["deviceId", "llcId", "timestampNs", "hitRate", "throughput", "mode",],
        "MEMORY_RECORD": ["component", "timestamp", "totalAllocated", "totalReserved", "totalActive", "streamPtr",
                          "deviceId"],
        "META_DATA": ["name", "value"],
        "NPU_INFO": ["id", "name"],
        "NPU_MEM": ["type", "ddr", "hbm", "timestampNs", "deviceId"],
        "NPU_MODULE_MEM": ["moduleId", "timestampNs", "totalReserved", "deviceId",],
        "NPU_OP_MEM": ["operatorName", "addr", "type", "size", "timestampNs", "globalTid", "totalAllocate",
                       "totalReserve", "component", "deviceId",],
        "OP_MEMORY": ["name", "size", "allocationTime", "releaseTime", "activeReleaseTime", "duration",
                      "activeDuration", "allocationTotalAllocated", "allocationTotalReserved", "allocationTotalActive",
                      "releaseTotalAllocated", "releaseTotalReserved", "releaseTotalActive", "streamPtr", "deviceId",],
        "PYTORCH_API": ["startNs", "endNs", "globalTid", "connectionId", "name", "sequenceNumber", "fwdThreadId",
                        "inputDtypes", "inputShapes", "callchainId", "type",],
        "RANK_DEVICE_MAP": ["rankId", "deviceId"],
        "SESSION_TIME_INFO": ["startTimeNs", "endTimeNs"],
        "SOC_BANDWIDTH_LEVEL": ["l2BufferBwLevel", "mataBwLevel", "timestampNs", "deviceId",],
        "STEP_TIME": ["id", "startNs", "endNs"],
        "STRING_IDS": ["id", "value"],
        "TASK": ["startNs", "endNs", "deviceId", "connectionId", "globalTaskId", "globalPid", "taskType", "contextId",
                 "streamId", "taskId", "modelId",],
        "TASK_PMU_INFO": ["globalTaskId", "name", "value"]
    }
