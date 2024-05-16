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

namespace Dic::Module::Timeline {
/*
 * timeline数据库抽象类，定义所有查询接口调用的数据库查询纯虚函数，
 */
class VirtualTraceDatabase : public Database {
public:
    explicit VirtualTraceDatabase(std::recursive_mutex &sqlMutex) : Database(sqlMutex) {};
    ~VirtualTraceDatabase() override = default;

    // search
    virtual bool QueryThreadTraces(const Protocol::UnitThreadTracesParams &requestParams,
                           Protocol::UnitThreadTracesBody &responseBody, uint64_t minTimestamp, int64_t traceId) = 0;
    virtual bool QueryThreads(const Protocol::UnitThreadsParams &requestParams, Protocol::UnitThreadsBody &responseBody,
                              uint64_t minTimestamp, int64_t traceId) = 0;
    virtual bool QueryThreadDetail(const Protocol::ThreadDetailParams &requestParams,
                           Protocol::UnitThreadDetailBody &responseBody, uint64_t minTimestamp, int64_t trackId) = 0;
    virtual bool QueryThreadTracesSummary(const Protocol::UnitThreadTracesSummaryParams &requestParams,
                                  Protocol::UnitThreadTracesSummaryBody &responseBody, uint64_t minTimestamp) = 0;
    virtual bool QueryFlowDetail(const Protocol::UnitFlowParams &requestParams, Protocol::UnitFlowBody &responseBody,
                         uint64_t minTimestamp) = 0;
    virtual bool QueryUnitsMetadata(const std::string &fileId,
        std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData) = 0;
    virtual bool QueryExtremumTimestamp(uint64_t &min, uint64_t &max) = 0;
    virtual void QueryFlowName(const Protocol::UnitFlowNameParams &requestParams,
        Protocol::UnitFlowNameBody &responseBody, uint64_t minTimestamp, uint64_t trackId) = 0;
    virtual bool QueryUintFlows(const Protocol::UnitFlowsParams &requestParams,
                               Protocol::UnitFlowsBody &responseBody, uint64_t minTimestamp, uint64_t trackId) = 0;
    virtual int SearchSliceNameCount(const Protocol::SearchCountParams &params) = 0;
    virtual bool SearchSliceName(const Protocol::SearchSliceParams &params, int index, uint64_t minTimestamp,
                         Protocol::SearchSliceBody &responseBody) = 0;
    virtual bool QueryFlowCategoryList(std::vector<std::string> &categories) = 0;
    virtual bool QueryFlowCategoryEvents(Protocol::FlowCategoryEventsParams &params, uint64_t minTimestamp,
                                 std::vector<std::unique_ptr<Protocol::FlowEvent>> &flowDetailList) = 0;
    virtual bool QueryUnitCounter(Protocol::UnitCounterParams &params, uint64_t minTimestamp,
                          std::vector<Protocol::UnitCounterData> &dataList) = 0;

    virtual bool QueryComputeStatisticsData(const Protocol::SummaryStatisticParams &requestParams,
                                    Protocol::SummaryStatisticsBody &responseBody) = 0;
    virtual bool QueryCommunicationStatisticsData(const Protocol::SummaryStatisticParams &requestParams,
                                          Protocol::SummaryStatisticsBody &responseBody) = 0;
    virtual bool QueryStepDuration(const std::string& stepId, uint64_t &min, uint64_t &max) = 0;
    virtual bool QuerySystemViewData(const Protocol::SystemViewParams &requestParams,
        Protocol::SystemViewBody &responseBody) = 0;
    virtual LayerStatData QueryLayerData(const std::string &layer, const std::string &name) = 0;
    virtual std::vector<std::string> QueryCoreType() = 0;
    virtual bool QueryKernelDetailData(const Protocol::KernelDetailsParams &requestParams,
                               Protocol::KernelDetailsBody &responseBody, uint64_t minTimestamp) = 0;
    virtual uint64_t QueryTotalKernel(const std::string &coreType, const std::string &name) = 0;
    virtual bool QueryKernelDepthAndThread(const Protocol::KernelParams &params,
                                   Protocol::OneKernelBody &responseBody, uint64_t minTimestamp) = 0;
    virtual OneKernelData QueryKernelTid(uint64_t trackId) = 0;
    virtual bool SearchAllSlicesDetails(const Protocol::SearchAllSliceParams &params,
                                        Protocol::SearchAllSlicesBody &body, uint64_t minTimestamp) = 0;
    virtual bool QueryAffinityOptimizer(const std::string &optimizers, std::vector<Protocol::ThreadTraces> &data,
                                        uint64_t minTimestamp) = 0;
    virtual bool QueryAICpuOpDurationExceedThreshold(const Protocol::KernelDetailsParams &params, uint64_t threshold,
        std::vector<Protocol::KernelBaseInfo> &data, uint64_t minTimestamp) = 0;
};
}
#endif // PROFILER_SERVER_TRACE_DATABASE_H
