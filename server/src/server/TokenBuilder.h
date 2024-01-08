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
};
} // end of namespace Server
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_TOKEN_BUILDER_H
