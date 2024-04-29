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
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<SearchCountResponse> responsePtr = std::make_unique<SearchCountResponse>();
    SearchCountResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    if (request.params.rankId.empty() || strcmp(request.params.rankId.c_str(), "Host") == 0) {
        auto fileIdList = DataBaseManager::Instance().GetAllFileId();
        for (const auto &fileId : fileIdList) {
            SearchResult searchResult;
            searchResult.rankId = fileId;
            auto database = DataBaseManager::Instance().GetTraceDatabase(fileId);
            if (database != nullptr) {
                searchResult.count = database->SearchSliceNameCount(request.params);
            }
            response.body.totalCount += searchResult.count;
            if (searchResult.count > 0) {
                response.body.countList.emplace_back(searchResult);
            }
        }
    } else {
        SearchResult searchResult;
        searchResult.rankId = request.params.rankId;
        auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
        if (database != nullptr) {
            searchResult.count = database->SearchSliceNameCount(request.params);
        }
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