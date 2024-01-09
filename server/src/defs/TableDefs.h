/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: defines declaration
 */

#ifndef PROFILER_SERVERDEFS_H
#define PROFILER_SERVERDEFS_H

#include <string>

namespace Dic {
    // table name
    const static std::string TABLE_TIME_INFO = "communication_time_info";
    const static std::string TABLE_BANDWIDTH = "communication_bandwidth_info";
    const static std::string TABLE_STEP_TRACE = "step_statistic_info";
    const static std::string TABLE_BASE_INFO = "cluster_base_info";
    const static std::string TABLE_COMMUNICATION_MATRIX = "communication_matrix";
    const static std::string TABLE_GROUP_ID = "group_id";

    // database version
#ifndef DATABASE_VERSION
#define DATABASE_VERSION 0
#endif
} // end of namespace Dic


#endif // PROFILER_SERVERDEFS_H
