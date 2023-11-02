/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_TRACE_DATABASE_H
#define PROFILER_SERVER_TRACE_DATABASE_H

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
    TraceDatabase() = default;
    ~TraceDatabase() override;

    bool SetConfig();
    bool CreateTable();
    bool CreateIndex();
    bool InitStmt();
    void ReleaseStmt();
    bool InsertSlice(const Trace::Slice &event);
    bool UpdateProcessName(const Trace::MetaData &event);
    bool UpdateProcessLabel(const Trace::MetaData &event);
    bool UpdateProcessSortIndex(const Trace::MetaData &event);
    bool UpdateThreadName(const Trace::MetaData &event);
    bool UpdateThreadSortIndex(const Trace::MetaData &event);
    bool InsertFlow(const Trace::Flow &event);
    bool InsertCounter(const Trace::Counter &event);
    bool InsertSliceList(const std::vector<Trace::Slice> &eventList);
    bool InsertFlowList(const std::vector<Trace::Flow> &eventList);
    bool InsertCounterList(const std::vector<Trace::Counter> &eventList);
    void CommitData();
    void UpdateDepth();
    void DeleteInvalidFlowData();

    // search
    std::vector<int64_t> GetTrackIdList();
    bool QueryThreadTraces(const Protocol::UnitThreadTracesParams &requestParams,
                           Protocol::UnitThreadTracesBody &responseBody, uint64_t minTimestamp, int64_t traceId);
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

private:
    const std::string sliceTable = "slice";
    const std::string threadTable = "thread";
    const std::string processTable = "process";
    const std::string flowTable = "flow";
    const std::string counterTable = "counter";
    const std::string idIndex = "id_index";
    const std::string trackIdTimeIndex = "track_id_time_index";
    const std::string flowIndex = "flow_cat_time_index";

    bool initStmt = false;
    sqlite3_stmt *insertSliceStmt = nullptr;
    sqlite3_stmt *updateProcessNameStmt = nullptr;
    sqlite3_stmt *updateProcessLabelStmt = nullptr;
    sqlite3_stmt *updateProcessSortIndexStmt = nullptr;
    sqlite3_stmt *updateThreadNameStmt = nullptr;
    sqlite3_stmt *updateThreadSortIndexStmt = nullptr;
    sqlite3_stmt *insertFlowStmt = nullptr;
    sqlite3_stmt *insertCounterStmt = nullptr;
    const int cacheSize = 1000;
    std::vector<Trace::Slice> sliceCache;
    std::vector<Trace::Flow> flowCache;
    std::vector<Trace::Counter> counterCache;

    struct SliceTimeData {
        int64_t id;
        uint64_t time;
        uint64_t dur;
    };

    bool InitSliceFlowCounterStmt();
    bool InitProcessThreadStmt();
    sqlite3_stmt *GetSliceStmt(uint64_t paramLen);
    sqlite3_stmt *GetFlowStmt(uint64_t paramLen);
    sqlite3_stmt *GetCounterStmt(uint64_t paramLen);
    void UpdateOneTrackDepth(int64_t trackId);
    bool SearchSliceTimeData(int64_t trackId, std::vector<SliceTimeData> &sliceTimeList);
    // depth, idList
    void CalcDepth(const std::vector<SliceTimeData> &sliceData, std::map<int, std::vector<int64_t>> &depthMap);
    void UpdateDepthByID(const std::vector<int64_t> &idList, int depth);

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
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_TRACE_DATABASE_H
