/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include "TokenBuilder.h"

namespace Dic {
namespace Server {
const int TokenBuilder::MAX_TOKEN_CODE = 0x7dffffff;
const std::string TokenBuilder::TOKEN_HEAD = "dic_server_token_";

TokenBuilder &TokenBuilder::Instance()
{
    static TokenBuilder instance;
    return instance;
}

const std::string TokenBuilder::Build()
{
    std::lock_guard<std::mutex> lock(tokenCodeMutex);
    if (tokenCode >= MAX_TOKEN_CODE) {
        tokenCode = 0;
    }
    std::string tokenString = TOKEN_HEAD + std::to_string(++tokenCode);
    return tokenString;
}
} // end of namespace Server
} // end of namespace Dic