//
// * Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
//
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TrackInfoManager.h"
#include "TraceTime.h"
#include "QueryThreadsHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
void QueryThreadsHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    UnitThreadsRequest &request = dynamic_cast<UnitThreadsRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<UnitThreadsResponse> responsePtr = std::make_unique<UnitThreadsResponse>();
    UnitThreadsResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);

    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Query threads failed to get connection.");
        session.OnResponse(std::move(responsePtr));
        return;
    }
    std::vector<uint64_t> trackIdList;
    trackIdList.reserve(request.params.metadataList.size());
    for (const auto &metadata: request.params.metadataList) {
        uint64_t trackId = TrackInfoManager::Instance().GetTrackId(request.params.rankId, metadata.pid, metadata.tid);
        trackIdList.push_back(trackId);
    }
    bool result = database->QueryThreads(request.params, response.body, TraceTime::Instance().GetStartTime(),
                                         trackIdList);
    SetResponseResult(response, result);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
} // Timeline
} // Module
} // Dic