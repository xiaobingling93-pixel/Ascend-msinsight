/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: Entry of data insight core server binary
 */

#include "pch.h"
#include "WsSessionManager.h"
#include "WsDefs.h"
#include "WsServer.h"

namespace Dic {
namespace Server {
WsServer::WsServer(const std::string &host, int port, const std::string &sid) : BaseServer(host, port, sid) {}

bool WsServer::Start()
{
    listenStart = false;
    listenThreadPtr = std::make_unique<std::thread>(WsServer::ListenThreadFunc, std::ref(*this));
    if (listenThreadPtr == nullptr) {
        return false;
    }
    listenThreadPtr->detach();
    std::unique_lock<std::mutex> lock(resultMutex);
    resultCv.wait_for(lock, std::chrono::milliseconds(resultTimeout), [&] { return this->listenStart; });
    return listenStart;
}

bool WsServer::Stop()
{
    listenStart = false;
    return true;
}

bool WsServer::IsStart() const
{
    return listenStart;
}

void WsServer::ListenThreadFunc(WsServer &server)
{
    server.StartListen();
}

void WsServer::StartListen()
{
    ServerLog::Info("Start server listen");
    wsApp = std::make_unique<uWS::App>();
    uWS::App::WebSocketBehavior<WsUserData> behavior = CreateWsBehavior();
    wsApp->ws<WsUserData>("/*", std::move(behavior));
    wsApp->listen(host, port, std::bind(&WsServer::ListenCb, this, std::placeholders::_1));
    ServerLog::Info("Run server");
    wsApp->run();
}

uWS::App::WebSocketBehavior<WsUserData> WsServer::CreateWsBehavior()
{
    const int maxPayLoadSize = 16 * 1024 * 1024;
    const int idleTimeout = 120;
    const int maxBackPressureSize = 1 * 1024 * 1024;
    uWS::App::WebSocketBehavior<WsUserData> wsBehavior = {
        .compression = uWS::SHARED_COMPRESSOR,
        .maxPayloadLength = maxPayLoadSize,
        .idleTimeout = idleTimeout,
        .maxBackpressure = maxBackPressureSize,
        .closeOnBackpressureLimit = false,
        .resetIdleTimeoutOnSend = false,
        .sendPingsAutomatically = true,
        .maxLifetime = 0,
        .upgrade =
            [](auto *res, uWS::HttpRequest *req, auto *context) {
                std::string_view sid = req->getQuery("sid");
                std::string_view url = req->getUrl();
                res->template upgrade<WsUserData>(
                    {
                        .reqUrl = url.data(), .sid = std::string(sid) },
                    req->getHeader("sec-websocket-key"), req->getHeader("sec-websocket-protocol"),
                    req->getHeader("sec-websocket-extensions"), context);
            },
        .open = std::bind(&WsServer::OnOpenCb, this, std::placeholders::_1),
        .message = std::bind(&WsServer::OnMessageCb, this, std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3),
        .drain = nullptr,
        .ping = nullptr,
        .pong = nullptr,
        .subscription = nullptr,
        .close =
            std::bind(&WsServer::OnCloseCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    };
    return wsBehavior;
}

void WsServer::ListenCb(us_listen_socket_t *listenSocket)
{
    if (listenSocket == nullptr) {
        ServerLog::Error("Failed to start server, listen error.");
        return;
    }
    listenStart = true;
    resultCv.notify_one();
    ServerLog::Info("Start server successfully.");
}

void WsServer::OnOpenCb(WsChannel *ws)
{
    ServerLog::Info("Accept new session, channel = ", ws);
    if (ws == nullptr) {
        ServerLog::Info("Accept new session, channel is null ");
        return;
    }
    std::string url = ws->getUserData()->reqUrl;
    if (url.empty()) {
        ServerLog::Info("Accept new session failed, channel = ", ws, " url is null");
        ws->end(URL_NULL_CODE, "url is null");
        return;
    }
    if (!sid.empty() && sid != ws->getUserData()->sid) {
        ServerLog::Info("Accept new session failed, channel = ", ws, " sid is not correct");
        ws->end(SID_UN_CORRECT_CODE, WS_CLOSE_CODE_REASON.at(SID_UN_CORRECT_CODE));
        return;
    }
    if (WsSessionManager::Instance().GetSession() != nullptr) {
        ServerLog::Error("Not Connect, already connecting");
        return;
    }
    std::unique_ptr<WsSession> session = std::make_unique<WsSession>(ws);
    WsSessionManager::Instance().AddSession(std::move(session));
}

void WsServer::OnCloseCb(WsChannel *ws, int code, std::string_view message)
{
    ServerLog::Info("Session close, channel = ", ws, ", code = ", code, ", message = ", message);
    if (ws == nullptr) {
        return;
    }
    WsSession *session = WsSessionManager::Instance().GetSession(ws);
    if (session != nullptr) {
        session->SetStatus(WsSession::Status::CLOSED);
        session->WaitForExit();
        WsSessionManager::Instance().RemoveSession();
        ServerLog::Info("Session remove ok.");
    }
}

void WsServer::OnMessageCb(WsChannel *ws, std::string_view message, uWS::OpCode opCode)
{
    if (ws == nullptr) {
        return;
    }
    WsSession *session = WsSessionManager::Instance().GetSession(ws);
    if (session == nullptr) {
        ServerLog::Error("Session is not valid, it will be closed at server.");
        return;
    }
    session->OnRequestMessage(std::string(message));
}
} // end of namespace Server
} // end of namespace Dic