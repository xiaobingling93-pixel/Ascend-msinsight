/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceFileParser.h"
#include "TraceTime.h"
#include "ParserStatusManager.h"
#include "RemoteDeleteHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
void RemoteDeleteHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    RemoteDeleteRequest &request = dynamic_cast<RemoteDeleteRequest &>(*requestPtr.get());
    std::string token = request.token;
    ServerLog::Info("Reset window, token = ", StringUtil::AnonymousString(token));
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(token),
                        ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<RemoteDeleteResponse> responsePtr = std::make_unique<RemoteDeleteResponse>();
    RemoteDeleteResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    for (const auto &fileId : request.params.rankId) {
        TraceFileParser::Instance().DeleteParseFile(fileId);
    }
    GetUpdateTime(response.body);
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

void RemoteDeleteHandler::GetUpdateTime(RemoteDeleteBody &body)
{
    auto fileIdList = DataBaseManager::Instance().GetAllFileId();
    TraceTime::Instance().Reset();
    auto &parseStatusInstance = ParserStatusManager::Instance();
    for (const auto &fileId : fileIdList) {
        if (parseStatusInstance.GetParserStatus(fileId) == ParserStatus::FINISH) {
            uint64_t min = UINT64_MAX;
            uint64_t max = 0;
            DataBaseManager::Instance().GetTraceDatabase(fileId)->QueryExtremumTimestamp(min, max);
            if (min != max || max != 0) {
                body.startTimeUpdated = true;
                TraceTime::Instance().UpdateTime(min, max);
            }
            ServerLog::Info("update time. rankId:", fileId, ", min:", min, ", max:", max);
        }
    }
    if (body.startTimeUpdated) {
        body.maxTimeStamp = TraceTime::Instance().GetDuration();
    }
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic