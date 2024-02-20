/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_TRACE_DATABASE_H
#define PROFILER_SERVER_TRACE_DATABASE_H

#include <memory>
#include <string>
#include <vector>
#include <set>
#include "TimelineProtocolRequest.h"
#include "TimelineProtocolResponse.h"
#include "TimelineProtocolEvent.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "Database.h"
#include "GlobalDefs.h"
#include "TraceDatabaseDef.h"
#include "EventDef.h"

namespace Dic {
namespace Module {
namespace Timeline {
class TraceDatabase : public Database {
public:
    explicit TraceDatabase(std::mutex &sqlMutex);
    ~TraceDatabase() override;

    bool SetConfig();
    bool CreateTable();
    bool DropTable();
    bool CreateIndex();
    bool InitStmt();
    void ReleaseStmt();
    bool InsertSlice(const Trace::Slice &event);
    bool AddThreadCache(const std::tuple<int64_t, std::string, std::string> &threadInfo);
    bool InsertThreadList(const std::set<std::tuple<int64_t, std::string, std::string>> &threadList);
    bool UpdateProcessName(const Trace::MetaData &event);
    bool UpdateProcessLabel(const Trace::MetaData &event);
    bool UpdateProcessSortIndex(const Trace::MetaData &event);
    bool UpdateThreadInfo(const std::tuple<int64_t, std::string, std::string> &thread);
    bool UpdateThreadName(const Trace::MetaData &event);
    bool UpdateThreadSortIndex(const Trace::MetaData &event);
    bool InsertFlow(const Trace::Flow &event);
    bool InsertCounter(const Trace::Counter &event);
    bool InsertSliceList(const std::vector<Trace::Slice> &eventList);
    bool InsertFlowList(const std::vector<Trace::Flow> &eventList);
    bool InsertCounterList(const std::vector<Trace::Counter> &eventList);
    void CommitData();
    void UpdateDepth();
    void CreateDepthTempTable();
    void DropDepthTempTable();
    void UpdateSliceDepth();
    void DeleteInvalidFlowData();

    // search
    bool QueryThreadTraces(const Protocol::UnitThreadTracesParams &requestParams,
                           Protocol::UnitThreadTracesBody &responseBody, uint64_t minTimestamp, int64_t traceId);
    bool QueryThreadTracesSummary(const Protocol::UnitThreadTracesSummaryParams &requestParams,
                                  Protocol::UnitThreadTracesSummaryBody &responseBody, uint64_t minTimestamp);
    bool QueryThreads(const Protocol::UnitThreadsParams &requestParams, Protocol::UnitThreadsBody &responseBody,
                      uint64_t minTimestamp, int64_t traceId);
    bool QueryThreadDetail(const Protocol::ThreadDetailParams &requestParams,
                           Protocol::UnitThreadDetailBody &responseBody, uint64_t minTimestamp, int64_t trackId);
    bool QueryFlowDetail(const Protocol::UnitFlowParams &requestParams, Protocol::UnitFlowBody &responseBody,
                         uint64_t minTimestamp);
    bool QueryUnitsMetadata(const std::string &fileId, std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData);
    bool QueryExtremumTimestamp(uint64_t &min, uint64_t &max);
    bool QueryFlowName(const Protocol::UnitFlowNameParams &requestParams, Protocol::UnitFlowNameBody &responseBody,
                       uint64_t minTimestamp, int64_t trackId);
    int SearchSliceNameCount(const std::string &name);
    bool SearchSliceName(const std::string &name, int index, uint64_t minTimestamp,
                         Protocol::SearchSliceBody &responseBody);
    bool QueryFlowCategoryList(std::vector<std::string> &categories);
    bool QueryFlowCategoryEvents(Protocol::FlowCategoryEventsParams &params, uint64_t minTimestamp,
                                 std::vector<std::unique_ptr<Protocol::FlowEvent>> &flowDetailList);
    bool QueryUnitCounter(Protocol::UnitCounterParams &params, uint64_t minTimestamp,
                          std::vector<Protocol::UnitCounterData> &dataList);

