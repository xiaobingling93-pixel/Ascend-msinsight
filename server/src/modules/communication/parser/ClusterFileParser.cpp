/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 *
 *
 */

#include <algorithm>
#include "pch.h"
#include "CommunicationMatrixRapidHandler.h"
#include "CommunicationRapidSaxHandler.h"
#include "ConstantDefs.h"
#include "DataBaseManager.h"
#include "ParserStatusManager.h"
#include "DbClusterDataBase.h"
#include "TraceTime.h"
#include "CollectionUtil.h"
#include "ClusterFileParser.h"


namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
using namespace rapidjson;
using namespace Dic::Module::FullDb;
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
    std::shared_ptr<TextClusterDatabase> textDb = std::dynamic_pointer_cast<TextClusterDatabase>(database);
    if (textDb == nullptr) {
        ServerLog::Error("Fail to parse json file, file type:", saxHandlerType);
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
        CommunicationRapidSaxHandler rapidSaxHandler(textDb, uniqueKey);
        reader.Parse<kParseNumbersAsStringsFlag>(is, rapidSaxHandler);
    } else {
        CommunicationMatrixRapidHandler matrixRapidHandler(textDb, uniqueKey);
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
    std::ifstream stepTraceFileCsv = OpenReadFileSafely(filePath);
    std::string line;
    std::map<std::string, size_t> indexMap;
    std::shared_ptr<TextClusterDatabase> textDb = std::dynamic_pointer_cast<TextClusterDatabase>(database);
    if (textDb == nullptr) {
        ServerLog::Error("Can't get cluster database when parse step statistics file.");
        return;
    }
    bool isHeader = true;
    std::map<std::string, size_t> dataMap;
    while (std::getline(stepTraceFileCsv, line)) {
        std::vector<std::string> tokens = StringUtil::StringSplit(line);
        if (!tokens.empty() and isHeader) {
            // 校验表头，求必要表头和当前文件表头的差集，如果差集数量大于0，则校验不通过
            std::vector<std::string> difference = CollectionUtil::CalDifferenceVector(VALID_STEP_STATISTICS_HEADERS,
                                                                                      tokens);
            if (difference.size() != 0) {
                ServerLog::Error("The header of step statistics file is invalid, "
                                 "missing header data as follows: %, filePath: %",
                                 StringUtil::join(difference, ","), filePath);
                return;
            }
            for (size_t i = 0; i < tokens.size(); ++i) {
                dataMap[tokens[i]] = i;
            }
            isHeader = false;
            continue;
        }
        // 行内容校验，列数不相等则跳过
        if (tokens.size() != dataMap.size()) {
            ServerLog::Warn("Row size is not equal to header number.");
            continue;
        }
        StepStatistic statistic = MapToStepStatistic(dataMap, tokens);
        textDb->InsertStepStatisticsInfo(statistic);
    }
    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("End parse step statistics file data into db, cost time:", (end - start).count());
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
    std::shared_ptr<TextClusterDatabase> textDb = std::dynamic_pointer_cast<TextClusterDatabase>(database);
    if (textDb == nullptr) {
        ServerLog::Error("Can't get cluster database when sava cluster base info.");
        return;
    }
    bool result = textDb->GetParallelConfigFromStepTrace(baseInfo.config, baseInfo.level);
    if (!result || (baseInfo.config.dpSize == 1 && baseInfo.config.ppSize == 1 && baseInfo.config.tpSize == 1)) {
        ServerLog::Warn("Get initial parallel config from step trace.");
    }
    textDb->InsertClusterBaseInfo(baseInfo);
    ServerLog::Info("End save cluster base info data into db, path: ", selectedPath, " collectStartTime= ",
                    baseInfo.collectStartTime);
}

