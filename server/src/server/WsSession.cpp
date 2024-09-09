/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */
#include "pch.h"
#include "LogUtil.h"
#include "ModuleManager.h"
#include "ProtocolManager.h"
#include "WsSession.h"

namespace Dic {
namespace Server {
using namespace Dic::Protocol;
using namespace Dic::Module;
WsSession::WsSession(WsChannel *channel) : channel(channel), status(Status::INIT)
{
    loop = uWS::Loop::get();
    createTime = TimeUtil::Instance().NowUTC();
    msgBuffer = std::make_unique<ProtocolMessageBuffer>();
}


void WsSession::OnHandleMsgBuffer(WsSession &session)
{
    ServerLog::Info("Handle message buffer thread start.");
    const int interval = 10;
    while (true) {
        if (session.status == Status::CLOSED) {
            session.msgBuffer->Clear();
            break;
        }
        std::unique_ptr<ProtocolMessage> msg = session.msgBuffer->Pop();
        if (msg == nullptr) {
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
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
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    }
    ServerLog::Info("Handle message buffer thread has exited.");
    session.OnNotifyExit();
}

void WsSession::OnHandleResponseQueue(WsSession &session)
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

void WsSession::OnNotifyExit()
{
    std::unique_lock<std::mutex> lock(onExitMutex);
    onExitCv.notify_all();
}

void WsSession::WaitForExit(int milliSeconds)
{
    std::unique_lock<std::mutex> lock(onExitMutex);
    onExitCv.wait_for(lock, std::chrono::milliseconds(milliSeconds));
}

const std::string WsSession::GetMessageHeader(int length) const
{
    const std::string delimiter = "\r\n\r\n";
    std::string result = "Content-Length:";
    result.append(std::to_string(length));
    result.append(delimiter);
    return result;
}

const WsChannel *WsSession::GetChannel() const
{
    return channel;
}

WsSession::Status WsSession::GetStatus() const
{
    return status;
}

void WsSession::SetStatus(Status sessionStatus)
{
    if (this->status != sessionStatus) {
        this->status = sessionStatus;
    }
}

uint32_t WsSession::GetCreateTime() const
{
    return createTime;
}

uint32_t WsSession::GetStartTime() const
{
    return startTime;
}

uint32_t WsSession::GetStopTime() const
{
    return stopTime;
}

void WsSession::SetDeadTime(const uint32_t time)
{
    this->deadTime = time;
}

uint32_t WsSession::GetDeadTime() const
{
    return this->deadTime;
}

void WsSession::OnRequestMessage(const std::string &data)
{
    if (msgBuffer == nullptr) {
        return;
    }
    if (data.empty()) {
        return;
    }
    (*msgBuffer.get()) << data;
}

void PrintResponseInfo(const Protocol::Response &response)
{
    if (response.result) {
        ServerLog::Info("Send response success: ", response.command, ", request id = ",
                        response.requestId, ", response id = ", response.id, "\n");
    } else {
        ServerLog::Info("Send response failure: ", response.command, ", request id = ",
                        response.requestId, ", response id = ", response.id, "\n");
    }
}

void WsSession::OnResponse(std::unique_ptr<Protocol::Response> responsePtr)
{
    if (responsePtr != nullptr) {
        if (useResponseQueue) {
            responseQueue << std::move(responsePtr);
            responseQueueCv.notify_one();
        } else {
            SendResponse(*responsePtr.get());
            PrintResponseInfo(*responsePtr);
        }
    }
}

void WsSession::OnEvent(std::unique_ptr<Protocol::Event> eventPtr)
{
    if (eventPtr != nullptr) {
        SendEvent(*eventPtr.get());
    }
}

void WsSession::Send(const std::string &message)
{
    if (GetStatus() == Status::CLOSED) {
        ServerLog::Info("Session has been closed.");
        return;
    }
    if (channel != nullptr) {
        channel->send(message, uWS::OpCode::TEXT, false);
    } else {
        ServerLog::Error("Channel is null, so that message can not be sent.");
    }
}

void WsSession::SendResponse(const Protocol::Response &response)
{
    std::string error;
    std::optional<document_t> json = ProtocolManager::Instance().ToJson(response, error);
    if (!json.has_value()) {
        ServerLog::Error(error);
        return;
    }
    std::string responseStr = JsonUtil::JsonDump(json.value());
    std::string responseHeader = GetMessageHeader(responseStr.length());
    // send header + response
    loop->defer([this, responseHeader, responseStr]() {
        Send(responseHeader);
        Send(responseStr);
    });
}

void WsSession::SendEvent(Protocol::Event &event)
{
    std::string error;
    std::optional<document_t> json = ProtocolManager::Instance().ToJson(event, error);
    if (!json.has_value()) {
        ServerLog::Info(error);
        return;
    }
    std::string eventStr = JsonUtil::JsonDump(json.value());
    std::string eventHeader = GetMessageHeader(eventStr.length());
    // send header + response
    loop->defer([this, eventStr, eventHeader]() {
        Send(eventHeader);
        Send(eventStr);
        ServerLog::Info("Send event end.");
    });
    if (event.result) {
        ServerLog::Info("Send event success: ", event.event, ", event id = ", event.id);
    } else {
        ServerLog::Info("Send event failure: ", event.event, ", event id = ", event.id);
    }
}

void WsSession::Start()
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

void WsSession::Stop()
{
    status = Status::CLOSED;
    if (channel != nullptr) {
        channel->close();
        channel = nullptr;
    }
    stopTime = TimeUtil::Instance().NowUTC();
}

std::string WsSession::GetDeviceKey()
{
    return deviceKey;
}

void WsSession::SetDeviceKey(const std::string &device)
{
    deviceKey = device;
}

std::string WsSession::GetBundleName()
{
    return bundleName;
}

void WsSession::SetBundleName(const std::string &bundle)
{
    bundleName = bundle;
}
} // end of namespace Server
} // end of namespace Dic
