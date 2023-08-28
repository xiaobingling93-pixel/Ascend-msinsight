/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include "ServerLog.h"
#include "BaseModule.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
void BaseModule::OnRequest(std::unique_ptr<Protocol::Request> request)
{
    std::string token = request->token;
    std::string command = request->command;
    if (requestHandlerMap.count(command) == 0) {
        ServerLog::Error("Failed to find request handler, token = ", StringUtil::AnonymousString(token),
            ", command = ", command);
        return;
    }
    requestHandlerMap.at(command)->HandleRequest(std::move(request));
};
} // end of namespace Module
} // end of namespace Dic