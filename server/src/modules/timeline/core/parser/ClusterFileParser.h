/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 */

#ifndef PROFILER_SERVER_CLUSTERFILEPARSER_H
#define PROFILER_SERVER_CLUSTERFILEPARSER_H

#include <string>
#include <vector>
#include <ClusterDef.h>

namespace Dic {
namespace Module {
namespace Timeline {
class ClusterFileParser {
public:
    bool ParseClusterFiles(const std::string &selectedPath);
    bool ParseCommunication(const std::vector<std::string> &filePathList);
    void ParseStepStatisticsFile(const std::vector<std::string> &filePathList);
    void SaveClusterBaseInfo(const std::string& selectedPath);
private:
    StepStatistic MapToStepStatistic(std::vector<std::string> tokens);
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_CLUSTERFILEPARSER_H
