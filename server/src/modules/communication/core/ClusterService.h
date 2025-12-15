/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_CLUSTERSERVICE_H
#define PROFILER_SERVER_CLUSTERSERVICE_H

#include "ClusterDomainObject.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"
#include "DataBaseManager.h"
#include "CommunicationErrorManage.h"

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
    static bool AnalyzeCommunicationSlowRanks(const Protocol::DurationListParams &params,
        CommunicationSlowRankAnalysisResponseBody &body);
private:
    static std::vector<Protocol::GroupInfo> MergeGroupInfoWithPgName(
        std::map<std::string, GroupInfoDo> &compareGroupMap, std::map<std::string, GroupInfoDo> &baselineGroupMap);
    static std::map<std::string, GroupInfoDo> GetRankSetAndOpTypeToGroupInfoMap(
        const std::vector<OpTypeStatistics> &StatsList, const std::vector<GroupInfoDo> &groupList);
    static std::vector<Protocol::GroupInfo> MergeGroupInfo(const Protocol::MatrixGroupRequest &request,
        std::vector<GroupInfoDo> &compareGroupList, std::vector<GroupInfoDo> &baselineGroupList);
    static std::vector<OpTypeStatistics> GetOpTypeStatByStepId(const std::string &stepId,
                                                               const std::string &clusterPath);
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
    static bool CheckOpNameList(const Protocol::DurationListParams &params,
                                const std::shared_ptr<VirtualClusterDatabase> &database);
    static void FindSlowRankByCommDuration(const std::shared_ptr<VirtualClusterDatabase> &database,
        const Protocol::DurationListParams &params, RankDetailsForSlowRank &fastestRank,
        CommunicationSlowRankAnalysisResponseBody &body);
    static bool IsHavePgName(const std::vector<GroupInfoDo> &groupList);
    const static inline std::string underline = "_";
    // 矩阵端点数量，从起始卡到目标卡，固定为2
    const static inline int matrixPointNumber = 2;
    // communication slow rank list
    const static inline std::string ppPgName = "pp";
    const static inline std::string totalOpInfo = "Total Op Info";
    const static inline int slowRankCnt = 3; // 展示Top3慢卡
    const static inline double thresholdForSlowRank = 0.1;
    const static inline int doubleReservedNum = 3;
};
}
}
}
#endif // PROFILER_SERVER_CLUSTERSERVICE_H
