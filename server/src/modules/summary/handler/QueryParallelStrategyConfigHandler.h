/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_QUERYPARALLELSTRATEGYCONFIGHANDLER_H
#define PROFILER_SERVER_QUERYPARALLELSTRATEGYCONFIGHANDLER_H

#include "DataBaseManager.h"
#include "ParallelStrategyAlgorithmManager.h"
#include "SummaryRequestHandler.h"

namespace Dic::Module::Summary {
class QueryParallelStrategyConfigHandler : public SummaryRequestHandler {
public:
    QueryParallelStrategyConfigHandler()
    {
        command = Protocol::REQ_RES_SUMMARY_QUERY_PARALLEL_STRATEGY;
    }
    ~QueryParallelStrategyConfigHandler() override = default;
    bool HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) override;
private:
    bool AddAlgorithmToManager(const std::shared_ptr<VirtualClusterDatabase> &database,
                               const ParallelStrategyConfig &config);
};
}

#endif // PROFILER_SERVER_QUERYPARALLELSTRATEGYCONFIGHANDLER_H
