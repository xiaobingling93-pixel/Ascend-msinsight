/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
*/

#include "DataBaseManager.h"
#include "QueryMemScopeEventHandler.h"

namespace Dic::Module::MemScope {
bool QueryMemScopeEventHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<MemScopeEventRequest &>(*requestPtr);
    std::unique_ptr<MemScopeEventResponse> responsePtr = std::make_unique<MemScopeEventResponse>();
    auto &response = *responsePtr;
    SetBaseResponse(request, response);
    std::string errorMsg;
    if (!request.params.CommonCheck(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetMemScopeDatabase("");
    if (database == nullptr) {
        errorMsg = "Get memscope database failed when querying events table.";
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    int64_t total = database->QueryEventsByRequestParams(request.params, responsePtr->events);
    if (total < 0) {
        errorMsg = "Failed to query events table using params: Prepare sql failed.";
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    responsePtr->total = total;
    responsePtr->withCallStackC = database->withCallStackC;
    responsePtr->withCallStackPython = database->withCallStackPython;
    SendResponse(std::move(responsePtr), true);
    return true;
}
}  // namespace Dic::Module::MemScope