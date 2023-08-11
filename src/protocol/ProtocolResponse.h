/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol Response declaration
 */

#ifndef DIC_PROTOCOL_RESPONSE_H
#define DIC_PROTOCOL_RESPONSE_H

#include <vector>
#include "GlobalDefs.h"
#include "ProtocolDefs.h"
#include "ProtocolEntity.h"
#include "ProtocolBase.h"

namespace Dic {
namespace Protocol {
#pragma region << Global>>

// token create
// token.create can be sent to master session and slave sessions.
struct TokenCreateResBody {
    uint32_t createTime = 0; // UTC
    std::optional<std::string> parentToken;
};

struct TokenCreateResponse : public Response {
    TokenCreateResponse() : Response(REQ_RES_TOKEN_CREATE) {};
    TokenCreateResBody body;
};

// token.destroy
struct TokenDestroyResBody {
    uint32_t destroyTime = 0; // UTC
    std::string destroyToken;
};

struct TokenDestroyResponse : public Response {
    TokenDestroyResponse() : Response(REQ_RES_TOKEN_DESTROY) {};
    TokenDestroyResBody body;
};

// token.check
struct TokenCheckResBody {
    uint32_t createTime = 0;
    uint32_t deadTime = 0;
    std::string checkedToken;
    bool isSubToken = false;
};

struct TokenCheckResponse : public Response {
    TokenCheckResponse() : Response(REQ_RES_TOKEN_CHECK) {};
    TokenCheckResBody body;
};

// config.get
struct ConfigGetResBody {
    std::optional<GlobalConfig> globalConfig;
    std::optional<HarmonyConfig> harmonyConfig;
};

struct ConfigGetResponse : public Response {
    ConfigGetResponse() : Response(REQ_RES_CONFIG_GET) {};
    ConfigGetResBody body;
};

// config.set
struct ConfigSetResBody {
    uint32_t configSetTime = 0;
    bool isAlertMsg = false;
};

struct ConfigSetResponse : public Response {
    ConfigSetResponse() : Response(REQ_RES_CONFIG_SET) {};
    ConfigSetResBody body;
};

#pragma endregion

#pragma region << harmony>>
// hdc device list
struct HdcDeviceListResBody {
    std::vector<Device> deviceList;
};

struct HdcDeviceListResponse : public Response {
    HdcDeviceListResponse() : Response(REQ_RES_HDC_DEVICE_LIST) {}
    HdcDeviceListResBody body;
};
#pragma endregion
} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_PROTOCOL_RESPONSE_H
