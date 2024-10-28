/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "QueryKernelDetailHandler.h"
namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;

bool QueryKernelDetailHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    KernelDetailsRequest &request = dynamic_cast<KernelDetailsRequest &>(*requestPtr.get());
    std::unique_ptr<KernelDetailsResponse> responsePtr = std::make_unique<KernelDetailsResponse>();
    KernelDetailsResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Query kernel detail failed to get connection. ");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    std::string error;
    request.params.Check(error);
    if (!std::empty(error)) {
        ServerLog::Warn(error);
        SetResponseResult(response, false, error);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    if (!database->QueryKernelDetailData(request.params, response.body, TraceTime::Instance().GetStartTime())) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get kernel detail response data.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    session.OnResponse(std::move(responsePtr));
    return true;
}
} // Timeline
} // Module
} // Dic