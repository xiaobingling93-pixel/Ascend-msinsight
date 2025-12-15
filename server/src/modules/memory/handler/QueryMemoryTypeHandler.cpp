/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "DataBaseManager.h"
#include "MemoryProtocolRequest.h"
#include "MemoryProtocolRespose.h"
#include "QueryMemoryTypeHandler.h"

namespace Dic {
namespace Module {
namespace Memory {
    using namespace Dic::Server;
    bool QueryMemoryTypeHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
    {
        MemoryTypeRequest &request = dynamic_cast<MemoryTypeRequest &>(*requestPtr.get());
        std::unique_ptr<MemoryTypeResponse> responsePtr = std::make_unique<MemoryTypeResponse>();
        MemoryTypeResponse &response = *responsePtr.get();
        SetBaseResponse(request, response);
        std::string errorMsg;
        if (!CheckStrParamValid(request.rankId, errorMsg)) {
            SetMemoryError(ErrorCode::PARAMS_ERROR);
            SendResponse(std::move(responsePtr), false);
            return false;
        }
        auto database = Timeline::DataBaseManager::Instance().GetMemoryDatabaseByRankId(request.rankId);
        if (!database || !database->QueryMemoryType(response.type, response.graphId)) {
            SetMemoryError(ErrorCode::QUERY_MEMORY_TYPE_FAILED);
            SendResponse(std::move(responsePtr), false);
            return false;
        }
        // add response to response queue in session
        SendResponse(std::move(responsePtr), true);
        return true;
    }
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic