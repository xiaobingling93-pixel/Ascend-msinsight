/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "DataBaseManager.h"
#include "QueryMemScopeMemoryDetailHandler.h"

namespace Dic {
namespace Module {
namespace MemScope {
bool QueryMemScopeMemoryDetailHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<MemScopeMemoryDetailRequest &>(*requestPtr);
    std::unique_ptr<MemScopeMemoryDetailsResponse> responsePtr = std::make_unique<MemScopeMemoryDetailsResponse>();
    auto &response = *responsePtr;
    SetBaseResponse(request, response);
    std::string errorMsg;
    if (!request.params.CommonCheck(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    if (!MemScopeService::IsValidMemoryEventType(MEM_SCOPE_DUMP_EVENT::MALLOC, request.params.eventType)) {
        errorMsg = "The eventType is invalid.";
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    if (!MemScopeService::ParseMemoryAllocDetailTreeByTimestamp(request.params.deviceId, request.params.timestamp,
                                                                request.params.eventType, responsePtr->detail,
                                                                request.params.relativeTime)) {
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
