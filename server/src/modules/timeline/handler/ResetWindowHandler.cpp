//
// * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//
#include "pch.h"
#include "WsSessionManager.h"
#include "TraceFileParser.h"
#include "KernelParse.h"
#include "MemoryParse.h"
#include "FullDbParser.h"
#include "DataBaseManager.h"
#include "SourceFileParser.h"
#include "ServitizationOpenApi.h"
#include "BaselineManagerService.h"
#include "ResetWindowHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
bool ResetWindowHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    ResetWindowRequest &request = dynamic_cast<ResetWindowRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<ResetWindowResponse> responsePtr = std::make_unique<ResetWindowResponse>();
    ResetWindowResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    std::shared_ptr<IE::ServitizationOpenApi> openApi = std::make_shared<IE::ServitizationOpenApi>();
    openApi->Reset();
    TraceFileParser::Instance().Reset();
    Summary::KernelParse::Instance().Reset();
    Memory::MemoryParse::Instance().Reset();
    Source::SourceFileParser::Instance().Reset();
    FullDb::FullDbParser::Instance().Reset();
    BaselineManagerService::ResetBaseline();
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    return true;
}

} // Timeline
} // Module
} // Dic