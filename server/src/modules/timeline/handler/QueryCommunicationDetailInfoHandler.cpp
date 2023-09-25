/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "QueryCommunicationDetailInfoHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceTime.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;

bool QueryCommunicationDetailInfoHandler::GetResponseData(const Protocol::CommunicationDetailParams& params,
                                                          CommunicationDetailResponse &response)
{
    std::string threadName = "Group %";
    auto database = Timeline::DataBaseManager::Instance().GetTraceDatabase(params.rankId);
    std::vector<std::string> opTrackId = database->GetTrackIdList(threadName);
    response.totalNum = database->QueryCommunicationTotalNum(opTrackId);
    if (!database->QueryCommunicationNum(params, response, opTrackId)) {
        ServerLog::Error("Failed to get Communication Details!");
        return false;
    }
    return true;
}

void QueryCommunicationDetailInfoHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    CommunicationDetailRequest &request = dynamic_cast<CommunicationDetailRequest &>(*requestPtr.get());
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(token),
                        ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<CommunicationDetailResponse> responsePtr = std::make_unique<CommunicationDetailResponse>();
    CommunicationDetailResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    if (!GetResponseData(request.params, response)) {
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

} // Timeline
} // Module
} // Dic