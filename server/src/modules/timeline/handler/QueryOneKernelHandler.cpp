/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/
#include "QueryOneKernelHandler.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "ProjectExplorerManager.h"
#include "TraceTime.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;

bool QueryOneKernelHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    KernelRequest &request = dynamic_cast<KernelRequest &>(*requestPtr.get());
    std::unique_ptr<OneKernelResponse> responsePtr = std::make_unique<OneKernelResponse>();
    OneKernelResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    uint64_t minTimestamp = TraceTime::Instance().GetStartTime();
    std::string warnMsg;
    if (!request.params.CheckParams(minTimestamp, warnMsg)) {
        SendResponse(std::move(responsePtr), false, warnMsg);
        return false;
    }
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(request.params.rankId);
    if (database == nullptr) {
        database = Timeline::DataBaseManager::Instance().GetTraceDatabaseWithOutHost(request.params.rankId);
        if (database == nullptr) {
            SendResponse(std::move(responsePtr), false, "Query one kernel failed to get connection.");
            return false;
        }
    }
    if (!database->QueryKernelDepthAndThread(request.params, response.body, TraceTime::Instance().GetStartTime())) {
        SendResponse(std::move(responsePtr), false, "Failed to query the operator response data.");
        return false;
    }

    SendResponse(std::move(responsePtr), true);
    return true;
}

} // Timeline
} // Module
} // Dic