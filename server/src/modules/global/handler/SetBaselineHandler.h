/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#ifndef PROFILER_SERVER_SETBASELINEHANDLER_H
#define PROFILER_SERVER_SETBASELINEHANDLER_H

#include "GlobalHandler.h"
#include "GlobalProtocolRequest.h"
namespace Dic {
namespace Module {
class SetBaselineHandler : public GlobalHandler {
public:
    SetBaselineHandler()
    {
        command = REQ_RES_PROJECT_SET_BASELINE;
    }

    ~SetBaselineHandler() override = default;

    bool HandleRequest(std::unique_ptr<Request> requestPtr) override;
};
}
}

#endif // PROFILER_SERVER_SETBASELINEHANDLER_H
