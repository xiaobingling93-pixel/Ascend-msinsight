/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "ProjectExplorerManager.h"
#include "GlobalDefs.h"
#include "BaselineManagerService.h"
#include "CancelBaselineHandler.h"
namespace Dic {
namespace Module {
using namespace Dic::Server;
using namespace Global;

void Dic::Module::CancelBaselineHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    auto &request = dynamic_cast<BaselineCancelRequest &>(*requestPtr.get());
    std::unique_ptr<BaselineCancelResponse> responsePtr = std::make_unique<BaselineCancelResponse>();
    BaselineCancelResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    BaselineManagerService::ResetBaseline();
    SendResponse(std::move(responsePtr), true);
}

}
}