/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */


#ifndef PROFILER_SERVER_DBTRACEDATABASE_H
#define PROFILER_SERVER_DBTRACEDATABASE_H

#include <TimelineProtocolRequest.h>
#include "VirtualTraceDatabase.h"
#include "TraceDatabaseDef.h"
#include "map"
#include "TimelineProtocolRequest.h"
#include "DomainObject.h"

namespace Dic::Module::FullDb {
using namespace Dic::Module::Timeline;
struct TASK_INFO {
    uint64_t start = 0;
    uint64_t end = 0;
    uint64_t depth = 0;
    uint64_t id = 0;
};

struct WAIT_TIME {
    int64_t waitTime = 0;
    int64_t id = 0;
    std::string type;
};

struct OVERLAP_INFO {
    OVERLAP_INFO() = default;
    OVERLAP_INFO(int64_t startNs, int64_t endNs, int64_t type) : startNs(startNs), endNs(endNs),
        type(type) {};
    int64_t startNs;
    int64_t endNs;
    int64_t type; // Computing = 0, Communication = 1, Communication(Not Overlapped) = 2, Free = 3
    bool operator < (const OVERLAP_INFO& right) const
    {
        if (startNs < right.startNs) {
            return true;
        }
        if (startNs == right.startNs && endNs < right.endNs) {
            return true;
        }
        return false;
    }
};
class DbTraceDataBase : public VirtualTraceDatabase {
public:
    explicit DbTraceDataBase(std::recursive_mutex &sqlMutex) : VirtualTraceDatabase(sqlMutex) {};
    ~DbTraceDataBase();

    bool OpenDb(const std::string &dbPath, bool clearAllTable) override;

    // search
    bool QueryThreads(const Protocol::UnitThreadsParams &requestParams, Protocol::UnitThreadsBody &responseBody,
                      uint64_t minTimestamp, const std::vector<uint64_t> &trackIdList) override;
    bool QueryUnitsMetadata(const std::string &fileId,
                            std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData) override;
    bool QueryExtremumTimestamp(uint64_t &min, uint64_t &max) override;
    bool QueryUintFlows(const Protocol::UnitFlowsParams &requestParams,
                        Protocol::UnitFlowsBody &responseBody, uint64_t minTimestamp, uint64_t trackId) override;
    int SearchSliceNameCount(const Protocol::SearchCountParams &params) override;
    bool SearchSliceName(const Protocol::SearchSliceParams &params, int index, uint64_t minTimestamp,
                         Protocol::SearchSliceBody &responseBody) override;
    bool QueryFlowCategoryList(std::vector<std::string> &categories, const std::string& rankId) override;
    bool QueryUnitCounter(Protocol::UnitCounterParams &params, uint64_t minTimestamp,
                          std::vector<Protocol::UnitCounterData> &dataList) override;

    bool QueryComputeStatisticsData(const Protocol::SummaryStatisticParams &requestParams,
                                    Protocol::SummaryStatisticsBody &responseBody) override;
    bool QueryCommunicationStatisticsData(const Protocol::SummaryStatisticParams &requestParams,
                                          Protocol::SummaryStatisticsBody &responseBody) override;
    bool QueryStepDuration(const std::string& stepId, uint64_t &min, uint64_t &max) override;
    bool QuerySystemViewData(const Protocol::SystemViewParams &requestParams,
                             Protocol::SystemViewBody &responseBody) override;
    LayerStatData QueryLayerData(const std::string &layer, const std::string &name) override;
    std::vector<std::string> QueryCoreType() override;
    bool QueryKernelDetailData(const Protocol::KernelDetailsParams &requestParams,
                               Protocol::KernelDetailsBody &responseBody, uint64_t minTimestamp) override;
    bool GetKernelDetailFilterSql(std::string& sql, const Protocol::KernelDetailsParams &requestParams);
    uint64_t QueryTotalKernel(const Protocol::KernelDetailsParams &requestParams) override;
    bool QueryKernelDepthAndThread(const Protocol::KernelParams &params,
                                   Protocol::OneKernelBody &responseBody, uint64_t minTimestamp) override;
    bool QueryCommunicationKernelInfo(const std::string &name, const std::string &rankId,
                                      Protocol::CommunicationKernelBody &body) override;
    OneKernelData QueryKernelTid(uint64_t trackId) override;
    bool QueryThreadTracesSummary(const Protocol::UnitThreadTracesSummaryParams &requestParams,
                                  Protocol::UnitThreadTracesSummaryBody &responseBody, uint64_t minTimestamp) override;

    bool SearchAllSlicesDetails(const Protocol::SearchAllSliceParams &params, Protocol::SearchAllSlicesBody &body,
                                uint64_t minTimestamp) override;
    bool QueryThreadSameOperatorsDetails(const Protocol::UnitThreadsOperatorsParams &requestParams,
         Protocol::UnitThreadsOperatorsBody &responseBody, uint64_t minTimestamp, int64_t traceId) override;
    bool QueryHostMetadata(std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData);

    std::vector<std::string> QueryRankId();
    std::string QueryHostInfo() override;
    std::string GetDeviceId(const std::string& fileId);
    std::unordered_map<std::string, std::string> QueryRankIdAndDeviceMap();

