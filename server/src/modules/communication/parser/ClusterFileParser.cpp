/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 *
 *
 */

#include "pch.h"
#include "CommunicationMatrixRapidHandler.h"
#include "CommunicationRapidSaxHandler.h"
#include "ConstantDefs.h"
#include "DataBaseManager.h"
#include "ParserStatusManager.h"
#include "NumDefs.h"
#include "DbClusterDataBase.h"
#include "TraceTime.h"
#include "FileUtil.h"
#include "NumberUtil.h"
#include "ClusterFileParser.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
using namespace rapidjson;
bool ClusterFileParser::ParseCommunication(const std::vector<std::string> &filePathList)
{
    const std::string &filePath = FileUtil::PathPreprocess(filePathList[0].c_str());
    SaxParseJsonFile(filePath, 0);
    return true;
}

void ClusterFileParser::ParseCommunicationMatrix(const std::vector<std::string> &filePathList)
{
    const std::string &filePath = FileUtil::PathPreprocess(filePathList[0].c_str());
    Server::ServerLog::Info("ParseCommunicationMatrix"+filePath);
    SaxParseJsonFile(filePath, 1);
}

void ClusterFileParser::SaxParseJsonFile(const std::string& filePath, int saxHandlerType)
{
    auto start = std::chrono::high_resolution_clock::now();
    bool checkFilePath = FileUtil::CheckFilePath(filePath);
    if (!checkFilePath) {
        return;
    }
    // 打开JSON文件
    FILE* fp = fopen(filePath.c_str(), "rb");
    if (fp == nullptr) {
        return;
    }
    char readBuffer[65536];
    rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    rapidjson::Reader reader;
    if (saxHandlerType == 0) {
        CommunicationRapidSaxHandler rapidSaxHandler;
        reader.Parse<kParseNumbersAsStringsFlag>(is, rapidSaxHandler);
    } else {
        CommunicationMatrixRapidHandler matrixRapidHandler;
        reader.Parse(is, matrixRapidHandler);
    }
    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("End sax_parse_json_file data into db, file:", filePath, "cost time:", (end - start).count());
    fclose(fp);
}

