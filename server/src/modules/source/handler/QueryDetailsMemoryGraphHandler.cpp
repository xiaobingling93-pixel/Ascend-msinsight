/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */
#include "pch.h"
#include "DetailsService.h"
#include "SourceProtocolResponse.h"
#include "WsSessionManager.h"
#include "SourceProtocolRequest.h"
#include "QueryDetailsMemoryGraphHandler.h"

namespace Dic {
namespace Module {
namespace Source {
using namespace Dic::Server;

bool QueryDetailsMemoryGraphHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<DetailsMemoryGraphRequest &>(*requestPtr);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<DetailsMemoryGraphResponse> responsePtr = std::make_unique<DetailsMemoryGraphResponse>();
    DetailsMemoryGraphResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    if (auto [isVaild, errMsg] = request.params.Vaild(); isVaild == false) {
        ServerLog::Error("Parameter of command ", request.command, " is invaild, error:", errMsg);
        SetResponseResult(response, false, errMsg, REQUEST_PARAMS_ERROR);
        session.OnResponse(std::move(responsePtr));
        return false;
    }
    bool result = DetailsService::QueryMemoryGraph(request, response);
    SetResponseResult(response, result);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    return result;
}
}
}
}