    bool QueryAffinityOptimizer(const Protocol::KernelDetailsParams &params, const std::string &optimizers,
        std::vector<Protocol::ThreadTraces> &data, uint64_t minTimestamp) override;
    bool QueryAICpuOpCanBeOptimized(const Protocol::KernelDetailsParams &params,
        const std::vector<std::string> &replace, const std::map<std::string, Timeline::AICpuCheckDataType> &dataType,
        std::vector<Protocol::KernelBaseInfo> &data, uint64_t minTimestamp) override;
    bool QueryAclnnOpCountExceedThreshold(const Protocol::KernelDetailsParams &params, uint64_t threshold,
        std::vector<Protocol::KernelBaseInfo> &data, uint64_t minTimestamp) override;
    bool QueryAffinityAPIData(const Protocol::KernelDetailsParams &params, const std::set<std::string> &pattern,
        uint64_t minTimestamp, std::map<uint64_t, std::vector<Protocol::FlowLocation>> &data,
        std::map<uint64_t, std::vector<uint32_t>> &indexes) override;
    bool QueryFuseableOpData(const Protocol::KernelDetailsParams &params, const Timeline::FuseableOpRule &rule,
        std::vector<Protocol::FlowLocation> &data, uint64_t minTimestamp) override;
    bool QueryEventsViewData(const Protocol::EventsViewParams &params, Protocol::EventsViewBody &body,
        uint64_t minTimestamp) override;

    bool CheckTableDataInvalid(std::string tableName);

    void UpdateStartTime(const std::string &fileId);
    void UpdateAllDepth();
    void InitStringsCache();
    void InitConnectionCats();
    void UpdateWaitTime();
    void GenerateOverlapAnalysis();
    static std::string GetStringCacheValue(const std::string& path, std::string key);
    static void Reset();
    bool QueryFwdBwdDataByFlow(const std::string &rankId, uint64_t offset, const Protocol::ExtremumTimestamp &range,
        std::vector<Protocol::ThreadTraces> &fwdBwdData) override;
    bool QueryP2PCommunicationOpData(const std::string &rankId, uint64_t offset,
        const Protocol::ExtremumTimestamp &range, std::vector<Protocol::ThreadTraces> &p2pOpData) override;

private:
    const uint32_t cacheSize = 5000;
    bool initStmt = false;
    bool isExistPytorch = false;
    bool isExistCann = false;
    bool isExistMstx = false;

    std::string host;

    std::unique_ptr<SqlitePreparedStatement> updateTaskDepthStmt = nullptr;
    std::unique_ptr<SqlitePreparedStatement> updateApiDepthStmt = nullptr;
    std::unique_ptr<SqlitePreparedStatement> updateCannApiDepthStmt = nullptr;
    std::unique_ptr<SqlitePreparedStatement> insertOverlapStmt = nullptr;

    std::vector<TASK_INFO> taskDepthCache;
    std::vector<WAIT_TIME> taskWaitTimeCache;
    std::vector<OVERLAP_INFO> timeInfoCache;
    std::vector<std::string> rankIds;

    bool SetConfig();
    bool InitStmt();

    void UpdateDepth(const std::string &sql, std::unique_ptr<SqlitePreparedStatement> &updateStmt);
    bool UpdateDepthList(std::unique_ptr<SqlitePreparedStatement> &stmt);
    bool QueryOperateMetadata(const std::string &fileId,
                              std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData);
    bool GenerateOverlapAnalysisMetadata(const std::string &fileId,
                                         std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData);
    bool QueryCounterMetadata(const std::string &fileId, std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData);
    bool NeedUpdateDepth(const std::string &table);
    void GenerateCounterMetadata(const std::string &fileId,
                                 std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData);
    std::string GetKernelDetailSql(const Protocol::KernelDetailsParams &requestParams);
    static std::unique_ptr<Protocol::UnitTrack> GenerateBaseUnitTrack(const std::string &type,
        const std::string &cardId, const std::string &processId, const std::string &processName,
        const std::string &metaType);
    void DealHostMetadata(std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData,
                          std::map<std::string, std::vector<MetaDataDto>> &threadMap);
    bool UpdateTaskInfoWaitTime(std::unique_ptr<SqlitePreparedStatement> &updateComputeStmt,
                                std::unique_ptr<SqlitePreparedStatement> &updateCommunicationStmt);
    std::string GetSearchSliceNameSql(bool isMatchExact, bool isMatchCase, std::string rankId,
                                      const std::string &order, const std::string &orderByField);
    std::string GetSearchSliceNameCountSql(bool isMatchExact, bool isMatchCase, std::string rankId);
    void QueryTaskTimeInfo(bool isComputing, std::vector<OVERLAP_INFO> &timeInfoList, const std::string &rankId);
    bool InsertOverlapAnalysisInfo(const std::vector<OVERLAP_INFO> &overlapInfoList, const std::string &rankId);
    void GetCounterUnitsAndDataTypes(PROCESS_TYPE type, std::vector<std::string> &units,
         std::vector<std::vector<std::string>> &dataTypes, std::unique_ptr<Protocol::UnitTrack> &counter);
    std::string GetSearchAllSlicesDetailsSql(bool isMatchExact, bool isMatchCase,
                                             const std::string& order, const std::string& orderByField);
    std::vector<Protocol::SimpleSlice>
    QueryThreadByPid(const Protocol::Metadata &metaData, uint64_t startTime, uint64_t endTime,
                     const std::string &rankId, std::map<std::string, uint64_t> &selfTimeKeyValue);

    void ProcessThreadUnit(std::unique_ptr<Protocol::UnitTrack> &process, std::unique_ptr<SqliteResultSet> &resultSet,
                           std::unique_ptr<Protocol::UnitTrack> &thread, const std::string &threadId) const;
    bool ExcecuteQueryKernelDetailData(std::unique_ptr<SqlitePreparedStatement> &stmt,
        const Protocol::KernelDetailsParams &requestParams, Protocol::KernelDetailsBody &responseBody,
        uint64_t minTimestamp);
};
}

#endif // PROFILER_SERVER_DBTRACEDATABASE_H
