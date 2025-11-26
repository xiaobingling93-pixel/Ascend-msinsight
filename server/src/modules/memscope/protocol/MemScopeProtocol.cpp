/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "pch.h"
#include "ProtocolDefs.h"
#include "TimelineProtocol.h"
#include "MemScopeProtocolRequest.h"
#include "MemScopeProtocol.h"

namespace Dic::Protocol {
void MemScopeProtocolUtil::RegisterJsonToRequestFuncs()
{
    jsonToReqFactory.emplace(REQ_RES_MEM_SCOPE_MEMORY_ALLOCATIONS,
                             ProtocolUtil::BuildRequestFromJson<MemScopeMemoryAllocationRequest>);
    jsonToReqFactory.emplace(REQ_RES_MEM_SCOPE_MEMORY_BLOCKS,
                             ProtocolUtil::BuildRequestFromJson<MemScopeMemoryBlockRequest>);
    jsonToReqFactory.emplace(REQ_RES_MEM_SCOPE_MEMORY_DETAILS,
                             ProtocolUtil::BuildRequestFromJson<MemScopeMemoryDetailRequest>);
    jsonToReqFactory.emplace(REQ_RES_MEM_SCOPE_PYTHON_TRACES,
                             ProtocolUtil::BuildRequestFromJson<MemScopePythonTraceRequest>);
    jsonToReqFactory.emplace(REQ_RES_MEM_SCOPE_EVENTS,
                             ProtocolUtil::BuildRequestFromJson<MemScopeEventRequest>);
}

void MemScopeProtocolUtil::RegisterResponseToJsonFuncs()
{
    resToJsonFactory.emplace(REQ_RES_MEM_SCOPE_MEMORY_ALLOCATIONS, ProtocolUtil::CommonResponseToJson);
    resToJsonFactory.emplace(REQ_RES_MEM_SCOPE_MEMORY_BLOCKS, ProtocolUtil::CommonResponseToJson);
    resToJsonFactory.emplace(REQ_RES_MEM_SCOPE_MEMORY_DETAILS, ProtocolUtil::CommonResponseToJson);
    resToJsonFactory.emplace(REQ_RES_MEM_SCOPE_PYTHON_TRACES, ProtocolUtil::CommonResponseToJson);
    resToJsonFactory.emplace(REQ_RES_MEM_SCOPE_EVENTS, ProtocolUtil::CommonResponseToJson);
}

void MemScopeProtocolUtil::RegisterEventToJsonFuncs()
{
    eventToJsonFactory.emplace(EVENT_PARSE_MEM_SCOPE_COMPLETED, ProtocolUtil::CommonEventToJson);
    eventToJsonFactory.emplace(EVENT_MODULE_RESET, TimelineProtocol::ToModuleResetEventJson);
    eventToJsonFactory.emplace(EVENT_ALL_SUCCESS, TimelineProtocol::ToAllSuccessEventJson);
}
}  // end of namespace Protocol
 // end of namespace Dic
