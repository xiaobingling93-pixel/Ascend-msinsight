//
// * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TrackInfoManager.h"
#include "TraceTime.h"
#include "QueryThreadDetailHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
bool QueryThreadDetailHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    ThreadDetailRequest &request = dynamic_cast<ThreadDetailRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<UnitThreadDetailResponse> responsePtr = std::make_unique<UnitThreadDetailResponse>();
    UnitThreadDetailResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Query thread detail failed to get connection.");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    int64_t trackId = TrackInfoManager::Instance()
        .GetTrackId(request.params.rankId, request.params.pid, request.params.tid);
    bool result = database->QueryThreadDetail(request.params, response.body, TraceTime::Instance().GetStartTime(),
                                              trackId);
    SetResponseResult(response, result);
    session.OnResponse(std::move(responsePtr));
    return result;
}

} // Timeline
} // Module
} // Dic
