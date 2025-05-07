/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "QueryModelInfoHandler.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "ExpertHotspotManager.h"

namespace Dic::Module::Summary {
    bool QueryModelInfoHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
    {
        QueryModelInfoRequest &request = dynamic_cast<QueryModelInfoRequest &>(*requestPtr.get());
        std::unique_ptr<QueryModelInfoResponse> responsePtr = std::make_unique<QueryModelInfoResponse>();
        QueryModelInfoResponse &response = *responsePtr.get();
        SetBaseResponse(request, response);
        std::string errorMsg;
        // 查询模型信息
        ModelInfo modelInfo = ExpertHotspotManager::GetModelInfo();
        response.body.expertNum = modelInfo.expertNumber;
        response.body.denseLayerList = modelInfo.denseLayerList;
        response.body.layerNum = modelInfo.modelLayer;
        SendResponse(std::move(responsePtr), true);
        return true;
    }
}

