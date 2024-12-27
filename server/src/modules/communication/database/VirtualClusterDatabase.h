/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_VIRTUAL_CLUSTER_DATABASE_H
#define PROFILER_SERVER_VIRTUAL_CLUSTER_DATABASE_H

#include <set>
#include <unordered_map>
#include "Database.h"
#include "ClusterDef.h"
#include "ProtocolMessage.h"
#include "SummaryProtocolResponse.h"
#include "TimelineProtocolResponse.h"
#include "TimelineProtocolRequest.h"
#include "SummaryProtocolRequest.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"

namespace Dic {
namespace Module {
class VirtualClusterDatabase : public Database {
public:
    explicit VirtualClusterDatabase(std::recursive_mutex &sqlMutex) : Database(sqlMutex) {};
    ~VirtualClusterDatabase() override = default;

    virtual std::string QueryParseClusterStatus() = 0;
    virtual void UpdateClusterParseStatus(std::string status) = 0;
    bool HasFinishedParseLastTime();
    bool UpdatesClusterParseStatus(const std::string& status);

    virtual bool QuerySummaryData(const Protocol::SummaryTopRankParams &requestParams,
                          Protocol::SummaryTopRankResBody &responseBody) = 0;
    virtual bool QueryBaseInfo(Protocol::SummaryBaseInfo &baseInfo) = 0;
    virtual bool GetStepIdList(Protocol::PipelineStepResponseBody &responseBody) = 0;
    virtual bool GetStages(Protocol::PipelineStageParam &param, Protocol::PipelineStageResponseBody &responseBody) = 0;
    virtual bool GetStageAndBubble(Protocol::PipelineStageTimeParam &param,
                                   Protocol::PipelineStageOrRankTimeResponseBody &responseBody) = 0;
    virtual bool GetRankAndBubble(Protocol::PipelineRankTimeParam &param,
                                  Protocol::PipelineStageOrRankTimeResponseBody &responseBody) = 0;
    virtual bool GetGroups(const std::string &iterationId, std::vector<std::string> &groupList) = 0;
    virtual bool QueryMatrixList(Protocol::MatrixBandwidthParam &param,
                                 Protocol::MatrixListResponseBody &responseBody) = 0;
    virtual bool QueryAllOperators(Protocol::OperatorDetailsParam &param,
        Protocol::OperatorDetailsResBody &resBody) = 0;
    virtual bool QueryOperatorsCount(Protocol::OperatorDetailsParam &param,
        Protocol::OperatorDetailsResBody &resBody) = 0;
    virtual bool QueryBandwidthData(Protocol::BandwidthDataParam &param, Protocol::BandwidthDataResBody &resBody) = 0;
    virtual bool QueryDistributionData(Protocol::DistributionDataParam &param,
        Protocol::DistributionResBody &resBody) = 0;

