/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "SourceFileParser.h"
#include "SourceProtocolRequest.h"
#include "SourceProtocolResponse.h"
#include "WsSessionManager.h"
#include "QueryInterCoreLoadAnalysisGraphHandler.h"

namespace Dic::Module::Source {
using namespace Dic::Server;

void QueryInterCoreLoadAnalysisGraphHandler::HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<DetailsInterCoreLoadGraphRequest &>(*requestPtr);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<DetailsInterCoreLoadGraphResponse> responsePtr =
        std::make_unique<DetailsInterCoreLoadGraphResponse>();
    DetailsInterCoreLoadGraphResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    // 获取核间负载数据
    bool result = SourceFileParser::Instance().GetDetailsInterCoreLoadAnalysisGraph(response.body);
    SetResponseResult(response, result);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
}