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
class DbTraceDataBase : public VirtualTraceDatabase {
public:
    explicit DbTraceDataBase(std::mutex &sqlMutex) : VirtualTraceDatabase(sqlMutex) {};
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
    bool QueryComputeTaskInfoById(int64_t id, Protocol::UnitThreadDetailBody &responseBody);
    bool QueryCommunicationTaskInfoById(int64_t id, Protocol::UnitThreadDetailBody &responseBody);
    bool QueryFlowDetail(const Protocol::UnitFlowParams &requestParams, Protocol::UnitFlowBody &responseBody,
                         uint64_t minTimestamp) override;
    bool QueryUnitsMetadata(const std::string &fileId,
                            std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData) override;
    bool QueryExtremumTimestamp(uint64_t &min, uint64_t &max) override;
    bool QueryFlowName(const Protocol::UnitFlowNameParams &requestParams, Protocol::UnitFlowNameBody &responseBody,
                       uint64_t minTimestamp, int64_t trackId) override;
    int SearchSliceNameCount(const Protocol::SearchCountParams &params) override;
    bool SearchSliceName(const std::string &name, int index, uint64_t minTimestamp,
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

private:
    const int cacheSize = 1000;
    bool initStmt = false;

    std::unique_ptr<SqlitePreparedStatement> updateTaskDepthStmt = nullptr;
    std::unique_ptr<SqlitePreparedStatement> updateApiDepthStmt = nullptr;

    std::vector<TASK_INFO> taskDepthCache;

    bool SetConfig();
    bool InitStmt();

    void UpdateDepth(const std::string &sql, std::unique_ptr<SqlitePreparedStatement> &updateStmt);
    bool UpdateDepthList(std::unique_ptr<SqlitePreparedStatement> &stmt);
    bool QueryAscendHardwareMetadata(const std::string &fileId,
                                     std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData);
    bool QueryHcclMetadata(const std::string &fileId,
                           std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData);
    bool QueryCounterMetadata(const std::string &fileId, std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData);
    bool NeedUpdateDepth(const std::string &table);
    bool GenerateCounterMetadata(const std::string &fileId,
                                 std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData);
    void SetKernelDetail(std::unique_ptr<SqliteResultSet> resultSet, uint64_t minTimestamp,
                         Protocol::KernelDetailsBody &responseBody) const;
    std::string GetKernelDetailSql(const Protocol::KernelDetailsParams &requestParams);
    static std::string ArgsDtoToJsonStr(const ArgsDto& argsDto);
    static std::unique_ptr<Protocol::UnitTrack> GenerateBaseUnitTrack(const std::string &type,
        const std::string &cardId, const std::string &processId, const std::string &processName,
        const std::string &metaType);
};
}

#endif // PROFILER_SERVER_DBTRACEDATABASE_H
