/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: implementation of channel object id for DIC
 */
#include "ChannelObjectId.h"

using namespace Dic;

namespace Dic {
    const int ChannelObjectId::maxId = 0x7dffffff;
}

ChannelObjectId &ChannelObjectId::ChannelId()
{
    static ChannelObjectId instance;
    return instance;
}

ChannelObjectId &ChannelObjectId::ThreadId()
{
    static ChannelObjectId instance;
    return instance;
}

int ChannelObjectId::Create()
{
    std::lock_guard<std::mutex> lock(mtx);
    if (id >= maxId) {
        id = 0;
    }
    return id++;
}
