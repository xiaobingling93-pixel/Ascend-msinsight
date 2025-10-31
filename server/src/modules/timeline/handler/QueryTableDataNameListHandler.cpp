/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "QueryTableDataNameListHandler.h"
namespace Dic::Module::Timeline {
using namespace Dic::Server;
bool QueryTableDataNameListHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<TableDataNameListRequest &>(*requestPtr);
    std::unique_ptr<TableDataNameListResponse> responsePtr = std::make_unique<TableDataNameListResponse>();
    WsSession &session = *WsSessionManager::Instance().GetSession();
    TableDataNameListResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Query table data name list failed to get connection.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    std::shared_ptr<TextTraceDatabase> databasePtr =
            std::dynamic_pointer_cast<TextTraceDatabase, VirtualTraceDatabase>(database);
    if (databasePtr == nullptr) {
        ServerLog::Warn("Failed to get text connection when query table names,ID: %", request.params.rankId);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    auto nameList = databasePtr->QueryTableDataNameList();
    auto translate = databasePtr->QueryTranslate(request.params.isZh);
    for (const auto &item: nameList) {
        response.body.layers.emplace_back(item.first, translate[item.first]);
    }
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    return true;
}
}