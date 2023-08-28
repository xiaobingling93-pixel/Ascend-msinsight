/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: declaration of channel / channel server factory
 */
#ifndef DIC_CHANNEL_FACTORY_H
#define DIC_CHANNEL_FACTORY_H

#include <functional>
#include "Channel.h"
#include "ChannelThread.h"

namespace Dic {
class ChannelFactory {
public:
    static std::shared_ptr<Channel> CreateChannelSocket(const std::string &host, const int &port);

    static std::shared_ptr<ChannelThread> CreateChannelThread(const std::shared_ptr<Channel> channel);

private:
    ChannelFactory() = default;
    ~ChannelFactory() = default;

    // for ut
    static std::function<std::shared_ptr<Channel>(const std::string &host, const int &port)> createChannelSocket;
    static std::function<std::shared_ptr<ChannelThread>(const std::shared_ptr<Channel> channel)> createChannelThread;
};
}

#endif // DIC_CHANNEL_FACTORY_H