void ClusterFileParser::ParseStepStatisticsFile(const std::vector<std::string> &filePathList)
{
    const std::string &filePath = FileUtil::PathPreprocess(filePathList[0].c_str());
    auto start = std::chrono::high_resolution_clock::now();
    if (!ValidateUtil::CheckCsvFile(filePath)) {
        return;
    }
    std::ifstream stepTraceFileCsv(filePath);
    std::string line;
    std::map<std::string, size_t> indexMap;
    auto database = dynamic_cast<TextClusterDatabase*>(DataBaseManager::Instance().GetWriteClusterDatabase());
    while (ParserStatusManager::Instance().GetClusterParserStatus() == ParserStatus::RUNNING &&
            std::getline(stepTraceFileCsv, line)) {
        std::vector<std::string> fields;
        std::string field;
        std::vector<std::string> tokens = StringUtil::StringSplit(line);
        if (tokens[0] != "Step") {
            StepStatistic statistic = MapToStepStatistic(tokens);
            database->InsertStepStatisticsInfo(statistic);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("End parse step statistics file data into db, file:", filePath, ",cost time:",
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
    baseInfo.collectStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    ParseCommunicationGroup(selectedPath, baseInfo);
    auto database = dynamic_cast<TextClusterDatabase*>(DataBaseManager::Instance().GetWriteClusterDatabase());
    bool result = database->GetParallelConfigFromStepTrace(baseInfo.config);
    if (!result || (baseInfo.config.dpSize == 1 && baseInfo.config.ppSize == 1 && baseInfo.config.tpSize == 1)) {
        baseInfo.level = PARALLEL_CONFIG_LEVEL_UNDEFINED;
    } else {
        baseInfo.level = PARALLEL_CONFIG_LEVEL_COLLECTED;
    }
    database->InsertClusterBaseInfo(baseInfo);
    ServerLog::Info("End save cluster base info data into db, path: ", selectedPath, " collectStartTime= ",
                    baseInfo.collectStartTime);
}

bool ClusterFileParser::ParseClusterFiles(const std::string &selectedPath)
{
    ParserStatusManager::Instance().SetClusterParseStatus(ParserStatus::RUNNING);
    if (!InitClusterDatabase(selectedPath)) {
        ServerLog::Error("Init cluster database occur an err");
        return false;
    }
    auto database = dynamic_cast<TextClusterDatabase*>(DataBaseManager::Instance().GetWriteClusterDatabase());
    if (database == nullptr) {
        ServerLog::Error("Can't get cluster database.");
        return false;
    }
    if (!needClearDb) {
        ServerLog::Info("cluster db file is already exist, skip parse ");
        ParserStatusManager::Instance().SetClusterParseStatus(ParserStatus::FINISH);
        uint64_t min = UINT64_MAX;
        uint64_t max = 0;
        database->QueryExtremumTimestamp(min, max);
        if (min != UINT64_MAX && max != 0) {
            Timeline::TraceTime::Instance().UpdateTime(min, max);
        }
        return true;
    }
    // parse communication file
    std::regex patternCommunicationMatrix(R"(cluster_communication_matrix.json)");
    std::vector<std::string> communicationMatrixFileList =
            FileUtil::FindFirstFileByRegex(selectedPath, patternCommunicationMatrix);
    // cluster analysis
    if (communicationMatrixFileList.empty() && !AttAnalyze(selectedPath, ATT_MODEL_MATRIX)) {
        return false;
    }
    std::vector<std::string> communicationMatrixList =
            FileUtil::FindFirstFileByRegex(selectedPath, patternCommunicationMatrix);
    if (!communicationMatrixList.empty()) {
        ParseCommunicationMatrix(communicationMatrixList);
    }
    database->SaveLastData();
    std::regex patternStepTrace(R"(cluster_step_trace_time.csv)");
    std::vector<std::string> stepTraceFileList = FileUtil::FindFirstFileByRegex(selectedPath, patternStepTrace);
    if (!stepTraceFileList.empty()) {
        ParseStepStatisticsFile(stepTraceFileList);
        SaveClusterBaseInfo(selectedPath);
    }
    if (!database->CreateIndex()) {
        ServerLog::Error("Failed to create index on cluster database. path:", selectedPath);
        return false;
    }
    if (ParserStatusManager::Instance().GetClusterParserStatus() != ParserStatus::RUNNING) {
        ServerLog::Warn("Parser Cluster Status Is Terminal");
        return false;
    }
    return true;
}

bool ClusterFileParser::ParseClusterStep2Files(const std::string &selectedPath)
{
    // parse communication file
    std::regex patternCommunication(R"(cluster_communication.json)");
    std::vector<std::string> communicationFileList =
            FileUtil::FindFirstFileByRegex(selectedPath, patternCommunication);
    std::regex patternCommunicationMatrix(R"(cluster_communication_matrix.json)");
    std::vector<std::string> communicationMatrixFileList =
            FileUtil::FindFirstFileByRegex(selectedPath, patternCommunicationMatrix);
    bool isCopyMatrixFile = false;
    std::string tempFilePath = FileUtil::SplicePath(selectedPath, "tmp_matrix.json");
    // cluster_communication_matrix存在，但cluster_communication不存在时，将matrix文件复制到selectedPath下的临时文件中
    if (!communicationMatrixFileList.empty() && communicationFileList.empty()) {
        isCopyMatrixFile = FileUtil::CopyFileByPath(FileUtil::PathPreprocess(communicationMatrixFileList[0].c_str()),
                                                    tempFilePath);
        if (!isCopyMatrixFile) {
            ServerLog::Warn("Copy matrix file failed.");
        }
    }
    // cluster analysis
    if (communicationFileList.empty() && !AttAnalyze(selectedPath, ATT_MODEL_TIME)) {
        return false;
    }

    // 如果发生过文件的复制，则将临时文件复制回cluster_analysis_output文件夹中，并且删除临时文件
    if (isCopyMatrixFile) {
        bool reductionFileRes =
                FileUtil::CopyFileByPath(tempFilePath, FileUtil::PathPreprocess(communicationMatrixFileList[0].c_str()))
                && FileUtil::RemoveFile(tempFilePath);
        if (!reductionFileRes) {
            ServerLog::Warn("Copy and clear matrix temp file failed.");
        }
    }
    return TransCommunicationToDb(selectedPath, patternCommunication);
}

bool ClusterFileParser::TransCommunicationToDb(const std::string &selectedPath, const std::regex &patternCommunication)
{
    auto database = dynamic_cast<TextClusterDatabase *>(DataBaseManager::Instance().GetWriteClusterDatabase());
    if (database == nullptr) {
        ServerLog::Error("Failed to connect to cluster database.", selectedPath);
        return false;
    }
    std::vector<std::string> communicationFileList =
            FileUtil::FindFirstFileByRegex(selectedPath, patternCommunication);
    if (!communicationFileList.empty()) {
        ParseCommunication(communicationFileList);
    }
    database->SaveLastData();
    if (!database->CreateTimeIndex()) {
        ServerLog::Error("Failed to CreateTimeIndex on cluster database. path:", selectedPath);
        return false;
    }
    if (ParserStatusManager::Instance().GetClusterParserStatus() != ParserStatus::RUNNING) {
        ServerLog::Warn("Parser Cluster Status Is Terminal");
        return false;
    }
    database->UpdateClusterParseStatus(FINISH_STATUS);
    ParserStatusManager::Instance().SetClusterParseStatus(ParserStatus::FINISH);
    uint64_t min = UINT64_MAX;
    uint64_t max = 0;
    database->QueryExtremumTimestamp(min, max);
    if (min != UINT64_MAX && max != 0) {
        Timeline::TraceTime::Instance().UpdateTime(min, max);
    }
    return true;
}

bool ClusterFileParser::InitClusterDatabase(const std::string& selectedPath)
{
    // 导入前清空cluster databaseWrite
    DataBaseManager::Instance().ClearClusterDb();
    clusterDbPath = selectedPath + FILE_SEPARATOR + "cluster.db";
    std::ifstream file(FileUtil::PathPreprocess(clusterDbPath));
    auto databaseWrite = dynamic_cast<TextClusterDatabase*>(DataBaseManager::Instance().GetWriteClusterDatabase());
    // 查询单独一个连接
    auto databaseRead = dynamic_cast<TextClusterDatabase *>(DataBaseManager::Instance().GetReadClusterDatabase());
    databaseRead->OpenDb(clusterDbPath, false);
    databaseRead->SetConfig();
    if (!file.good()) {
        if (!(databaseWrite->OpenDb(clusterDbPath, true) && databaseWrite->CreateTable() &&
              databaseWrite->SetConfig() && databaseWrite->SetDbVersion() && databaseWrite->InitStmt())) {
            ServerLog::Error("Failed to open databaseWrite. path:", selectedPath);
            return false;
        }
    } else {
        if (!(databaseWrite->OpenDb(clusterDbPath, false))) {
            ServerLog::Error("Failed to open databaseWrite. path:", selectedPath);
            return false;
        }
        // 判断数据库版本是否变更，若变更不能跳过解析
        auto isChange = databaseWrite->IsDatabaseVersionChange();
        std::string status = databaseWrite->QueryParseClusterStatus();
        needClearDb = isChange || status.empty() || strcmp(status.c_str(), FINISH_STATUS.c_str()) != 0;
        if (needClearDb && !(databaseWrite->DropAllTable() && databaseWrite->CreateTable())) {
            ServerLog::Error("Failed to dropAllTable. path:", selectedPath, "isChange:", isChange);
            return false;
        }
        if (!(databaseWrite->SetConfig() && databaseWrite->SetDbVersion() && databaseWrite->InitStmt())) {
            ServerLog::Error("Failed to init databaseWrite. path:", selectedPath);
            return false;
        }
    }
    return true;
}

void ClusterFileParser::ParseCommunicationGroup(const std::string selectedPath, ClusterBaseInfo &baseInfo)
{
    std::vector<std::string> communicationGroupList =
            FileUtil::FindFirstFileByRegex(selectedPath, std::regex(R"(communication_group.json)"));
    if (communicationGroupList.empty()) {
        ServerLog::Error("Failed to get communicationGroup files");
        return;
    }
    const std::string &filePath = FileUtil::PathPreprocess(communicationGroupList[0].c_str());
    auto start = std::chrono::high_resolution_clock::now();
    std::ifstream communicationGroup(filePath, std::ios::binary);
    if (communicationGroup.good()) {
        Document doc;
        std::string fileContent;
        std::copy(std::istream_iterator<unsigned char>(communicationGroup), std::istream_iterator<unsigned char>(),
                  back_inserter(fileContent));
        doc.Parse(fileContent.c_str());
        if (doc.HasParseError()) {
            ServerLog::Error("JSON file is invalid.");
            return;
        }
        auto p2p = doc.FindMember("p2p")->value.GetArray();
        auto collective = doc.FindMember("collective")->value.GetArray();
        auto orderByLenDesAndNumAsc = [](const Value& a, const Value& b) {
            if (a.Size() == b.Size() && a.Size() > 0) {
                return a[0].GetInt() < b[0].GetInt();
            }
            return a.Size() > b.Size();
        };
        std::sort(p2p.Begin(), p2p.End(), orderByLenDesAndNumAsc);
        std::sort(collective.Begin(), collective.End(), orderByLenDesAndNumAsc);
        std::for_each(p2p.begin(), p2p.end(), [&collective](auto& item) {
            auto pos = std::find(collective.begin(), collective.end(), item);
            if (pos != collective.end()) {
                collective.Erase(pos);
            }
        });
        auto endIt = std::unique(collective.begin(), collective.end()); // 去重
        if (!collective.Empty()) {
            collective.Erase(endIt, collective.End());
        }
        baseInfo.ppStages = JsonUtil::JsonDump(p2p);
        baseInfo.stages = JsonUtil::JsonDump(collective);
        auto end = std::chrono::high_resolution_clock::now();
        ServerLog::Info("end parseCommunicationGroupFile data into db ,file:", filePath, ",cost time:",
                        (end - start).count());
    } else {
        ServerLog::Error("parseCommunicationGroupFile fail, path:", filePath);
    }
}

bool ClusterFileParser::AttAnalyze(const std::string& selectedPath, const std::string& model)
{
    ServerLog::Info("Start execute cluster analysis");
    if (!StringUtil::ValidateCommandFilePathParam(selectedPath)) {
        ServerLog::Warn("validate string select path failed! select path", selectedPath);
        return false;
    }
    std::string regex;
    std::string currPath = FileUtil::GetCurrPath();
    std::string switchCommand = "";
#ifdef _WIN32
    regex = "cluster_analysis.exe";
    // windows 下数据和安装目录不在一个磁盘需要切换磁盘
    if (std::strcmp(currPath.substr(0, 1).c_str(), selectedPath.substr(0, 1).c_str()) != 0) {
        switchCommand = " && " + selectedPath.substr(0, INT_TWO);
    }
#else
    regex = "cluster_analysis";
#endif
    ServerLog::Info("Start find cluster analysis executable file in dir: ", currPath);
    std::vector<std::string> exePathVector =
            FileUtil::FindFilesByRegex(currPath, std::regex(regex));
    if (!exePathVector.empty()) {
        std::string command = "cd \"" + FileUtil::PathPreprocess(selectedPath) +
                "\"" + switchCommand + " && \"" + exePathVector[0] + "\" -d .";
        if (!model.empty()) {
            command.append(" -m ").append(model);
        }
        ServerLog::Info("start execute command:", command);
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
    size_t index = 0;
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
    statistic.computingTime = NumberUtil::StringToDouble(tokens[index++]);
    statistic.pureCommunicationTime = NumberUtil::StringToDouble(tokens[index++]);
    statistic.overlapCommunicationTime = NumberUtil::StringToDouble(tokens[index++]);
    statistic.communicationTime = NumberUtil::StringToDouble(tokens[index++]);
    statistic.freeTime = NumberUtil::StringToDouble(tokens[index++]);
    statistic.stageTime = NumberUtil::StringToDouble(tokens[index++]);
    statistic.bubbleTime = NumberUtil::StringToDouble(tokens[index++]);
    statistic.pureCommunicationExcludeReceiveTime = NumberUtil::StringToDouble(tokens[index++]);
    if (index >= tokens.size()) {
        statistic.prepareTime = -1; // 代表csv文件中没有Preparing字段
        return statistic;
    } else {
        statistic.prepareTime = NumberUtil::StringToDouble(tokens[index]);
    }
    // 该部分需要进一步优化为按csv文件表头查询数据
    if (index + 3 < tokens.size()) { // 3 for dp_index, pp_index, tp_index
        statistic.dpIndex = NumberUtil::StringToLong(tokens[++index]);
        statistic.ppIndex = NumberUtil::StringToLong(tokens[++index]);
        statistic.tpIndex = NumberUtil::StringToLong(tokens[++index]);
    }
    return statistic;
}

bool ClusterFileParser::ParserClusterOfDb(const std::string& selectedPath)
{
    std::string tempPath(selectedPath);
    // 如果selectedPath是单个文件，则使用该文件所在文件夹作为分析路径
    if (!FileUtil::IsFolder(selectedPath)) {
        tempPath = FileUtil::GetParentPath(selectedPath);
    }
    // cluster analysis
    if (!AttAnalyze(tempPath, ATT_MODEL_DEFAULT)) {
        return false;
    }

    std::vector<std::string> clusterPath = FileUtil::FindFilesWithFilter(tempPath, std::regex(clusterDBReg));
    if (clusterPath.empty()) {
        return false;
    }
    auto clusterDatabase =
        dynamic_cast<FullDb::DbClusterDataBase *>(DataBaseManager::Instance().GetReadClusterDatabase());
    if (clusterDatabase == nullptr) {
        ServerLog::Error("Failed to get Cluster connection.");
        return false;
    }

    ServerLog::Info("Cluster Db Path: " + clusterPath[0]);
    if (!clusterDatabase->OpenDb(clusterPath[0], false)) {
        ServerLog::Error("Failed to open Cluster. rankId:", "FullDb");
        return false;
    }
    clusterDatabase->UpdateClusterParseStatus(FINISH_STATUS);
    ServerLog::Info("ParseClusterFiles is success");

    ParserStatusManager::Instance().SetClusterParseStatus(ParserStatus::FINISH);
    return true;
}

std::string ClusterFileParser::GetClusterDbPath()
{
    return clusterDbPath;
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic