/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_TRACE_DATABASE_H
#define PROFILER_SERVER_TRACE_DATABASE_H

#include <memory>
#include <string>
#include <vector>
#include <list>
#include <set>
#include "Database.h"
#include "TimelineProtocolRequest.h"
#include "TimelineProtocolResponse.h"
#include "TimelineProtocolEvent.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "TraceDatabaseDef.h"
#include "EventDef.h"
#include "SystemViewOverallHelper.h"
#include "DominQuery.h"
#include "ClusterDef.h"

namespace Dic::Module::Timeline {
const uint64_t AICPU_OP_DURATION_THRESHOLD = 20000; // 20us

struct ParamsForCalCSData {
    const std::string &sql;
    SystemViewOverallHelper &overallHelper;
};
struct ParamsForOAData {
    const std::string &sql;
    const std::string &type;
    uint64_t offset;
};
/*
 * timeline数据库抽象类，定义所有查询接口调用的数据库查询纯虚函数，
 */
class VirtualTraceDatabase : public Database {
public:
    explicit VirtualTraceDatabase(std::recursive_mutex &sqlMutex) : Database(sqlMutex) {};
    ~VirtualTraceDatabase() override = default;

    virtual std::string GetDeviceId(const std::string& fileId) = 0;
    // search
    virtual bool QueryThreads(const Protocol::UnitThreadsParams &requestParams, Protocol::UnitThreadsBody &responseBody,
                              uint64_t minTimestamp, const std::vector<uint64_t> &trackIdList) = 0;
    virtual bool QueryThreadTracesSummary(const Protocol::UnitThreadTracesSummaryParams &requestParams,
                                  Protocol::UnitThreadTracesSummaryBody &responseBody, uint64_t minTimestamp) = 0;
    virtual bool QueryUnitsMetadata(const std::string &fileId,
        std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData) = 0;
    virtual bool QueryExtremumTimestamp(uint64_t &min, uint64_t &max) = 0;
    virtual bool QueryUintFlows(const Protocol::UnitFlowsParams &requestParams,
                               Protocol::UnitFlowsBody &responseBody, uint64_t minTimestamp, uint64_t trackId) = 0;
    virtual bool SetCardAlias(const Protocol::SetCardAliasParams &requestParams,
                                Protocol::SetCardAliasBody &responseBody) = 0;
    virtual std::string QueryCardAlias() = 0;
    virtual uint32_t SearchSliceNameCount(const Protocol::SearchCountParams &params,
        const std::vector<TrackQuery> &trackQuery) = 0;
    virtual bool SearchSliceName(const Protocol::SearchSliceParams &params, int index, uint64_t minTimestamp,
                         Protocol::SearchSliceBody &responseBody, const std::vector<TrackQuery> &trackQuery) = 0;
    virtual bool QueryFlowCategoryList(std::vector<std::string> &categories, const std::string& rankId) = 0;
    virtual bool QueryUnitCounter(Protocol::UnitCounterParams &params, uint64_t minTimestamp,
                          std::vector<Protocol::UnitCounterData> &dataList) = 0;

