/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "pch.h"
#include "WsSessionManager.h"
#include "ParserStatusManager.h"
#include "TraceFileParser.h"
#include "FullDbParser.h"
#include "ParseCardsHandler.h"
namespace Dic::Module::Timeline {
using namespace Dic;
using namespace Dic::Server;
bool Dic::Module::Timeline::ParseCardsHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    ParseCardsRequest &request = dynamic_cast<ParseCardsRequest &>(*requestPtr.get());
    for (size_t i = 0; i < request.params.cards.size() && i < request.params.fileIds.size(); i++) {
        std::string item = request.params.cards[i];
        std::pair<ProjectTypeEnum, std::vector<std::string>> filePathPair =
            ParserStatusManager::Instance().QueryPendingFilePath(item);
        if (std::empty(filePathPair.second)) {
            ServerLog::Warn("Parse cards file path is empty. card: %", item);
            continue;
        }
        if (filePathPair.first == ProjectTypeEnum::TRACE) {
            TraceFileParser::Instance().Parse(filePathPair.second, item, "", request.params.fileIds[i]);
            continue;
        }
        FullDb::FullDbParser::Instance().Parse({ item }, filePathPair.second[0]);
    }
    std::unique_ptr<ParseCardsResponse> responsePtr = std::make_unique<ParseCardsResponse>();
    ParseCardsResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    response.body.isContinueParse = true;
    WsSession &session = *WsSessionManager::Instance().GetSession();
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    return true;
}
}
