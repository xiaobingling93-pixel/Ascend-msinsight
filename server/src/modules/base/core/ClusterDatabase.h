/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_CLUSTER_DATABASE_H
#define PROFILER_SERVER_CLUSTER_DATABASE_H

#include <set>
#include "Database.h"
#include "ClusterDef.h"
#include "Protocol.h"
#include "SummaryProtocolResponse.h"
#include "SummaryProtocolRequest.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"

namespace Dic {
namespace Module {
class ClusterDatabase : public Database {
public:
    ClusterDatabase() = default;
    ~ClusterDatabase() override;

    bool SetConfig();
    bool CreateTable();
    bool CreateIndex();
    bool InitStmt();
    void ReleaseStmt();
    void InsertTimeInfo(CommunicationTimeInfo &timeInfo);
    void InsertTimeInfoList(std::vector<CommunicationTimeInfo> &timeInfoList);
    void InsertBandwidth(CommunicationBandWidth &bandWidth);
    void InsertBandwidthList(std::vector<CommunicationBandWidth> &bandWidthList);
    void InsertStepStatisticsInfo(StepStatistic &stepStatistic);
    void InsertClusterBaseInfo(ClusterBaseInfo &clusterBaseInfo);
    void InsertGroupId(std::set<std::string> &groupIds);
    void InsertCommunicationMatrix(CommunicationMatrixInfo &communicationMatrix);
    void InsertCommunicationMatrixInfo(std::vector<CommunicationMatrixInfo> &communicationMatrixInfo);
    bool QuerySummaryData(const Protocol::SummaryTopRankParams &requestParams,
                          Protocol::SummaryTopRankResBody &responseBody);
    bool QueryBaseInfo(Protocol::SummaryTopRankResBody &responseBody);
    bool GetStepIdList(Protocol::PipelineStepResponseBody &responseBody);
    bool GetStages(Protocol::PipelineStageParam param, Protocol::PipelineStageResponseBody &responseBody);
    bool GetStageAndBubble(Protocol::PipelineStageTimeParam param,
                           Protocol::PipelineStageOrRankTimeResponseBody &responseBody);
    bool GetRankAndBubble(Protocol::PipelineRankTimeParam param,
                          Protocol::PipelineStageOrRankTimeResponseBody &responseBody);
    bool GetGroups(Protocol::MatrixGroupParam param, Protocol::MatrixGroupResponseBody &responseBody);
    bool QueryMatrixList(Protocol::MatrixBandwidthParam param, Protocol::MatrixListResponseBody &responseBody);
    bool QueryAllOperators(Protocol::OperatorDetailsParam &param, Protocol::OperatorDetailsResBody &resBody);
    bool QueryOperatorsCount(Protocol::OperatorDetailsParam &param, Protocol::OperatorDetailsResBody &resBody);
    bool QueryBandwidthData(Protocol::BandwidthDataParam &param, Protocol::BandwidthDataResBody &resBody);
    bool QueryDistributionData(Protocol::DistributionDataParam &param, Protocol::DistributionResBody &resBody);
    void SaveLastData();

    bool QueryRanksHandler(std::vector<Protocol::IterationsOrRanksObject> &responseBody);
    bool QueryOperatorNames(Protocol::OperatorNamesParams &requestParams,
                            std::vector<Protocol::OperatorNamesObject> &responseBody);
    bool QueryIterations(std::vector<Protocol::IterationsOrRanksObject> &responseBody);
    bool QueryDurationList(Protocol::DurationListParams &requestParams,
                           std::vector<Protocol::Duration> &responseBody);
    bool QueryCommunicationGroup(Document &responseBody);

private:
    sqlite3_stmt *insertTimeInfoStmt = nullptr;
    sqlite3_stmt *insertBandwidthStmt = nullptr;
    sqlite3_stmt *stepStmt = nullptr;
    sqlite3_stmt *matrixStmt = nullptr;
    bool isInitStmt = false;
    std::vector<CommunicationTimeInfo> timeInfoCache;
    std::vector<CommunicationBandWidth> bandwidthCache;
    std::vector<CommunicationMatrixInfo> matrixCache;
    sqlite3_stmt *GetTimeInfoStmtSql(int len);
    sqlite3_stmt *GetBandwidthStmtSql(int len);
    sqlite3_stmt *GetMatrixStmtSql(int len);

    sqlite3_stmt *BuildCondition(const Protocol::SummaryTopRankParams &requestParams);
    std::string GetRanksSql(std::vector<std::string> rankList);
    void GetStepsOrRanksObject(const std::string& jsonStr,
                               std::vector<Protocol::IterationsOrRanksObject> &responseBody);
};
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_CLUSTER_DATABASE_H