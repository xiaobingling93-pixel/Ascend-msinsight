/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2024. All rights reserved.
 */
#include "EventNotifyThreadPoolExecutor.h"
#include "ServerLog.h"
namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;

EventNotifyThreadPoolExecutor &EventNotifyThreadPoolExecutor::Instance()
{
    static EventNotifyThreadPoolExecutor instance;
    return instance;
}

EventNotifyThreadPoolExecutor::EventNotifyThreadPoolExecutor()
{
    singleThreadPool = std::make_shared<ThreadPool>(1);
    ServerLog::Info("singleThreadPool init success");
}

EventNotifyThreadPoolExecutor::~EventNotifyThreadPoolExecutor()
{
    singleThreadPool->ShutDown();
}

std::shared_ptr<ThreadPool> EventNotifyThreadPoolExecutor::GetThreadPool()
{
    return singleThreadPool;
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic