/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_DELETEPROJECTEXPLORERINFOHANDLER_H
#define PROFILER_SERVER_DELETEPROJECTEXPLORERINFOHANDLER_H

#include "GlobalHandler.h"
#include "GlobalProtocolRequest.h"

namespace Dic {
namespace Module {
class DeleteProjectExplorerInfoHandler : public GlobalHandler {
public:
    DeleteProjectExplorerInfoHandler()
    {
        command = REQ_RES_PROJECT_EXPLORER_INFO_DELETE;
    }
    ~DeleteProjectExplorerInfoHandler() override = default;

    bool HandleRequest(std::unique_ptr<Request> requestPtr) override;
};
} // end of namespace Module
} // Dic

#endif // PROFILER_SERVER_DELETEPROJECTEXPLORERINFOHANDLER_H
