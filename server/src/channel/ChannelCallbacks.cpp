/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: implementation of channel callbacks for DIC
 */
#include <cstdio>
#include <uv.h>
#include "UvDefs.h"
#include "Channel.h"
#include "ChannelCallbacks.h"

using namespace Dic;

void ChannelCallbacks::AllocBufferCb(uv_handle_t *handle, size_t suggestedSize, uv_buf_t *buf)
{
    buf->base = (char *)malloc(suggestedSize);
    if (buf->base == nullptr) {
        buf->len = 0;
        return;
    }
    buf->len = suggestedSize;
}

void ChannelCallbacks::ConnectCbSocket(uv_connect_t *req, int status)
{
    if (req == nullptr) {
        return;
    }
    UvReqConnect *connReq = reinterpret_cast<UvReqConnect *>(req);
    Channel *channel = static_cast<Channel *>(connReq->arg);
    if (status < 0) {
        channel->OnError(status, "Channel - tcp client: Connect status error.");
        channel->Close();
    } else {
        int ret = uv_read_start(req->handle, ChannelCallbacks::AllocBufferCb, ChannelCallbacks::ReadCbSocket);
        if (ret != 0) {
            channel->OnError(ret, "Channel - tcp client: Connect read error.");
            channel->Close();
        }
    }
    free(connReq);
}

void ChannelCallbacks::ReadCbSocket(uv_stream_t *client, ssize_t readSize, const uv_buf_t *buf)
{
    if (client == nullptr) {
        return;
    }
    UvTcp *tcp = reinterpret_cast<UvTcp *>(client);
    Channel *channel = static_cast<Channel *>(tcp->arg);
    if (readSize > 0) {
        channel->OnData(std::string(buf->base, readSize));
    } else if (readSize < 0) {
        if (readSize != UV_EOF) {
            channel->OnError(UV_RET_FAIL, "Channel - tcp client: Read error.");
        }
        channel->Close();
    }
    free(buf->base);
}

void ChannelCallbacks::WriteCb(uv_write_t *req, int status)
{
    if (req == nullptr) {
        return;
    }
    // note: release req
    free(req);
    req = nullptr;
}

void ChannelCallbacks::ShutDownCb(uv_shutdown_t *req, int status)
{
    if (req == nullptr) {
        return;
    }
    free(req);
    req = nullptr;
}

void ChannelCallbacks::CloseCb(uv_handle_t *handle) {}