// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.

#include "DataBaseManager.h"
#include "MemoryProtocolRequest.h"
#include "MemoryProtocolRespose.h"
#include "QueryMemoryResourceTypeHandler.h"

namespace Dic {
namespace Module {
namespace Memory {
    using namespace Dic::Server;
    bool QueryMemoryResourceTypeHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
    {
        MemoryResourceTypeRequest &request = dynamic_cast<MemoryResourceTypeRequest &>(*requestPtr.get());
        std::unique_ptr<MemoryResourceTypeResponse> responsePtr = std::make_unique<MemoryResourceTypeResponse>();
        MemoryResourceTypeResponse &response = *responsePtr.get();
        SetBaseResponse(request, response);
        std::string errorMsg;
        if (!CheckStrParamValid(request.rankId, errorMsg)) {
            SetMemoryError(ErrorCode::PARAMS_ERROR);
            SendResponse(std::move(responsePtr), false);
            return false;
        }
        auto database = Timeline::DataBaseManager::Instance().GetMemoryDatabaseByRankId(request.rankId);
        if (!database || !database->QueryMemoryResourceType(response.type)) {
            SetMemoryError(ErrorCode::QUERY_MEMORY_RESOURCE_TYPE_FAILED);
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