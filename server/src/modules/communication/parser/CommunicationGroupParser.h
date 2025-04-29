/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_COMMUNICATIONGROUPPARSER_H
#define PROFILER_SERVER_COMMUNICATIONGROUPPARSER_H
#include <vector>
#include "ClusterDef.h"
#include "JsonUtil.h"

namespace Dic {
namespace Module {
namespace Communication {
class CommunicationGroupParser {
public:
    static std::vector<CommGroupParallelInfo> ParseCommunicationGroup(const std::string &selectedPath);
    static std::vector<CommGroupParallelInfo> ParseCommunicationGroupByText(const std::string &fileContent);
private:
    static std::vector<CommGroupParallelInfo> GetGroupFromParallelInfo(const json_t &json);
    static std::vector<CommGroupParallelInfo> GetGroupFromP2pAndCollective(const json_t &json);
};
}
}
}
#endif // PROFILER_SERVER_COMMUNICATIONGROUPPARSER_H
