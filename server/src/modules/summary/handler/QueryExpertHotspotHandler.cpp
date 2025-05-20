/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "QueryExpertHotspotHandler.h"
#include "ExpertHotspotManager.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"

namespace Dic::Module::Summary {
bool QueryExpertHotspotHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<QueryExpertHotspotRequest &>(*requestPtr);
    std::unique_ptr<QueryExpertHotspotResponse> responsePtr = std::make_unique<QueryExpertHotspotResponse>();
    QueryExpertHotspotResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    std::string errorMsg;
    if (!request.params.CheckParams(errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    ModelInfo modelInfo;
    modelInfo.denseLayerList = request.params.denseLayerList;
    modelInfo.expertNumber = request.params.expertNum;
    modelInfo.modelLayer = request.params.layerNum;
    if (!ExpertHotspotManager::UpdateModelInfo(request.params.clusterPath, modelInfo, errorMsg)) {
        SendResponse(std::move(responsePtr), false, errorMsg);
        return false;
    }
    response.body.hotspotInfos = ExpertHotspotManager::QueryExpertHotspotData(request.params.clusterPath,
                                                                              request.params.modelStage,
                                                                              request.params.version);
    SendResponse(std::move(responsePtr), true, "");
    return true;
}
}