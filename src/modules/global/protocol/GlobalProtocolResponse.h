/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol Response declaration
 */

#ifndef DIC_GLOBAL_PROTOCOL_RESPONSE_H
#define DIC_GLOBAL_PROTOCOL_RESPONSE_H

#include <vector>
#include "GlobalDefs.h"
#include "ProtocolDefs.h"
#include "ProtocolBase.h"

namespace Dic {
namespace Protocol {
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

} // end of namespace Protocol
} // end of namespace Dic

#endif // DIC_GLOBAL_PROTOCOL_RESPONSE_H
