/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_SERVER_SESSION_H
#define DATA_INSIGHT_CORE_SERVER_SESSION_H

#include <memory>
#include <mutex>
#include <condition_variable>
#include "ServerDefs.h"
#include "Protocol.h"
#include "ProtocolMessageBuffer.h"
#include "ResponseQueue.h"

namespace Dic {
namespace Server {
class WsSession {
public:
    enum class Status {
        INIT,
        STARTED,
        CLOSED
    };
    explicit WsSession(WsChannel *channel);
    virtual ~WsSession();
    const WsChannel *GetChannel() const;
    bool BindToken(const std::string &token, const std::string &parentToken);
    const std::string &GetTokenString() const;
    const std::string &GetParentTokenString() const;
    const bool IsSubSession() const;
    const Status GetStatus() const;
    void SetStatus(Status sessionStatus);
    const uint32_t GetCreateTime() const;
    const uint32_t GetStartTime() const;
    const uint32_t GetStopTime() const;
    void SetDeadTime(const uint32_t time);
    const uint32_t GetDeadTime() const;

    void OnRequestMessage(const std::string &data);
    void OnResponse(std::unique_ptr<Protocol::Response> responsePtr);
    void OnEvent(std::unique_ptr<Protocol::Event> eventPtr);
    void Send(const std::string &message);
    void SendResponse(const Protocol::Response &response);
    void SendEvent(Protocol::Event &event);
    void Start();
    void Stop();
    void WaitForBindToken(int timeoutSeconds = 5);
    void WaitForExit(int milliSeconds = 10000);
    std::string GetDeviceKey();
    void SetDeviceKey(const std::string &device);
    std::string GetBundleName();
    void SetBundleName(const std::string &bundle);

protected:
    static void OnHandleMsgBuffer(WsSession &session);
    static void OnHandleResponseQueue(WsSession &session);
    bool CheckMessage(Protocol::ProtocolMessage &msg);
    void OnNotifyExit();
    const std::string GetMessageHeader(int length) const;

    WsChannel *channel = nullptr;
    uWS::Loop *loop = nullptr;
    uint32_t createTime = 0;
    uint32_t startTime = 0;
    uint32_t stopTime = 0;
    uint32_t deadTime = 0;
    volatile Status status;
    std::string parentTokenString = "";
    std::string tokenString;
    volatile bool hasBindToken = false;
    // wait for token
    std::unique_ptr<std::thread> waitForTokenThread = nullptr;
    // message buffer
    std::unique_ptr<Protocol::ProtocolMessageBuffer> msgBuffer = nullptr;
    std::unique_ptr<std::thread> onHandleMsgThread;
    std::mutex onExitMutex;
    std::condition_variable onExitCv;
    // response queue
    const bool useResponseQueue = false; // do not use response queue
    std::unique_ptr<ResponseQueue> responseQueue = nullptr;
    std::unique_ptr<std::thread> onHandleResponseThread = nullptr;
    std::condition_variable reqAndResCv;
    std::string deviceKey;
    std::string bundleName;
};
} // end of namespace Server
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_SERVER_SESSION_H
