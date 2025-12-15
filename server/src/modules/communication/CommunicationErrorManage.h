/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_COMMUNICATIONERRORMANAGE_H
#define PROFILER_SERVER_COMMUNICATIONERRORMANAGE_H

#include "ModuleRequestHandler.h"

namespace Dic::Module::Communication {
// 错误码枚举
enum class ErrorCode {
    RESET_ERROR = 0,
    // 接口入参错误
    PARAMS_ERROR = 1101,

    // 中间变量
    GET_DEVICE_ID_FAILED = 2001,

    // 数据库错误
    CONNECT_DATABASE_FAILED = 3001,
    QUERY_COMMUNICATION_BANDWIDTH_FAILED = 3101,
    QUERY_COMMUNICATION_OPERATOR_FAILED = 3102,
    QUERY_SLOW_OPERATOR_FAILED = 3103,
    QUERY_COMMUNICATION_DISTRIBUTION_FAILED = 3104,
    QUERY_MATRIX_SORT_OPERATOR_NAMES_FAILED = 3105,
    QUERY_OPERATOR_NAMES_FAILED = 3106,
};

// 错误信息映射
extern const std::map<ErrorCode, std::string> errorMessages;

const std::string& GetErrorMessage(ErrorCode code);
void SetCommunicationError(ErrorCode code);
}  // namespace Dic::Module::Communication
#endif  // PROFILER_SERVER_COMMUNICATIONERRORMANAGE_H
