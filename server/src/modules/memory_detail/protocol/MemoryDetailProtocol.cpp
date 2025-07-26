/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "pch.h"
#include "ProtocolDefs.h"
#include "TimelineProtocol.h"
#include "MemoryDetailProtocolRequest.h"
#include "MemoryDetailProtocol.h"

namespace Dic::Protocol {
void MemoryDetailProtocolUtil::RegisterJsonToRequestFuncs()
{
    jsonToReqFactory.emplace(REQ_RES_LEAKS_MEMORY_ALLOCATIONS, ProtocolUtil::BuildRequestFromJson<LeaksMemoryAllocationRequest>);
    jsonToReqFactory.emplace(REQ_RES_LEAKS_MEMORY_BLOCKS, ProtocolUtil::BuildRequestFromJson<LeaksMemoryBlockRequest>);
    jsonToReqFactory.emplace(REQ_RES_LEAKS_MEMORY_DETAILS, ProtocolUtil::BuildRequestFromJson<LeaksMemoryDetailRequest>);
    jsonToReqFactory.emplace(REQ_RES_LEAKS_MEMORY_TRACES, ProtocolUtil::BuildRequestFromJson<LeaksMemoryTraceRequest>);
}

void MemoryDetailProtocolUtil::RegisterResponseToJsonFuncs()
{
    resToJsonFactory.emplace(REQ_RES_LEAKS_MEMORY_ALLOCATIONS, ProtocolUtil::CommonResponseToJson);
    resToJsonFactory.emplace(REQ_RES_LEAKS_MEMORY_BLOCKS, ProtocolUtil::CommonResponseToJson);
    resToJsonFactory.emplace(REQ_RES_LEAKS_MEMORY_DETAILS, ProtocolUtil::CommonResponseToJson);
    resToJsonFactory.emplace(REQ_RES_LEAKS_MEMORY_TRACES, ProtocolUtil::CommonResponseToJson);
}

void MemoryDetailProtocolUtil::RegisterEventToJsonFuncs()
{
    eventToJsonFactory.emplace(EVENT_PARSE_LEAKS_MEMORY_COMPLETED, TimelineProtocol::ToLeaksParseSuccessEventJson);
    eventToJsonFactory.emplace(EVENT_MODULE_RESET, TimelineProtocol::ToModuleResetEventJson);
    eventToJsonFactory.emplace(EVENT_ALL_SUCCESS, TimelineProtocol::ToAllSuccessEventJson);
}
}  // end of namespace Protocol
 // end of namespace Dic