    virtual bool QueryComputeStatisticsData(const Protocol::SummaryStatisticParams &requestParams,
                                    Protocol::SummaryStatisticsBody &responseBody) = 0;
    virtual bool QueryCommunicationStatisticsData(const Protocol::SummaryStatisticParams &requestParams,
                                          Protocol::SummaryStatisticsBody &responseBody) = 0;
    virtual bool QueryStepDuration(const std::string& stepId, uint64_t &min, uint64_t &max) = 0;
    virtual bool QuerySystemViewData(const Protocol::SystemViewParams &requestParams,
        Protocol::SystemViewBody &responseBody) = 0;
    virtual bool QueryExpAnaAICoreFreqData(const Protocol::SystemViewAICoreFreqParams &requestParams,
        std::vector<std::pair<uint64_t, uint64_t>> &freqs, uint64_t &maxFreq, uint64_t &minFreq) = 0;
    virtual LayerStatData QueryLayerData(const Protocol::SystemViewParams &requestParams, const std::string &name) = 0;
    virtual std::vector<std::string> QueryCoreType() = 0;
    virtual bool QueryKernelDetailData(const Protocol::KernelDetailsParams &requestParams,
                               Protocol::KernelDetailsBody &responseBody, uint64_t minTimestamp) = 0;
    virtual uint64_t QueryTotalKernel(const Protocol::KernelDetailsParams &requestParams) = 0;
    virtual bool QueryKernelDepthAndThread(const Protocol::KernelParams &params,
                                   Protocol::OneKernelBody &responseBody, uint64_t minTimestamp) = 0;
    virtual bool QueryCommunicationKernelInfo(const std::string &name, const std::string &rankId,
                                              Protocol::CommunicationKernelBody &body) = 0;
    virtual OneKernelData QueryKernelTid(uint64_t trackId) = 0;
    virtual bool SearchAllSlicesDetails(const Protocol::SearchAllSliceParams &params,
        Protocol::SearchAllSlicesBody &body, uint64_t minTimestamp, const std::vector<TrackQuery> &trackQueryVec) = 0;
    virtual bool QueryAffinityOptimizer(const Protocol::KernelDetailsParams &params, const std::string &optimizers,
        std::vector<Protocol::ThreadTraces> &data, uint64_t minTimestamp) = 0;
    virtual bool QueryThreadSameOperatorsDetails(const Protocol::UnitThreadsOperatorsParams &requestParams,
        Protocol::UnitThreadsOperatorsBody &responseBody,
        uint64_t minTimestamp, const std::vector<std::string> &trackIdList) = 0;
    virtual bool QueryAICpuOpCanBeOptimized(const Protocol::KernelDetailsParams &params,
        const std::vector<std::string> &replace, const std::map<std::string, Timeline::AICpuCheckDataType> &dataType,
        std::vector<Protocol::KernelBaseInfo> &data, uint64_t minTimestamp) = 0;
    virtual bool QueryAclnnOpCountExceedThreshold(const Protocol::KernelDetailsParams &params, uint64_t threshold,
        std::vector<Protocol::KernelBaseInfo> &data, uint64_t minTimestamp) = 0;
    virtual bool QueryAffinityAPIData(const Protocol::KernelDetailsParams &params,
        const std::set<std::string> &pattern, uint64_t minTimestamp,
        std::map<uint64_t, std::vector<Protocol::FlowLocation>> &data,
        std::map<uint64_t, std::vector<uint32_t>> &indexs) = 0;
    virtual bool QueryFuseableOpData(const Protocol::KernelDetailsParams &params, const Timeline::FuseableOpRule &rule,
        std::vector<Protocol::FlowLocation> &data, uint64_t minTimestamp) = 0;
    virtual bool QueryOperatorDispatchData(const Protocol::KernelDetailsParams &params,
        std::vector<Protocol::KernelBaseInfo> &data, uint64_t minTimestamp,
        uint64_t threshold, const std::string filePath) = 0;
    virtual bool QueryEventsViewData(const Protocol::EventsViewParams &params, Protocol::EventsViewBody &body,
        uint64_t minTimestamp) = 0;
    virtual std::string QueryHostInfo() = 0;
    virtual bool QueryFwdBwdDataByFlow(const std::string &rankId, uint64_t offset,
        const Protocol::ExtremumTimestamp &range, std::vector<Protocol::ThreadTraces> &fwdBwdData) = 0;
    virtual bool QueryP2PCommunicationOpData(const std::string &rankId, uint64_t offset,
        const Protocol::ExtremumTimestamp &range, std::vector<Protocol::ThreadTraces> &p2pOpData) = 0;
    virtual bool QueryByteAlignmentAnalyzerData(std::vector<CommunicationLargeOperatorInfo> &data) = 0;
// LCOV_EXCL_BR_START
    // 调用前需保证uncovered、sql等不为空
    template<class T>
    bool CalculateCommunicationSummaryData(const std::vector<Protocol::ThreadTraces> &uncovered,
        const std::map<std::string, std::string> &groupInfoMap, ParamsForCalCSData paramsForCalCsData,
        T &deviceId, Protocol::SystemViewOverallRes &result)
    {
        auto stmt = CreatPreparedStatement(paramsForCalCsData.sql);
        if (stmt == nullptr) {
            Server::ServerLog::Error("Failed to prepare sql for query communication detail info.");
            return false;
        }
        stmt->BindParams(deviceId);
        auto resultSet = stmt->ExecuteQuery();
        if (resultSet == nullptr) {
            Server::ServerLog::Error("Failed to get result set for query communication detail info.",
                                     stmt->GetErrorMessage());
            return false;
        }
        std::map<std::string, Protocol::CommunicationSummaryInfoByGroup> summaryInfoMap{};
        ExecuteQueryCommunicationSummaryData(summaryInfoMap, resultSet, groupInfoMap, uncovered);

        // 最终数据整理，按Group整理出Wait和Transmit时间
        ComputeCommunicationWaitAndTransmitTimeByGroup(summaryInfoMap, paramsForCalCsData.overallHelper, result);

        return true;
    };

