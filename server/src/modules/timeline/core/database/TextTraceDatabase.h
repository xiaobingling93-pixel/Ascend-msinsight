/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_JSON_TRACE_DATABASE_H
#define PROFILER_SERVER_JSON_TRACE_DATABASE_H

#include "VirtualTraceDatabase.h"
#include "SliceCacheManager.h"
#include "SliceAnalyzer.h"
#include "FlowAnalyzer.h"
#include "TextSqlConstant.h"
#include "TimelineProtocolRequest.h"


namespace Dic::Module::Timeline {
/*
 * 解析原始的traceView.json文件以及其他csv文件情况下的数据库处理类
 */
class TextTraceDatabase : public VirtualTraceDatabase {
public:
    explicit TextTraceDatabase(std::recursive_mutex &sqlMutex);
    ~TextTraceDatabase() override;

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
    std::vector<uint64_t> QueryAllTrackIdsByPid(std::string pid);

    // search
    bool QueryThreadTracesSummary(const Protocol::UnitThreadTracesSummaryParams &requestParams,
        Protocol::UnitThreadTracesSummaryBody &responseBody, uint64_t minTimestamp) override;

    bool QueryThreads(const Protocol::UnitThreadsParams &requestParams, Protocol::UnitThreadsBody &responseBody,
                      uint64_t minTimestamp, const std::vector<uint64_t> &trackIdList) override;
    bool QueryThreadDetail(const Protocol::ThreadDetailParams &requestParams,
        Protocol::UnitThreadDetailBody &responseBody, uint64_t minTimestamp, int64_t trackId) override;
    bool QueryUnitsMetadata(const std::string &fileId,
        std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData) override;
    bool QueryExtremumTimestamp(uint64_t &min, uint64_t &max) override;
    bool QueryUintFlows(const Protocol::UnitFlowsParams &requestParams, Protocol::UnitFlowsBody &responseBody,
        uint64_t minTimestamp, uint64_t trackId) override;
    int SearchSliceNameCount(const Protocol::SearchCountParams &params) override;
    bool SearchSliceName(const Protocol::SearchSliceParams &params, int index, uint64_t minTimestamp,
        Protocol::SearchSliceBody &responseBody) override;
    bool QueryFlowCategoryList(std::vector<std::string> &categories, const std::string &rankId) override;
    bool QueryUnitCounter(Protocol::UnitCounterParams &params, uint64_t minTimestamp,
        std::vector<Protocol::UnitCounterData> &dataList) override;

    bool QueryComputeStatisticsData(const Protocol::SummaryStatisticParams &requestParams,
        Protocol::SummaryStatisticsBody &responseBody) override;
    bool QueryCommunicationStatisticsData(const Protocol::SummaryStatisticParams &requestParams,
        Protocol::SummaryStatisticsBody &responseBody) override;
    bool QueryStepDuration(const std::string &stepId, uint64_t &min, uint64_t &max) override;
    bool QuerySystemViewData(const Protocol::SystemViewParams &requestParams,
        Protocol::SystemViewBody &responseBody) override;
    LayerStatData QueryLayerData(const std::string &layer, const std::string &name) override;
    std::vector<std::string> QueryCoreType() override;

    bool QueryKernelDetailData(const Protocol::KernelDetailsParams &requestParams,
        Protocol::KernelDetailsBody &responseBody, uint64_t minTimestamp) override;
    uint64_t QueryTotalKernel(const Protocol::KernelDetailsParams &requestParams) override;

    bool QueryKernelDepthAndThread(const Protocol::KernelParams &params, Protocol::OneKernelBody &responseBody,
        uint64_t minTimestamp) override;
    OneKernelData QueryKernelTid(const uint64_t trackId) override;

    bool SearchAllSlicesDetails(const Protocol::SearchAllSliceParams &params, Protocol::SearchAllSlicesBody &body,
        uint64_t minTimestamp) override;

    KernelShapesDataDto QueryKernelShapes(const std::vector<SliceDto> &rows);

