/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "WsSessionManager.h"
#include "SourceProtocolRequest.h"
#include "SourceProtocolResponse.h"
#include "SourceFileParser.h"
#include "QueryDetailsBaseInfoHandler.h"

namespace Dic {
namespace Module {
namespace Source {
using namespace Dic::Server;

void QueryDetailsBaseInfoHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<SourceDetailBaseInfoRequest &>(*requestPtr);
    std::unique_ptr<DetailsBaseInfoResponse> responsePtr = std::make_unique<DetailsBaseInfoResponse>();
    DetailsBaseInfoResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    bool compareDataRes = SourceFileParser::Instance().GetDetailsBaseInfo(response.body.compare, false);
    if (!compareDataRes) {
        SendResponse(std::move(responsePtr), false, "Get detail base info error", ErrorCode::UNKNOW_ERROR);
        return;
    }

    // 如果设置了对比状态，但是获取基线数据失败，则打印警告日志，但不影响前端返回（只返回了对比数据）
    if (request.params.isCompared && !SourceFileParser::Instance().GetDetailsBaseInfo(response.body.baseline, true)) {
        ServerLog::Warn("Get baseline details base info fail.");
    }
    SendResponse(std::move(responsePtr), true);
}
}
}
}