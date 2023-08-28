/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: implementation of channel / channel server factory
 */

#include "ChannelSocket.h"
#include "ChannelFactory.h"

using namespace Dic;

std::function<std::shared_ptr<Channel>(const std::string &host, const int &port)>
    ChannelFactory::createChannelSocket = [](const std::string &host, const int &port) {
    return std::shared_ptr<Channel>(std::make_shared<ChannelSocket>(host, port));
};

std::function<std::shared_ptr<ChannelThread>(const std::shared_ptr<Channel> channel)>
    ChannelFactory::createChannelThread = [](const std::shared_ptr<Channel> channel) {
    return std::make_shared<ChannelThread>(channel);
};

std::shared_ptr<Channel> ChannelFactory::CreateChannelSocket(const std::string &host, const int &port)
{
    return createChannelSocket(host, port);
}

std::shared_ptr<ChannelThread> ChannelFactory::CreateChannelThread(const std::shared_ptr<Channel> channel)
{
    return createChannelThread(channel);
}