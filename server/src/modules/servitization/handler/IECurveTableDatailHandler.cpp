/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "IEProtocolResquest.h"
#include "IEProtocolResponse.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "IECurveTableDatailHandler.h"
namespace Dic::Module::IE {
bool IECurveTableDatailHandler::HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<IETableRequest &>(*requestPtr);
    std::unique_ptr<IETableViewResponse> responsePtr = std::make_unique<IETableViewResponse>();
    IETableViewResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    PageQuery query;
    query.fileId = request.params.rankId;
    query.curPage = request.params.currentPage;
    query.size = request.params.pageSize;
    query.viewName = request.params.type;
    query.start = request.params.startTime;
    query.end = request.params.endTime;
    query.order = request.params.order;
    query.orderBy = request.params.orderBy;
    if (!std::empty(request.params.type)) {
        QueryViewData(response, query);
    }
    SendResponse(std::move(responsePtr), true);
    return true;
}

void IECurveTableDatailHandler::QueryViewData(IETableViewResponse &response, const PageQuery &query)
{
    TableDatas datas;
    datas.att = repo->QueryTableInfoByName(query.fileId, query.viewName);
    if (datas.att.empty()) {
        return;
    }
    datas.datas = repo->QueryDataByColumnPage(query, datas.att);
    datas.count = repo->QueryCountByTableName(query, datas.att[0].key);
    for (const auto &item : datas.att) {
        Column column;
        column.name = item.name;
        column.type = item.type;
        column.key = item.key;
        response.data.columnAttr.emplace_back(column);
    }
    for (const auto &item : datas.datas) {
        response.data.columnData.emplace_back(item);
    }
    response.data.totalNum = datas.count;
}
}