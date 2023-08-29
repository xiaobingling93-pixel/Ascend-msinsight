/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol Requests declaration
 */

#ifndef DIC_TIMELINE_PROTOCOL_REQUEST_H
#define DIC_TIMELINE_PROTOCOL_REQUEST_H

#include <string>
#include <optional>
#include "ProtocolDefs.h"
#include "ProtocolBase.h"

namespace Dic {
namespace Protocol {
struct ImportActionParams {
    std::string path;
};

struct ImportActionRequest : public Request {
    ImportActionRequest() : Request(REQ_RES_IMPORT_ACTION) {};
    ImportActionParams params;
};

struct UnitThreadTracesParams {
    std::string cardId;
    std::string processId;
    int32_t threadId = 0;
    uint64_t startTime = 0;
    uint64_t endTime = 0;
};

struct UnitThreadTracesRequest : public Request {
    UnitThreadTracesRequest() : Request(REQ_RES_UNIT_THREAD_TRACES) {};
    UnitThreadTracesParams params;
};

struct UnitThreadsParams {
    std::string rankId;
    uint32_t tid = 0;
    std::string pid;
    uint64_t startTime = 0;
    uint64_t endTime = 0;
};

struct UnitThreadsRequest : public Request {
    UnitThreadsRequest() : Request(REQ_RES_UNIT_THREADS) {};
    UnitThreadsParams params;
};

struct ThreadDetailParams {
    std::string rankId;
    std::string pid;
    uint32_t tid = 0;
    uint64_t startTime = 0;
    uint32_t depth = 0;
};

struct ThreadDetailRequest : public Request {
    ThreadDetailRequest() : Request(REQ_RES_UNIT_THREAD_DETAIL) {};
    ThreadDetailParams params;
};

struct UnitFlowNameParams {
    std::string rankId;
    int64_t tid = 0;
    std::string pid;
    uint64_t startTime = 0;
};

struct UnitFlowNameRequest : public Request {
    UnitFlowNameRequest() : Request(REQ_RES_UNIT_FLOW_NAME) {};
    UnitFlowNameParams params;
};

struct UnitFlowParams {
    std::string flowId;
    std::string rankId;
};

struct UnitFlowRequest : public Request {
    UnitFlowRequest() : Request(REQ_RES_UNIT_FLOW) {};
    UnitFlowParams params;
};

struct ResetWindowParams {
};

struct ResetWindowRequest : public Request {
    ResetWindowRequest() : Request(REQ_RES_RESET_WINDOW) {};
    ResetWindowParams params;
};

struct UnitChartParams {
    std::string param;
};

struct UnitChartRequest : public Request {
    UnitChartRequest() : Request(REQ_RES_UNIT_CHART) {};
    UnitChartParams params;
};

} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_TIMELINE_PROTOCOL_REQUEST_H