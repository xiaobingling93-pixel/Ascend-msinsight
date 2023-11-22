/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "SearchCountHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
void SearchCountHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    SearchCountRequest &request = dynamic_cast<SearchCountRequest &>(*requestPtr.get());
    std::string token = request.token;
    ServerLog::Info("Reset window, token = ", StringUtil::AnonymousString(token));
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(token),
                        ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<SearchCountResponse> responsePtr = std::make_unique<SearchCountResponse>();
    SearchCountResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    if (request.params.rankId.empty()) {
        auto fileIdList = DataBaseManager::Instance().GetAllFileId();
        for (const auto &fileId : fileIdList) {
            SearchResult searchResult;
            searchResult.rankId = fileId;
            searchResult.count = DataBaseManager::Instance().
                GetTraceDatabase(fileId)->SearchSliceNameCount(request.params.searchContent);
            response.body.totalCount += searchResult.count;
            if (searchResult.count > 0) {
                response.body.countList.emplace_back(searchResult);
            }
        }
    } else {
        SearchResult searchResult;
        searchResult.rankId = request.params.rankId;
        searchResult.count = DataBaseManager::Instance().
            GetTraceDatabase(request.params.rankId)->SearchSliceNameCount(request.params.searchContent);
        response.body.countList.emplace_back(searchResult);
        response.body.totalCount = searchResult.count;
    }
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

} // Timeline
} // Module
} // Dic