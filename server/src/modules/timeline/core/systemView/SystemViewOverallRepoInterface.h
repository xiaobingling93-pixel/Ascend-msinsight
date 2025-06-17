/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_SYSTEMVIEWOVERALLREPOINTERFACE_H
#define PROFILER_SERVER_SYSTEMVIEWOVERALLREPOINTERFACE_H

#include "SystemViewOverallHelper.h"
#include "TimelineProtocolRequest.h"
#include "DataBaseManager.h"
using namespace Dic::Protocol;
namespace Dic::Module::Timeline {
class SystemViewOverallRepoInterface {
public:
    virtual ~SystemViewOverallRepoInterface() = default;
    virtual std::vector<OverallTmpInfo> QueryOverlapAnalysisDataForOverallMetric(
        const Protocol::SystemViewOverallReqParam &requestParams,
        const std::shared_ptr<VirtualTraceDatabase> &database) = 0;
    virtual bool QueryDataForComputingOverallMetric(const Protocol::SystemViewOverallReqParam &requestParams,
        SystemViewOverallHelper &computeHelper, const std::shared_ptr<VirtualTraceDatabase> &database) = 0;
    virtual void QueryCommunicationOverlapOverallInfos(const Protocol::SystemViewOverallReqParam &requestParams,
        SystemViewOverallHelper &overallHelper, std::vector<Protocol::SystemViewOverallRes> &responseBody,
        const std::shared_ptr<VirtualTraceDatabase> &database) = 0;
    virtual bool QueryCommunicationOpsTimeDataByGroupName(const SystemViewOverallReqParam &params, uint64_t offset,
        const std::vector<Protocol::ThreadTraces> &notOverlapData, std::vector<SameOperatorsDetails> &opsDetails,
        const std::shared_ptr<VirtualTraceDatabase> &database) = 0;
};
}

#endif // PROFILER_SERVER_SYSTEMVIEWOVERALLREPOINTERFACE_H
