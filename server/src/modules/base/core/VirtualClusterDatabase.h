/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_CLUSTER_DATABASE_H
#define PROFILER_SERVER_CLUSTER_DATABASE_H

#include <set>
#include "Database.h"
#include "ClusterDef.h"
#include "Protocol.h"
#include "SummaryProtocolResponse.h"
#include "SummaryProtocolRequest.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"

namespace Dic {
namespace Module {
class VirtualClusterDatabase : public Database {
public:
    VirtualClusterDatabase() = default;
    ~VirtualClusterDatabase() override = default;

    virtual std::string QueryParseClusterStatus() = 0;
    virtual void UpdateClusterParseStatus(std::string status) = 0;

    virtual bool QuerySummaryData(const Protocol::SummaryTopRankParams &requestParams,
                          Protocol::SummaryTopRankResBody &responseBody) = 0;
    virtual bool QueryBaseInfo(Protocol::SummaryTopRankResBody &responseBody) = 0;
    virtual bool GetStepIdList(Protocol::PipelineStepResponseBody &responseBody) = 0;
    virtual bool GetStages(Protocol::PipelineStageParam param, Protocol::PipelineStageResponseBody &responseBody) = 0;
    virtual bool GetStageAndBubble(Protocol::PipelineStageTimeParam param,
                           Protocol::PipelineStageOrRankTimeResponseBody &responseBody) = 0;
    virtual bool GetRankAndBubble(Protocol::PipelineRankTimeParam param,
                          Protocol::PipelineStageOrRankTimeResponseBody &responseBody) = 0;
    virtual bool GetGroups(Protocol::MatrixGroupParam param, Protocol::MatrixGroupResponseBody &responseBody) = 0;
    virtual bool QueryMatrixList(Protocol::MatrixBandwidthParam param,
        Protocol::MatrixListResponseBody &responseBody) = 0;
    virtual bool QueryAllOperators(Protocol::OperatorDetailsParam &param,
        Protocol::OperatorDetailsResBody &resBody) = 0;
    virtual bool QueryOperatorsCount(Protocol::OperatorDetailsParam &param,
        Protocol::OperatorDetailsResBody &resBody) = 0;
    virtual bool QueryBandwidthData(Protocol::BandwidthDataParam &param, Protocol::BandwidthDataResBody &resBody) = 0;
    virtual bool QueryDistributionData(Protocol::DistributionDataParam &param,
        Protocol::DistributionResBody &resBody) = 0;

    virtual bool QueryRanksHandler(std::vector<Protocol::IterationsOrRanksObject> &responseBody) = 0;
    virtual bool QueryOperatorNames(Protocol::OperatorNamesParams &requestParams,
                            std::vector<Protocol::OperatorNamesObject> &responseBody) = 0;
    virtual bool QueryIterations(std::vector<Protocol::IterationsOrRanksObject> &responseBody) = 0;
    virtual bool QueryDurationList(Protocol::DurationListParams &requestParams,
                           std::vector<Protocol::Duration> &responseBody) = 0;
    virtual bool QueryCommunicationGroup(rapidjson::Document &responseBody) = 0;
};
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_CLUSTER_DATABASE_H