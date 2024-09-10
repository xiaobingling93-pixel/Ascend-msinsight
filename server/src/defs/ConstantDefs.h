/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_CONSTANTS_DEFS_H
#define DATA_INSIGHT_CORE_CONSTANTS_DEFS_H

#include <string>

namespace Dic {
static const std::string FINISH_STATUS = "FINISH";
static const std::string NOT_FINISH_STATUS = "NOT_FINISH";
static const std::string KERNEL_PREFIX = "[Kernel]";
static const std::string MEMORY_PREFIX = "[Memory]";
// cluster parse status
static const std::string PARSE_RESULT_NONE = "none";
static const std::string PARSE_RESULT_OK = "ok";
static const std::string PARSE_RESULT_FAIL = "fail";

// dbTrace database status key
static const std::string CONFIG_STATUS = "CONFIG_STATUS";
static const std::string OVERLAP_ANALYSIS_STATUS = "OVERLAP_ANALYSIS_STATUS";
static const std::string WAIT_TIME_STATUS = "WAIT_TIME_STATUS";
static const std::string CONNECTION_STATUS = "CONNECTION_STATUS";
static std::list<std::string> DB_STATUS_LIST =  { OVERLAP_ANALYSIS_STATUS, WAIT_TIME_STATUS, CONNECTION_STATUS };

// att model
static const std::string ATT_MODEL_DEFAULT = "all";
static const std::string ATT_MODEL_TIME = "communication_time";
static const std::string ATT_MODEL_MATRIX = "communication_matrix";

} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_CONSTANTS_DEFS_H
