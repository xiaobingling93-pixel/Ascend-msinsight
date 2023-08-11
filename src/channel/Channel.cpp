/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: implementation of channel for DIC
 */

#include "ChannelCallbacks.h"
#include "UvLoopMgr.h"
#include "Channel.h"

using namespace Dic;

bool Channel::IsConnected() const
{
    return isConnected;
}

void Channel::RegChannelHandler(std::shared_ptr<ChannelHandler> handler)
{
    dataHandler = handler;
}

const int Channel::Id() const
{
    return this->id;
}

const ChannelLinkMode Channel::LinkMode() const
{
    return linkMode;
}

const std::shared_ptr<ChannelHandler> Channel::GetChannelHandler() const
{
    return dataHandler;
}

void Channel::OnData(const std::string &data) const
{
    if (dataHandler != nullptr) {
        dataHandler->OnChannelData(this->id, data);
    }
}

void Channel::OnError(const int &what, const std::string &error) const
{
    if (dataHandler != nullptr) {
        dataHandler->OnChannelError(this->id, what, error);
    }
}

void Channel::OnMessage(const std::string &msg) const
{
    if (dataHandler != nullptr) {
        dataHandler->OnChannelError(this->id, 0, msg);
    }
}

void Channel::OnClose() const
{
    if (dataHandler != nullptr) {
        dataHandler->OnChannelClose(this->id);
    }
}

void Channel::SetLoopId(const int lId)
{
    loopId = lId;
}

int Channel::GetLoopId() const
{
    return loopId;
}

int Channel::WriteData(const uv_stream_t *stream, const std::string &data) const
{
    if (stream == nullptr) {
        return -1;
    }
    uv_write_t *writeReq = (uv_write_t *)malloc(sizeof(uv_write_t));
    if (writeReq == nullptr) {
        return -1;
    }
    uv_buf_t buf;
    buf.base = (char *)data.c_str();
    buf.len = data.length();
    int bufCount = 1;
    return uv_write(writeReq, const_cast<uv_stream_t *>(stream), &buf, bufCount, ChannelCallbacks::WriteCb);
}

int Channel::ReadStop(const uv_stream_t *handle) const
{
    if (handle == nullptr) {
        return -1;
    }
    return uv_read_stop(const_cast<uv_stream_t *>(handle));
}

int Channel::ShutDown(const uv_stream_t *handle) const
{
    if (handle == nullptr) {
        return -1;
    }
    uv_shutdown_t *req = (uv_shutdown_t *)malloc(sizeof(uv_shutdown_t));
    if (req == nullptr) {
        return -1;
    }
    return uv_shutdown(req, const_cast<uv_stream_t *>(handle), ChannelCallbacks::ShutDownCb);
}

void Channel::CloseStream(const uv_handle_t *handle) const
{
    if (handle == nullptr) {
        return;
    }
    if (uv_is_closing(const_cast<uv_handle_t *>(handle)) == 0) {
        uv_close(const_cast<uv_handle_t *>(handle), ChannelCallbacks::CloseCb);
    }
}

bool Channel::InitializeAsync()
{
    sendAsync.arg = this;
    uv_loop_t *loop = UvLoopMgr::Instance().Loop(loopId);
    if (loop == nullptr) {
        return false;
    }
    int ret = uv_async_init(loop, &sendAsync.handle, AsyncSendCb);
    return (ret == 0);
}

void Channel::UnInitializeAsync()
{
    if (uv_is_closing((uv_handle_t *)(&sendAsync.handle)) == 0) {
        uv_close((uv_handle_t *)(&sendAsync.handle), ChannelCallbacks::CloseCb);
    }
}

void Channel::AsyncSendCb(uv_async_t *handle)
{
    if (handle == nullptr) {
        return;
    }
    UvAsync *asyncHandle = (UvAsync *)handle;
    Channel *channel = (Channel *)asyncHandle->arg;
    std::string data((char *)handle->data, asyncHandle->len);
    channel->Send(data);
    std::unique_lock<std::mutex> lock(channel->sendMutex);
    channel->sendCv.notify_one();
}

void Channel::AsyncSend(const std::string &data)
{
    if (!this->isConnected || data.empty()) {
        return;
    }
    std::unique_lock<std::mutex> lock(sendMutex);
    sendAsync.handle.data = (char *)data.c_str();
    sendAsync.len = data.size();
    uv_async_send(&sendAsync.handle);
    int waitTime = 2;
    (void)sendCv.wait_for(lock, std::chrono::seconds(waitTime));
}