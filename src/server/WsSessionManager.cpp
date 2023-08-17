/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#include "WsSessionManager.h"

namespace Dic {
namespace Server {
WsSessionManager &WsSessionManager::Instance()
{
    static WsSessionManager instance;
    return instance;
}

void WsSessionManager::AddSession(const std::string &token, std::unique_ptr<WsSession> session)
{
    std::unique_lock<std::mutex> lock(sessionMutex);
    sessionMap.emplace(token, std::move(session));
    sessionMap.at(token)->Start();
    sessionMap.at(token)->WaitForBindToken();
}

void WsSessionManager::RemoveSession(const std::string &token)
{
    std::unique_lock<std::mutex> lock(sessionMutex);
    sessionMap.erase(token);
}

void WsSessionManager::ClearSessions()
{
    std::unique_lock<std::mutex> lock(sessionMutex);
    sessionMap.clear();
}

WsSession *WsSessionManager::GetSession(const std::string &token)
{
    std::unique_lock<std::mutex> lock(sessionMutex);
    if (sessionMap.count(token) == 0) {
        return nullptr;
    }
    return sessionMap.at(token).get();
}

WsSession *WsSessionManager::GetSession(const WsChannel *channel)
{
    std::unique_lock<std::mutex> lock(sessionMutex);
    WsSession *session = nullptr;
    for (auto &iter : sessionMap) {
        if (iter.second->GetChannel() == channel) {
            session = iter.second.get();
            break;
        }
    }
    return session;
}

bool WsSessionManager::CheckSession(const std::string &token)
{
    WsSession *session = GetSession(token);
    if (session == nullptr) {
        return false;
    }
    if (session->GetStatus() == WsSession::Status::CLOSED) {
        return false;
    }
    return true;
}

void WsSessionManager::OnEventByMainSession(Protocol::Event &event)
{
    std::unique_lock<std::mutex> lock(sessionMutex);
    for (auto &iter : sessionMap) {
        if (!iter.second->IsSubSession()) {
            iter.second->SendEvent(event);
        }
    }
}

void WsSessionManager::OnParseSuccessEvent(const std::string &token, Protocol::ParseSuccessEvent &event)
{
    std::unique_lock<std::mutex> lock(sessionMutex);
    if (sessionMap.count(token) == 0) {
        return;
    }
    sessionMap.at(token)->SendEvent(event);
}

} // end of namespace Server
} // end of namespace Dic