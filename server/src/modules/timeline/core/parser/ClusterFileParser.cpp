/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 *
 */

#include <fstream>
#include "filereadstream.h"
#include "json.hpp"
#include "ServerLog.h"
#include "CommunicationMatrixRapidHandler.h"
#include "CommunicationRapidSaxHandler.h"
#include "FileUtil.h"
#include "ExecUtil.h"
#include "DataBaseManager.h"
#include "ParserStatusManager.h"
#include "ClusterFileParser.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
bool ClusterFileParser::ParseCommunication(const std::vector<std::string> &filePathList)
{
    const std::string &filePath = filePathList[0];
    SaxParseJsonFile(filePath, 0);
    return true;
}

void ClusterFileParser::ParseCommunicationMatrix(const std::vector<std::string> &filePathList)
{
    const std::string &filePath = filePathList[0];
    SaxParseJsonFile(filePath, 1);
}

void ClusterFileParser::SaxParseJsonFile(const std::string& filePath, int saxHandlerType)
{
    auto start = std::chrono::high_resolution_clock::now();
    ServerLog::Info("start SaxParseJsonFile data into db ,file:", filePath);
    bool checkFilePath = FileUtil::CheckFilePath(filePath);
    if (!checkFilePath) {
        return;
    }
    // 打开JSON文件
    FILE* fp = fopen(filePath.c_str(), "rb");
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    rapidjson::Reader reader;
    if (saxHandlerType == 0) {
        CommunicationRapidSaxHandler rapidSaxHandler;
        reader.Parse(is, rapidSaxHandler);
    } else {
        CommunicationMatrixRapidHandler matrixRapidHandler;
        reader.Parse(is, matrixRapidHandler);
    }
    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("end SaxParseJsonFile data into db ,file:", filePath, "cost time:", (end - start).count());
    fclose(fp);
}

void ClusterFileParser::ParseStepStatisticsFile(const std::vector<std::string> &filePathList)
{
    const std::string &filePath = filePathList[0];
    auto start = std::chrono::high_resolution_clock::now();
    ServerLog::Info("start parseStepStatisticsFile data into db ,file:", filePath);
    std::ifstream stepTraceFileCsv(filePath);
    std::string line;
    std::map<std::string, int> indexMap;
    auto database = DataBaseManager::Instance().GetClusterDatabase();
    while (ParserStatusManager::Instance().GetClusterParserStatus() == ParserStatus::RUNNING &&
            std::getline(stepTraceFileCsv, line)) {
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
    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("end parseStepStatisticsFile data into db ,file:", filePath, ",cost time:",
                    (end - start).count());
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
    ParserStatusManager::Instance().SetClusterParseStatus(ParserStatus::RUNNING);
    // 导入前清空cluster db
    DataBaseManager::Instance().ClearClusterDb();
    auto database = DataBaseManager::Instance().GetClusterDatabase();
    if (!(database->OpenDb(selectedPath + "/cluster.db", true) && database->CreateTable() &&
          database->SetConfig() && database->InitStmt())) {
        ServerLog::Error("Failed to open database. path:", selectedPath);
    }
    // parse communication file
    std::regex patternCommunication(R"(cluster_communication.json)");
    std::vector<std::string> communicationFileList =
            FileUtil::FindFilesByRegex(selectedPath, patternCommunication);
    // cluster analysis
    if (communicationFileList.empty() && !AttAnalyze(selectedPath)) {
        return false;
    }
    communicationFileList =
            FileUtil::FindFilesByRegex(selectedPath, patternCommunication);
    if (!communicationFileList.empty()) {
        ParseCommunication(communicationFileList);
    }
    // matrix
    std::regex patternCommunicationMatrix(R"(cluster_communication_matrix.json)");
    std::vector<std::string> communicationMatrixList =
            FileUtil::FindFilesByRegex(selectedPath, patternCommunicationMatrix);
    if (!communicationMatrixList.empty()) {
        ParseCommunicationMatrix(communicationMatrixList);
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
    if (ParserStatusManager::Instance().GetClusterParserStatus() != ParserStatus::RUNNING) {
        ServerLog::Warn("Parser Cluster Status Is Terminal");
        return false;
    }
    ParserStatusManager::Instance().SetClusterParseStatus(ParserStatus::FINISH);
    return true;
}

bool ClusterFileParser::AttAnalyze(const std::string& selectedPath)
{
    ServerLog::Info("can not find cluster analysis file, start execute cluster analysis");
    std::string regex;
#ifdef _WIN32
    regex = "cluster_analysis.exe";
#else
    regex = "cluster_analysis";
#endif
    std::vector<std::string> exePathVector =
            FileUtil::FindFilesByRegex(FileUtil::GetCurrPath(), std::regex(regex));
    if (!exePathVector.empty()) {
        std::string command = "\"" + exePathVector[0] +  "\" -d " + FileUtil::PathPreprocess(selectedPath);
        int result = std::system(command.c_str());
        if (result != 0) {
            ServerLog::Warn("Execute cluster analysis failed, skip parse cluster file, command:", command);
            return false;
        }
        ServerLog::Info("Execute cluster analysis success, command:", command);
    } else {
        ServerLog::Warn("Can not find cluster analysis execute file under.", selectedPath);
        return false;
    }
    return true;
}

StepStatistic ClusterFileParser::MapToStepStatistic(std::vector<std::string> tokens)
{
    StepStatistic statistic;
    int index = 0;
    statistic.stepId = tokens[index].empty() ? "0" : tokens[index];
    index++;
    std::string flag = tokens[index++];
    std::string order = tokens[index++];
    statistic.rankId = std::strcmp(flag.c_str(), "rank") == 0 ? order : "";
    // 去掉stage的首尾引号
    if (std::strcmp(flag.c_str(), "stage") == 0 &&
        order.find('\"') != std::string::npos &&
        order.length() > subStrlen) {
        order = order.substr(1, order.length() - subStrlen);
        statistic.stageId = order;
    }
    statistic.computingTime = tokens[index].empty() ? 0 : std::stod(tokens[index]);
    index++;
    statistic.pureCommunicationTime = tokens[index].empty() ? 0 : std::stod(tokens[index]);
    index++;
    statistic.overlapCommunicationTime = tokens[index].empty() ? 0 : std::stod(tokens[index]);
    index++;
    statistic.communicationTime = tokens[index].empty() ? 0 : std::stod(tokens[index]);
    index++;
    statistic.freeTime = tokens[index].empty() ? 0 : std::stod(tokens[index]);
    index++;
    statistic.stageTime = tokens[index].empty() ? 0 : std::stod(tokens[index]);
    index++;
    statistic.bubbleTime = tokens[index].empty() ? 0 : std::stod(tokens[index]);
    index++;
    statistic.pureCommunicationExcludeReceiveTime =
            tokens[index].empty() ? 0 : std::stod(tokens[index]);
    return statistic;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic