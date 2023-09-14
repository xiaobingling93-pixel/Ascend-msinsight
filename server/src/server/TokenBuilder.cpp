/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include "TokenBuilder.h"

namespace Dic {
namespace Server {

TokenBuilder &TokenBuilder::Instance()
{
    static TokenBuilder instance;
    return instance;
}

const std::string TokenBuilder::Build()
{
    static const std::string TOKEN_HEAD = "dic_server_token_";
    std::lock_guard<std::mutex> lock(tokenCodeMutex);
    if (tokenCode >= maxTokenCode) {
        tokenCode = 0;
    }
    std::string tokenString = TOKEN_HEAD + std::to_string(++tokenCode);
    return tokenString;
}
} // end of namespace Server
} // end of namespace Dic