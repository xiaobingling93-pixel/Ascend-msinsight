/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
 */
#include "pch.h"
#include "WsSessionManager.h"
#include "SourceProtocolRequest.h"
#include "SourceProtocolResponse.h"
#include "SourceFileParser.h"
#include "QueryApiInstructionsDynamicHandler.h"

namespace Dic {
namespace Module {
namespace Source {
using namespace Dic::Server;

bool QueryApiInstructionsDynamicHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<SourceApiInstrDynamicRequest &>(*requestPtr);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<SourceApiInstrDynamicResponse> responsePtr = std::make_unique<SourceApiInstrDynamicResponse>();
    SourceApiInstrDynamicResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    if (!request.params.coreName.empty()) { // 如果请求的coreName为空字符串，那么默认查找第一组
        if (auto [isValid, errMsg] = request.params.Valid(); !isValid) {
            ServerLog::Error("Parameter of command ", request.command, "is invalid, error:", errMsg);
            SetResponseResult(response, false, errMsg, REQUEST_PARAMS_ERROR);
            session.OnResponse(std::move(responsePtr));
            return false;
        }
    }
    SetResponseBody(response, request);
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    return true;
}

void QueryApiInstructionsDynamicHandler::SetResponseBody(SourceApiInstrDynamicResponse &response,
                                                         SourceApiInstrDynamicRequest &request)
{
    auto instructions = SourceFileParser::Instance().GetInstrDynamic(request.params.coreName);
    auto columNameMap = SourceFileParser::Instance().GetInstructionColumnTypeMap();
    response.body.coreName = request.params.coreName;
    // 组装每一列的表头信息
    response.body.columnNameMap = columNameMap;
    // 组装每一列的数据
    for (const auto &item: instructions) {
        SourceColumnValueMap col;
        TransformColumnData(item.floatColumnMap, col.floatMap);
        TransformColumnData(item.intColumnMap, col.intMap);
        TransformColumnData(item.stringColumnMap, col.stringMap);
        response.body.columnValues.emplace_back(col);
    }
}

template<typename T>
void QueryApiInstructionsDynamicHandler::TransformColumnData(
    const std::unordered_map<std::string, std::vector<T>> &source,
    std::unordered_map<std::string, T> &target)
{
    for (const auto &innerItem: source) {
        if (innerItem.second.empty()) {
            continue;
        }
        target[innerItem.first] = innerItem.second[0];
    }
}

} // Source
} // Module
} // Dic