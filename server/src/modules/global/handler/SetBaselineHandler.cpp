/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "BaselineManagerService.h"
#include "ProjectExplorerManager.h"
#include "GlobalDefs.h"
#include "BaselineManager.h"
#include "SetBaselineHandler.h"
namespace Dic {
namespace Module {
using namespace Dic::Server;
using namespace Global;

bool SetBaselineHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    auto &request = dynamic_cast<BaselineSettingRequest &>(*requestPtr);
    std::unique_ptr<BaselineSettingResponse> responsePtr = std::make_unique<BaselineSettingResponse>();
    BaselineSettingResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    if (request.params.projectName.empty()) {
        response.body.errorMessage = "Set baseline failed, project name or filepath can't be empty!";
        SendResponse(std::move(responsePtr), false);
        return false;
    }
    BaselineInfo baselineInfo;
    baselineInfo.clusterBaseLine = request.params.baselineClusterPath;
    bool res =
            BaselineManagerService::InitBaselineData(request.params.projectName, request.params.filePath, baselineInfo,
                                                     request.params.currentClusterPath);
    response.body.rankId = baselineInfo.rankId;
    response.body.host = baselineInfo.host;
    response.body.errorMessage = baselineInfo.errorMessage;
    response.body.cardName = baselineInfo.cardName;
    response.body.isCluster = baselineInfo.isCluster;
    response.body.cluster = baselineInfo.clusterBaseLine;
    response.body.fileId = baselineInfo.fileId;
    SendResponse(std::move(responsePtr), res);
    return res;
}
} // end of namespace Module
} // Dic
