/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "CommunicationErrorManage.h"

namespace Dic::Module::Communication {

static const std::string unknownError = "Unknown error code";

const std::map<ErrorCode, std::string> errorMessages = {
    {ErrorCode::RESET_ERROR, ""},
    {ErrorCode::PARAMS_ERROR, "Request parameter exception"},

    {ErrorCode::GET_DEVICE_ID_FAILED, "Failed to get device id"},

    {ErrorCode::CONNECT_DATABASE_FAILED, "Failed to connect to database"},
    {ErrorCode::QUERY_COMMUNICATION_BANDWIDTH_FAILED, "Failed to query communication bandwidth data"},
    {ErrorCode::QUERY_COMMUNICATION_OPERATOR_FAILED, "Failed to query communication operator data"},
    {ErrorCode::QUERY_SLOW_OPERATOR_FAILED, "Failed to query slow operator data"},
    {ErrorCode::QUERY_COMMUNICATION_DISTRIBUTION_FAILED, "Failed to query communication distribution data"},
    {ErrorCode::QUERY_MATRIX_SORT_OPERATOR_NAMES_FAILED, "Failed to query matrix sort operator names"},
    {ErrorCode::QUERY_OPERATOR_NAMES_FAILED, "Failed to query operator names"},
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

void SetCommunicationError(ErrorCode code)
{
    ModuleRequestHandler::SetResponseError({.code = static_cast<int>(code), .message = GetErrorMessage(code)});
}
}  // namespace Dic::Module::Communication