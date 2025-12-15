/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "SummaryErrorManager.h"

namespace Dic::Module::Summary {
static const std::string unknownError = "Unknown error code";

const std::map<ErrorCode, std::string> errorMessages = {
    {ErrorCode::PARAMS_ERROR, "Request parameter exception"},

    {ErrorCode::GET_ALGORITHM_FAILED, "Failed to get algorithm"},
    {ErrorCode::GET_ALGORITHM_CONNECTIONS_FAILED,
     "Failed to get connections. Parallel strategy configs have not been updated yet"},
    {ErrorCode::GET_MASK_FAILED, "Failed to get mask for generate orthogonal rank groups. Unexpected order or token"},
    {ErrorCode::GET_RANK_ID_FAILED, "Failed to get rank ids"},
    {ErrorCode::GET_REAL_PATH_FAILED, "Failed to get real path"},
    {ErrorCode::GET_PARSED_FILES_FAILED, "No parsable files found"},
    {ErrorCode::GENERATE_ORTHOGONAL_FAILED, "Failed to generate orthogonal rank groups"},

    {ErrorCode::CONNECT_DATABASE_FAILED, "Failed to connect to database"},
    {ErrorCode::QUERY_COMPUTE_STATISTICS_FAILED, "Failed to query compute statistics data"},
    {ErrorCode::QUERY_COMMUNICATION_STATISTICS_FAILED, "Failed to query communication statistics data"},
    {ErrorCode::QUERY_PARALLEL_STATISTICS_FAILED, "Failed to query parallel strategy config"},
    {ErrorCode::QUERY_PARALLELISM_PERFORMANCE_FAILED, "Failed to query parallelism performance info"},
    {ErrorCode::QUERY_COMPUTE_DETAIL_FAILED, "Failed to query compute detail"},
    {ErrorCode::QUERY_COMMUNICATION_DETAIL_FAILED, "Failed to query communication detail"},
    {ErrorCode::UPDATE_PARALLEL_STRATEGY_FAILED, "Failed to update parallel strategy config"},
    {ErrorCode::UPDATE_PARALLEL_SHOW_MAP_FAILED, "Failed to update show map for parallel view. Unexpected dimension"},
    {ErrorCode::UPDATE_PARALLEL_VIEW_FAILED, "Failed to update parallel view"},
    {ErrorCode::UPDATE_MODEL_INFO_MODIFY_FAILED,
     "Failed to update model info, the number of expert number can't be modify"},
    {ErrorCode::UPDATE_MODEL_INFO_NOT_EQUAL_FAILED, "Failed to update model info, the sum of moe and dense layers is "
                                                    "less than the total number of layers in the model"},
    {ErrorCode::MERGE_AND_SAVE_MODEL_INFO_FAILED, "Failed to merge and save model info"},
    {ErrorCode::ADD_ALGORITHM_FAILED, "Failed to add algorithm to manager. Unexpected algorithm"},
    {ErrorCode::CLEAR_EXPERT_HOTSPOT_FAILED, "Failed to clear old expert hotspot data"},
    {ErrorCode::CLEAR_DEPLOYMENT_FAILED, "Failed to clear old expert deployment data"},

    {ErrorCode::PARSER_MODEL_GEN_CONFIG_FILE_FAILED, "Failed to parser config file"},
    {ErrorCode::PARSER_META_DATA_FILE_CONTEXT_FAILED, "Failed to parser meta data file context"},
    {ErrorCode::READ_MODEL_GEN_CONFIG_FILE_FAILED, "Failed to read model gen config file"},
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

void SetSummaryError(ErrorCode code)
{
    ModuleRequestHandler::SetResponseError({.code = static_cast<int>(code), .message = GetErrorMessage(code)});
}
}  // namespace Dic::Module::Summary