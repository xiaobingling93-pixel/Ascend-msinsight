// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.

#include "WsSessionManager.h"
#include "SourceProtocolRequest.h"
#include "SourceProtocolResponse.h"
#include "SourceFileParser.h"
#include "QueryDetailsRooflineHandler.h"

namespace Dic::Module::Source {
using namespace Dic::Server;

bool QueryDetailsRooflineHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<DetailsRooflineRequest &>(*requestPtr);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    auto responsePtr = std::make_unique<DetailsRooflineResponse>();
    DetailsRooflineResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    bool result = SourceFileParser::Instance().GetDetailsRoofline(response.body);
    // 解析失败时返回空数据
    if (!result) {
        response.body.soc = "";
        response.body.data.clear();
        response.body.advice.clear();
        result = true;
    }
    SetResponseResult(response, result);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    return result;
}
}