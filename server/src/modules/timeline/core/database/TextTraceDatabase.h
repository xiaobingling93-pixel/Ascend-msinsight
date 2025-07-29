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
    bool DeleteEmptyThread();
    bool DeleteEmptyFlow();
    bool DropTable();
    bool InitStmt();
    void ReleaseStmt();
    bool InsertSlice(const Trace::Slice &event);
    bool AddSimulationThreadCache(const Trace::ThreadEvent &event);
    bool AddSimulationProcessCache(const Trace::ProcessEvent &event);
    bool InsertSimulationThreadList();
    bool InsertSimulationProcessList();
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
    std::vector<uint64_t> QueryAllTrackIdsByPid(std::string pid);
    std::vector<std::pair<std::string, std::string>> QueryTableDataNameList();

    // search
    bool QueryThreadTracesSummary(const Protocol::UnitThreadTracesSummaryParams &requestParams,
        Protocol::UnitThreadTracesSummaryBody &responseBody, uint64_t minTimestamp) override;

    bool QueryThreads(const Protocol::UnitThreadsParams &requestParams, Protocol::UnitThreadsBody &responseBody,
                      uint64_t minTimestamp, const std::vector<uint64_t> &trackIdList) override;
    bool QueryUnitsMetadata(const std::string &fileId,
        std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData) override;
    bool QueryExtremumTimestamp(uint64_t &min, uint64_t &max) override;
    bool QueryUnitFlows(const Protocol::UnitFlowsParams &requestParams, Protocol::UnitFlowsBody &responseBody,
        uint64_t minTimestamp, uint64_t trackId) override;
    bool SetCardAlias(const Protocol::SetCardAliasParams &requestParams,
                              Protocol::SetCardAliasBody &responseBody) override;
    std::string QueryCardAlias() override;
    uint32_t SearchSliceNameCount(const Protocol::SearchCountParams &params,
        const std::vector<TrackQuery> &trackQuery) override;
    bool SearchSliceName(const Protocol::SearchSliceParams &params, int index, uint64_t minTimestamp,
        Protocol::SearchSliceBody &responseBody, const std::vector<TrackQuery> &trackQuery) override;
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
    bool QueryExpAnaAICoreFreqData(const Protocol::SystemViewAICoreFreqParams &requestParams,
        Protocol::ExpAnaAICoreFreqBody &responseBody,
        std::vector<std::pair<uint64_t, uint64_t>> &freqs, uint64_t &maxFreq, uint64_t &minFreq) override;
    LayerStatData QueryLayerData(const Protocol::SystemViewParams &requestParams, const std::string &name) override;
    std::vector<std::string> QueryCoreType() override;

    bool QueryKernelDetailData(const Protocol::KernelDetailsParams &requestParams,
        Protocol::KernelDetailsBody &responseBody, uint64_t minTimestamp) override;
    uint64_t QueryTotalKernel(const Protocol::KernelDetailsParams &requestParams) override;

    bool QueryKernelDepthAndThread(const Protocol::KernelParams &params, Protocol::OneKernelBody &responseBody,
        uint64_t minTimestamp) override;
    bool QueryCommunicationKernelInfo(const std::string &name, const std::string &rankId,
                                      Protocol::CommunicationKernelBody &body) override;
    OneKernelData QueryKernelTid(const uint64_t trackId) override;

    bool SearchAllSlicesDetails(const Protocol::SearchAllSliceParams &params, Protocol::SearchAllSlicesBody &body,
        uint64_t minTimestamp, const std::vector<TrackQuery> &trackQueryVec) override;

    bool QueryThreadSameOperatorsDetails(const Protocol::UnitThreadsOperatorsParams &requestParams,
                                         Protocol::UnitThreadsOperatorsBody &responseBody, uint64_t minTimestamp,
                                         const std::vector<uint64_t> &trackIdList) override;

    std::map<uint64_t, std::pair<std::string, std::string>> QueryAllThreadMap();
    uint64_t SameOperatorsCount(const std::string &name, const std::vector<uint64_t> &trackIdList,
                                uint64_t &startTime, uint64_t &endTime);
    void ExecuteQueryThreadSameOperatorsDetails(const std::unique_ptr<SqliteResultSet>& resultSet,
        uint64_t minTimestamp, const Protocol::UnitThreadsOperatorsParams &requestParams,
        Protocol::UnitThreadsOperatorsBody &responseBody);
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
    bool QueryOperatorDispatchData(const Protocol::KernelDetailsParams &params,
        std::vector<Protocol::KernelBaseInfo> &data, uint64_t minTimestamp,
        uint64_t threshold, const std::string filePath) override;
    bool UpdateParseStatus(const std::string &status);
    bool HasFinishedParseLastTime(const std::string &statuInfo);

    void SimulationUpdateProcessSortIndex();

    bool QueryEventsViewData(const Protocol::EventsViewParams &params, Protocol::EventsViewBody &body,
        uint64_t minTimestamp) override;
    std::string QueryHostInfo() override;
    bool QueryFwdBwdDataByFlow(const std::string &rankId, uint64_t offset, const Protocol::ExtremumTimestamp &range,
        std::vector<Protocol::ThreadTraces> &fwdBwdData) override;
    bool QueryP2PCommunicationOpData(const std::string &rankId, uint64_t offset,
        const Protocol::ExtremumTimestamp &range, std::vector<Protocol::ThreadTraces> &p2pOpData) override;
    bool QueryByteAlignmentAnalyzerData(std::vector<CommunicationLargeOperatorInfo> &data) override;
    bool QueryByteAlignmentAnalyzerRawData(std::vector<std::pair<std::string, std::string>> &rawData);

