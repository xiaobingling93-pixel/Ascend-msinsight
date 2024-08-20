/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SETPARALLELSTRATEGYCONFIGHANDLER_H
#define PROFILER_SERVER_SETPARALLELSTRATEGYCONFIGHANDLER_H

#include "SummaryRequestHandler.h"

namespace Dic::Module::Summary {
class SetParallelStrategyConfigHandler : public SummaryRequestHandler {
public:
    SetParallelStrategyConfigHandler()
    {
        command = Protocol::REQ_RES_SUMMARY_SET_PARALLEL_STRATEGY;
    }
    ~SetParallelStrategyConfigHandler() override = default;
    void HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
};
}

#endif // PROFILER_SERVER_SETPARALLELSTRATEGYCONFIGHANDLER_H
