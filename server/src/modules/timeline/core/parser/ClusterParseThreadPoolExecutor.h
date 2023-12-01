/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_CLUSTERPARSETHREADPOOLEXECUTOR_H
#define PROFILER_SERVER_CLUSTERPARSETHREADPOOLEXECUTOR_H

#include "ThreadPool.h"

namespace Dic {
namespace Module {
namespace Timeline {
class ClusterParseThreadPoolExecutor {
public:
    static ClusterParseThreadPoolExecutor &Instance();
    std::shared_ptr<ThreadPool> GetThreadPool();
private:
    ClusterParseThreadPoolExecutor();
    ~ClusterParseThreadPoolExecutor();
    std::shared_ptr<ThreadPool> singleThreadPool;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_CLUSTERPARSETHREADPOOLEXECUTOR_H
