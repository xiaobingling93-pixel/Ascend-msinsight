/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol Requests declaration
 */

#ifndef DIC_PROTOCOL_REQUEST_H
#define DIC_PROTOCOL_REQUEST_H

#include <string>
#include <optional>
#include "ProtocolDefs.h"
#include "ProtocolBase.h"
#include "ProtocolEntity.h"

namespace Dic {
namespace Protocol {
#pragma region << Global>>

// token.create
struct TokenCreateParams {
    uint32_t deadTime = 0;
    // when parentToken has value, request must be sent to sub sessions
    std::optional<std::string> parentToken;
};

struct TokenCreateRequest : public Request {
    TokenCreateRequest() : Request(REQ_RES_TOKEN_CREATE) {}
    TokenCreateParams params;
};

// security.subAuth
struct TokenDestroyParams {
    std::string destroyToken;
};

struct TokenDestroyRequest : public Request {
    TokenDestroyRequest() : Request(REQ_RES_TOKEN_DESTROY) {}
    TokenDestroyParams params;
};

// token.check
struct TokenCheckParams {
    std::string checkedToken;
};

struct TokenCheckRequest : public Request {
    TokenCheckRequest() : Request(REQ_RES_TOKEN_CHECK) {}
    TokenCheckParams params;
};

// config.get
struct ConfigGetParams {
    int sceneMask = -1; // if -1, get all configs;
};

struct ConfigGetRequest : public Request {
    ConfigGetRequest() : Request(REQ_RES_CONFIG_GET) {}
    ConfigGetParams params;
};

// config.set
struct ConfigSetParams {
    std::optional<GlobalConfig> globalConfig;
    std::optional<HarmonyConfig> harmonyConfig;
};

struct ConfigSetRequest : public Request {
    ConfigSetRequest() : Request(REQ_RES_CONFIG_SET) {}
    ConfigSetParams params;
};

#pragma endregion

#pragma region << harmony>>
// hdc device list
struct HdcDeviceListParams {
    int timeout = 3000;
};

struct HdcDeviceListRequest : public Request {
    HdcDeviceListRequest() : Request(REQ_RES_HDC_DEVICE_LIST) {};
    HdcDeviceListParams params;
};

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
    uint32_t threadId = 0;
    uint64_t startTime;
    uint64_t endTime;
};

struct UnitThreadTracesRequest : public Request {
    UnitThreadTracesRequest() : Request(REQ_RES_UNIT_THREAD_TRACES) {};
    UnitThreadTracesParams params;
};

struct UnitThreadsParams {
    std::string rankId;
    uint32_t tid = 0;
    std::string pid;
    uint64_t startTime;
    uint64_t endTime;
};

struct UnitThreadsRequest : public Request {
    UnitThreadsRequest() : Request(REQ_RES_UNIT_THREADS) {};
    UnitThreadsParams params;
};

struct ThreadDetailParams {
    std::string rankId;
    std::string pid;
    uint32_t tid = 0;
    uint64_t startTime;
    uint32_t depth = 0;
};

struct ThreadDetailRequest : public Request {
    ThreadDetailRequest() : Request(REQ_RES_UNIT_THREAD_DETAIL) {};
    ThreadDetailParams params;
};

struct UnitFlowNameParams {
    std::string rankId;
    uint32_t tid = 0;
    std::string pid;
    uint64_t startTime;
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
#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_PROTOCOL_REQUEST_H