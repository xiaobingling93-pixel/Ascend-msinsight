/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: declaration of Channel Socket for DIC
 */
#ifndef DIC_CHANNEL_SOCKET_H
#define DIC_CHANNEL_SOCKET_H

#include "Channel.h"
#include "UvDefs.h"

namespace Dic {
class ChannelSocket : public Channel {
public:
    ChannelSocket(const std::string &host, const int &port);

    explicit ChannelSocket(const UvTcp &server);

    ~ChannelSocket() override;

    bool Connect() override;

    bool Close() override;

    int Send(const std::string &data) override;

protected:
    virtual bool Initialize();

    virtual void UnInitialize();

private:
    bool IsPassive() const;

    bool ConnectActive();

    bool ConnectPassive();

    int ConnectToServer();

    const UvTcp *uvTcpServer = nullptr;
    UvTcp uvTcpClient;
    std::string host;
    int port = 0;
};
}
#endif // DIC_CHANNEL_SOCKET_H
