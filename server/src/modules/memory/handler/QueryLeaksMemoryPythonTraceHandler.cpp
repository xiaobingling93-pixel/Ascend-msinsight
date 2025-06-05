/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "DataBaseManager.h"
#include "QueryLeaksMemoryPythonTraceHandler.h"

namespace Dic {
namespace Module {
namespace Memory {
bool QueryLeaksMemoryPythonTraceHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<LeaksMemoryTraceRequest &>(*requestPtr);
    std::unique_ptr<LeaksMemoryTracesResponse> responsePtr = std::make_unique<LeaksMemoryTracesResponse>();
    auto &response = *responsePtr;
    SetBaseResponse(request, response);
    std::string errorMsg;
    if (!request.params.CommonCheck(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetLeaksMemoryDatabase("");
    if (database == nullptr) {
        errorMsg = "Get leaks memory database failed when querying python traces.";
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    database->QueryPythonTrace(request.params, response.trace);
    if (response.trace.Empty()) {
        Server::ServerLog::Warn("Query memory traces: empty data.");
        SendResponse(std::move(responsePtr), true);
        return true;
    }
    response.trace.threadId = request.params.threadId;
    LeaksMemoryService::ParseThreadPythonTrace(response.trace);
    SendResponse(std::move(responsePtr), true);
    return true;
}
} // Memory
} // Module
} // Dic