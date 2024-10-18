/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_CLEARPROJECTEXPLORERHANDLER_H
#define PROFILER_SERVER_CLEARPROJECTEXPLORERHANDLER_H
#include "GlobalHandler.h"
#include "GlobalProtocolRequest.h"

namespace Dic {
namespace Module {
class ClearProjectExplorerHandler : public GlobalHandler {
public:
    ClearProjectExplorerHandler()
    {
        command = REQ_RES_PROJECT_EXPLORER_CLEAR;
    }
    ~ClearProjectExplorerHandler() override = default;

    void HandleRequest(std::unique_ptr<Request> requestPtr) override;
};
}
}

#endif // PROFILER_SERVER_CLEARPROJECTEXPLORERHANDLER_H