bool ClusterFileParser::ParseClusterFiles()
{
    ParserStatusManager::Instance().SetClusterParseStatus(uniqueKey, ParserStatus::INIT);
    if (!InitClusterDatabase()) {
        ServerLog::Error("Init cluster database occur an err");
        return false;
    }
    std::shared_ptr<TextClusterDatabase> textDb = std::dynamic_pointer_cast<TextClusterDatabase>(database);
    if (textDb == nullptr) {
        ServerLog::Error("Can't get cluster database when parse cluster files.");
        return false;
    }
    if (!needClearDb) {
        ServerLog::Info("cluster db file is already exist, skip parse ");
        ParserStatusManager::Instance().SetClusterParseStatus(uniqueKey, ParserStatus::FINISH);
        uint64_t min = UINT64_MAX;
        uint64_t max = 0;
        textDb->QueryExtremumTimestamp(min, max);
        if (min != UINT64_MAX && max != 0) {
            Timeline::TraceTime::Instance().UpdateTime(min, max);
        }
        return true;
    }
    ParserStatusManager::Instance().SetClusterParseStatus(uniqueKey, ParserStatus::RUNNING);
    // parse communication file
    std::regex patternCommunicationMatrix(R"(cluster_communication_matrix.json)");
    std::vector<std::string> communicationMatrixFileList =
            FileUtil::FindFirstFileByRegex(selectedFilePath, patternCommunicationMatrix);
    // cluster analysis
    if (communicationMatrixFileList.empty() && !AttAnalyze(selectedFilePath, ATT_MODEL_MATRIX, AttDataType::TEXT)) {
        return false;
    }
    std::vector<std::string> communicationMatrixList =
            FileUtil::FindFirstFileByRegex(selectedFilePath, patternCommunicationMatrix);
    if (!communicationMatrixList.empty()) {
        ParseCommunicationMatrix(communicationMatrixList);
    }
    textDb->SaveLastData();
    std::regex patternStepTrace(R"(cluster_step_trace_time.csv)");
    std::vector<std::string> stepTraceFileList = FileUtil::FindFirstFileByRegex(selectedFilePath, patternStepTrace);
    if (!stepTraceFileList.empty()) {
        ParseStepStatisticsFile(stepTraceFileList);
        SaveClusterBaseInfo(selectedFilePath);
    }
    if (!textDb->CreateIndex()) {
        ServerLog::Error("Failed to create index on cluster database. path:", selectedFilePath);
        return false;
    }
    return true;
}

