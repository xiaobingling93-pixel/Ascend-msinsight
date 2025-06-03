// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
#include "WsSender.h"
#include "ModuleRequestHandler.h"
#include "WsSessionManager.h"
#include "ServerLog.h"
namespace Dic {
void SendEvent(std::unique_ptr<Dic::Protocol::Event> eventPtr)
{
    auto& wsSessionManager = Dic::Server::WsSessionManager::Instance();
    if (!wsSessionManager.CheckSession()) {
        Server::ServerLog::Warn("SendEvent failed. Can't get session");
        return;
    }
    Dic::Server::WsSession *session = wsSessionManager.GetSession();
    session->OnEvent(std::move(eventPtr));
}
void SendResponse(std::unique_ptr<Protocol::Response> responsePtr, bool result,
                  const std::string &errorMsg, const int errorCode)
{
    if (!Dic::Server::WsSessionManager::Instance().CheckSession()) {
        Server::ServerLog::Warn("SendResponse failed. Can't get session");
        return;
    }
    auto session = Server::WsSessionManager::Instance().GetSession();
    Module::ModuleRequestHandler::SetResponseResult(*responsePtr, result, errorMsg, errorCode);
    session->OnResponse(std::move(responsePtr));
}
}