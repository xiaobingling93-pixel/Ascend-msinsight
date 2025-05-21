/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 *
 */

#ifndef PROFILER_SERVER_CLUSTERFILEPARSER_H
#define PROFILER_SERVER_CLUSTERFILEPARSER_H

#include <string>
#include <vector>
#include "VirtualClusterDatabase.h"
#include "ClusterDef.h"
#include "DbClusterDataBase.h"
#include "pch.h"

namespace Dic {
namespace Module {
namespace Timeline {
enum class AttDataType {
    TEXT,
    DB
};
class ClusterFileParser {
public:
    ClusterFileParser(const std::string &filePath, std::shared_ptr<VirtualClusterDatabase> database,
                      const std::string &uniqueKey);
    bool ParseClusterFiles();
    bool ParseClusterStep2Files();
    bool ParseCommunication(const std::vector<std::string> &filePathList);
    void ParseStepStatisticsFile(const std::vector<std::string> &filePathList);
    void SaveClusterBaseInfo(const std::string& selectedPath);
    void ParseCommunicationMatrix(const std::vector<std::string> &filePathList);
    bool ParserClusterOfDb();
    std::string GetClusterDbPath();
    static bool CheckIsCluster(const std::string &filePath);
    inline std::string GetClusterPath()
    {
        return selectedFilePath;
    }
private:
    void SaxParseJsonFile(const std::string& filePath, int saxHandlerType);
    bool InitClusterDatabase();
    StepStatistic MapToStepStatistic(std::map<std::string, size_t> &dataMap,
                                     const std::vector<std::string> &tokens);
    static void InitFullDbClusterBaseInfo(std::shared_ptr<FullDb::DbClusterDataBase> &clusterDatabase,
                                          ClusterBaseInfo &baseInfo);
    size_t subStrlen = 2;
    std::string clusterDbPath;
    bool needClearDb = true;
    std::string selectedFilePath;
    std::string uniqueKey;
    const std::string CLUSTER_IDENTIFY = "cluster_";
    std::shared_ptr<VirtualClusterDatabase> database;
    static bool AttAnalyze(const std::string &selectedPath,
                           const std::string &mode,
                           AttDataType dataType = AttDataType::TEXT);
    bool TransCommunicationToDb(const std::string &selectedPath, const std::regex &patternCommunication);
    bool InitCommunicationGroupInfo(std::vector<CommGroupParallelInfo> &groupInfos);
    static bool CheckDocumentValid(const Document &doc);
    static std::string GetStrValue(std::map<std::string, size_t> &dataMap, const std::vector<std::string> &tokens,
                                   const std::string &key);
    bool SkipClusterParse();
    bool InitBaseInfoAndMatrixData();
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_CLUSTERFILEPARSER_H
