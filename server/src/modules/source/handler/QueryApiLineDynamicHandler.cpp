/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
 */
#include "pch.h"
#include "WsSessionManager.h"
#include "SourceFileParser.h"
#include "QueryApiLineDynamicHandler.h"

namespace Dic {
namespace Module {
namespace Source {
using namespace Dic::Server;

bool QueryApiLineDynamicHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<SourceApiLineDynamicRequest &>(*requestPtr);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<SourceApiLineDynamicResponse> responsePtr = std::make_unique<SourceApiLineDynamicResponse>();
    SourceApiLineDynamicResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    if (auto [isValid, errMsg] = request.params.Valid(); isValid == false) {
        ServerLog::Error("Parameter of command ", request.command, "is invalid, error:", errMsg);
        SetResponseResult(response, false, errMsg, REQUEST_PARAMS_ERROR);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    SetResponseBody(response, request);
    SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
    return true;
}

void QueryApiLineDynamicHandler::SetResponseBody(SourceApiLineDynamicResponse &response,
                                                 SourceApiLineDynamicRequest &request)
{
    auto lines = SourceFileParser::Instance().GetApiLinesDynamic(request.params.coreName, request.params.sourceName);
    response.body.columnNameMap = SourceFileParser::Instance().GetSourceLineColumnTypeMap();
    auto temp = response.body.sourceFileLines;
    for (const auto &item: lines) {
        SourceFileLineDynamic line;
        line.addressRange = item.addressRange;
        TransformColumnData(item.floatColumnMap, line.columnValueMap.floatMap);
        TransformColumnData(item.intColumnMap, line.columnValueMap.intMap);
        TransformColumnData(item.stringColumnMap, line.columnValueMap.stringMap);
        response.body.sourceFileLines.emplace_back(line);
    }
}

template<typename T> void QueryApiLineDynamicHandler::TransformColumnData(
    const std::unordered_map<std::string, std::vector<T>> &source, std::unordered_map<std::string, T> &target)
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