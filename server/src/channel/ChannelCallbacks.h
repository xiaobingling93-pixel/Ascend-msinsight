/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: declaration of channel callbacks for DIC
 */
#ifndef DIC_CHANNEL_CALLBACKS_H
#define DIC_CHANNEL_CALLBACKS_H

#include <uv.h>
#include "UvDefs.h"

namespace Dic {
class ChannelCallbacks {
public:
    static void AllocBufferCb(uv_handle_t *handle, size_t suggestedSize, uv_buf_t *buf);

    static void ConnectCbSocket(uv_connect_t *req, int status);

    static void ReadCbSocket(uv_stream_t *client, ssize_t readSize, const uv_buf_t *buf);

    static void WriteCb(uv_write_t *req, int status);

    static void ShutDownCb(uv_shutdown_t *req, int status);

    static void CloseCb(uv_handle_t *handle);
};
}

#endif // DIC_CHANNEL_CALLBACKS_H
