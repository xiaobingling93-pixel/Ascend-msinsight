/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: declaration of ChannelHandler for DIC
 */
#ifndef DIC_CHANNEL_HANDLER_H
#define DIC_CHANNEL_HANDLER_H

#include <string>

namespace Dic {
class ChannelHandler {
public:
    ChannelHandler() = default;

    virtual ~ChannelHandler() = default;

    virtual void OnChannelData(const int &channelId, const std::string &data) = 0;

    virtual void OnChannelError(const int &channelId, int what, const std::string &error) = 0;

    virtual void OnChannelClose(const int &channelId) = 0;
};
}
#endif // DIC_CHANNEL_HANDLER_H
