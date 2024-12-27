/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_CLUSTERSERVICE_H
#define PROFILER_SERVER_CLUSTERSERVICE_H

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
private:
    static std::vector<Protocol::GroupInfo> MergeGroupInfo(std::vector<std::string> &compareGroupList,
                                                           std::vector<std::string> &baselineGroupList);
};
}
}
}
#endif // PROFILER_SERVER_CLUSTERSERVICE_H
