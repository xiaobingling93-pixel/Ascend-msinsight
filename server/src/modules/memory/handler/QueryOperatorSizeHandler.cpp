/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "QueryOperatorSizeHandler.h"

namespace Dic {
namespace Module {
namespace Memory {
using namespace Dic::Server;
bool QueryOperatorSizeHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    MemoryOperatorSizeRequest &request = dynamic_cast<MemoryOperatorSizeRequest &>(*requestPtr.get());
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<MemoryOperatorSizeResponse> responsePtr = std::make_unique<MemoryOperatorSizeResponse>();
    MemoryOperatorSizeResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    std::string errorMsg;
    if (!request.params.CommonCheck(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetMemoryDatabase(request.params.rankId);
    if (!database->QueryOperatorSize(response.size.minSize, response.size.maxSize, request.params.rankId)) {
        SendResponse(std::move(responsePtr), false, "Failed to query operator size data.");
        return false;
    }
    SendResponse(std::move(responsePtr), true);
    return true;
}

} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic