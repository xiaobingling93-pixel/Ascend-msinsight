/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol Requests declaration
 */

#ifndef DIC_GLOBAL_PROTOCOL_REQUEST_H
#define DIC_GLOBAL_PROTOCOL_REQUEST_H

#include <string>
#include <optional>
#include "ProtocolDefs.h"
#include "ProtocolBase.h"

namespace Dic {
namespace Protocol {
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

} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_GLOBAL_PROTOCOL_REQUEST_H