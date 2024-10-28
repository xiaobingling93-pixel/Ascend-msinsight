/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "SearchCountHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
bool SearchCountHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    SearchCountRequest &request = dynamic_cast<SearchCountRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<SearchCountResponse> responsePtr = std::make_unique<SearchCountResponse>();
    SearchCountResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    if (request.params.rankId.empty() || !DataBaseManager::Instance().GetDbPathByHost(request.params.rankId).empty()) {
        auto fileIdList = DataBaseManager::Instance().GetDbPathByHost(request.params.rankId);
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
    session.OnResponse(std::move(responsePtr));
    return true;
}

} // Timeline
} // Module
} // Dic