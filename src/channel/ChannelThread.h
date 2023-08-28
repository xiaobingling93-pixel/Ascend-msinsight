/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: declaration of Channel Thread for DIC
 */
#ifndef DIC_CHANNEL_THREAD_H
#define DIC_CHANNEL_THREAD_H

#include <thread>
#include "Channel.h"
#include "ChannelObjectId.h"

namespace Dic {
class ChannelThread {
public:
    explicit ChannelThread(const std::shared_ptr<Channel> channelPtr);
    ~ChannelThread();

    bool Start();
    bool Stop();
    bool IsStart() const;

    const int ThreadId() const;
    const int ChannelId() const;
    const std::shared_ptr<Channel> GetChannel() const;

private:
    static void ThreadFunc(ChannelThread &ct);

    bool isStart = false;
    std::unique_ptr<std::thread> threadPtr;
    std::shared_ptr<Channel> channelPtr;
    std::mutex resultMutex;
    std::condition_variable resultCv;
    const int resultTimeout = 3000;
    int id = ChannelObjectId::ThreadId().Create();
};
}

#endif // DIC_CHANNEL_THREAD_H
