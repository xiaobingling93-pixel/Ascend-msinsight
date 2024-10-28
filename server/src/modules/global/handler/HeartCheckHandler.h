/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_TOKENHEARTCHECKHANDLER_H
#define PROFILER_SERVER_TOKENHEARTCHECKHANDLER_H

#include "GlobalHandler.h"
#include "GlobalProtocolRequest.h"

namespace Dic {
namespace Module {
class HeartCheckHandler : public GlobalHandler {
public:
    HeartCheckHandler()
    {
        command = REQ_RES_HEART_CHECK;
    }
    ~HeartCheckHandler() override = default;

    bool HandleRequest(std::unique_ptr<Request> requestPtr) override;
};
} // end of namespace Module
} // Dic

#endif // PROFILER_SERVER_TOKENHEARTCHECKHANDLER_H
