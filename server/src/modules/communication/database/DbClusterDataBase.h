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
    bool GetGroups(const std::string &iterationId, std::vector<std::string> &groupList) override;
    bool QueryMatrixList(Protocol::MatrixBandwidthParam &param, std::vector<MatrixInfoDo> &matrixInfoDoList) override;
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
        std::vector<DurationDo> &durationDoList) override;
    bool QueryOperatorList(Protocol::DurationListParams &requestParams,
        std::vector<OperatorTimeDo> &operatorTimeDoList) override;
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

    bool HasClusterBaseInfoTable();
    void SetHasClusterBaseInfoTable();
    bool QueryDistributedArgs(ParallelStrategyConfig &config, std::string &level);
private:
    std::string parseStatus = "UN_FINISH";
    // 标记初始状态的数据库是否有ClusterBaseInfo表
    bool hasClusterBaseInfoTable = false;
    bool ExecuteQueryDistributedArgs(ParallelStrategyConfig &config, std::string &level, std::string &sql);
};
}
}
}

#endif // PROFILER_SERVER_DBCLUSTERDATABASE_H
