/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include <cmath>
#include "DataBaseManager.h"
#include "BaselineManager.h"
#include "QueryMemoryOperatorSizeHandler.h"

namespace Dic {
namespace Module {
namespace Memory {
using namespace Dic::Server;
bool QueryMemoryOperatorSizeHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    MemoryOperatorSizeRequest &request = dynamic_cast<MemoryOperatorSizeRequest &>(*requestPtr);
    std::unique_ptr<MemoryOperatorSizeResponse> responsePtr = std::make_unique<MemoryOperatorSizeResponse>();
    MemoryOperatorSizeResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    std::string errorMsg;
    if (!request.params.CommonCheck(errorMsg)) {
        SetMemoryError(ErrorCode::PARAMS_ERROR);
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetMemoryDatabaseByRankId(request.params.rankId);
    if (!database) {
        SetMemoryError(ErrorCode::CONNECT_DATABASE_FAILED);
        SendResponse(std::move(responsePtr), false);
        return false;
    }

    std::string deviceId = Timeline::DataBaseManager::Instance().GetDeviceIdFromRankId(request.params.rankId);
    if (deviceId.empty()) {
        SetMemoryError(ErrorCode::GET_DEVICE_ID_FAILED);
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    request.params.deviceId = deviceId;
    if (!request.params.isCompare) {
        if (!database->QueryOperatorSize(request.params, response.size.minSize, response.size.maxSize)) {
            SetMemoryError(ErrorCode::QUERY_OPERATOR_SIZE_FAILED);
            SendResponse(std::move(responsePtr), false);
            return false;
        }
    } else {
        OperatorSize compareData;
        OperatorSize baselineData;
        if (!GetRespectiveData(database, compareData, baselineData, request, errorMsg)) {
            SendResponse(std::move(responsePtr), false);
            return false;
        }
        ExecuteComparisonAlgorithm(compareData, baselineData, response);
    }
    // 接口查询成功时，为了避免浮点数舍入误差导致的问题，对minSize下取整，maxSize上取整，保证表格中算子size在minSize到maxSize之内
    response.size.minSize = std::floor(response.size.minSize);
    response.size.maxSize = std::ceil(response.size.maxSize);
    SendResponse(std::move(responsePtr), true);
    return true;
}

bool QueryMemoryOperatorSizeHandler::GetRespectiveData(std::shared_ptr<VirtualMemoryDataBase> database,
                                                       Dic::Protocol::OperatorSize &compareData,
                                                       Dic::Protocol::OperatorSize &baselineData,
                                                       Dic::Protocol::MemoryOperatorSizeRequest &request,
                                                       std::string &errorMsg)
{
    std::string baselineId = Global::BaselineManager::Instance().GetBaselineId();
    if (baselineId.empty()) {
        errorMsg = "Failed to get baseline id.";
        SetMemoryError(ErrorCode::GET_BASELINE_ID_FAILED);
        return false;
    }
    auto databaseBaseline = Timeline::DataBaseManager::Instance().GetMemoryDatabaseByRankId(baselineId);
    if (!databaseBaseline) {
        errorMsg = "Failed to connect to database of baseline.";
        SetMemoryError(ErrorCode::CONNECT_DATABASE_FAILED);
        return false;
    }
    if (!database->QueryOperatorSize(request.params, compareData.minSize, compareData.maxSize)) {
        errorMsg = "Failed to query memory operator size compare data.";
        SetMemoryError(ErrorCode::QUERY_OPERATOR_SIZE_COMPARE_FAILED);
        return false;
    }
    request.params.deviceId = FullDb::DataBaseManager::Instance().GetDeviceIdFromRankId(baselineId);
    if (!databaseBaseline->QueryOperatorSize(request.params, baselineData.minSize, baselineData.maxSize)) {
        errorMsg = "Failed to query memory operator size baseline data.";
        SetMemoryError(ErrorCode::QUERY_OPERATOR_SIZE_BASELINE_FAILED);
        return false;
    }
    return true;
}

void QueryMemoryOperatorSizeHandler::ExecuteComparisonAlgorithm(const Dic::Protocol::OperatorSize &compareData,
                                                                const Dic::Protocol::OperatorSize &baselineData,
                                                                Dic::Protocol::MemoryOperatorSizeResponse &response)
{
    response.size.minSize = compareData.minSize - baselineData.maxSize;
    response.size.maxSize = compareData.maxSize - baselineData.minSize;
}

} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic