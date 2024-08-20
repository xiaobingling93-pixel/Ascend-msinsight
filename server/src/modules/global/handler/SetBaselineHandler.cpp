/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "SetBaselineHandler.h"
#include "ProjectExplorerManager.h"
#include "GlobalDefs.h"
#include "BaselineManager.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
using namespace Global;

void SetBaselineHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    auto &request = dynamic_cast<BaselineSettingRequest &>(*requestPtr.get());
    std::unique_ptr<BaselineSettingResponse> responsePtr = std::make_unique<BaselineSettingResponse>();
    BaselineSettingResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    if (request.params.projectName.empty() || request.params.filePath.empty()) {
        response.body.errorMessage = "Set baseline failed, project name or filepath can't be empty!";
        SendResponse(std::move(responsePtr), false);
        return;
    }

    bool res = BaselineManager::Instance().InitBaselineData(request.params.projectName, request.params.filePath,
                                                            response.body.errorMessage, response.body.rankId);
    SendResponse(std::move(responsePtr), res);
}


} // end of namespace Module
} // Dic
