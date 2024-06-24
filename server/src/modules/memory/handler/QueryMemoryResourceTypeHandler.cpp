// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.

#include "QueryMemoryResourceTypeHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "MemoryProtocolRequest.h"
#include "MemoryProtocolRespose.h"

namespace Dic {
namespace Module {
namespace Memory {
    using namespace Dic::Server;
    void QueryMemoryResourceTypeHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
    {
        MemoryResourceTypeRequest &request = dynamic_cast<MemoryResourceTypeRequest &>(*requestPtr.get());
        std::string token = request.token;
        if (!WsSessionManager::Instance().CheckSession(token)) {
            ServerLog::Error("Failed to check session token , command = ", command);
            return;
        }
        WsSession &session = *WsSessionManager::Instance().GetSession(token);
        std::unique_ptr<MemoryResourceTypeResponse> responsePtr = std::make_unique<MemoryResourceTypeResponse>();
        MemoryResourceTypeResponse &response = *responsePtr.get();
        SetBaseResponse(request, response);
        auto database = Timeline::DataBaseManager::Instance().GetMemoryDatabase(request.rankId);
        if (!database->QueryMemoryResourceType(response.type)) {
            SetResponseResult(response, false);
            ServerLog::Error("Failed to query memory type data.");
            session.OnResponse(std::move(responsePtr));
            return;
        }
        SetResponseResult(response, true);
        // add response to response queue in session
        session.OnResponse(std::move(responsePtr));
    }
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic