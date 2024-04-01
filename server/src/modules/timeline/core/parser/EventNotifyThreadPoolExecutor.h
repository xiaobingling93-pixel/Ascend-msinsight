/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_EVENTNOTIFYTHREADPOOLEXECUTOR_H
#define PROFILER_SERVER_EVENTNOTIFYTHREADPOOLEXECUTOR_H
#include "ThreadPool.h"
namespace Dic {
namespace Module {
namespace Timeline {
class EventNotifyThreadPoolExecutor {
public:
    static EventNotifyThreadPoolExecutor &Instance();
    std::shared_ptr<ThreadPool> GetThreadPool();

private:
    EventNotifyThreadPoolExecutor();
    ~EventNotifyThreadPoolExecutor();
    std::shared_ptr<ThreadPool> singleThreadPool;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_EVENTNOTIFYTHREADPOOLEXECUTOR_H