    // 查询所有通信未掩盖的时间段，用于后续计算落在通信未掩盖区间的等待时间
    template<class T>
    bool QueryOverlapAnalysisData(ParamsForOAData paramsForOaData, T &deviceId,
        std::vector<Protocol::ThreadTraces> &overlapData, uint64_t &totalTime)
    {
        if (paramsForOaData.sql.empty() || paramsForOaData.type.empty()) {
            Server::ServerLog::Error("Failed to get overlap analysis data due to empty sqlite cmd.");
            return false;
        }
        auto stmt = CreatPreparedStatement(paramsForOaData.sql);
        if (stmt == nullptr) {
            Server::ServerLog::Error("Failed to prepare sql for query overlap analysis data.");
            return false;
        }
        auto resultSet = stmt->ExecuteQuery(paramsForOaData.offset, paramsForOaData.offset,
                                            deviceId, paramsForOaData.type);
        if (resultSet == nullptr) {
            Server::ServerLog::Error("Failed to get result set for query overlap analysis data.",
                                     stmt->GetErrorMessage());
            return false;
        }
        while (resultSet->Next()) {
            Protocol::ThreadTraces ele{};
            ele.name = resultSet->GetString("name"); // at the moment, no used
            ele.startTime = resultSet->GetUint64("startNs");
            ele.endTime = resultSet->GetUint64("endNs");
            ele.duration = resultSet->GetUint64("duration");
            if (totalTime > UINT64_MAX - ele.duration) {
                totalTime = 0;
            } else {
                totalTime += ele.duration;
            }
            overlapData.push_back(ele);
        }
        if (overlapData.empty()) {
            Server::ServerLog::Error("Failed to get overlap analysis data due to no data.");
            return false;
        }
        return true;
    };

    template<class T>
    bool QueryCommunicationGroupMap(const std::string &sql, T &deviceId,
        std::map<std::string, std::string> &groupMap)
    {
        if (sql.empty()) {
            Server::ServerLog::Error("Failed to get communication group data due to empty sql.");
            return false;
        }
        auto stmt = CreatPreparedStatement(sql);
        if (stmt == nullptr) {
            Server::ServerLog::Error("Failed to prepare sql for query communication group data.");
            return false;
        }
        stmt->BindParams(deviceId);
        auto resultSet = stmt->ExecuteQuery();
        if (resultSet == nullptr) {
            Server::ServerLog::Error("Failed to get result set for query communication group data.",
                                     stmt->GetErrorMessage());
            return false;
        }
        std::string lastGroup;
        // sql中保证按照通信组和plane升序排列
        while (resultSet->Next()) {
            std::string groupName = std::to_string(resultSet->GetUint64("groupName"));
            std::string plane = std::to_string(resultSet->GetInt64("planeId"));
            std::string threadName = resultSet->GetString("threadName");
            if (StringUtil::StartWith(threadName, "Group ") && StringUtil::EndWith(threadName, " Communication")) {
                groupMap.emplace(groupName.append("@").append(plane), threadName);
                lastGroup = threadName;
            } else {
                if (lastGroup.empty()) {
                    continue;
                }
                groupMap.emplace(groupName.append("@").append(plane), lastGroup);
            }
        }
        if (groupMap.empty()) {
            Server::ServerLog::Error("Failed to get communication group data due to no data.");
            return false;
        }
        return true;
    };
// LCOV_EXCL_BR_STOP
    bool hasMacTime = false;
private:
    // 给定一个通信算子或Task，计算其未被通信掩盖部分的耗时
    uint64_t CalculateUncoveredTime(const std::vector<Protocol::ThreadTraces> &uncovered, size_t &index,
        const Protocol::ThreadTraces &element);
    void ExecuteQueryCommunicationSummaryData(
        std::map<std::string, Protocol::CommunicationSummaryInfoByGroup>& summaryInfoMap,
        const std::unique_ptr<SqliteResultSet>& resultSet, const std::map<std::string, std::string> &groupInfoMap,
        const std::vector<Protocol::ThreadTraces> &uncovered);

    void ComputeCommunicationWaitAndTransmitTimeByGroup(
        const std::map<std::string, CommunicationSummaryInfoByGroup> &summaryData,
        SystemViewOverallHelper &overallHelper, Protocol::SystemViewOverallRes &result);
};
}
#endif // PROFILER_SERVER_TRACE_DATABASE_H
