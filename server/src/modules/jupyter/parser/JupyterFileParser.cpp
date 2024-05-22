/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "JupyterFileParser.h"

#include <memory>
#include "ServerLog.h"

namespace Dic {
namespace Module {
namespace Jupyter {
using namespace Dic::Server;

JupyterFileParser &JupyterFileParser::Instance()
{
    static JupyterFileParser instance;
    return instance;
}

JupyterFileParser::JupyterFileParser()
{
    singleThreadPool = std::make_shared<ThreadPool>(1);
    ServerLog::Info("singleThreadPool init success");
}

JupyterFileParser::~JupyterFileParser()
{
    singleThreadPool->ShutDown();
}

std::shared_ptr<ThreadPool> JupyterFileParser::GetThreadPool()
{
    return singleThreadPool;
}

} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic