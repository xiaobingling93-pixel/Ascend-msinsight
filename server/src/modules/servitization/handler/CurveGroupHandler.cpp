/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "ServerLog.h"
#include "CurveGroupHandler.h"
namespace Dic::Module::IE {
bool CurveGroupHandler::HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<IEGroupRequest&>(*requestPtr);
    std::unique_ptr<IEGroupResponse> responsePtr = std::make_unique<IEGroupResponse>();
    IEGroupResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    std::vector<std::string> groups = repo->QueryAllViews(request.params.rankId);
    for (const auto &item: groups) {
        IEGroupData data;
        data.label = item;
        data.value = item;
        response.data.emplace_back(data);
    }
    SendResponse(std::move(responsePtr), true);
    return true;
}
}