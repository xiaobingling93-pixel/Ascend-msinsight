/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "SummaryTopRankHandler.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "TraceTime.h"
#include "DataBaseManager.h"
#include "WsSessionManager.h"
#include "SummaryService.h"

namespace Dic {
namespace Module {
namespace Summary {
using namespace Dic;
using namespace Dic::Server;

bool SummaryTopRankHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<SummaryTopRankRequest &>(*requestPtr);
    std::unique_ptr<SummaryTopRankResponse> responsePtr = std::make_unique<SummaryTopRankResponse>();
    SummaryTopRankResponse &response = *responsePtr;
    std::string errMsg;
    if (!request.params.CheckParams(errMsg)) {
        SendResponse(std::move(responsePtr), false, errMsg);
        return true;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession();
    SummaryService::QueryCompareSummaryBaseInfo(request, response);
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // Timeline
} // Module
} // Dic