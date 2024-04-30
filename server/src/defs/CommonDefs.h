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

    const static std::string KERNEL_DETAIL_REG =
            R"((kernel_details|op_summary_(slice_[0-9]{1,4}_)?[0-9]{1,14}).csv$)";
    const std::string traceViewReg =
    R"((((trace_view|msprof(_slice)?(_[0-9]{1,15}){1,4})\.json)|(operator_memory|operator_memory_[0-9]{1,14})\.csv)$)";
    const std::string clusterReg = R"(cluster_analysis_output$)";
    const std::string DBReg =
            R"((msprof_[0-9]{1,16}|((ascend_pytorch_profiler)(_[0-9]{1,16}){0,1})|cluster_analysis)\.db$)";
    const std::string msprofDBReg = R"(msprof_[0-9]{1,16}\.db$)";
    const std::string pytorchDBReg = R"((ascend_pytorch_profiler)(_[0-9]{1,16}){0,1}\.db$)";
    const std::string clusterDBReg = R"(cluster_analysis\.db$)";
    const std::string computeBinSuffix = ".bin";

    const static std::vector<std::string> CANN_APIS = {"acl", "runtime", "mode", "node"};
    const static std::vector<std::string> OVERLAP_TYPES = {"Computing", "Communication",
                                                           "Communication(Not Overlapped)", "Free"};
}

#endif // PROFILER_SERVER_COMMONDEFS_H
