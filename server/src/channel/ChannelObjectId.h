/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: declaration of channel object id for DIC
 */
#ifndef DIC_CHANNEL_OBJECT_ID_H
#define DIC_CHANNEL_OBJECT_ID_H

#include <mutex>

namespace Dic {
class ChannelObjectId {
public:
    static ChannelObjectId &ChannelId();
    static ChannelObjectId &ThreadId();
    int Create();

private:
    ChannelObjectId() = default;
    ~ChannelObjectId() = default;

    static const int MAX_ID;
    int id = 0;
    std::mutex mtx;
};
}

#endif // DIC_CHANNEL_OBJECT_ID_H
