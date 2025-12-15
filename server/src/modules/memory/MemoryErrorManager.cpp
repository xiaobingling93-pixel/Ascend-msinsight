/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "MemoryErrorManager.h"

namespace Dic::Module::Memory {
static const std::string unknownError = "Unknown error code";

const std::map<ErrorCode, std::string> errorMessages = {
    {ErrorCode::PARAMS_ERROR, "Request parameter exception"},

    {ErrorCode::GET_DEVICE_ID_FAILED, "Failed to get device id"},
    {ErrorCode::GET_BASELINE_ID_FAILED, "Failed to get baseline id"},

    {ErrorCode::CONNECT_DATABASE_FAILED, "Failed to connect to database"},
    {ErrorCode::QUERY_SLICE_FAILED, "Failed to find slice. timeline not exist"},
    {ErrorCode::QUERY_MEMORY_OPERATOR_FAILED, "Failed to query memory operator data"},
    {ErrorCode::QUERY_SLICE_IN_TIMELINE_FAILED, "Failed to find slice in timeline"},
    {ErrorCode::QUERY_MEMORY_COMPONENT_FAILED, "Failed to query memory component data"},
    {ErrorCode::QUERY_MEMORY_COMPONENT_COMPARE_FAILED, "Failed to query memory component compare data"},
    {ErrorCode::QUERY_MEMORY_COMPONENT_BASELINE_FAILED, "Failed to query memory component baseline data"},
    {ErrorCode::QUERY_MEMORY_OPERATOR_COMPARE_FAILED, "Failed to query memory operator compare data"},
    {ErrorCode::QUERY_MEMORY_OPERATOR_BASELINE_FAILED, "Failed to query memory operator baseline data"},
    {ErrorCode::QUERY_OPERATOR_SIZE_FAILED, "Failed to query operator size data"},
    {ErrorCode::QUERY_OPERATOR_SIZE_COMPARE_FAILED, "Failed to query operator size compare data"},
    {ErrorCode::QUERY_OPERATOR_SIZE_BASELINE_FAILED, "Failed to query operator size baseline data"},
    {ErrorCode::QUERY_MEMORY_RESOURCE_TYPE_FAILED, "Failed to query memory resource type data"},
    {ErrorCode::QUERY_MEMORY_STATIC_OPERATOR_GRAPH_FAILED, "Failed to query memory static operator graph data"},
    {ErrorCode::QUERY_MEMORY_STATIC_OPERATOR_GRAPH_COMPARE_FAILED,
     "Failed to query memory static operator graph compare data"},
    {ErrorCode::QUERY_MEMORY_STATIC_OPERATOR_GRAPH_BASELINE_FAILED,
     "Failed to query memory static operator graph baseline data"},
    {ErrorCode::QUERY_MEMORY_STATIC_OPERATOR_FAILED, "Failed to query memory static operator data"},
    {ErrorCode::QUERY_MEMORY_STATIC_OPERATOR_COMPARE_FAILED, "Failed to query memory static operator compare data"},
    {ErrorCode::QUERY_MEMORY_STATIC_OPERATOR_BASELINE_FAILED, "Failed to query memory static operator baseline data"},
    {ErrorCode::QUERY_MEMORY_STATIC_OPERATOR_SIZE_FAILED, "Failed to query memory static operator size data"},
    {ErrorCode::QUERY_MEMORY_STATIC_OPERATOR_SIZE_COMPARE_FAILED,
     "Failed to query memory static operator size compare data"},
    {ErrorCode::QUERY_MEMORY_STATIC_OPERATOR_SIZE_BASELINE_FAILED,
     "Failed to query memory static operator size baseline data"},
    {ErrorCode::QUERY_MEMORY_TYPE_FAILED, "Failed to query memory type data"},
    {ErrorCode::QUERY_MEMORY_VIEW_DATA_FAILED, "Failed to query memory view data"},
    {ErrorCode::QUERY_MEMORY_VIEW_DATA_COMPARE_FAILED, "Failed to query memory view compare data"},
    {ErrorCode::QUERY_MEMORY_VIEW_DATA_BASELINE_FAILED, "Failed to query memory view baseline data"},
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

void SetMemoryError(ErrorCode code)
{
    ModuleRequestHandler::SetResponseError({.code = static_cast<int>(code), .message = GetErrorMessage(code)});
}
}  // namespace Dic::Module::Memory