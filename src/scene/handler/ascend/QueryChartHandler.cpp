//
// * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//

#include "QueryChartHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Scene {
using namespace Dic::Server;
using namespace Dic::Scene::Core;
void QueryChartHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) {
    UnitChartRequest &request = dynamic_cast<UnitChartRequest &>(*requestPtr.get());
    std::string token = request.token;
    ServerLog::Info("Query chart, token = ", StringUtil::AnonymousString(token));
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(token),
                        ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<UnitChartResponse> responsePtr = std::make_unique<UnitChartResponse>();
    UnitChartResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    // mock
    // query data from database, code in core package

    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
}
}