    bool QueryComputeStatisticsData(const Protocol::SummaryStatisticParams &requestParams,
                                    Protocol::SummaryStatisticsBody &responseBody);
    bool QueryCommunicationStatisticsData(const Protocol::SummaryStatisticParams &requestParams,
                                          Protocol::SummaryStatisticsBody &responseBody);
    bool QueryStepDuration(const std::string& stepId, uint64_t &min, uint64_t &max);
    bool QueryPythonViewData(const Protocol::SystemViewParams &requestParams, Protocol::SystemViewBody &responseBody);
    LayerStatData QueryLayerData(const std::string &layer, const std::string &name);
    std::vector<std::string> QueryCoreType();
    bool QueryKernelDetailData(const Protocol::KernelDetailsParams &requestParams,
                                              Protocol::KernelDetailsBody &responseBody, uint64_t minTimestamp);
    uint64_t QueryTotalKernel(const std::string &coreType, const std::string &name);
    bool QueryKernelDepthAndThread(const Protocol::KernelParams &params,
                                                  Protocol::OneKernelBody &responseBody, uint64_t minTimestamp);
    OneKernelData QueryKernelTid(const uint64_t trackId);

private:
    std::mutex &mutex;
    const std::string sliceTable = "slice";
    const std::string threadTable = "thread";
    const std::string processTable = "process";
    const std::string flowTable = "flow";
    const std::string counterTable = "counter";
    const std::string idIndex = "id_index";
    const std::string trackIdTimeIndex = "track_id_time_index";
    const std::string flowIndex = "flow_id_time_index";
    const std::string kernelDetail = "kernel_detail";
    const std::string lineStart = "s";
    const std::string lineEnd = "f";
    const std::string lineEndOptional = "t";
    const int summaryPerpix = 10;
    // 3G size limit 2024.02.01
    const double middleImage = 1.5;
    // 5G size limit 2024.02.01
    const double lowImage = 5.0;

    bool initStmt = false;
    std::unique_ptr<SqlitePreparedStatement> insertSliceStmt = nullptr;
    std::unique_ptr<SqlitePreparedStatement> updateProcessNameStmt = nullptr;
    std::unique_ptr<SqlitePreparedStatement> updateProcessLabelStmt = nullptr;
    std::unique_ptr<SqlitePreparedStatement> updateProcessSortIndexStmt = nullptr;
    std::unique_ptr<SqlitePreparedStatement> updateThreadInfoStmt = nullptr;
    std::unique_ptr<SqlitePreparedStatement> updateThreadNameStmt = nullptr;
    std::unique_ptr<SqlitePreparedStatement> updateThreadSortIndexStmt = nullptr;
    std::unique_ptr<SqlitePreparedStatement> insertFlowStmt = nullptr;
    std::unique_ptr<SqlitePreparedStatement> insertCounterStmt = nullptr;
    const int cacheSize = 1000;
    const int unit = 1000;
    std::vector<Trace::Slice> sliceCache;
    std::vector<Trace::Flow> flowCache;
    std::vector<Trace::Counter> counterCache;
    std::set<std::tuple<int64_t, std::string, std::string>> threadInfoCache;

    bool InitSliceFlowCounterStmt();
    bool InitProcessThreadStmt();
    std::unique_ptr<SqlitePreparedStatement> GetSliceStmt(uint64_t paramLen);
    std::unique_ptr<SqlitePreparedStatement> GetFlowStmt(uint64_t paramLen);
    std::unique_ptr<SqlitePreparedStatement> GetCounterStmt(uint64_t paramLen);

    bool QueryExtremumTimeOfFirstDepth(int64_t trackId, uint64_t startTime, uint64_t endTime,
                                       Protocol::ExtremumTimestamp &extremumTimestamp);
    void CalculateSelfTime(std::vector<Protocol::SimpleSlice> &simpleSliceVec,
                           std::map<std::string, uint64_t> &selfTimeKeyValue, uint64_t startTime, uint64_t endTime);
    void AddData(std::map<std::string, uint64_t> &selfTimeKeyValue, const std::string &name, uint64_t tmpSelfTime);
    std::vector<Protocol::SimpleSlice> ThreadsInfoFilter(const std::vector<Protocol::SimpleSlice> &simpleSliceVec,
                                                         uint64_t startTime, uint64_t endTime);
    void ReduceThread(const std::vector<Protocol::SimpleSlice> &rows,
                      const std::map<std::string, uint64_t> &selfTimeKeyValue,
                      Protocol::UnitThreadsBody &responseBody);
    bool QueryDurationFromSliceByTimeRange(const Protocol::ThreadDetailParams &requestParams,
                                           const std::vector<SliceDto> &rows,
                                           std::vector<uint64_t> &nextDepthResult, int64_t trackId);
    bool FlowDetailToResponse(const std::vector<FlowDetailDto> &flowDetailVec, uint64_t minTimestamp,
                              Protocol::UnitFlowBody &responseBody);
    void FlowEventsToResponse(const std::vector<FlowCategoryEventsDto> &flowEventsVec,
                              const std::string &category,
                              std::vector<std::unique_ptr<Protocol::FlowEvent>> &flowDetailList);
    void MetaDataToResponse(const std::vector<MetaDataDto> &metaDataVec, const std::string &fileId,
                            std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData);
    std::vector<std::string> GetCounterDataType(const std::string &args);
    bool DealLastData(std::vector<Protocol::SimpleSlice> &simpleSliceVec,
                      std::map<std::string, uint64_t> &selfTimeKeyValue, uint64_t startTime,
                      uint64_t endTime, uint64_t index);
    void SetKernelDetail(std::unique_ptr<SqliteResultSet> resultSet, uint64_t minTimestamp,
                         Protocol::KernelDetailsBody &responseBody) const;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_TRACE_DATABASE_H