    bool QueryThreadSameOperatorsDetails(const Protocol::UnitThreadsOperatorsParams &requestParams,
        Protocol::UnitThreadsOperatorsBody &responseBody, uint64_t minTimestamp, int64_t traceId) override;

    std::map<uint64_t, std::pair<std::string, std::string>> QueryAllThreadMap();

    uint64_t SameOperatorsCount(const std::string &name, int64_t &trackId, uint64_t &startTime, uint64_t &endTime);
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
    bool UpdateParseStatus(const std::string &status);
    bool HasFinishedParseLastTime(const std::string &statuInfo);

    void SimulationUpdateProcessSortIndex();

    bool QueryEventsViewData(const Protocol::EventsViewParams &params, Protocol::EventsViewBody &body,
        uint64_t minTimestamp) override;
    std::string QueryHostInfo() override;

private:
    const std::string sliceTable = "slice";
    const std::string threadTable = "thread";
    const std::string processTable = "process";
    const std::string flowTable = "flow";
    const std::string counterTable = "counter";
    const std::string timelineParseStatus = "Timeline files parsing status";
    const std::string hcclType = "HCCL";
    const uint32_t unit = 1000;
    const uint32_t tolerance = 500; // 匹配算子时的范围为±500
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
    std::unique_ptr<SqlitePreparedStatement> simulationInsertThreadNameStmt = nullptr;
    std::unique_ptr<SqlitePreparedStatement> simulationInsertProcessNameStmt = nullptr;

    std::vector<Trace::Slice> sliceCache;
    std::list<Protocol::SimpleSlice> sliceDepthHelper;
    std::vector<Trace::Flow> flowCache;
    std::vector<Trace::Counter> counterCache;
    std::set<std::tuple<int64_t, std::string, std::string>> threadInfoCache;
    std::set<Trace::ThreadEvent> simulationThreadInfoCache;
    std::set<Trace::ProcessEvent> simulationProcessInfoCache;
    std::unique_ptr<SliceAnalyzer> sliceAnalyzerPtr = nullptr;
    std::unique_ptr<FlowAnalyzer> flowAnalyzerPtr = nullptr;

    bool SetConfig();
    bool InitSliceFlowCounterStmt();
    bool InitProcessThreadStmt();
    std::unique_ptr<SqlitePreparedStatement> GetSliceStmt(uint64_t paramLen);
    std::unique_ptr<SqlitePreparedStatement> GetFlowStmt(uint64_t paramLen);
    std::unique_ptr<SqlitePreparedStatement> GetCounterStmt(uint64_t paramLen);
    bool QueryDurationFromSliceByTimeRange(const Protocol::ThreadDetailParams &requestParams,
        const std::vector<SliceDto> &rows, std::vector<std::pair<uint64_t, uint64_t>> &nextDepthResult,
        int64_t trackId);
    void MetaDataToResponse(const std::vector<MetaDataDto> &metaDataVec, const std::string &fileId,
        std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData);
    std::vector<std::string> GetCounterDataType(const std::string &args);

    uint64_t ComputeSingleSliceSelfTime(const Protocol::ThreadDetailParams &requestParams, int64_t trackId,
        std::vector<SliceDto> &sliceDtoVec);
    std::vector<Protocol::SimpleSlice> QuerySimpleSliceByFlagAndTrackId(const std::string &flagId, uint64_t trackId);

    std::vector<FlowDetailDto> QuerySingleFlowDetail(const std::string &flowId);

    bool QuerySliceDtoById(const std::string &sliceId, SliceDto &sliceDto);

    void QuerySimulationUintFlows(const Protocol::UnitFlowsParams &requestParams, Protocol::UnitFlowsBody &responseBody,
        uint64_t minTimestamp);
    static void AssembleUnitFlowsBody(Protocol::UnitFlowsBody &responseBody, uint64_t minTimestamp,
             std::unordered_map<std::string, std::vector<FlowPoint>> &flowPointMap) ;
};
} // end of namespace Timeline
// end of namespace Module
// end of namespace Dic

#endif // PROFILER_SERVER_JSON_TRACE_DATABASE_H
