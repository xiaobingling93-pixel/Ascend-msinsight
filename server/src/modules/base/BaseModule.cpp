/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include "ServerLog.h"
#include "SystemUtil.h"
#include "BaseModule.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
using namespace Dic::Protocol;
void BaseModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    int maxThreadNum = 20;
    maxThreadNum = std::max(maxThreadNum, SystemUtil::GetCpuCoreCount());
    static ThreadPool threadPool(maxThreadNum);
    std::string command = request->command;
    if (requestHandlerMap.count(command) == 0) {
        std::string token = request->token;
        ServerLog::Error("Failed to find request handler, token = ", StringUtil::AnonymousString(token),
            ", command = ", command);
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