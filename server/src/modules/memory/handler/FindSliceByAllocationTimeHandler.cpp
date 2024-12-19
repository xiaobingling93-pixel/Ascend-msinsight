/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "TraceTime.h"
#include "FindSliceByAllocationTimeHandler.h"

namespace Dic::Module::Memory {
using namespace Dic::Server;

bool FindSliceByAllocationTimeHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    MemoryFindSliceRequest &request = dynamic_cast<MemoryFindSliceRequest &>(*requestPtr.get());
    std::unique_ptr<MemoryFindSliceResponse> responsePtr = std::make_unique<MemoryFindSliceResponse>();
    MemoryFindSliceResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    if (renderEngine == nullptr) {
        SendResponse(std::move(responsePtr), false, "Failed to find slice. timeline not exist!");
        ServerLog::Warn("Failed to find slice. timeline not exist!");
        return false;
    }
    OperatorDomain target = operatorMemoryService->ComputeAllocationTimeById(request.params.fileId, request.params.id);
    if (std::empty(target.metaType)) {
        SendResponse(std::move(responsePtr), false, "Failed to query memory operator");
        ServerLog::Warn("Failed to query memory operator!");
        return false;
    }
    Timeline::CompeteSliceDomain slice = renderEngine->FindSliceByTimePoint(request.params.fileId, request.params.name,
        target.allocationTime, target.metaType);
    if (std::empty(slice.tid) && std::empty(slice.pid)) {
        SendResponse(std::move(responsePtr), false, "Failed to find slice in timeline!");
        return false;
    }
    response.data.metaType = target.metaType;
    response.data.depth = slice.depth;
    response.data.processId = slice.pid;
    response.data.threadId = slice.tid;
    response.data.rankId = slice.cardId;
    response.data.startTime = slice.timestamp - Timeline::TraceTime::Instance().GetStartTime();
    response.data.id = std::to_string(slice.id);
    response.data.duration = slice.duration;
    SendResponse(std::move(responsePtr), true);
    return true;
}
}
