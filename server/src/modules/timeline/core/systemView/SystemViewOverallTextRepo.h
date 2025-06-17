/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_SYSTEMVIEWOVERALLTEXTREPO_H
#define PROFILER_SERVER_SYSTEMVIEWOVERALLTEXTREPO_H
#include "SystemViewOverallRepoInterface.h"
using namespace Dic::Protocol;
namespace Dic::Module::Timeline {
class SystemViewOverallTextRepo : public SystemViewOverallRepoInterface {
public:
    ~SystemViewOverallTextRepo() override = default;
    std::vector<OverallTmpInfo> QueryOverlapAnalysisDataForOverallMetric(
        const Protocol::SystemViewOverallReqParam &requestParams,
        const std::shared_ptr<VirtualTraceDatabase> &database) override;
    bool QueryDataForComputingOverallMetric(const Protocol::SystemViewOverallReqParam &requestParams,
        SystemViewOverallHelper &computeHelper, const std::shared_ptr<VirtualTraceDatabase> &database) override;
    void QueryCommunicationOverlapOverallInfos(const Protocol::SystemViewOverallReqParam &requestParams,
        SystemViewOverallHelper &overallHelper, std::vector<Protocol::SystemViewOverallRes> &responseBody,
        const std::shared_ptr<VirtualTraceDatabase> &database) override;
    bool QueryCommunicationOpsTimeDataByGroupName(const SystemViewOverallReqParam &params, uint64_t offset,
        const std::vector<Protocol::ThreadTraces> &notOverlapData, std::vector<SameOperatorsDetails> &opsDetails,
        const std::shared_ptr<VirtualTraceDatabase> &database) override;
private:
    static bool CheckDataForSystemViewOverall(const std::shared_ptr<VirtualTraceDatabase> &database);
    static std::map<uint64_t, uint64_t> QueryFlowDict(const Protocol::SystemViewOverallReqParam &requestParams,
        const std::shared_ptr<VirtualTraceDatabase> &database);
    static std::vector<CpuCubeOpInfo> QueryCpuCubeOp(const std::shared_ptr<VirtualTraceDatabase> &database);
    static std::vector<OverallTmpInfo> QueryKernelEventsForSystemViewOverall(
        const Protocol::SystemViewOverallReqParam &requestParams,
        const std::map<uint64_t, uint64_t> &flowDict, const std::shared_ptr<VirtualTraceDatabase> &database);
    static void QueryBwdTrackIdForComputingOverall(uint64_t& bwdTrackId,
        const std::shared_ptr<VirtualTraceDatabase> &database);
};
}

#endif // PROFILER_SERVER_SYSTEMVIEWOVERALLTEXTREPO_H
