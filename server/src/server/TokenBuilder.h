/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_TOKEN_BUILDER_H
#define DATA_INSIGHT_CORE_TOKEN_BUILDER_H

#include <string>
#include <mutex>

namespace Dic {
namespace Server {
class TokenBuilder {
public:
    static TokenBuilder &Instance();
    const std::string Build();

private:
    TokenBuilder() = default;
    ~TokenBuilder() = default;

    static const int maxTokenCode = 0x7dffffff;
    int tokenCode = 0;
    std::mutex tokenCodeMutex;
};
} // end of namespace Server
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_TOKEN_BUILDER_H
