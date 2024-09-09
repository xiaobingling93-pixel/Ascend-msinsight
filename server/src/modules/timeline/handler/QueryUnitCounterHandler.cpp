/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */
#include "WsSessionManager.h"
#include "TraceTime.h"
#include "DataBaseManager.h"
#include "QueryUnitCounterHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
void QueryUnitCounterHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    UnitCounterRequest &request = dynamic_cast<UnitCounterRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<UnitCounterResponse> responsePtr = std::make_unique<UnitCounterResponse>();
    UnitCounterResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Query unit counter failed to get connection.");
        session.OnResponse(std::move(responsePtr));
        return;
    }
    bool result = database->QueryUnitCounter(request.params, TraceTime::Instance().GetStartTime(), response.body.data);
    SetResponseResult(response, result);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic