/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "StageHandler.h"

namespace Dic {
namespace Module {
namespace Summary {
using namespace Dic::Server;
bool StageHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    PipelineStageRequest &request = dynamic_cast<PipelineStageRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<PipelineStageResponse> responsePtr = std::make_unique<PipelineStageResponse>();
    PipelineStageResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    auto database = Timeline::DataBaseManager::Instance().GetReadClusterDatabase();
    if (!database->GetStages(request.params, response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get stage response data.");
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    session.OnResponse(std::move(responsePtr));
    return true;
}

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic