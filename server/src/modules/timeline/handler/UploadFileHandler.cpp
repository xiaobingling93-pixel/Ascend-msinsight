/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "UploadFileHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Module::Timeline;
using namespace Dic::Server;
void UploadFileHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    UploadFileRequest &request = dynamic_cast<UploadFileRequest &>(*requestPtr.get());
    std::string sessionToken = request.token;
    if (!WsSessionManager::Instance().CheckSession(sessionToken)) {
        ServerLog::Warn("Failed to check session, command = ", command);
        return;
    }
    UploadFileParser::Instance().Parse(request);
}
} // Timeline
} // Module
} // Dic