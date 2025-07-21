/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

export enum FilePath {
    TEXT = 'D:\\GUI_TEST_DATA 路徑测试\\系统调优\\CANN_8.0.RC2.B050_GPT3_level1_ACLAICORE_text\\profiling_data',
    TEXT_330 = 'D:\\GUI_TEST_DATA 路徑测试\\系统调优\\test_level1_PipeUtilization\\profiling_data', // 给 Memory 模块使用
    TEXT_330_RANK_0 = 'D:\\GUI_TEST_DATA 路徑测试\\系统调优\\test_level1_PipeUtilization\\profiling_data\\ubuntu2204_3929556_20250411014455476_ascend_pt\\ASCEND_PROFILER_OUTPUT',
    TEXT_330_RANK_1 = 'D:\\GUI_TEST_DATA 路徑测试\\系统调优\\test_level1_PipeUtilization\\profiling_data\\ubuntu2204_3929557_20250411014455475_ascend_pt\\ASCEND_PROFILER_OUTPUT',
    DB = 'D:\\GUI_TEST_DATA 路徑测试\\系统调优\\CANN_8.0.RC2.B050_GPT3_level2_ACLAICORE_db',
    MIND_SPORE = 'D:\\GUI_TEST_DATA 路徑测试\\mindspore\\profile',
    DB_2025330 = 'D:\\GUI_TEST_DATA 路徑测试\\系统调优\\CANN_9.0.RC1.B090_db',
    DB_memory = 'D:\\GUI_TEST_DATA 路徑测试\\系统调优\\test_level1_Memory_db',
    MULTI_MACHINES = 'D:\\GUI_TEST_DATA 路徑测试\\多机多卡\\MultiProfLevel2MemoryUB_db',
    TRACE_JSON='D:\\GUI_TEST_DATA 路徑测试\\算子调优\\trace_json\\trace.json',
    SOURCE = 'D:\\GUI_TEST_DATA 路徑测试\\算子调优\\source\\visualize_data.bin',
    SOURCE_MULTIFILE = 'D:\\GUI_TEST_DATA 路徑测试\\算子调优\\source\\visualize_data_multifile.bin',
    DETAILS = 'D:\\GUI_TEST_DATA 路徑测试\\算子调优\\details\\visualize_data_910_mix.bin',
    DETAILS_ROOFLINE = 'D:\\GUI_TEST_DATA 路徑测试\\算子调优\\details\\visualize_data_roofline_910_mix.bin',
    JUPYTER = 'D:\\GUI_TEST_DATA 路徑测试\\jupyter\\test.ipynb',
    TEXT_RANK_0 = 'D:\\GUI_TEST_DATA 路徑测试\\系统调优\\CANN_8.0.RC2.B050_GPT3_level1_ACLAICORE_text\\profiling_data\\ubuntu2204_2045554_20240612082838731_ascend_pt\\ASCEND_PROFILER_OUTPUT',
    TEXT_RANK_1 = 'D:\\GUI_TEST_DATA 路徑测试\\系统调优\\CANN_8.0.RC2.B050_GPT3_level1_ACLAICORE_text\\profiling_data\\ubuntu2204_2045555_20240612082838733_ascend_pt\\ASCEND_PROFILER_OUTPUT',
    TEXT_CLUSTER = 'D:\\GUI_TEST_DATA 路徑测试\\系统调优\\profile_dir_llm\\cluster_analysis_output',
    DB_HOST_0_RANK_0 = 'D:\\GUI_TEST_DATA 路徑测试\\多机多卡\\MultiProfLevel2MemoryUB_db\\node1_2166651_20240619060505060_ascend_pt\\ASCEND_PROFILER_OUTPUT',
    DB_HOST_0_RANK_1 = 'D:\\GUI_TEST_DATA 路徑测试\\多机多卡\\MultiProfLevel2MemoryUB_db\\node1_2166652_20240619060505059_ascend_pt\\ASCEND_PROFILER_OUTPUT',
    DB_HOST_1_RANK_0 = 'D:\\GUI_TEST_DATA 路徑测试\\多机多卡\\MultiProfLevel2MemoryUB_db\\ubuntu2204_1660963_20240619060440181_ascend_pt\\ASCEND_PROFILER_OUTPUT',
    DB_HOST_1_RANK_1 = 'D:\\GUI_TEST_DATA 路徑测试\\多机多卡\\MultiProfLevel2MemoryUB_db\\ubuntu2204_1660964_20240619060440179_ascend_pt\\ASCEND_PROFILER_OUTPUT',
    // 专家负载均衡数据
    MOE_PROFILING = 'D:\\GUI_TEST_DATA 路徑测试\\系统调优\\MoE_balancing\\profiling',
    MOE_DUMP = 'D:\\GUI_TEST_DATA 路徑测试\\系统调优\\MoE_balancing\\dump',
    LEAKS_DUMP='D:\\GUI_TEST_DATA 路徑测试\\系统调优\\leaks_dump_20250603145530.db'
}

export const WEBSOCKET_URL = 'ws://localhost:9000/';
