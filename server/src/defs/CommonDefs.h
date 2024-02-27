/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 * Description: defines declaration
 */

#ifndef PROFILER_SERVER_COMMONDEFS_H
#define PROFILER_SERVER_COMMONDEFS_H

#include <string>

namespace Dic {
    const static std::string MSPROF_PREFIX = "[MSPROF]";
    const static std::string MSPROF_CONNECT = "__";

    const static std::string KERNEL_DETAIL_REG = R"((kernel_details|op_summary_[0-9]{1,14})\.csv$)";
    const std::string traceViewReg =
        R"((((trace_view|msprof(_[0-9]{1,15}){1,4})\.json)|(operator_memory|operator_memory_[0-9]{1,14})\.csv)$)";
    const std::string DBReg = R"((report_[0-9]{1,16})\.db$)";
}

#endif // PROFILER_SERVER_COMMONDEFS_H
