/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_JSON_TRACE_DATABASE_H
#define PROFILER_SERVER_JSON_TRACE_DATABASE_H

#include "VirtualTraceDatabase.h"
#include "CacheManager.h"

namespace Dic {
namespace Module {
namespace Timeline {
/*
 * 解析原始的traceView.json文件以及其他csv文件情况下的数据库处理类
 */
class JsonTraceDatabase : public VirtualTraceDatabase {
public:
    explicit JsonTraceDatabase(std::mutex &sqlMutex);
    ~JsonTraceDatabase() override;

    bool OpenDb(const std::string &dbPath, bool clearAllTable) override;

    bool CreateTable();
    bool CreateIndex();
    bool DropTable();
    bool InitStmt();
    void ReleaseStmt();
    bool InsertSlice(const Trace::Slice &event);
    bool AddThreadCache(const std::tuple<int64_t, std::string, std::string> &threadInfo);
    bool AddSimulationThreadCache(const Trace::ThreadEvent &event);
    bool AddSimulationProcessCache(const Trace::ProcessEvent &event);
    bool InsertThreadList(const std::set<std::tuple<int64_t, std::string, std::string>> &threadList);
    bool InsertSimulationThreadList();
    bool InsertSimulationProcessList();
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
    void UpdateSimulationDepthWithNoOverlap();
    void UpdateSimulationDepthByCodeWithNoOverlap();
    void UpdateAllSimulationSliceDepthWithNoOverlap(std::vector<Protocol::SimpleSlice> &rowThreadTraceVec,
        const uint64_t trackId);
    std::pair<int64_t, int64_t> QueryExtremTrackIdPairByPid(std::string pid);

    // search
    bool QueryThreadTraces(const Protocol::UnitThreadTracesParams &requestParams,
        Protocol::UnitThreadTracesBody &responseBody, uint64_t minTimestamp, int64_t traceId) override;
    bool QueryThreadTracesSummary(const Protocol::UnitThreadTracesSummaryParams &requestParams,
        Protocol::UnitThreadTracesSummaryBody &responseBody, uint64_t minTimestamp) override;

    bool QueryThreads(const Protocol::UnitThreadsParams &requestParams, Protocol::UnitThreadsBody &responseBody,
        uint64_t minTimestamp, int64_t traceId) override;
    bool QueryThreadDetail(const Protocol::ThreadDetailParams &requestParams,
        Protocol::UnitThreadDetailBody &responseBody, uint64_t minTimestamp, int64_t trackId) override;

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
    bool QueryStepDuration(const std::string &stepId, uint64_t &min, uint64_t &max) override;
    bool QueryPythonViewData(const Protocol::SystemViewParams &requestParams,
        Protocol::SystemViewBody &responseBody) override;
    LayerStatData QueryLayerData(const std::string &layer, const std::string &name) override;
    std::vector<std::string> QueryCoreType() override;

    bool QueryKernelDetailData(const Protocol::KernelDetailsParams &requestParams,
        Protocol::KernelDetailsBody &responseBody, uint64_t minTimestamp) override;
    uint64_t QueryTotalKernel(const std::string &coreType, const std::string &name) override;

    bool QueryKernelDepthAndThread(const Protocol::KernelParams &params, Protocol::OneKernelBody &responseBody,
        uint64_t minTimestamp) override;
    OneKernelData QueryKernelTid(const uint64_t trackId) override;

    KernelShapesDataDto QueryKernelShapes(const std::vector<SliceDto> &rows);

    bool QueryThreadSameOperatorsDetails(const Protocol::UnitThreadsOperatorsParams &requestParams,
        Protocol::UnitThreadsOperatorsBody &responseBody, uint64_t minTimestamp, int64_t traceId);

    std::map<uint64_t, std::pair<std::string, std::string>> QueryAllThreadMap();

    uint64_t SameOperatorsCount(const std::string &name, int64_t &trackId, uint64_t &startTime, uint64_t &endTime);