private:
    const std::string sliceTable = "slice";
    const std::string threadTable = "thread";
    const std::string processTable = "process";
    const std::string flowTable = "flow";
    const std::string counterTable = "counter";
    const std::string timelineParseStatus = "Timeline files parsing status";
    const uint32_t tolerance = 500; // 匹配算子时的范围为±500
    const std::string cardAliasName = "RANK_LABEL";
    bool initStmt = false;
    std::unique_ptr<SqlitePreparedStatement> insertSliceStmt = nullptr;
    std::unique_ptr<SqlitePreparedStatement> updateProcessNameStmt = nullptr;
    std::unique_ptr<SqlitePreparedStatement> updateProcessLabelStmt = nullptr;
    std::unique_ptr<SqlitePreparedStatement> updateProcessSortIndexStmt = nullptr;
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
    std::set<Trace::ThreadEvent> simulationThreadInfoCache;
    std::set<Trace::ProcessEvent> simulationProcessInfoCache;
    std::unique_ptr<SliceAnalyzer> sliceAnalyzerPtr = nullptr;
    std::unique_ptr<FlowAnalyzer> flowAnalyzerPtr = nullptr;

    bool SetConfig() override;
    bool InitSliceFlowCounterStmt();
    bool InitProcessThreadStmt();
    std::unique_ptr<SqlitePreparedStatement> GetSliceStmt(uint64_t paramLen);
    std::unique_ptr<SqlitePreparedStatement> GetFlowStmt(uint64_t paramLen);
    std::unique_ptr<SqlitePreparedStatement> GetCounterStmt(uint64_t paramLen);
    std::vector<std::string> GetCounterDataType(const std::string &args);
    std::vector<Protocol::SimpleSlice> QuerySimpleSliceByFlagAndTrackId(const std::string &flagId, uint64_t trackId);

    std::vector<FlowDetailDto> QuerySingleFlowDetail(const std::string &flowId);

    bool QuerySliceDtoById(const std::string &sliceId, SliceDto &sliceDto);

    void QuerySimulationUintFlows(const Protocol::UnitFlowsParams &requestParams, Protocol::UnitFlowsBody &responseBody,
        uint64_t minTimestamp);
    static void AssembleUnitFlowsBody(Protocol::UnitFlowsBody &responseBody, uint64_t minTimestamp,
             std::unordered_map<std::string, std::vector<FlowPoint>> &flowPointMap) ;
    static std::string ExtractGroupNameValue(const std::string &str);

    std::vector<Process> QueryAllProcess();

    std::map<std::string, std::vector<Thread>> QueryAllThreadInfo();

    std::map<std::pair<std::string, std::string>, std::string> QueryAllCounterInfo();

    void AddThreadTrack(const std::string &fileId, std::map<std::pair<std::string, std::string>, std::string> &counters,
        std::unique_ptr<Protocol::UnitTrack> &process, const Thread &tThread);

    bool SearchSliceNameWithOutLock(const Protocol::SearchSliceParams &params, int index, uint64_t minTimestamp,
                                    Protocol::SearchSliceBody &responseBody);

    uint32_t SearchSliceNameCount(const Protocol::SearchCountParams &params);

    bool SearchAllSlicesDetails(const Protocol::SearchAllSliceParams &params, Protocol::SearchAllSlicesBody &body,
                                uint64_t minTimestamp);

    std::string GetSearchAllSliceWithLockRangeSql(const Protocol::SearchAllSliceParams &params,
        const std::vector<TrackQuery> &trackQueryVec) const;

    void GetSearchAllSliceData(const Protocol::SearchAllSliceParams &params, Protocol::SearchAllSlicesBody &body,
        uint64_t minTimestamp, std::unique_ptr<SqliteResultSet> &resultSet) const;
};
} // end of namespace Timeline
// end of namespace Module
// end of namespace Dic

#endif // PROFILER_SERVER_JSON_TRACE_DATABASE_H
