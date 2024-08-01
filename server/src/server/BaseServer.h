/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_BASE_SERVER_H
#define DATA_INSIGHT_CORE_BASE_SERVER_H

#include <string>
#include "WsSession.h"

namespace Dic {
namespace Server {
class BaseServer {
public:
    BaseServer(const std::string &host, int port, const std::string &sid) : host(host), port(port), sid(sid) {}
    virtual ~BaseServer() = default;
    virtual bool Start() = 0;
    virtual bool Stop() = 0;
    const std::string &GetHost() const
    {
        return host;
    }
    int GetPort() const
    {
        return port;
    };
    const std::string &GetSid() const
    {
        return sid;
    };

protected:
    std::string host = "127.0.0.1";
    int port;
    std::string sid;
};
} // end of namespace Server
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_BASE_SERVER_H
