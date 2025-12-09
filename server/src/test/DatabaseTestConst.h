/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_DATABASETESTCONST_H
#define PROFILER_SERVER_DATABASETESTCONST_H

#include <string>
#include <map>
namespace Dic::Global::PROFILER::MockUtil {
enum class TableName {
    // db
    DB_STRING_IDS,
    DB_PYTORCH_API,
    DB_CONNECTION_IDS,
    DB_PYTORCH_CALLCHAINS,
    DB_CANN_API,
    DB_MSTX_EVENTS,
    DB_OSRT_API,
    DB_TASK,
    DB_COMPUTE_TASK_INFO,
    DB_COMMUNICATION_OP,
    DB_COMMUNICATION_TASK_INFO,
    DB_COMMUNICATION_SCHEDULE_TASK_INFO,
    DB_NPU_INFO,
    DB_RANK_DEVICE_MAP,
    DB_ENUM_API_TYPE,
    // db counter
    DB_AICORE_FREQ,
    DB_ACC_PMU,
    DB_SOC_BANDWIDTH_LEVEL,
    DB_NPU_MEM,
    DB_HCCS,
    DB_PCIE,
    // db Insight创建的辅助表
    DB_OVERLAP_ANALYSIS,
    DB_CONNECTION_CATS
};

// db
extern const std::string CREATE_TABLE_DB_STRING_IDS_SQL;
extern const std::string CREATE_TABLE_DB_PYTORCH_API_SQL;
extern const std::string CREATE_TABLE_DB_CONNECTION_IDS_SQL;
extern const std::string CREATE_TABLE_DB_PYTORCH_CALLCHAINS_SQL;
extern const std::string CREATE_TABLE_DB_CANN_API_SQL;
extern const std::string CREATE_TABLE_DB_MSTX_EVENTS_SQL;
extern const std::string CREATE_TABLE_DB_OSRT_API_SQL;
extern const std::string CREATE_TABLE_DB_TASK_SQL;
extern const std::string CREATE_TABLE_DB_COMPUTE_TASK_INFO_SQL;
extern const std::string CREATE_TABLE_DB_COMMUNICATION_OP_SQL;
extern const std::string CREATE_TABLE_DB_COMMUNICATION_TASK_INFO_SQL;
extern const std::string CREATE_TABLE_DB_COMMUNICATION_SCHEDULE_TASK_INFO_SQL;
extern const std::string CREATE_TABLE_DB_NPU_INFO_SQL;
extern const std::string CREATE_TABLE_DB_RANK_DEVICE_MAP_SQL;
extern const std::string CREATE_TABLE_DB_ENUM_API_TYPE_SQL;

// db counter
extern const std::string CREATE_TABLE_DB_AICORE_FREQ_SQL;
extern const std::string CREATE_TABLE_DB_ACC_PMU_SQL;
extern const std::string CREATE_TABLE_DB_SOC_BANDWIDTH_LEVEL_SQL;
extern const std::string CREATE_TABLE_DB_NPU_MEM_SQL;
extern const std::string CREATE_TABLE_DB_HCCS_SQL;
extern const std::string CREATE_TABLE_DB_PCIE_SQL;

// db Insight创建的辅助表
extern const std::string CREATE_TABLE_DB_OVERLAP_ANALYSIS_SQL;
extern const std::string CREATE_TABLE_DB_CONNECTION_CATS_SQL;

extern const std::map<TableName, std::string> CREATE_TABLE_SQL_MAP;
}

#endif // PROFILER_SERVER_DATABASETESTCONST_H
