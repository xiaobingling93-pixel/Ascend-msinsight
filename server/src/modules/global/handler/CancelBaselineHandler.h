/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#ifndef PROFILER_SERVER_CANCELBASELINEHANDLER_H
#define PROFILER_SERVER_CANCELBASELINEHANDLER_H
#include "GlobalHandler.h"
#include "GlobalProtocolRequest.h"
namespace Dic {
namespace Module {
class CancelBaselineHandler : public GlobalHandler {
public:
    CancelBaselineHandler()
    {
        command = REQ_RES_PROJECT_CANCEL_BASELINE;
    }

    ~CancelBaselineHandler() override = default;

    bool HandleRequest(std::unique_ptr<Request> requestPtr) override;
};
}
}

#endif // PROFILER_SERVER_CANCELBASELINEHANDLER_H
