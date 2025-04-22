/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_VIRTUAL_CLUSTER_DATABASE_H
#define PROFILER_SERVER_VIRTUAL_CLUSTER_DATABASE_H

#include <set>
#include <unordered_map>
#include "Database.h"
#include "ClusterDomainObject.h"
#include "ClusterDef.h"
#include "ProtocolMessage.h"
#include "SummaryProtocolResponse.h"
#include "TimelineProtocolResponse.h"
#include "TimelineProtocolRequest.h"
#include "SummaryProtocolRequest.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"
#include "TableDefs.h"

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

    virtual bool QueryBaseInfo(Protocol::SummaryBaseInfo &baseInfo) = 0;
    virtual bool GetStepIdList(Protocol::PipelineStepResponseBody &responseBody) = 0;
    virtual bool GetStages(Protocol::PipelineStageParam &param, Protocol::PipelineStageResponseBody &responseBody) = 0;
    virtual bool GetStageAndBubble(Protocol::PipelineStageTimeParam &param,
                                   Protocol::PipelineStageOrRankTimeResponseBody &responseBody) = 0;
    virtual bool GetRankAndBubble(Protocol::PipelineRankTimeParam &param,
                                  Protocol::PipelineStageOrRankTimeResponseBody &responseBody) = 0;
    virtual std::vector<std::string> GetAllRankFromStepStatisticInfo() = 0;
    virtual std::vector<CommInfoUnderRank> GetCommTimeForRankDim(const std::string &stepId) = 0;
    virtual bool GetGroups(const std::string &iterationId, std::vector<std::string> &groupList) = 0;
    virtual bool QueryMatrixList(Protocol::MatrixBandwidthParam &param,
                                 std::vector<MatrixInfoDo> &matrixInfoDoList) = 0;
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
        std::vector<DurationDo> &durationDoList) = 0;
    virtual bool QueryOperatorList(Protocol::DurationListParams &requestParams,
        std::vector<OperatorTimeDo> &operatorTimeDoList) = 0;
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
    bool BatchInsertExpertHotspotData(const std::vector<ExpertHotspotStruct> &expertHotspotInfos);
    void InsertExpertHotspotDataForCache(const ExpertHotspotStruct &info);
    void SaveExpertHotspot();
    bool DeleteExpertHotspot(const std::string &modelStage, const std::string &version);
    std::vector<ExpertHotspotStruct> QueryExpertHotspotData(const std::string &modelStage, const std::string &version);

protected:
    const std::string totalOpInfo = "Total Op Info";
    const std::string clusterParseStatus = "Cluster files parsing status";
    const std::string commonSql = "CREATE TABLE IF NOT EXISTS " + TABLE_EXPERT_HOTSPOT_INTO +
        " (id INTEGER PRIMARY KEY AUTOINCREMENT, localExpertId INTEGER, modelStage TEXT, rankId INTEGER,"
        " visits INTEGER, layer INTEGER, version TEXT);"
        "CREATE INDEX IF NOT EXISTS idx_ms ON " + TABLE_EXPERT_HOTSPOT_INTO + "(modelStage, version);";
    sqlite3_stmt *insertHotspotStmt = nullptr;
    std::vector<ExpertHotspotStruct> expertHotspotCache;
    const double overlapThreshold = 0.05;
    bool HasColumn(const std::string &tableName, const std::string &columnName);
    bool ExecuteQueryBaseInfo(Protocol::SummaryBaseInfo &baseInfo, std::string sql);
    bool ExecuteGetStepIdList(Protocol::PipelineStepResponseBody &responseBody, std::string sql);
    bool ExecuteGetStages(Protocol::PipelineStageParam param, Protocol::PipelineStageResponseBody &responseBody,
        std::string sql);
    bool ExecuteGetStageAndBubble(Protocol::PipelineStageTimeParam param, std::vector<std::string> stageIds,
        Protocol::PipelineStageOrRankTimeResponseBody &responseBody, std::string sql);
    bool ExecuteGetRankAndBubble(const Protocol::PipelineRankTimeParam &param, std::vector<std::string> &&stageIds,
                                 Protocol::PipelineStageOrRankTimeResponseBody &responseBody, std::string &&sql);
    std::vector<std::string> ExecuteGetAllRankFromStepStatisticInfo(std::string &sql);
    std::vector<CommInfoUnderRank> ExecuteGetCommTimeForRankDim(std::string &sql, const std::string &step);
    bool ExecuteGetGroups(const std::string &iterationId, std::vector<std::string> &groupList, std::string sql);
    bool ExecuteQueryMatrixList(Protocol::MatrixBandwidthParam &param, std::vector<MatrixInfoDo> &matrixInfoDoList,
        const std::string &sql);
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
                                  std::vector<DurationDo> &durationDoList, std::string sql, uint64_t startTime);
    bool ExecuteQueryOperatorList(Protocol::DurationListParams &requestParams,
        std::vector<OperatorTimeDo> &operatorTimeDoList, const std::string &sql, uint64_t startTime);
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
    std::string FindValueByKey(const std::map<std::string, std::string> &info,
        const std::string &key, const std::string defaultValue);
    void GetStepsOrRanksObject(const std::string &jsonStr,
                               std::vector<Protocol::IterationsOrRanksObject> &responseBody);
    sqlite3_stmt *GetExpertHotspotInsertStmt(uint64_t paramLen);
    sqlite3_stmt *InitExpertHotspotInsertStmt(uint64_t paramLen);
};
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_VIRTUAL_CLUSTER_DATABASE_H