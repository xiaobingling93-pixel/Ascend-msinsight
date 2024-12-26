/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_DBCLUSTERDATABASE_H
#define PROFILER_SERVER_DBCLUSTERDATABASE_H

#include "VirtualClusterDatabase.h"

namespace Dic {
namespace Module {
namespace FullDb {

class DbClusterDataBase : public VirtualClusterDatabase {
public:
    explicit DbClusterDataBase(std::recursive_mutex &sqlMutex) : VirtualClusterDatabase(sqlMutex) {};
    ~DbClusterDataBase() override;

    bool CreateTable();
    bool DropTable();
    bool QuerySummaryData(const Protocol::SummaryTopRankParams &requestParams,
                          Protocol::SummaryTopRankResBody &responseBody) override;
    std::string QueryParseClusterStatus() override;
    void UpdateClusterParseStatus(std::string status) override;
    bool QueryBaseInfo(Protocol::SummaryBaseInfo &baseInfo) override;
    bool GetStepIdList(Protocol::PipelineStepResponseBody &responseBody) override;
    bool GetStages(Protocol::PipelineStageParam &param, Protocol::PipelineStageResponseBody &responseBody) override;
    bool GetStageAndBubble(Protocol::PipelineStageTimeParam &param,
                           Protocol::PipelineStageOrRankTimeResponseBody &responseBody) override;
    bool GetRankAndBubble(Protocol::PipelineRankTimeParam &param,
                          Protocol::PipelineStageOrRankTimeResponseBody &responseBody) override;
    bool GetGroups(Protocol::MatrixGroupParam &param, Protocol::MatrixGroupResponseBody &responseBody) override;
    bool QueryMatrixList(Protocol::MatrixBandwidthParam &param,
                         Protocol::MatrixListResponseBody &responseBody) override;
    bool QueryAllOperators(Protocol::OperatorDetailsParam &param, Protocol::OperatorDetailsResBody &resBody) override;
    bool QueryOperatorsCount(Protocol::OperatorDetailsParam &param, Protocol::OperatorDetailsResBody &resBody) override;
    bool QueryBandwidthData(Protocol::BandwidthDataParam &param, Protocol::BandwidthDataResBody &resBody) override;
    bool QueryDistributionData(Protocol::DistributionDataParam &param, Protocol::DistributionResBody &resBody) override;

    bool QueryRanksHandler(std::vector<Protocol::IterationsOrRanksObject> &responseBody) override;
    bool QueryOperatorNames(Protocol::OperatorNamesParams &requestParams,
                            std::vector<Protocol::OperatorNamesObject> &responseBody) override;
    bool QueryMatrixSortOpNames(Protocol::OperatorNamesParams &requestParams,
                                std::vector<Protocol::OperatorNamesObject> &responseBody);
    bool QueryIterations(std::vector<Protocol::IterationsOrRanksObject> &responseBody) override;
    bool QueryDurationList(Protocol::DurationListParams &requestParams,
        Protocol::DurationListsResponseBody &responseBody) override;
    bool QueryOperatorList(Protocol::DurationListParams &requestParams,
        Protocol::OperatorListsResponseBody &responseBody) override;
    bool QueryCommunicationGroup(Document &responseBody) override;
    bool QueryExtremumTimestamp(uint64_t &min, uint64_t &max) override;
    bool QueryIterationAndCommunicationGroup(Protocol::CommunicationKernelParams &params,
                                             Protocol::CommunicationKernelBody &responseBody) override;
    bool GetParallelConfigFromStepTrace(ParallelStrategyConfig &config, std::string &level) override;
    bool QueryParallelStrategyConfig(ParallelStrategyConfig &config, std::string &level) override;
    bool UpdateParallelStrategyConfig(const ParallelStrategyConfig &config,
        std::string &level, std::string &msg) override;
    bool QueryAllPerformanceDataByStep(const std::string &step,
                                       std::unordered_map<std::uint32_t, StepStatistic> &data) override;

    void PrepareForStageId(std::string &stageIdStr, std::string &sql, std::vector<std::string> &stageIds);

    void InsertClusterBaseInfo(ClusterBaseInfo &baseInfo);
private:
    std::string parseStatus = "UN_FINISH";
};
}
}
}

#endif // PROFILER_SERVER_DBCLUSTERDATABASE_H
