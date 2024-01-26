/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "MemoryParse.h"
#include "ServerLog.h"

#include "FileUtil.h"
#include "ValidateUtil.h"
#include "TraceFileParser.h"
#include "ParserStatusManager.h"
#include "DataBaseManager.h"
#include "FileDef.h"
#include "NumberUtil.h"
#include "WsSession.h"
#include "WsSessionManager.h"

namespace Dic {
namespace Module {
namespace Memory {
using namespace Dic::Server;
MemoryParse &MemoryParse::Instance()
{
    static MemoryParse instance;
    return instance;
}

MemoryParse::MemoryParse()
{
    threadPool = std::make_unique<ThreadPool>(MemoryParse::maxThreadNum);
    ranks = {};
}

MemoryParse::~MemoryParse()
{
    threadPool->ShutDown();
}

bool MemoryParse::Parse(const std::vector<std::string> &filePaths, const std::string &fileId,
                        const std::string &selectedFolder)
{
    // 待废弃
    return false;
}

bool MemoryParse::OperatorParse(const std::string &filePath, const std::string &fileId)
{
    auto start = std::chrono::high_resolution_clock::now();
    ServerLog::Info("Start parse Operator Memory:", filePath);
    if (!ValidateUtil::CheckCsvFile(filePath)) {
        return false;
    }
    auto memoryDatabase = Timeline::DataBaseManager::Instance().GetMemoryDatabase(fileId);
    std::ifstream file(FileUtil::PathPreprocess(filePath));
    std::string line;
    std::map<std::string, size_t> dataMap;
    while (Timeline::ParserStatusManager::Instance().GetParserStatus(MEMORY_PREFIX + fileId) ==
            Timeline::ParserStatus::RUNNING && getline(file, line)) {
        std::stringstream ss(line);
        std::vector<std::string> row;
        std::string cell;

        while (getline(ss, cell, ',')) {
            row.push_back(cell);
        }
        if (row[0] == Dic::NAME || row[0] == Dic::DEVICE_ID) {
            for (size_t i = 0; i < row.size(); i++) {
                dataMap[row[i]] = i;
            }
            if (dataMap.size() < operatorTableNum) {
                ServerLog::Error("The header of the file is incorrect or incomplete. The file path is: " + filePath);
                return false;
            }
            GetMapValid((row[0] == Dic::NAME ? OPERATOR_CSV : OPERATOR_CSV_MSPROF), dataMap);
            memoryDatabase->SetInferenceType(row[0] == Dic::DEVICE_ID);
            continue;
        }
        // 如果某一行数据个数与表头不一致，则跳过
        if (dataMap.size() != row.size()) {
            continue;
        }
        Operator opePtr = MemoryParse::mapperToOperatorDetail(dataMap, row);
        // 读取每一行数据并插入到operator内
        memoryDatabase->insertOperatorDetail(opePtr);
    }
    // 读取剩下的数据并插入到operator内
    memoryDatabase->SaveOperatorDetail();
    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("End parse Operator Memory: ", filePath, ", cost time: ",
                    std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    return true;
}

void MemoryParse::GetMapValid(const std::vector<std::string>& vec, std::map<std::string, size_t> dataMap)
{
    for (const std::string& col : vec) {
        if (dataMap.find(col) == dataMap.end()) {
            ServerLog::Warn("The file lacks a parameter column : ", col);
        }
    }
}

Operator MemoryParse::mapperToOperatorDetail(std::map<std::string, size_t> dataMap, std::vector<std::string> row)
{
    Operator anOperator {};
    size_t nameIndex = dataMap[NAME];
    size_t allocationTimeIndex = dataMap[ALLOCATION_TIME];
    size_t sizeIndex = dataMap[SIZE];
    size_t durationIndex = dataMap[DURATION];
    anOperator.name = row[nameIndex];
    anOperator.size = atof(row[sizeIndex].c_str());
    anOperator.allocationTime = NumberUtil::TimestampUsToNs(
        NumberUtil::StringToLongDouble(row[allocationTimeIndex]));
    anOperator.duration = atof(row[durationIndex].c_str());

    if (dataMap.count(RELEASE_TIME)) {
        size_t releaseTimeIndex = dataMap[RELEASE_TIME];
        anOperator.releaseTime = NumberUtil::TimestampUsToNs(
            NumberUtil::StringToLongDouble(row[releaseTimeIndex]));
    } else {
        anOperator.releaseTime = NumberUtil::TimestampUsToNs(
            NumberUtil::StringToLongDouble(row[allocationTimeIndex]) + anOperator.duration);
    }

    return anOperator;
}

Record MemoryParse::mapperToRecordDetail(std::map<std::string, size_t> dataMap, std::vector<std::string> row)
{
    Record record {};
    size_t nameIndex = dataMap[COMPONENT];
    size_t timeStampIndex = dataMap[TIMESTAMP];
    record.component = row[nameIndex];
    record.timesTamp = NumberUtil::TimestampUsToNs(NumberUtil::StringToLongDouble(row[timeStampIndex]));
    // msprof场景
    if (dataMap.count(Dic::DEVICE_ID)) {
        size_t totalAllocatedIndex = dataMap[TOTAL_ALLOCATED_KB];
        size_t totalReservedIndex = dataMap[TOTAL_RESERVED_KB];
        size_t deviceTypeIndex = dataMap[DEVICE];
        record.totalAllocated = atof(row[totalAllocatedIndex].c_str()) / KB_TO_MB;
        record.totalReserved = atof(row[totalReservedIndex].c_str()) / KB_TO_MB;
        record.deviceType = row[deviceTypeIndex];
    } else {
        size_t totalAllocatedIndex = dataMap[TOTAL_ALLOCATED_MB];
        size_t totalReservedIndex = dataMap[TOTAL_RESERVED_MB];
        size_t deviceTypeIndex = dataMap[DEVICETYPE];
        record.totalAllocated = atof(row[totalAllocatedIndex].c_str());
        record.totalReserved = atof(row[totalReservedIndex].c_str());
        record.deviceType = row[deviceTypeIndex];
    }
    return record;
}

bool MemoryParse::RecordToParse(const std::string &filePath, const std::string &fileId)
{
    auto start = std::chrono::high_resolution_clock::now();
    ServerLog::Info("Start parse Memory Record: ", filePath);
    if (!ValidateUtil::CheckCsvFile(filePath)) {
        return false;
    }
    auto database = Timeline::DataBaseManager::Instance().GetMemoryDatabase(fileId);
    std::ifstream file(FileUtil::PathPreprocess(filePath));
    std::string line;
    std::map<std::string, size_t> dataMap;
    while (Timeline::ParserStatusManager::Instance().GetParserStatus(MEMORY_PREFIX + fileId) ==
           Timeline::ParserStatus::RUNNING && getline(file, line)) {
        std::stringstream ss(line);
        std::vector<std::string> row;
        std::string cell;
        while (getline(ss, cell, ',')) {
            row.push_back(cell);
        }
        if (row[0] == Dic::COMPONENT || row[0] == Dic::DEVICE_ID) {
            for (size_t i = 0; i < row.size(); i++) {
                dataMap[row[i]] = i;
            }
            if (dataMap.size() < recordTableNum) {
                ServerLog::Error("The header of the file is incorrect or incomplete. The path is: " + filePath);
                return false;
            }
            GetMapValid((row[0] == Dic::COMPONENT ? RECORD_CSV : RECORD_CSV_MSPROF), dataMap);
            continue;
        }
        if (row.size() != dataMap.size()) {
            continue;
        }
        Record recordPtr = MemoryParse::mapperToRecordDetail(dataMap, row);
        // 读取每一行数据并插入到record内
        database->insertRecordDetail(recordPtr);
    }
    // 读取剩下的数据并插入到record内
    database->SaveRecordDetail();
    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("End parse Memory Record: ", filePath, ", cost time:",
                    std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    return true;
}

void MemoryParse::Reset()
{
    ServerLog::Info("Memory reset. Wait task completed.");
    threadPool->Reset();
    ranks.clear();
    ServerLog::Info("Memory task completed.");
    auto databaseList = Timeline::DataBaseManager::Instance().GetAllMemoryDatabase();
    for (auto &db: databaseList) {
        db->ReleaseStmt();
        db->CloseDb();
    }

    Timeline::DataBaseManager::Instance().Clear(Timeline::DatabaseType::MEMORY);
}

std::vector<std::pair<std::string, MemoryFilePair>> MemoryParse::GetMemoryFiles(const std::vector<std::string>& paths)
{
    std::vector<std::string> fileList = {};
    for (const std::string& path : paths) {
        auto files = FileUtil::FindFilesWithFilter(path, std::regex(memoryOperatorReg));
        fileList.insert(fileList.end(), files.begin(), files.end());
    }
    if (fileList.empty()) {
        ServerLog::Warn("There is no memory operator file.");
        return {};
    }

    std::vector<std::pair<std::string, MemoryFilePair>> results = {};
    for (const auto& operatorFile : fileList) {
        // 寻找同级目录下的memory_record文件
        std::vector<std::string> folders;
        std::vector<std::string> files;
        if (!FileUtil::FindFolders(FileUtil::GetParentPath(operatorFile), folders, files)) {
            ServerLog::Warn("There is no file under ", FileUtil::GetParentPath(operatorFile));
            continue;
        }
        std::string recordFile;
        for (const auto& one : files) {
            if (RegexUtil::RegexMatch(one, memoryRecordReg)) {
                recordFile = FileUtil::SplicePath(FileUtil::GetParentPath(operatorFile), one);
                break;
            }
        }
        if (recordFile.empty()) {
            ServerLog::Warn("There is no memory record file paired with ", operatorFile);
            continue;
        }
        std::string fileId = FileUtil::GetProfilerFileId(operatorFile);
        int i = 1;
        std::string tempId = fileId;
        while (Timeline::DataBaseManager::Instance().HasFileId(Timeline::DatabaseType::MEMORY, tempId)) {
            tempId = fileId + "_" + std::to_string(++i);
        }
        ServerLog::Info("Operator memory file: ", operatorFile, ", memory record file: ", recordFile, ", Id: ", tempId);
        Timeline::DataBaseManager::Instance().GetMemoryDatabase(tempId);
        MemoryFilePair filePair = {operatorFile, recordFile};
        results.emplace_back(tempId, filePair);
        Protocol::MemorySuccess one = {tempId, false, true};
        ranks.emplace(tempId, one);
    }

    isCluster = (results.size() > 1);
    return results;
}

bool MemoryParse::Parse(const std::vector<std::string> &pathList, const std::string &token)
{
    auto memoryFiles = GetMemoryFiles(pathList);
    if (memoryFiles.empty()) {
        ServerLog::Warn("Memory file is empty.");
        return false;
    }
    SetParseCallBack(token);
    for (const auto& memoryFile : memoryFiles) {
        threadPool->AddTask(PreParseTask, memoryFile.second, memoryFile.first);
    }
    return true;
}

void MemoryParse::PreParseTask(const MemoryFilePair& filePair, const std::string& fileId)
{
    if (!InitParser(filePair, fileId)) {
        ServerLog::Warn("Failed to parse memory files for fileId:", fileId);
    }
}

bool MemoryParse::ParseTask(const MemoryFilePair& filePair, const std::string& fileId)
{
    Timeline::ParserStatusManager::Instance().SetRunningStatus(MEMORY_PREFIX + fileId);
    std::string operatorFile = filePair.operatorFile;
    std::string recordFile = filePair.recordFile;
    if (!MemoryParse::Instance().OperatorParse(operatorFile, fileId)) {
        ParseEndCallBack(fileId, false, "Failed to parse operator memory file: " + operatorFile);
        ServerLog::Error("Failed to parse operator memory file, path = ", operatorFile);
        return false;
    }

    if (!MemoryParse::Instance().RecordToParse(recordFile, fileId)) {
        ParseEndCallBack(fileId, false, "Failed to parse memory record file: " + recordFile);
        ServerLog::Error("Failed to parse operator record file, path = ", recordFile);
        return false;
    }

    ParseEndCallBack(fileId, true, "");
    Timeline::ParserStatusManager::Instance().SetFinishStatus(MEMORY_PREFIX + fileId);
    return true;
}

bool MemoryParse::InitParser(const MemoryFilePair& filePair, const std::string& fileId)
{
    Timeline::ParserStatusManager::Instance().SetParserStatus(MEMORY_PREFIX + fileId, Timeline::ParserStatus::INIT);
    std::string dbPath = FileUtil::GetDbPath(filePair.operatorFile, fileId);
    auto db = Timeline::DataBaseManager::Instance().GetMemoryDatabase(fileId);
    if (!(db->OpenDb(dbPath, false) && db->DropTable() &&
            db->CreateTable() && db->SetConfig() && db->InitStmt())) {
        ServerLog::Error("Failed to init memory database. Path:", dbPath);
        ParseEndCallBack(fileId, false, "Failed to init memory database for rank " + fileId);
        return false;
    }

    if (!ParseTask(filePair, fileId)) {
        ServerLog::Error("Failed to parse memory file. Path:", dbPath);
        return false;
    }

    return true;
}

void MemoryParse::SetParseCallBack(const std::string &token)
{
    std::function<void(const std::string, bool, const std::string)> func =
            std::bind(ParseCallBack, token, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    MemoryParse::Instance().SetParseEndCallBack(func);
}

void MemoryParse::ParseEndCallBack(const std::string &fileId, bool result, const std::string &message)
{
    // 错误处理逻辑后续增加
    if (!result) {
        return;
    }
    if (MemoryParse::Instance().ranks.count(fileId) == 0) {
        return;
    } else {
        MemoryParse::Instance().ranks[fileId].parseSuccess = true;
    }

    auto &instance = MemoryParse::Instance();
    if (instance.paserEndCallback != nullptr) {
        instance.paserEndCallback(fileId, result, message);
    }
}

void MemoryParse::ParseCallBack(const std::string &token, const std::string &fileId, bool result,
    const std::string &msg)
{
    WsSession *session = WsSessionManager::Instance().GetSession(token);
    if (session == nullptr) {
        ServerLog::Error("[Memory]Failed to get session token");
        return;
    }

    auto event = std::make_unique<Protocol::ParseMemoryCompletedEvent>();
    event->moduleName = Protocol::ModuleType::TIMELINE;
    event->token = token;
    event->result = true;
    event->isCluster = MemoryParse::Instance().isCluster;
    std::vector<Protocol::MemorySuccess> memoryResult;
    for (const auto& [rank, info] : MemoryParse::Instance().ranks) {
        memoryResult.push_back(info);
    }
    event->memoryResult = memoryResult;
    session->OnEvent(std::move(event));
}

} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic
