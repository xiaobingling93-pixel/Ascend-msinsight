/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 *
 */

#include <sysinfoapi.h>
#include <fstream>
#include "json.hpp"
#include "ServerLog.h"
#include "CommunicationSaxHandler.h"
#include "FileUtil.h"
#include "ExecUtil.h"
#include "DataBaseManager.h"
#include "ClusterFileParser.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
bool ClusterFileParser::ParseCommunication(const std::vector<std::string> &filePathList)
{
    const std::string &filePath = filePathList[0];
    DWORD start = GetTickCount();
    ServerLog::Info("start save communication data into db ,file:", filePath);
    std::ifstream ifs(filePath);
    CommunicationSaxHandler handler;
    nlohmann::json::sax_parse(ifs, &handler);
    DWORD end = GetTickCount();
    ServerLog::Info("end parse communication data into db ,file:", filePath, "cost time:", end - start);
    ifs.close();
}

void ClusterFileParser::ParseStepStatisticsFile(const std::vector<std::string> &filePathList)
{
    const std::string &filePath = filePathList[0];
    DWORD start = GetTickCount();
    ServerLog::Info("start parseStepStatisticsFile data into db ,file:", filePath);
    std::ifstream stepTraceFileCsv(filePath);
    std::string line;
    std::map<std::string, int> indexMap;
    auto database = DataBaseManager::Instance().GetClusterDatabase();
    while (std::getline(stepTraceFileCsv, line)) {
        std::vector<std::string> fields;
        std::string field;
        std::regex pattern(R"(,(?=(?:[^"]*"[^"]*")*[^"]*$))");
        std::vector<std::string> tokens(std::sregex_token_iterator(line.begin(), line.end(), pattern, -1),
                                        std::sregex_token_iterator());
        if (tokens[0] != "Step") {
            StepStatistic statistic = MapToStepStatistic(tokens);
            database->InsertStepStatisticsInfo(statistic);
        }
    }
    DWORD end = GetTickCount();
    ServerLog::Info("end parseStepStatisticsFile data into db ,file:", filePath, "cost time:",
                    end - start);
    stepTraceFileCsv.close();
}

void ClusterFileParser::SaveClusterBaseInfo(const std::string &selectedPath)
{
    ClusterBaseInfo baseInfo;
    FileUtil::CalculateDirSize(selectedPath, baseInfo.dataSize, 0);
    baseInfo.filePath = selectedPath;
    baseInfo.collectDuration = 0;
    auto now = std::chrono::system_clock::now();
    // 转换为毫秒数
    baseInfo.collectStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
    auto database = DataBaseManager::Instance().GetClusterDatabase();
    database->InsertClusterBaseInfo(baseInfo);
    ServerLog::Info("end saveClusterBaseInfo data into db ,path:", selectedPath, " collectStartTime=",
                    baseInfo.collectStartTime);
}

bool ClusterFileParser::ParseClusterFiles(const std::string &selectedPath)
{
    auto database = DataBaseManager::Instance().GetClusterDatabase();
    if (!(database->OpenDb(selectedPath + "/cluster.db", true) && database->CreateTable() &&
          database->SetConfig() && database->InitStmt())) {
        ServerLog::Error("Failed to open database. path:", selectedPath);
    }
    // parse communication file
    std::regex patternCommunication(R"(cluster_communication.json)");
    std::vector<std::string> communicationFileList =
            FileUtil::FindFilesByRegex(selectedPath, patternCommunication);
    if (communicationFileList.empty()) {
        // todo 集群解析
    }
    communicationFileList =
            FileUtil::FindFilesByRegex(selectedPath, patternCommunication);
    if (!communicationFileList.empty()) {
        ParseCommunication(communicationFileList);
    }
    database->SaveLastData();
    // parse cluster_step_trace_time csv
    std::regex patternStepTrace(R"(cluster_step_trace_time.csv)");
    std::vector<std::string> stepTraceFileList = FileUtil::FindFilesByRegex(selectedPath, patternStepTrace);
    if (!stepTraceFileList.empty()) {
        ParseStepStatisticsFile(stepTraceFileList);
        // parse cluster_step_trace_time csv
        SaveClusterBaseInfo(selectedPath);
    }
    return true;
}

StepStatistic ClusterFileParser::MapToStepStatistic(std::vector<std::string> tokens)
{
    StepStatistic statistic;
    int index = 0;
    statistic.stepId = tokens[index++];
    std::string flag = tokens[index++];
    std::string order = tokens[index++];
    statistic.rankId = std::strcmp(flag.c_str(), " rank") == 0 ? order : "";
    statistic.stageId = std::strcmp(flag.c_str(), " stage") == 0  ? order : "";
    statistic.computingTime = tokens[index].empty() ? 0 : std::stod(tokens[index++]);
    statistic.pureCommunicationTime = tokens[index].empty() ? 0 : std::stod(tokens[index++]);
    statistic.overlapCommunicationTime = tokens[index].empty() ? 0 : std::stod(tokens[index++]);
    statistic.communicationTime = tokens[index].empty() ? 0 : std::stod(tokens[index++]);
    statistic.freeTime = tokens[index].empty() ? 0 : std::stod(tokens[index++]);
    statistic.stageTime = tokens[index].empty() ? 0 : std::stod(tokens[index++]);
    statistic.bubbleTime = tokens[index].empty() ? 0 : std::stod(tokens[index++]);
    statistic.pureCommunicationExcludeReceiveTime =
            tokens[index].empty() ? 0 : std::stod(tokens[index]);
    return statistic;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic