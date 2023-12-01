/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "ClusterParseThreadPoolExecutor.h"

#include <memory>
#include "ServerLog.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;

ClusterParseThreadPoolExecutor &ClusterParseThreadPoolExecutor::Instance()
{
    static ClusterParseThreadPoolExecutor instance;
    return instance;
}

ClusterParseThreadPoolExecutor::ClusterParseThreadPoolExecutor()
{
    singleThreadPool = std::make_shared<ThreadPool>(1);
    ServerLog::Info("singleThreadPool init success");
}

ClusterParseThreadPoolExecutor::~ClusterParseThreadPoolExecutor()
{
    singleThreadPool->ShutDown();
}

std::shared_ptr<ThreadPool> ClusterParseThreadPoolExecutor::GetThreadPool()
{
    return singleThreadPool;
}

} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