    bool UpdateParseStatus(const std::string& status);
    bool HasFinishedParseLastTime();

private:
    const std::string sliceTable = "slice";
    const std::string threadTable = "thread";
    const std::string processTable = "process";
    const std::string flowTable = "flow";
    const std::string counterTable = "counter";
    const std::string timelineParseStatus = "Timeline files parsing status";
    const std::string trackIdTimeIndex = "track_id_timestamp_end_time_index";
    const std::string simpleSliceIndex = "track_id_depth_timestamp_end_time_index";
    const std::string flowIndex = "flow_id_time_index";
    const std::string kernelDetail = "kernel_detail";
    const std::string hcclType = "HCCL";
    const int cacheSize = 1000;
    const int unit = 1000;
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
    std::unique_ptr<SqlitePreparedStatement> updateSliceStmt = nullptr;

    std::vector<Trace::Slice> sliceCache;
    std::list<Protocol::SimpleSlice> sliceDepthHelper;
    std::vector<Trace::Flow> flowCache;
    std::vector<Trace::Counter> counterCache;
    std::set<std::tuple<int64_t, std::string, std::string>> threadInfoCache;
    std::set<Trace::ThreadEvent> simulationThreadInfoCache;
    std::set<Trace::ProcessEvent> simulationProcessInfoCache;

    bool SetConfig();
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
    bool QueryDurationFromSliceByTimeRange(const Protocol::ThreadDetailParams &requestParams,
        const std::vector<SliceDto> &rows, std::vector<std::pair<uint64_t, uint64_t>> &nextDepthResult,
        int64_t trackId);
    bool FlowDetailToResponse(const std::vector<FlowDetailDto> &flowDetailVec, uint64_t minTimestamp,
        Protocol::UnitFlowBody &responseBody);
    void FlowEventsToResponse(const std::vector<FlowCategoryEventsDto> &flowEventsVec, const std::string &category,
        std::vector<std::unique_ptr<Protocol::FlowEvent>> &flowDetailList);
    void MetaDataToResponse(const std::vector<MetaDataDto> &metaDataVec, const std::string &fileId,
        std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData);
    std::vector<std::string> GetCounterDataType(const std::string &args);
    void SetKernelDetail(std::unique_ptr<SqliteResultSet> resultSet, uint64_t minTimestamp,
        Protocol::KernelDetailsBody &responseBody) const;

    bool UpdateSimulationSliceDepth(std::list<Protocol::SimpleSlice> &sliceLinkedList);

    std::vector<int32_t> QueryAllTrackId();

    std::vector<Protocol::SimpleSlice> QueryAllSliceByTrackId(const int32_t &trackId);

    std::vector<Protocol::RowThreadTrace> QuerySliceByCondition(const Protocol::UnitThreadTracesParams &requestParams,
        uint64_t minTimestamp, int64_t traceId);

    uint64_t ComputeSingleSliceSelfTime(const Protocol::ThreadDetailParams &requestParams, int64_t trackId,
        std::vector<SliceDto> &sliceDtoVec);

    void QueryAllSliceInRangeByTrackId(int64_t &traceId, std::vector<CacheSlice> &cacheSlices);

    static std::set<int64_t> ComputeResultIds(const Protocol::UnitThreadTracesParams &requestParams,
        uint64_t minTimestamp, std::vector<CacheSlice> &cacheSlices);

    std::vector<Protocol::RowThreadTrace> QuerySliceByIdList(uint64_t minTimestamp, int64_t traceId,
        std::set<int64_t> &ids);

    std::vector<Protocol::SimpleSlice> QuerySimpleSliceByTimeRange(uint64_t startTime, uint64_t endTime,
        uint64_t minTimestamp, int64_t trackId);

    std::vector<Protocol::SimpleSlice> QuerySimpleSliceByTimePoint(uint64_t startTime, uint64_t minTimestamp,
        int64_t trackId);

    void ComputeSimulationSliceDepth(std::vector<Protocol::SimpleSlice> &rowThreadTraceVec);
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_JSON_TRACE_DATABASE_H
