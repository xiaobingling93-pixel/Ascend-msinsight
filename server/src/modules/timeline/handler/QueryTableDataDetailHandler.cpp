/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "QueryTableDataDetailHandler.h"
namespace Dic::Module::Timeline {
using namespace Dic::Server;
bool QueryTableDataDetailHandler::HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr)
{
    auto& request = dynamic_cast<TableDataDetailRequest&>(*requestPtr);
    std::unique_ptr<TableDataDetailResponse> responsePtr = std::make_unique<TableDataDetailResponse>();
    WsSession& session = *WsSessionManager::Instance().GetSession();
    TableDataDetailResponse& response = *responsePtr;
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Query table data detail failed to get connection.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    if (request.params.type.empty()) {
        ComputeTableDetail(request, response, database);
    } else if (request.params.type == "1") {
        ComputeLinkPageDetail(request, response, database);
    }
    session.OnResponse(std::move(responsePtr));
    return true;
}

void QueryTableDataDetailHandler::ComputeLinkPageDetail(TableDataDetailRequest& request,
                                                        TableDataDetailResponse& response,
                                                        const std::shared_ptr<VirtualTraceDatabase>& database) const
{
    if (request.params.equalConditions.empty()) {
        return;
    }
    std::shared_ptr<TextTraceDatabase> databasePtr =
        std::dynamic_pointer_cast<TextTraceDatabase, VirtualTraceDatabase>(database);
    if (databasePtr == nullptr) {
        ServerLog::Warn("Failed to get text connection when query table detail,ID: %", request.params.rankId);
        return;
    }
    auto linkInfos = databasePtr->QueryTableNameAndCol(request.params.equalConditions[0].col);
    if (linkInfos.empty()) {
        return;
    }
    const std::string tableName = linkInfos[0].tableName;
    auto colums = databasePtr->QueryTableInfoByName(tableName);
    PageQuery query;
    query.fileId = request.params.rankId;
    query.curPage = request.params.currentPage;
    query.size = request.params.pageSize;
    query.viewName = tableName;
    query.order = request.params.order;
    query.orderBy = request.params.orderBy;
    for (const auto& item : request.params.filterconditions) {
        query.pageFilters.push_back({item.col, item.content});
    }
    const std::string col = linkInfos[0].col;
    for (const auto& item : request.params.equalConditions) {
        query.equalFilters.push_back({col, item.content});
    }
    auto datas = databasePtr->QueryDataByPage(query, colums);
    uint64_t count = databasePtr->QueryCountByTableName(query, colums);
    for (const auto& item : colums) {
        TableColumn column;
        column.name = item.name;
        column.type = item.type;
        column.key = item.key;
        response.body.columnAttr.emplace_back(column);
    }
    for (const auto& item : datas) {
        response.body.columnData.emplace_back(item);
    }
    response.body.totalNum = count;
}

void QueryTableDataDetailHandler::ComputeTableDetail(const TableDataDetailRequest& request,
                                                     TableDataDetailResponse& response,
                                                     std::shared_ptr<VirtualTraceDatabase> database)
{
    std::shared_ptr<TextTraceDatabase> databasePtr =
        std::dynamic_pointer_cast<TextTraceDatabase, VirtualTraceDatabase>(database);
    if (databasePtr == nullptr) {
        ServerLog::Warn("Failed to get text connection when query table detail,ID: %", request.params.rankId);
        return;
    }
    auto nameList = databasePtr->QueryTableDataNameList();
    if (nameList.size() <= request.params.tableIndex) {
        return;
    }
    const std::string tableName = nameList[request.params.tableIndex].second;
    auto colums = databasePtr->QueryTableInfoByName(tableName);
    PageQuery query;
    query.fileId = request.params.rankId;
    query.curPage = request.params.currentPage;
    query.size = request.params.pageSize;
    query.viewName = tableName;
    query.order = request.params.order;
    query.orderBy = request.params.orderBy;
    for (const auto &item: request.params.filterconditions) {
        query.pageFilters.push_back({item.col, item.content});
    }
    auto datas = databasePtr->QueryDataByPage(query, colums);
    uint64_t count = databasePtr->QueryCountByTableName(query, colums);
    for (const auto& item : colums) {
        TableColumn column;
        column.name = item.name;
        column.type = item.type;
        column.key = item.key;
        response.body.columnAttr.emplace_back(column);
    }
    for (const auto& item : datas) {
        response.body.columnData.emplace_back(item);
    }
    response.body.totalNum = count;
}
}  // namespace Dic::Module::Timeline