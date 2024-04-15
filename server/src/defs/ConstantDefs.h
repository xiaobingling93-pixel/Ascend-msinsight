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

// att model
static const std::string ATT_MODEL_DEFAULT = "all";
static const std::string ATT_MODEL_TIME = "communication_time";
static const std::string ATT_MODEL_MATRIX = "communication_matrix";

} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_CONSTANTS_DEFS_H
