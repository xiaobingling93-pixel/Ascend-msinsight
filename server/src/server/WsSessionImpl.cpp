/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */
#include "pch.h"
#include "TimeUtil.h"
#include "ModuleManager.h"
#include "ProtocolManager.h"
#include "WsSessionImpl.h"

namespace Dic {
namespace Server {
using namespace Dic::Protocol;
using namespace Dic::Module;
WsSessionImpl::WsSessionImpl(WsChannel *channel) : channel(channel), status(Status::INIT)
{
    loop = uWS::Loop::get();
    createTime = TimeUtil::Instance().NowUTC();
    msgBuffer = std::make_unique<ProtocolMessageBuffer>();
}


void WsSessionImpl::OnHandleMsgBuffer(WsSessionImpl &session)
{
    ServerLog::Info("Handle message buffer thread start.");
    const int interval = 10;
    while (true) {
        if (session.status == Status::CLOSED) {
            session.msgBuffer->Clear();
            break;
        }
        BatchHandleMsg(session);
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    }
    ServerLog::Info("Handle message buffer thread has exited.");
    session.OnNotifyExit();
}

void WsSessionImpl::BatchHandleMsg(WsSessionImpl &session)
{
    const uint8_t batchSize = 20;
    for (uint8_t i = 0; i < batchSize; i++) {
        std::unique_ptr<ProtocolMessage> msg = session.msgBuffer->Pop();
        if (msg == nullptr) {
            continue;
        }
        if (msg->type == ProtocolMessage::Type::REQUEST) {
            Request *reqPtr = dynamic_cast<Request *>(msg.release());
            if (reqPtr != nullptr) {
                ModuleManager::Instance().OnDispatchModuleRequest(std::unique_ptr<Request>(reqPtr));
            } else {
                ServerLog::Info("Request is not supported");
            }
        }
    }
}

void WsSessionImpl::OnHandleResponseQueue(WsSessionImpl &session)
{
    ServerLog::Info("Handle response queue thread start.");
    while (session.useResponseQueue) {
        if (session.status == Status::CLOSED) {
            session.responseQueue.Clear();
            break;
        }
        std::unique_ptr<Response> responsePtr = nullptr;
        if (session.responseQueue.Pop(responsePtr) && responsePtr == nullptr) {
            std::unique_lock<std::mutex> lock(session.responseQueueMutex);
            session.responseQueueCv.wait(lock);
            continue;
        }
        session.SendResponse(*responsePtr.get());
    }
    ServerLog::Info("Handle response queue thread has exited.");
}

void WsSessionImpl::OnNotifyExit()
{
    std::unique_lock<std::mutex> lock(onExitMutex);
    onExitCv.notify_all();
}

void WsSessionImpl::WaitForExit(int milliSeconds)
{
    std::unique_lock<std::mutex> lock(onExitMutex);
    onExitCv.wait_for(lock, std::chrono::milliseconds(milliSeconds));
}

const WsChannel *WsSessionImpl::GetChannel() const
{
    return channel;
}

WsSessionImpl::Status WsSessionImpl::GetStatus() const
{
    return status;
}

void WsSessionImpl::SetStatus(Status sessionStatus)
{
    if (this->status != sessionStatus) {
        this->status = sessionStatus;
    }
}

uint32_t WsSessionImpl::GetCreateTime() const
{
    return createTime;
}

uint32_t WsSessionImpl::GetStartTime() const
{
    return startTime;
}

uint32_t WsSessionImpl::GetStopTime() const
{
    return stopTime;
}

void WsSessionImpl::SetDeadTime(const uint32_t time)
{
    this->deadTime = time;
}

uint32_t WsSessionImpl::GetDeadTime() const
{
    return this->deadTime;
}

void WsSessionImpl::OnRequestMessage(const std::string &data)
{
    if (msgBuffer == nullptr) {
        return;
    }
    if (data.empty()) {
        return;
    }
    (*msgBuffer.get()) << data;
}

void WsSessionImpl::OnResponse(std::unique_ptr<Protocol::Response> responsePtr)
{
    if (responsePtr != nullptr) {
        if (useResponseQueue) {
            responseQueue << std::move(responsePtr);
            responseQueueCv.notify_one();
        } else {
            SendResponse(*responsePtr.get());
        }
    }
}

void WsSessionImpl::OnEvent(std::unique_ptr<Protocol::Event> eventPtr)
{
    if (eventPtr != nullptr) {
        SendEvent(*eventPtr.get());
    }
}

bool WsSessionImpl::Send(const std::string &message)
{
    if (GetStatus() == Status::CLOSED) {
        ServerLog::Info("Session has been closed.");
        return false;
    }
    if (channel != nullptr) {
        WsChannel::SendStatus res = channel->send(message, uWS::OpCode::TEXT, false);
        return res == WsChannel::SendStatus::SUCCESS;
    } else {
        ServerLog::Error("Channel is null, so that message cannot be sent.");
        return false;
    }
}

void WsSessionImpl::SendResponse(const Protocol::Response &response)
{
    std::string error;
    std::optional<document_t> json = ProtocolManager::Instance().ToJson(response, error);
    if (!json.has_value()) {
        ServerLog::Error(error);
        return;
    }
    std::string responseStr = JsonUtil::JsonDump(json.value());
    // send header + response
    loop->defer([this, responseStr, response]() {
        bool res = Send(responseStr);
        ServerLog::Info("Send response status: ", res, ", response result: ", response.result, ", command: ",
                        response.command, ", request id = ", response.requestId, ", response id = ", response.id);
    });
}

void WsSessionImpl::SendEvent(Protocol::Event &event)
{
    std::string error;
    std::optional<document_t> json = ProtocolManager::Instance().ToJson(event, error);
    if (!json.has_value()) {
        ServerLog::Info(error);
        return;
    }
    std::string eventStr = JsonUtil::JsonDump(json.value());
    // send header + response
    loop->defer([this, eventStr, event]() {
        bool res = Send(eventStr);
        ServerLog::Info("Send event status: ", res, ", event result: ", event.result, ", event name:", event.event,
                        ", event id = ", event.id);
    });
}

void WsSessionImpl::Start()
{
    SetStatus(Status::STARTED);
    startTime = TimeUtil::Instance().NowUTC();
    if (useResponseQueue) {
        // start response queue handle thread
        onHandleResponseThread = std::make_unique<std::thread>(OnHandleResponseQueue, std::ref(*this));
        onHandleResponseThread->detach();
    }
    // start request message handle thread
    onHandleMsgThread = std::make_unique<std::thread>(OnHandleMsgBuffer, std::ref(*this));
    onHandleMsgThread->detach();
}

void WsSessionImpl::Stop()
{
    status = Status::CLOSED;
    if (channel != nullptr) {
        channel->close();
        channel = nullptr;
    }
    stopTime = TimeUtil::Instance().NowUTC();
}

std::string WsSessionImpl::GetDeviceKey()
{
    return deviceKey;
}

void WsSessionImpl::SetDeviceKey(const std::string &device)
{
    deviceKey = device;
}

std::string WsSessionImpl::GetBundleName()
{
    return bundleName;
}

void WsSessionImpl::SetBundleName(const std::string &bundle)
{
    bundleName = bundle;
}
} // end of namespace Server
} // end of namespace Dic
