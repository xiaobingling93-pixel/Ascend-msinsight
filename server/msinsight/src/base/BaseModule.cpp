/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include "ServerLog.h"
#include "../utils/SystemUtil.h"
#include "../utils/ThreadPool.h"
#include "BaseModule.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
using namespace Dic::Protocol;
void BaseModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    static constexpr int MIN_SIZE = 20;
    static constexpr int MAX_SIZE = 200;
    int maxThreadNum = MIN_SIZE;
    unsigned int cpuCount = SystemUtil::GetCpuCoreCount();
    if (cpuCount >= MIN_SIZE && cpuCount <= MAX_SIZE) {
        maxThreadNum = static_cast<int>(cpuCount);
    }
    static ThreadPool threadPool(maxThreadNum);
    std::string command = request->command;
    if (requestHandlerMap.count(command) == 0) {
        ServerLog::Error("Failed to find request handler");
        return;
    }
    auto &requestHandler = requestHandlerMap.at(command);
    if (requestHandler->IsAsync()) {
        threadPool.AddTask([&requestHandler, requestPtr = request.release()]() {
            requestHandler->HandleRequest(std::unique_ptr<Protocol::Request>(requestPtr));
        });
    } else {
        requestHandler->HandleRequest(std::move(request));
    }
};
} // end of namespace Module
} // end of namespace Dic