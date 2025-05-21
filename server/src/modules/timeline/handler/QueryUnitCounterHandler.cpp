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
bool QueryUnitCounterHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    UnitCounterRequest &request = dynamic_cast<UnitCounterRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<UnitCounterResponse> responsePtr = std::make_unique<UnitCounterResponse>();
    UnitCounterResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    uint64_t minTimestamp = TraceTime::Instance().GetStartTime();
    std::string warnMsg;
    if (!request.params.CheckParams(minTimestamp, warnMsg)) {
        ServerLog::Warn(warnMsg);
        SetResponseResult(response, false, warnMsg);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    const std::string hostString = "Host";
    if (StringUtil::EndWith(request.params.rankId, hostString)) {
        request.params.rankId = DataBaseManager::Instance().GetAnyTraceDatabaseId();
    }
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Query unit counter failed to get connection.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    bool result = database->QueryUnitCounter(request.params, minTimestamp, response.body.data);
    SetResponseResult(response, result);
    session.OnResponse(std::move(responsePtr));
    return result;
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic