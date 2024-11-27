/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_JSON_CLUSTER_DATABASE_H
#define PROFILER_SERVER_JSON_CLUSTER_DATABASE_H

#include <set>
#include "VirtualClusterDatabase.h"
#include "ClusterDef.h"
#include "ProtocolMessage.h"
#include "SummaryProtocolResponse.h"
#include "TimelineProtocolResponse.h"
#include "SummaryProtocolRequest.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"

namespace Dic {
namespace Module {
class TextClusterDatabase : public VirtualClusterDatabase {
public:
    explicit TextClusterDatabase(std::recursive_mutex &sqlMutex) : VirtualClusterDatabase(sqlMutex) {};
    ~TextClusterDatabase() override;

    bool SetConfig();
    bool SetDbVersion();
    bool CreateTable();
    bool CreateIndex();
    bool CreateTimeIndex();
    bool InitStmt();
    void ReleaseStmt();
    void InsertTimeInfo(CommunicationTimeInfo &timeInfo);
    void InsertTimeInfoList(std::vector<CommunicationTimeInfo> &timeInfoList);
    void InsertBandwidth(CommunicationBandWidth &bandWidth);
    void InsertBandwidthList(std::vector<CommunicationBandWidth> &bandWidthList);
    void InsertStepStatisticsInfo(StepStatistic &stepStatistic);
    void InsertClusterBaseInfo(ClusterBaseInfo &baseInfo);
    void InsertGroupId(const std::unordered_map<std::string, int64_t> &groupIds);
    void InsertCommunicationMatrix(CommunicationMatrixInfo &communicationMatrix);
    void InsertCommunicationMatrixInfo(std::vector<CommunicationMatrixInfo> &matrixInfos);

    bool QuerySummaryData(const Protocol::SummaryTopRankParams &requestParams,
                          Protocol::SummaryTopRankResBody &responseBody) override;
    std::string QueryParseClusterStatus() override;
    void UpdateClusterParseStatus(std::string status) override;
    bool QueryBaseInfo(Protocol::SummaryTopRankResBody &responseBody) override;
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
    void SaveLastData();

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

    void PrepareForStageId(std::string &stageIdStr, std::string &sql, std::vector<std::string> &stageIds);
    std::unordered_map<std::string, int64_t> GetAllGroupMap();

private:
    sqlite3_stmt *insertTimeInfoStmt = nullptr;
    sqlite3_stmt *insertBandwidthStmt = nullptr;
    sqlite3_stmt *stepStmt = nullptr;
    sqlite3_stmt *matrixStmt = nullptr;
    bool isInitStmt = false;
    std::vector<CommunicationTimeInfo> timeInfoCache;
    std::vector<CommunicationBandWidth> bandwidthCache;
    std::vector<CommunicationMatrixInfo> matrixCache;
    std::string GetTimeInfoStmtSql(int len);
    std::string GetBandwidthStmtSql(int len);
    std::string GetMatrixStmtSql(int len);

    std::string BuildCondition(const Protocol::SummaryTopRankParams &requestParams);
    std::string GetStageIdByGroupId(const std::string &groupId);
};
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_JSON_CLUSTER_DATABASE_H