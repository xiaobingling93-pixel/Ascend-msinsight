/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
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
