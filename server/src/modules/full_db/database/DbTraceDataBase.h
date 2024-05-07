/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */


#ifndef PROFILER_SERVER_DBTRACEDATABASE_H
#define PROFILER_SERVER_DBTRACEDATABASE_H

#include "VirtualTraceDatabase.h"
#include "TraceDatabaseDef.h"
#include "map"

namespace Dic::Module::FullDb {
using namespace Dic::Module::Timeline;
struct TASK_INFO {
    int64_t start = 0;
    int64_t end = 0;
    int64_t depth = 0;
    int64_t id = 0;
};

struct WAIT_TIME {
    int64_t waitTime = 0;
    int64_t id = 0;
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
    bool QueryThreadTraces(const Protocol::UnitThreadTracesParams &requestParams,
        Protocol::UnitThreadTracesBody &responseBody, uint64_t minTimestamp, int64_t traceId) override;
    bool QueryThreads(const Protocol::UnitThreadsParams &requestParams, Protocol::UnitThreadsBody &responseBody,
                      uint64_t minTimestamp, int64_t traceId) override;
    bool QueryThreadDetail(const Protocol::ThreadDetailParams &requestParams,
                           Protocol::UnitThreadDetailBody &responseBody, uint64_t minTimestamp,
                           int64_t trackId) override;
    bool QueryFlowDetail(const Protocol::UnitFlowParams &requestParams, Protocol::UnitFlowBody &responseBody,
                         uint64_t minTimestamp) override;
    bool QueryUnitsMetadata(const std::string &fileId,
                            std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData) override;
    bool QueryExtremumTimestamp(uint64_t &min, uint64_t &max) override;
    void QueryFlowName(const Protocol::UnitFlowNameParams &requestParams, Protocol::UnitFlowNameBody &responseBody,
                       uint64_t minTimestamp, uint64_t trackId) override;
    bool QueryUintFlows(const Protocol::UnitFlowsParams &requestParams,
                        Protocol::UnitFlowsBody &responseBody, uint64_t minTimestamp, uint64_t trackId) override;
    int SearchSliceNameCount(const Protocol::SearchCountParams &params) override;
    bool SearchSliceName(const Protocol::SearchSliceParams &params, int index, uint64_t minTimestamp,
                         Protocol::SearchSliceBody &responseBody) override;
    bool QueryFlowCategoryList(std::vector<std::string> &categories) override;
    bool QueryFlowCategoryEvents(Protocol::FlowCategoryEventsParams &params, uint64_t minTimestamp,
                                 std::vector<std::unique_ptr<Protocol::FlowEvent>> &flowDetailList) override;
    bool QueryUnitCounter(Protocol::UnitCounterParams &params, uint64_t minTimestamp,
                          std::vector<Protocol::UnitCounterData> &dataList) override;

    bool QueryComputeStatisticsData(const Protocol::SummaryStatisticParams &requestParams,
                                    Protocol::SummaryStatisticsBody &responseBody) override;
    bool QueryCommunicationStatisticsData(const Protocol::SummaryStatisticParams &requestParams,
                                          Protocol::SummaryStatisticsBody &responseBody) override;
    bool QueryStepDuration(const std::string& stepId, uint64_t &min, uint64_t &max) override;
    bool QueryPythonViewData(const Protocol::SystemViewParams &requestParams,
                             Protocol::SystemViewBody &responseBody) override;
    LayerStatData QueryLayerData(const std::string &layer, const std::string &name) override;
    std::vector<std::string> QueryCoreType() override;
    bool QueryKernelDetailData(const Protocol::KernelDetailsParams &requestParams,
                               Protocol::KernelDetailsBody &responseBody, uint64_t minTimestamp) override;
    uint64_t QueryTotalKernel(const std::string &coreType, const std::string &name) override;
    bool QueryKernelDepthAndThread(const Protocol::KernelParams &params,
                                   Protocol::OneKernelBody &responseBody, uint64_t minTimestamp) override;
    OneKernelData QueryKernelTid(uint64_t trackId) override;
    bool QueryThreadTracesSummary(const Protocol::UnitThreadTracesSummaryParams &requestParams,
                                  Protocol::UnitThreadTracesSummaryBody &responseBody, uint64_t minTimestamp) override;
    bool QueryDurationFromTaskByTimeRange(const Protocol::ThreadDetailParams &requestParams,
                                      SliceDto sliceDto, std::vector<uint64_t> &nextDepthResult,
                                      int64_t streamId);
    bool QueryHostMetadata(std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData);

    std::vector<std::string> QueryRankId();

    bool CheckTableDataInvalid(std::string tableName);

    void UpdateStartTime();
    void UpdateAllDepth();
    void InitStringsCache();
    void UpdateWaitTime();
    void GenerateOverlapAnalysis();

private:
    const int cacheSize = 5000;
    bool initStmt = false;

    std::unique_ptr<SqlitePreparedStatement> updateTaskDepthStmt = nullptr;
    std::unique_ptr<SqlitePreparedStatement> updateApiDepthStmt = nullptr;
    std::unique_ptr<SqlitePreparedStatement> updateCannApiDepthStmt = nullptr;
    std::unique_ptr<SqlitePreparedStatement> insertOverlapStmt = nullptr;

    std::vector<TASK_INFO> taskDepthCache;
    std::vector<WAIT_TIME> taskWaitTimeCache;
    std::vector<OVERLAP_INFO> timeInfoCache;

    bool SetConfig();
    bool InitStmt();

    void UpdateDepth(const std::string &sql, std::unique_ptr<SqlitePreparedStatement> &updateStmt);
    bool UpdateDepthList(std::unique_ptr<SqlitePreparedStatement> &stmt);
    bool QueryAscendHardwareMetadata(const std::string &fileId,
                                     std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData);
    bool QueryHcclMetadata(const std::string &fileId,
                           std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData);
    bool GenerateOverlapAnalysisMetadata(const std::string &fileId,
                                         std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData);
    bool QueryCounterMetadata(const std::string &fileId, std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData);
    bool NeedUpdateDepth(const std::string &table);
    void GenerateCounterMetadata(const std::string &fileId,
                                 std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData);
    void SetKernelDetail(std::unique_ptr<SqliteResultSet> resultSet, uint64_t minTimestamp,
                         Protocol::KernelDetailsBody &responseBody) const;
    std::string GetKernelDetailSql(const Protocol::KernelDetailsParams &requestParams);
    static std::unique_ptr<Protocol::UnitTrack> GenerateBaseUnitTrack(const std::string &type,
        const std::string &cardId, const std::string &processId, const std::string &processName,
        const std::string &metaType);
    bool DealHostMetadata(std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData,
                          std::map<std::string, std::vector<MetaDataDto>> &threadMap);
    bool UpdateTaskInfoWaitTime(std::unique_ptr<SqlitePreparedStatement> &stmt);
    std::string GetSearchSliceNameSql(bool isMatchExact, bool isMatchCase, std::string rankId);
    std::string GetSearchSliceNameCountSql(bool isMatchExact, bool isMatchCase, std::string rankId);
    void QueryTaskTimeInfo(bool isComputing, std::vector<OVERLAP_INFO> &timeInfoList, const std::string &rankId);
    bool InsertOverlapAnalysisInfo(const std::vector<OVERLAP_INFO> &overlapInfoList, const std::string &rankId);
    void GetCounterUnitsAndDataTypes(Protocol::PROCESS_TYPE type, std::vector<std::string> &units,
         std::vector<std::vector<std::string>> &dataTypes, std::unique_ptr<Protocol::UnitTrack> &counter);
};
}

#endif // PROFILER_SERVER_DBTRACEDATABASE_H
