/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "DataBaseManager.h"
#include "ProjectExplorerManager.h"
#include "QueryMemScopeAllocationHandler.h"

namespace Dic {
namespace Module {
namespace MemScope {
bool QueryMemScopeAllocationHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<MemScopeMemoryAllocationRequest &>(*requestPtr.get());
    std::unique_ptr<MemScopeMemoryAllocationsResponse> responsePtr = std::make_unique<MemScopeMemoryAllocationsResponse>();
    MemScopeMemoryAllocationsResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    std::string errorMsg;
    if (!request.params.CommonCheck(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    auto memoryDatabase = Timeline::DataBaseManager::Instance().GetMemScopeDatabase("");
    if (memoryDatabase == nullptr) {
        errorMsg = "Get memscope memory database failed when querying allocations.";
        Server::ServerLog::Error(errorMsg);
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    std::vector<MemoryAllocation> allocations;
    memoryDatabase->QueryMemoryAllocations(request.params, allocations);
    if (allocations.empty()) {
        Server::ServerLog::Warn("Query memory allocations: empty data.");
        SendResponse(std::move(responsePtr), true);
        return true;
    }
    if (request.params.startTimestamp > 0 || request.params.endTimestamp > 0) {
        PaddingAllocations(allocations, request.params);
    }
    responsePtr->minTimestamp = allocations[0].timestamp;
    responsePtr->maxTimestamp = allocations.back().timestamp;
    responsePtr->allocations = std::move(allocations);
    SendResponse(std::move(responsePtr), true);
    return true;
}

void QueryMemScopeAllocationHandler::PaddingAllocations(std::vector<MemoryAllocation> &allocations,
                                                        const MemScopeMemoryAllocationParams &queryParams)
{
    auto memoryDatabase = Timeline::DataBaseManager::Instance().GetMemScopeDatabase("");
    if (memoryDatabase == nullptr) {
        Server::ServerLog::Error("Get memscope memory database failed when padding allocations.");
        return;
    }
    uint64_t minTimestamp = 0;
    if (queryParams.relativeTime) {
        minTimestamp = memoryDatabase->GetGlobalMinTimestamp();
    }
    auto beforeAllocation = memoryDatabase->QueryLatestAllocationWithinTimestamp(queryParams.deviceId,
                                                                                 queryParams.eventType,
                                                                                 queryParams.startTimestamp + minTimestamp);
    if (!beforeAllocation.has_value()) {
        beforeAllocation = std::make_optional<MemoryAllocation>(queryParams.startTimestamp, 0,
                                                                queryParams.deviceId, queryParams.eventType,
                                                                queryParams.optimized);
    } else {
        beforeAllocation.value().timestamp = queryParams.startTimestamp;
    }
    allocations.insert(allocations.begin(), beforeAllocation.value());
    auto afterAllocation = memoryDatabase->QueryNextAllocationAfterTimestamp(queryParams.deviceId,
                                                                             queryParams.eventType,
                                                                             queryParams.endTimestamp + minTimestamp);
    if (!afterAllocation.has_value()) {
        afterAllocation =  std::make_optional<MemoryAllocation>(queryParams.endTimestamp,
                                                                allocations.back().totalSize,
                                                                queryParams.deviceId,
                                                                queryParams.eventType,
                                                                queryParams.optimized);
    } else {
        afterAllocation.value().timestamp = queryParams.endTimestamp;
    }
    allocations.emplace_back(afterAllocation.value());
}
} // Memory
} // Module
} // Dic