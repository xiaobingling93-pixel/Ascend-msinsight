/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_WS_SERVER_H
#define DATA_INSIGHT_CORE_WS_SERVER_H

#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>
#include "ServerDefs.h"
#include "BaseServer.h"
#include "ApiHandler.h"

namespace Dic {
namespace Server {
class WsServer : public BaseServer {
public:
    WsServer(const std::string &host, int port, const std::string &sid);

    ~WsServer() noexcept override = default;

    bool Start() override;
    bool Stop() override;
    bool IsStart() const;

protected:
    static void ListenThreadFunc(WsServer &server);

    void StartListen();
    uWS::App::WebSocketBehavior<WsUserData> CreateWsBehavior();
    void ListenCb(us_listen_socket_t *listenSocket);
    void OnOpenCb(WsChannel *ws);
    void OnCloseCb(WsChannel *ws, int code, std::string_view message);
    void OnMessageCb(WsChannel *ws, std::string_view message, uWS::OpCode opCode);
    void LoadHandlers();
    void AddPostHandler(const std::string& key, std::shared_ptr<Core::ApiHandler> handler);
    void AddGetHandler(const std::string& key, std::shared_ptr<Core::ApiHandler> handler);

    std::unique_ptr<uWS::App> wsApp = nullptr;
    volatile bool listenStart = false;
    std::unique_ptr<std::thread> listenThreadPtr;
    std::mutex resultMutex;
    std::condition_variable resultCv;
    const int resultTimeout = 2000;
};
} // end of namespace Server
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_WS_SERVER_H
