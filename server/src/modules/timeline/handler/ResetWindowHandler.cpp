//
// * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//

#include "ResetWindowHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "TraceFileParser.h"
#include "KernelParse.h"
#include "MemoryParse.h"
#include "FullDbParser.h"
#include "DataBaseManager.h"
#include "UploadFileParser.h"
#include "SourceFileParser.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
void ResetWindowHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    ResetWindowRequest &request = dynamic_cast<ResetWindowRequest &>(*requestPtr.get());
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<ResetWindowResponse> responsePtr = std::make_unique<ResetWindowResponse>();
    ResetWindowResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    DataType type = DataBaseManager::Instance().GetDataType();
    if (type == DataType::JSON) {
        TraceFileParser::Instance().Reset();
        Summary::KernelParse::Instance().Reset();
        Memory::MemoryParse::Instance().Reset();
        UploadFileParser::Instance().ResetAllFiles();
        Source::SourceFileParser::Instance().Reset();
    } else {
        FullDb::FullDbParser::Instance().Reset();
    }
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

} // Timeline
} // Module
} // Dic