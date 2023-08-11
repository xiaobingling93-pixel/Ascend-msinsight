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
#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_PROTOCOL_REQUEST_H