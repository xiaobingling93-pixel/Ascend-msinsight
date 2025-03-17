/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_SYSTEMVIEWOVERALLDBREPO_H
#define PROFILER_SERVER_SYSTEMVIEWOVERALLDBREPO_H

#include <mutex>
#include "SystemViewOverallRepoInterface.h"
namespace Dic::Module::Timeline {
class SystemViewOverallDbRepo : public SystemViewOverallRepoInterface {
public:
    std::vector<OverallTmpInfo> QueryOverlapAnalysisDataForOverallMetric(
        const Protocol::SystemViewOverallReqParam &requestParams,
        const std::shared_ptr<VirtualTraceDatabase> &database) override;
    bool QueryDataForComputingOverallMetric(const Protocol::SystemViewOverallReqParam &requestParams,
        SystemViewOverallHelper &computeHelper, const std::shared_ptr<VirtualTraceDatabase> &database) override;
    void QueryCommunicationOverlapOverallInfos(const Protocol::SystemViewOverallReqParam &requestParams,
        double e2eTime, std::vector<Protocol::SystemViewOverallRes> &responseBody,
        const std::shared_ptr<VirtualTraceDatabase> &database) override;
    bool QueryCommunicationOpsTimeDataByGroupName(const std::string &name, uint64_t offset,
        const std::vector<Protocol::ThreadTraces> &notOverlapData, std::vector<SameOperatorsDetails> &opsDetails,
        const std::shared_ptr<VirtualTraceDatabase> &database) override;
private:
    static void QueryGroupMapAndCalculateSummary(const std::shared_ptr<VirtualTraceDatabase> &database,
        std::vector<Protocol::SystemViewOverallRes> &responseBody,
        std::vector<Protocol::SystemViewOverallRes>::iterator it,
        const std::vector<Protocol::ThreadTraces>& uncovered, double e2eTime);
    static bool CheckDataForSystemViewOverall(const std::shared_ptr<VirtualTraceDatabase> &database);
    static bool GetTmpTableForOverall(const std::shared_ptr<VirtualTraceDatabase> &database);
    static std::map<uint64_t, uint64_t> QueryFlowDict(const std::shared_ptr<VirtualTraceDatabase> &database,
                                                      int deviceId);
    std::vector<CpuCubeOpInfo> QueryCpuCubeOp(const std::shared_ptr<VirtualTraceDatabase> &database);
    std::vector<OverallTmpInfo> QueryKernelEventsForSystemViewOverall(
        const std::shared_ptr<VirtualTraceDatabase> &database, const std::map<uint64_t, uint64_t> &flowDict,
        int deviceId);
    static void QueryBwdTrackIdForComputingOverall(const std::shared_ptr<VirtualTraceDatabase> &database,
        uint64_t& bwdTrackId);
    std::string GetOrUpdateStringCacheValue(const std::shared_ptr<VirtualTraceDatabase> &database,
        const std::string& path, const std::string& key);
    void UpdateStringCacheValue(const std::shared_ptr<VirtualTraceDatabase> &database, const std::string& path);
    std::map<std::string, std::map<std::string, std::string>> stringsCache;
    std::recursive_mutex mutex;
};
}

#endif // PROFILER_SERVER_SYSTEMVIEWOVERALLDBREPO_H
