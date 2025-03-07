/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_SERVER_SESSION_H
#define DATA_INSIGHT_CORE_SERVER_SESSION_H

#include <memory>
#include <mutex>
#include <condition_variable>
#include "WsSession.h"
#include "ServerDefs.h"
#include "SafeQueue.h"
#include "ProtocolUtil.h"
#include "ProtocolMessageBuffer.h"

namespace Dic {
namespace Server {
class WsSessionImpl : public WsSession {
public:
    explicit WsSessionImpl(WsChannel *channel);
    virtual ~WsSessionImpl(){};
    const WsChannel *GetChannel() const;
    Status GetStatus() const override;
    void SetStatus(Status sessionStatus) override;
    uint32_t GetCreateTime() const;
    uint32_t GetStartTime() const;
    uint32_t GetStopTime() const;
    void SetDeadTime(const uint32_t time);
    uint32_t GetDeadTime() const;

    void OnRequestMessage(const std::string &data);
    void OnResponse(std::unique_ptr<Protocol::Response> responsePtr) override;
    void OnEvent(std::unique_ptr<Protocol::Event> eventPtr) override;
    bool Send(const std::string &message);
    void SendResponse(const Protocol::Response &response);
    void SendEvent(Protocol::Event &event);
    void Start() override;
    void Stop();
    void WaitForExit(int milliSeconds = 10000) override;
    std::string GetDeviceKey();
    void SetDeviceKey(const std::string &device);
    std::string GetBundleName();
    void SetBundleName(const std::string &bundle);

protected:
    static void OnHandleMsgBuffer(WsSessionImpl &session);
    static void OnHandleResponseQueue(WsSessionImpl &session);
    void OnNotifyExit();

    WsChannel *channel = nullptr;
    uWS::Loop *loop = nullptr;
    uint32_t createTime = 0;
    uint32_t startTime = 0;
    uint32_t stopTime = 0;
    uint32_t deadTime = 0;
    volatile Status status;
    // message buffer
    std::unique_ptr<Protocol::ProtocolMessageBuffer> msgBuffer = nullptr;
    std::unique_ptr<std::thread> onHandleMsgThread;
    std::mutex onExitMutex;
    std::condition_variable onExitCv;
    // response queue
    const bool useResponseQueue = false; // do not use response queue
    SafeQueue<std::unique_ptr<Protocol::Response>> responseQueue;;
    std::unique_ptr<std::thread> onHandleResponseThread = nullptr;
    std::mutex responseQueueMutex;
    std::condition_variable responseQueueCv;
    std::string deviceKey;
    std::string bundleName;

    static void BatchHandleMsg(WsSessionImpl &session);
};
} // end of namespace Server
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_SERVER_SESSION_H
