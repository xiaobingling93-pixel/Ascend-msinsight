/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "CancelBaselineHandler.h"
#include "ProjectExplorerManager.h"
#include "GlobalDefs.h"
#include "BaselineManager.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
using namespace Global;

void Dic::Module::CancelBaselineHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    std::unique_ptr<BaselineCancelResponse> responsePtr = std::make_unique<BaselineCancelResponse>();
    BaselineManager::Instance().ResetBaseline();
    SendResponse(std::move(responsePtr), true);
}

}
}