    virtual bool QueryRanksHandler(std::vector<Protocol::IterationsOrRanksObject> &responseBody) = 0;
    virtual bool QueryOperatorNames(Protocol::OperatorNamesParams &requestParams,
                            std::vector<Protocol::OperatorNamesObject> &responseBody) = 0;
    virtual bool QueryIterations(std::vector<Protocol::IterationsOrRanksObject> &responseBody) = 0;
    virtual bool QueryDurationList(Protocol::DurationListParams &requestParams,
        Protocol::DurationListsResponseBody &responseBody) = 0;
    virtual bool QueryOperatorList(Protocol::DurationListParams &requestParams,
        Protocol::OperatorListsResponseBody &responseBody) = 0;
    virtual bool QueryCommunicationGroup(rapidjson::Document &responseBody) = 0;
    virtual bool QueryMatrixSortOpNames(Protocol::OperatorNamesParams &requestParams,
        std::vector<Protocol::OperatorNamesObject> &responseBody) = 0;
    virtual bool QueryExtremumTimestamp(uint64_t &min, uint64_t &max) = 0;
    virtual bool QueryIterationAndCommunicationGroup(Protocol::CommunicationKernelParams &params,
        Protocol::CommunicationKernelBody &responseBody) = 0;
    virtual bool GetParallelConfigFromStepTrace(ParallelStrategyConfig &config, std::string &level) = 0;
    virtual bool QueryParallelStrategyConfig(ParallelStrategyConfig &config, std::string &level) = 0;
    virtual bool UpdateParallelStrategyConfig(const ParallelStrategyConfig &config,
        std::string &level, std::string &msg) = 0;
    virtual bool QueryAllPerformanceDataByStep(const std::string &step,
                                               std::unordered_map<std::uint32_t, StepStatistic> &data) = 0;

protected:
    const std::string totalOpInfo = "Total Op Info";
    const std::string clusterParseStatus = "Cluster files parsing status";
    const double overlapThreshold = 0.05;
    bool HasColumn(const std::string &tableName, const std::string &columnName);
    bool ExecuteQuerySummaryData(const Protocol::SummaryTopRankParams &requestParams,
        Protocol::SummaryTopRankResBody &responseBody, std::string sql);
    bool ExecuteQueryBaseInfo(Protocol::SummaryBaseInfo &baseInfo, std::string sql);
    bool ExecuteGetStepIdList(Protocol::PipelineStepResponseBody &responseBody, std::string sql);
    bool ExecuteGetStages(Protocol::PipelineStageParam param, Protocol::PipelineStageResponseBody &responseBody,
        std::string sql);
    bool ExecuteGetStageAndBubble(Protocol::PipelineStageTimeParam param, std::vector<std::string> stageIds,
        Protocol::PipelineStageOrRankTimeResponseBody &responseBody, std::string sql);
    bool ExecuteGetRankAndBubble(const Protocol::PipelineRankTimeParam &param, std::vector<std::string> &&stageIds,
                                 Protocol::PipelineStageOrRankTimeResponseBody &responseBody, std::string &&sql);
    bool ExecuteGetGroups(const std::string &iterationId, std::vector<std::string> &groupList, std::string sql);
    bool ExecuteQueryMatrixList(Protocol::MatrixBandwidthParam param, Protocol::MatrixListResponseBody &responseBody,
        std::string sql);
    bool ExecuteQueryAllOperators(Protocol::OperatorDetailsParam &param, Protocol::OperatorDetailsResBody &resBody,
        std::string sql, uint64_t startTime);
    bool ExecuteQueryOperatorsCount(Protocol::OperatorDetailsParam &param, Protocol::OperatorDetailsResBody &resBody,
        std::string sql);
    bool ExecuteQueryBandwidthData(Protocol::BandwidthDataParam &param, Protocol::BandwidthDataResBody &resBody,
        std::string sql);
    bool ExecuteQueryDistributionData(Protocol::DistributionDataParam &param, Protocol::DistributionResBody &resBody,
        std::string sql);

    bool ExecuteQueryRanksHandler(std::vector<Protocol::IterationsOrRanksObject> &responseBody, std::string sql);
    bool ExecuteQueryOperatorNames(Protocol::OperatorNamesParams &requestParams,
        std::vector<Protocol::OperatorNamesObject> &responseBody, std::string sql);
    bool ExecuteQueryIterations(std::vector<Protocol::IterationsOrRanksObject> &responseBody, std::string sql);
    bool ExecuteQueryDurationList(Protocol::DurationListParams &requestParams,
        Protocol::DurationListsResponseBody &responseBody, std::string sql, uint64_t startTime);
    bool ExecuteQueryOperatorList(Protocol::DurationListParams &requestParams,
        Protocol::OperatorListsResponseBody &responseBody, const std::string &sql, uint64_t startTime);
    bool ExecuteQueryCommunicationGroup(rapidjson::Document &responseBody, std::string sql);
    bool ExecuteQueryMatrixSortOpNames(Protocol::OperatorNamesParams &requestParams,
                                       std::vector<Protocol::OperatorNamesObject> &responseBody, std::string sql);
    std::string GetRanksSql(const std::vector<std::string> &rankList);

    bool ExecuteQueryExtremumTimestamp(std::string &sql, uint64_t &min, uint64_t &max);
    bool ExecuteQueryIterationAndCommunicationGroup(std::string &sql,
        std::string &opName, const std::string &rankId, std::string &iteration, std::string &communicationGroup);
    bool ExecuteGetParallelConfigFromStepTrace(std::string &sql, ParallelStrategyConfig &config, std::string &level);
    bool ExecuteQueryParallelStrategyConfig(std::string &sql, ParallelStrategyConfig &config, std::string &level);
    bool ExecuteSetParallelStrategyConfig(std::string &sql, const ParallelStrategyConfig &config, std::string &level);
    bool ExecuteQueryAllPerformanceDataByStep(const std::string &sql,
        const std::string &step, std::unordered_map<std::uint32_t, StepStatistic> &data);

private:
    void GetStepsOrRanksObject(const std::string &jsonStr,
        std::vector<Protocol::IterationsOrRanksObject> &responseBody);
    void StatisticBandwidthData(const Protocol::Duration &item, std::vector<Protocol::BandwidthStatistic> &bwStat);
    void GetBandwidthStatisticResult(std::vector<Protocol::BandwidthStatistic> &bwStat,
        Protocol::DurationListsResponseBody &responseBody);
};
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_VIRTUAL_CLUSTER_DATABASE_H