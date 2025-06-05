/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "DataBaseManager.h"
#include "QueryLeaksMemoryDetailHandler.h"

namespace Dic {
namespace Module {
namespace Memory {
bool QueryLeaksMemoryDetailHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<LeaksMemoryDetailRequest &>(*requestPtr);
    std::unique_ptr<LeaksMemoryDetailsResponse> responsePtr = std::make_unique<LeaksMemoryDetailsResponse>();
    auto &response = *responsePtr;
    SetBaseResponse(request, response);
    std::string errorMsg;
    if (!request.params.CommonCheck(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }

    if (!LeaksMemoryService::ParseMemoryAllocDetailTreeByTimestamp(request.params.deviceId, request.params.timestamp,
        responsePtr->detail, request.params.relativeTime)) {
        errorMsg = "Failed to query memory allocation detail.";
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    SendResponse(std::move(responsePtr), true);
    return true;
}
}
}
}
