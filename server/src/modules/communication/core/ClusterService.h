/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_CLUSTERSERVICE_H
#define PROFILER_SERVER_CLUSTERSERVICE_H

#include "ClusterDomainObject.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"

namespace Dic {
namespace Module {
namespace Communication {
class ClusterService {
public:
    static void QueryIterations(const Protocol::IterationsRequest &request,
                                Protocol::IterationsOrRanksResponse &response);
    static void QueryGroupInfo(const Protocol::MatrixGroupRequest &request, Protocol::MatrixGroupResponse &response);
    static void QueryMatrixInfo(Protocol::MatrixBandwidthParam &params, Protocol::MatrixListResponseBody &body);
    static void QueryOperatorList(Protocol::DurationListParams &params, Protocol::OperatorListsResponseBody &body);
    static void QueryDurationList(Protocol::DurationListParams &params, Protocol::DurationListsResponseBody &body);
private:
    static std::vector<Protocol::GroupInfo> MergeGroupInfo(std::vector<std::string> &compareGroupList,
                                                           std::vector<std::string> &baselineGroupList);
    static void MergeMatrixInfo(Protocol::MatrixListResponseBody &body, const std::vector<MatrixInfoDo> &compare,
                                const std::vector<MatrixInfoDo> &baseline);
    static void MergeOperatorList(Protocol::OperatorListsResponseBody &body, const std::vector<OperatorTimeDo> &compare,
                                  const std::vector<OperatorTimeDo> &baseline, const std::string &operatorName);
    static void MergeDurationData(Protocol::DurationListsResponseBody &body, std::vector<DurationDo> &compare,
                                  std::vector<DurationDo> &baseline, const std::string &clusterPath);
    static void StatisticBandwidthData(const DurationDo &item, std::vector<Protocol::BandwidthStatistic> &bwStat);
    static void GetBandwidthStatisticResult(std::vector<Protocol::BandwidthStatistic> &bwStat,
                                            Protocol::DurationListsResponseBody &responseBody);
    static void CalBandwidthData(Protocol::DurationListsResponseBody &body,
                                 const std::vector<DurationDo> &durationDoList);
    const static inline std::string underline = "_";
    // 矩阵端点数量，从起始卡到目标卡，固定为2
    const static inline int matrixPointNumber = 2;
};
}
}
}
#endif // PROFILER_SERVER_CLUSTERSERVICE_H