bool ClusterFileParser::ParseClusterStep2Files()
{
    if (ParserStatusManager::Instance().IsClusterParserFinalState(uniqueKey)) {
        return true;
    }
    // parse communication file
    std::regex patternCommunication(R"(cluster_communication.json)");
    std::vector<std::string> communicationFileList =
            FileUtil::FindFirstFileByRegex(selectedFilePath, patternCommunication);
    std::regex patternCommunicationMatrix(R"(cluster_communication_matrix.json)");
    std::vector<std::string> communicationMatrixFileList =
            FileUtil::FindFirstFileByRegex(selectedFilePath, patternCommunicationMatrix);
    bool isCopyMatrixFile = false;
    std::string tempFilePath = FileUtil::SplicePath(selectedFilePath, "tmp_matrix.json");
    // cluster_communication_matrix存在，但cluster_communication不存在时，将matrix文件复制到selectedPath下的临时文件中
    if (!communicationMatrixFileList.empty() && communicationFileList.empty()) {
        isCopyMatrixFile = FileUtil::CopyFileByPath(FileUtil::PathPreprocess(communicationMatrixFileList[0].c_str()),
                                                    tempFilePath);
        if (!isCopyMatrixFile) {
            ServerLog::Warn("Copy matrix file failed.");
        }
    }
    // cluster analysis
    if (communicationFileList.empty() && !AttAnalyze(selectedFilePath, ATT_MODEL_TIME, AttDataType::TEXT)) {
        ParserStatusManager::Instance().SetClusterParseStatus(uniqueKey, ParserStatus::FINISH);
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
    bool res = TransCommunicationToDb(selectedFilePath, patternCommunication);
    ParserStatusManager::Instance().SetClusterParseStatus(uniqueKey, ParserStatus::FINISH);
    return res;
}

bool ClusterFileParser::TransCommunicationToDb(const std::string &selectedPath, const std::regex &patternCommunication)
{
    std::shared_ptr<TextClusterDatabase> textDb = std::dynamic_pointer_cast<TextClusterDatabase>(database);
    if (textDb == nullptr) {
        ServerLog::Error("Failed to connect to cluster database.", selectedPath);
        return false;
    }
    std::vector<std::string> communicationFileList =
            FileUtil::FindFirstFileByRegex(selectedPath, patternCommunication);
    if (!communicationFileList.empty()) {
        ParseCommunication(communicationFileList);
    }
    textDb->SaveLastData();
    if (!textDb->CreateTimeIndex()) {
        ServerLog::Error("Failed to CreateTimeIndex on cluster database. path:", selectedPath);
        return false;
    }
    if (ParserStatusManager::Instance().IsClusterParserFinalState(uniqueKey)) {
        ServerLog::Warn("Parser Cluster Status Is Terminal");
        return false;
    }
    textDb->UpdateClusterParseStatus(FINISH_STATUS);
    uint64_t min = UINT64_MAX;
    uint64_t max = 0;
    textDb->QueryExtremumTimestamp(min, max);
    if (min != UINT64_MAX && max != 0) {
        Timeline::TraceTime::Instance().UpdateTime(min, max);
    }
    return true;
}

bool ClusterFileParser::InitClusterDatabase()
{
    clusterDbPath = selectedFilePath + FILE_SEPARATOR + "cluster.db";
    std::ifstream file = OpenReadFileSafely(clusterDbPath, std::ios::in);
    std::shared_ptr<TextClusterDatabase> databaseWrite = std::dynamic_pointer_cast<TextClusterDatabase>(database);
    if (databaseWrite == nullptr) {
        ServerLog::Error("Can't get cluster database.");
        return false;
    }
    if (!file.good()) {
        if (!(databaseWrite->OpenDb(clusterDbPath, true) && databaseWrite->CreateTable() &&
              databaseWrite->SetConfig() && databaseWrite->SetDbVersion() && databaseWrite->InitStmt())) {
            ServerLog::Error("Failed to open databaseWrite. path:", selectedFilePath);
            return false;
        }
    } else {
        if (!(databaseWrite->OpenDb(clusterDbPath, false))) {
            ServerLog::Error("Failed to open databaseWrite. path:", selectedFilePath);
            return false;
        }
        // 判断数据库版本是否变更，若变更不能跳过解析
        auto isChange = databaseWrite->IsDatabaseVersionChange();
        std::string status = databaseWrite->QueryParseClusterStatus();
        needClearDb = isChange || status.empty() || strcmp(status.c_str(), FINISH_STATUS.c_str()) != 0;
        if (needClearDb && !(databaseWrite->DropAllTable() && databaseWrite->CreateTable())) {
            ServerLog::Error("Failed to dropAllTable. path:", selectedFilePath, "isChange:", isChange);
            return false;
        }
        if (!(databaseWrite->SetConfig() && databaseWrite->SetDbVersion() && databaseWrite->InitStmt())) {
            ServerLog::Error("Failed to init databaseWrite. path:", selectedFilePath);
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
    std::ifstream communicationGroup = OpenReadFileSafely(filePath, std::ios::binary);
    if (communicationGroup.good()) {
        Document doc;
        std::string fileContent;
        std::copy(std::istream_iterator<unsigned char>(communicationGroup), std::istream_iterator<unsigned char>(),
                  back_inserter(fileContent));
        doc.Parse(fileContent.c_str());
        if (!CheckDocumentValid(doc)) {
            return;
        }
        auto p2p = doc.FindMember("p2p")->value.GetArray();
        auto collective = doc.FindMember("collective")->value.GetArray();
        std::sort(p2p.Begin(), p2p.End(), OrderByLenDesAndNumAsc);
        std::sort(collective.Begin(), collective.End(), OrderByLenDesAndNumAsc);
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

bool ClusterFileParser::OrderByLenDesAndNumAsc(const Value& a, const Value& b)
{
    if (!a.IsArray() || !b.IsArray()) {
        return true;
    }
    if (a.Size() == b.Size() && a.Size() > 0) {
        if (!a[0].IsInt() || !b[0].IsInt()) {
            return true;
        }
        return a[0].GetInt() < b[0].GetInt();
    }
    return a.Size() > b.Size();
}

bool ClusterFileParser::CheckDocumentValid(const Document &doc)
{
    if (doc.HasParseError()) {
        ServerLog::Error("JSON file is invalid.");
        return false;
    }
    bool isLegal = doc.IsObject() && doc.HasMember("p2p") && doc.FindMember("p2p")->value.IsArray() &&
                   doc.HasMember("collective") && doc.FindMember("collective")->value.IsArray();
    if (!isLegal) {
        ServerLog::Error("JSON file is illegal.");
        return false;
    }
    return true;
}

bool ClusterFileParser::AttAnalyze(const std::string &selectedPath, const std::string &mode, AttDataType dataType)
{
    ServerLog::Info("Start execute cluster analysis");
    if (!StringUtil::ValidateCommandFilePathParam(selectedPath)) {
        ServerLog::Warn("validate string select path failed! select path", selectedPath);
        return false;
    }
    std::string currPath = FileUtil::GetCurrPath();
    std::string command = "cd \"" + FileUtil::PathPreprocess(selectedPath);

#ifdef _WIN32
    std::string analysisPath = FileUtil::SplicePath(currPath, "cluster_analysis.exe");
    // windows 下数据和安装目录不在一个磁盘需要切换磁盘
    std::string switchCommand = "";
    if (std::strcmp(currPath.substr(0, 1).c_str(), selectedPath.substr(0, 1).c_str()) != 0) {
        switchCommand = " && " + selectedPath.substr(0, INT_TWO);
    }
    command += "\"" + switchCommand + " && \"" + FileUtil::PathPreprocess(analysisPath) + "\" -d .";
#else
    #ifdef __APPLE__
    std::string analysisPath = FileUtil::SplicePath(currPath, "cluster_analysis");
    command += "\" && \"" + analysisPath + "\" -d .";
    #else
    std::string analysisPath = currPath + FILE_SEPARATOR + "msprof_analyze";
    analysisPath = analysisPath + FILE_SEPARATOR + "cluster_analyse" + FILE_SEPARATOR + "cluster_analysis.py";
    command += "\" && python3 \"" + analysisPath + "\" -d .";
    #endif
#endif

    if (!FileUtil::CheckFilePathExist(analysisPath) || !StringUtil::ValidateCommandFilePathParam(analysisPath)) {
        ServerLog::Error("Can not find cluster analysis execute file: % or"
            " path of cluster analysis contains illegal character.", analysisPath);
        return false;
    } else {
        if (!mode.empty()) {
            command.append(" -m ").append(mode);
        }
        // Db类型的数据需要开启字段精简功能
        if (dataType == AttDataType::DB) {
            command.append(" --data_simplification");
        }
        ServerLog::Info("Start execute command, selected path:", selectedPath, " ,mode: ", mode);
        int result = std::system(command.c_str());
        if (result != 0) {
            ServerLog::Warn("Execute cluster analysis failed, skip parse cluster file, selected path:",
                            selectedPath, " ,mode: ", mode);
            return false;
        }
        ServerLog::Info("Execute cluster analysis success, selected path:", selectedPath, " ,mode: ", mode);
    }
    return true;
}

StepStatistic ClusterFileParser::MapToStepStatistic(std::map<std::string, size_t> &dataMap,
                                                    const std::vector<std::string> &tokens)
{
    StepStatistic statistic;
    std::string stepId = GetStrValue(dataMap, tokens, FIELD_STEP);
    std::string flag = GetStrValue(dataMap, tokens, FIELD_TYPE);
    std::string order = GetStrValue(dataMap, tokens, FIELD_INDEX);
    statistic.stepId = stepId.empty() ? "0" : stepId;
    statistic.rankId = std::strcmp(flag.c_str(), "rank") == 0 ? order : "";
    // 去掉stage的首尾引号
    if (std::strcmp(flag.c_str(), "stage") == 0 &&
        order.find('\"') != std::string::npos &&
        order.length() > subStrlen) {
        order = order.substr(1, order.length() - subStrlen);
        statistic.stageId = order;
    }
    statistic.computingTime = NumberUtil::StringToDouble(GetStrValue(dataMap, tokens, FIELD_COMPUTING));
    statistic.pureCommunicationTime = NumberUtil::StringToDouble(
        GetStrValue(dataMap, tokens, FIELD_COMMUNICATION_NOT_OVERLAPPED));
    statistic.overlapCommunicationTime = NumberUtil::StringToDouble(GetStrValue(dataMap, tokens, FIELD_OVERLAPPED));
    statistic.communicationTime = NumberUtil::StringToDouble(GetStrValue(dataMap, tokens, FIELD_COMMUNICATION));
    statistic.freeTime = NumberUtil::StringToDouble(GetStrValue(dataMap, tokens, FIELD_FREE));
    statistic.stageTime = NumberUtil::StringToDouble(GetStrValue(dataMap, tokens, FIELD_STAGE));
    statistic.bubbleTime = NumberUtil::StringToDouble(GetStrValue(dataMap, tokens, FIELD_BUBBLE));
    statistic.pureCommunicationExcludeReceiveTime = NumberUtil::StringToDouble(
        GetStrValue(dataMap, tokens, FIELD_COMMUNICATION_NOT_OVERLAPPED_AND_RECEIVE));

    std::string prepareTime = GetStrValue(dataMap, tokens, FIELD_PREPARE_TIME);
    statistic.prepareTime = prepareTime.empty() ? -1 : NumberUtil::StringToDouble(prepareTime);

    // 判断表头中是否存在所有并行策略的key值
    bool allParallelKeys = std::all_of(PARALLEL_STRATEGY_HEADERS.begin(),
                                       PARALLEL_STRATEGY_HEADERS.end(), [&dataMap](const std::string& str) {
        return dataMap.find(str) != dataMap.end();
    });
    // 存在则读取相关的值
    if (allParallelKeys) {
        // 如果非数字字符串，这里会返回0
        statistic.dpIndex = NumberUtil::StringToLong(GetStrValue(dataMap, tokens, FIELD_DP_INDEX));
        statistic.ppIndex = NumberUtil::StringToLong(GetStrValue(dataMap, tokens, FIELD_PP_INDEX));
        statistic.tpIndex = NumberUtil::StringToLong(GetStrValue(dataMap, tokens, FIELD_TP_INDEX));
    }
    return statistic;
}

std::string ClusterFileParser::GetStrValue(std::map<std::string, size_t> &dataMap,
                                           const std::vector<std::string> &tokens, const std::string &key)
{
    if (dataMap.find(key) == dataMap.end()) {
        return "";
    }
    size_t index = dataMap[key];
    return tokens[index];
}

bool ClusterFileParser::ParserClusterOfDb()
{
    ParserStatusManager::Instance().SetClusterParseStatus(uniqueKey, ParserStatus::INIT);
    std::string tempPath(selectedFilePath);
    // 如果selectedPath是单个文件，则使用该文件所在文件夹作为分析路径
    if (!FileUtil::IsFolder(selectedFilePath)) {
        tempPath = FileUtil::GetParentPath(selectedFilePath);
    }
    // cluster analysis
    if (!AttAnalyze(tempPath, ATT_MODEL_DEFAULT, AttDataType::DB)) {
        ParserStatusManager::Instance().SetClusterParseStatus(uniqueKey, ParserStatus::FINISH);
        return false;
    }

    std::vector<std::string> clusterPath = FileUtil::FindFilesWithFilter(tempPath, std::regex(clusterDBReg));
    if (clusterPath.empty()) {
        ParserStatusManager::Instance().SetClusterParseStatus(uniqueKey, ParserStatus::FINISH);
        return false;
    }
    std::shared_ptr<FullDb::DbClusterDataBase> clusterDatabase = std::dynamic_pointer_cast<DbClusterDataBase>(database);
    if (clusterDatabase == nullptr) {
        ServerLog::Error("Failed to get Cluster connection.");
        ParserStatusManager::Instance().SetClusterParseStatus(uniqueKey, ParserStatus::FINISH);
        return false;
    }

    ServerLog::Info("Cluster Db Path: " + clusterPath[0]);
    clusterDbPath = clusterPath[0];
    if (!clusterDatabase->OpenDb(clusterPath[0], false)) {
        ServerLog::Error("Failed to open Cluster. File path:", clusterDbPath);
        ParserStatusManager::Instance().SetClusterParseStatus(uniqueKey, ParserStatus::FINISH);
        return false;
    }
    if (!clusterDatabase->IsDatabaseVersionChange() && clusterDatabase->HasFinishedParseLastTime()) {
        ParserStatusManager::Instance().SetClusterParseStatus(uniqueKey, ParserStatus::FINISH);
        return true;
    }

    // 如果数据库中初始就有ClusterBaseInfo表，将其中的并行策略信息保存到baseInfo结构体中
    // 将并行策略信息保存到baseInfo结构体后，删除ClusterBaseInfo表，然后新建同名表，按照指定格式存储信息
    // 如果数据库中初始无ClusterBaseInfo表，从ClusterStepTraceTime表获取并行策略信息
    clusterDatabase->SetHasClusterBaseInfoTable();
    ClusterBaseInfo baseInfo;
    if (clusterDatabase->HasClusterBaseInfoTable()) {
        clusterDatabase->QueryDistributedArgs(baseInfo.config, baseInfo.level);
    } else {
        clusterDatabase->GetParallelConfigFromStepTrace(baseInfo.config, baseInfo.level);
    }

    if (!clusterDatabase->DropTable() or !clusterDatabase->CreateTable() or !clusterDatabase->SetDataBaseVersion() or
        !clusterDatabase->UpdatesClusterParseStatus(NOT_FINISH_STATUS)) {
        ParserStatusManager::Instance().SetClusterParseStatus(uniqueKey, ParserStatus::FINISH);
        return false;
    }

    clusterDatabase->InsertClusterBaseInfo(baseInfo);

    clusterDatabase->UpdatesClusterParseStatus(FINISH_STATUS);
    ServerLog::Info("ParseClusterFiles is success");
    ParserStatusManager::Instance().SetClusterParseStatus(uniqueKey, ParserStatus::FINISH);
    return true;
}

std::string ClusterFileParser::GetClusterDbPath()
{
    return clusterDbPath;
}

ClusterFileParser::ClusterFileParser(const std::string &filePath, std::shared_ptr<VirtualClusterDatabase> database,
                                     const std::string &uniqueKey)
    : selectedFilePath(filePath), uniqueKey(uniqueKey), database(database) {}

bool ClusterFileParser::CheckIsCluster(const std::string &filePath)
{
    std::vector<std::string> folders;
    std::vector<std::string> files;
    if (filePath.find(CLUSTER_ANALYSIS_OUTPUT) != std::string::npos) {
        ServerLog::Info("this folder is cluster_analysis_output, Check Cluster is true");
        return true;
    }
    if (!FileUtil::FindFolders(filePath, folders, files)) {
        ServerLog::Info("FindFolders is empty, Check Cluster is false");
        return false;
    }
    return std::any_of(folders.begin(), folders.end(),
                       [](std::string &folder) { return folder == CLUSTER_ANALYSIS_OUTPUT; });
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic