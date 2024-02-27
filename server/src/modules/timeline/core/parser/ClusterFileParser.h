/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
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
    bool ParseClusterStep2Files(const std::string &selectedPath);
    static bool ParseCommunication(const std::vector<std::string> &filePathList);
    void ParseStepStatisticsFile(const std::vector<std::string> &filePathList);
    static void SaveClusterBaseInfo(const std::string& selectedPath);
    static void ParseCommunicationMatrix(const std::vector<std::string> &filePathList);
    static void ParseCommunicationGroup(const std::string selectedPath, ClusterBaseInfo &baseInfo);
private:
    static void SaxParseJsonFile(const std::string& filePath, int saxHandlerType);
    static bool InitClusterDatabase(const std::string& selectedPath, bool dbIsAlreadyExist);
    StepStatistic MapToStepStatistic(std::vector<std::string> tokens);
    int subStrlen = 2;

    static bool AttAnalyze(const std::string& selectedPath, const std::string& model);
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_CLUSTERFILEPARSER_H
