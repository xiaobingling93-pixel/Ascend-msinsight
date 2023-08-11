/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_SERVER_SESSION_MANAGER_H
#define DATA_INSIGHT_CORE_SERVER_SESSION_MANAGER_H

#include <memory>
#include <string>
#include <mutex>
#include <optional>
#include "WsSession.h"
#include "ProtocolEvent.h"

namespace Dic {
namespace Server {
class WsSessionManager {
public:
    static WsSessionManager &Instance();

    void AddSession(const std::string &token, std::unique_ptr<WsSession> session);
    void RemoveSession(const std::string &token);
    WsSession *GetSession(const std::string &token);
    WsSession *GetSession(const WsChannel *channel);
    bool CheckSession(const std::string &token);
    void ClearSessions();
    void OnEventByMainSession(Protocol::Event &event);

private:
    WsSessionManager() = default;
    ~WsSessionManager() = default;

    std::mutex sessionMutex;
    std::map<std::string, std::unique_ptr<WsSession>> sessionMap;
};
} // end of namespace Server
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_SERVER_SESSION_MANAGER_H
