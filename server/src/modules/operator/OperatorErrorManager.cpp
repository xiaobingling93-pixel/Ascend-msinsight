/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "OperatorErrorManager.h"

namespace Dic::Module::Operator {
static const std::string unknownError = "Unknown error code";

const std::map<ErrorCode, std::string> errorMessages = {
    {ErrorCode::PARAMS_ERROR, "Request parameter exception"},

    {ErrorCode::GET_DEVICE_ID_FAILED, "Failed to get device id"},
    {ErrorCode::GET_CSV_HANDLE_FAILED, "Failed to get csvHandle"},
    {ErrorCode::GET_BASELINE_ID_FAILED, "Failed to get baseline id"},

    {ErrorCode::QUERY_DURATION_FAILED, "Failed to query operator duration info"},
    {ErrorCode::QUERY_STATISTIC_FAILED, "Failed to query operator statistic info"},
    {ErrorCode::QUERY_DETAIL_FAILED, "Failed to query operator detail info"},
    {ErrorCode::QUERY_MORE_INFO_FAILED, "Failed to query operator more info"},
    {ErrorCode::QUERY_ALL_STATISTIC_FAILED, "Failed to query all operator statistic info"},
    {ErrorCode::QUERY_ALL_DETAIL_FAILED, "Failed to query all operator detail info"},
};

const std::string& GetErrorMessage(ErrorCode code)
{
    auto it = errorMessages.find(code);
    if (it != errorMessages.end()) {
        return it->second;
    } else {
        return unknownError;
    }
}

void SetOperatorError(ErrorCode code)
{
    ModuleRequestHandler::SetResponseError({.code = static_cast<int>(code), .message = GetErrorMessage(code)});
}
}  // namespace Dic::Module::Operator