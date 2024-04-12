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
#include "TraceTime.h"

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
    ServerLog::Info("Start parse Operator Memory: ", filePath, ", FileId: ", fileId);
    if (!ValidateUtil::CheckCsvFile(filePath)) {
        return false;
    }
    auto memoryDatabase =
            dynamic_cast<JsonMemoryDataBase*>(Timeline::DataBaseManager::Instance().GetMemoryDatabase(fileId));
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
        if (!row.empty() && (row[0] == Dic::NAME || row[0] == Dic::DEVICE_ID)) {
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
    uint64_t minStartTime = memoryDatabase->QueryMinOperatorAllocationTime();
    Timeline::TraceTime::Instance().UpdateTime(minStartTime, 0);
    file.close();
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

    if (dataMap.find(ALLOCATION_ACTIVE_MB) != dataMap.end()) {
        anOperator.activeDuration = atof(row[dataMap[ACTIVE_DURATION]].c_str());
        anOperator.activeReleaseTime = NumberUtil::TimestampUsToNs(
            NumberUtil::StringToLongDouble(row[dataMap[ACTIVE_RELEASE_TIME]]));
        anOperator.allocationActive = atof(row[dataMap[ALLOCATION_ACTIVE_MB]].c_str());
        anOperator.releaseActive = atof(row[dataMap[RELEASE_ACTIVE_MB]].c_str());
        anOperator.streamId = row[dataMap[STREAM_PTR]];
    }
    if (dataMap.find(ALLOCATION_ALLOCATED_KB) != dataMap.end()) {
        anOperator.allocationAllocated = atof(row[dataMap[ALLOCATION_ALLOCATED_KB]].c_str()) / KB_TO_MB;
        anOperator.allocationReserved = atof(row[dataMap[ALLOCATION_RESERVED_KB]].c_str()) / KB_TO_MB;
        anOperator.releaseAllocated = atof(row[dataMap[RELEASE_ALLOCATED_KB]].c_str()) / KB_TO_MB;
        anOperator.releaseReserved = atof(row[dataMap[RELEASE_RESERVED_KB]].c_str()) / KB_TO_MB;
    } else {
        anOperator.allocationAllocated = atof(row[dataMap[ALLOCATION_ALLOCATED_MB]].c_str());
        anOperator.allocationReserved = atof(row[dataMap[ALLOCATION_RESERVED_MB]].c_str());
        anOperator.releaseAllocated = atof(row[dataMap[RELEASE_ALLOCATED_MB]].c_str());
        anOperator.releaseReserved = atof(row[dataMap[RELEASE_RESERVED_MB]].c_str());
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
        record.totalActivated = dataMap.count(TOTAL_ACTIVE_KB) == 0 ?
                0 : atof(row[dataMap[TOTAL_ACTIVE_KB]].c_str()) / KB_TO_MB;
        record.deviceType = row[deviceTypeIndex];
    } else {
        size_t totalAllocatedIndex = dataMap[TOTAL_ALLOCATED_MB];
        size_t totalReservedIndex = dataMap[TOTAL_RESERVED_MB];
        size_t deviceTypeIndex = dataMap[DEVICETYPE];
        record.totalAllocated = atof(row[totalAllocatedIndex].c_str());
        record.totalReserved = atof(row[totalReservedIndex].c_str());
        record.totalActivated = dataMap.count(TOTAL_ACTIVE_MB) == 0 ?
                0 : atof(row[dataMap[TOTAL_ACTIVE_MB]].c_str());
        record.deviceType = row[deviceTypeIndex];
    }
    if (dataMap.find(STREAM_PTR) != dataMap.end()) {
        record.streamId = row[dataMap[STREAM_PTR]];
    }
    return record;
}

bool MemoryParse::RecordToParse(const std::string &filePath, const std::string &fileId)
{
    auto start = std::chrono::high_resolution_clock::now();
    ServerLog::Info("Start parse Memory Record: ", filePath, ", FileId: ", fileId);
    if (!ValidateUtil::CheckCsvFile(filePath)) {
        return false;
    }
    auto database =
            dynamic_cast<JsonMemoryDataBase*>(Timeline::DataBaseManager::Instance().GetMemoryDatabase(fileId));
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
        if (!row.empty() && (row[0] == Dic::COMPONENT || row[0] == Dic::DEVICE_ID)) {
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
    uint64_t minTimestamp = database->QueryMinRecordTimestamp();
    Timeline::TraceTime::Instance().UpdateTime(minTimestamp, 0);
    file.close();
    return true;
}

void MemoryParse::Reset()
{
    ServerLog::Info("Memory reset. Wait task completed.");
    ParseEndCallBack("", true, "");
    threadPool->Reset();
    ranks.clear();
    ServerLog::Info("Memory task completed.");
    auto databaseList = Timeline::DataBaseManager::Instance().GetAllMemoryDatabase();
    for (auto &db: databaseList) {
        auto database = dynamic_cast<JsonMemoryDataBase*>(db);
        database->ReleaseStmt();
        database->CloseDb();
    }

    Timeline::DataBaseManager::Instance().Clear(Timeline::DatabaseType::MEMORY);
}

std::vector<std::string> MemoryParse::GetPeerDirRecordFile(const std::string& operatorFile)
{
    std::vector<std::string> recordFiles;
    std::vector<std::string> folders;
    std::vector<std::string> files;
    if (!FileUtil::FindFolders(FileUtil::GetParentPath(operatorFile), folders, files)) {
        ServerLog::Warn("There is no file under ", FileUtil::GetParentPath(operatorFile));
        return recordFiles;
    }

    for (const auto& one : files) {
        if (RegexUtil::RegexMatch(one, memoryRecordReg)) {
            recordFiles.push_back(FileUtil::SplicePath(FileUtil::GetParentPath(operatorFile), one));
            if (!RegexUtil::RegexSearch(one, SLICE_STR).has_value()) {
                break;
            }
        }
    }

    return recordFiles;
}

std::map<std::string, MemoryFilePairs> MemoryParse::GetMemoryFiles(const std::vector<std::string>& paths)
{
    std::vector<std::string> fileList = {};
    for (const std::string& path : paths) {
        std::vector<std::string> files = {};
        if (FileUtil::IsFolder(path)) {
            files = FileUtil::FindFilesWithFilter(path, std::regex(memoryOperatorReg));
        } else { // 当选中其中operator或record单个文件时，搜索所在目录下的文件
            files = FileUtil::FindFilesWithFilter(FileUtil::GetParentPath(path), std::regex(memoryOperatorReg));
        }
        fileList.insert(fileList.end(), files.begin(), files.end());
    }
    if (fileList.empty()) {
        ServerLog::Warn("There is no memory operator file.");
        return {};
    }

    std::map<std::string, MemoryFilePairs> results = {};
    for (const auto& operatorFile : fileList) {
        std::vector<std::string> recordFiles = GetPeerDirRecordFile(operatorFile);
        if (recordFiles.empty()) {
            ServerLog::Warn("There is no memory record file paired with ", operatorFile);
            continue;
        }
        std::string fileId = FileUtil::GetProfilerFileId(operatorFile);
        int i = 1;
        std::string tempId = fileId;
        while (Timeline::DataBaseManager::Instance().HasFileId(Timeline::DatabaseType::MEMORY, tempId)) {
            std::string dbPath = Timeline::DataBaseManager::Instance().GetMemoryDatabase(tempId)->GetDbPath();
            if (RegexUtil::RegexSearch(FileUtil::GetFileName(operatorFile), SLICE_STR).has_value() &&
                    FileUtil::GetParentPath(operatorFile) == FileUtil::GetParentPath(dbPath)) {
                break;
            }
            tempId = fileId + "_" + std::to_string(++i);
        }
        std::string dbPath = FileUtil::GetDbPath(operatorFile, tempId);
        Timeline::DataBaseManager::Instance().GetMemoryDatabase(tempId)->SetDbPath(dbPath);
        results[tempId].operatorFiles.insert(operatorFile);
        results[tempId].recordFiles.insert(recordFiles.begin(), recordFiles.end());
        if (ranks.count(tempId) == 0) {
            Protocol::MemorySuccess one = {tempId, false, true};
            ranks.emplace(tempId, one);
        }
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
    if (memoryFiles.size() > 1) {
        ParseEndCallBack("", true, "");
    }
    for (const auto& memoryFile : memoryFiles) {
        Timeline::ParserStatusManager::Instance().SetParserStatus(MEMORY_PREFIX + memoryFile.first,
                                                                  Timeline::ParserStatus::INIT);
        threadPool->AddTask(PreParseTask, memoryFile.second, memoryFile.first);
    }
    return true;
}

void MemoryParse::PreParseTask(const MemoryFilePairs& filePair, const std::string& fileId)
{
    std::string message;
    if (!InitParser(filePair, fileId, message)) {
        ServerLog::Error("Failed to parse memory files for fileId:", fileId, "reason: ", message);
        ParseEndCallBack(fileId, false, message);
    }
}

bool MemoryParse::ParseTask(const MemoryFilePairs& filePair, const std::string& fileId, std::string &message)
{
    if (!Timeline::ParserStatusManager::Instance().SetRunningStatus(MEMORY_PREFIX + fileId)) {
        message = "Failed to set run memory status for file ";
        // 如果文件解析信息不存在或状态不为INIT则返回false
        return false;
    }
    std::set<std::string> operatorFiles = filePair.operatorFiles;
    std::set<std::string> recordFiles = filePair.recordFiles;
    std::vector<std::string> files;
    std::copy(operatorFiles.begin(), operatorFiles.end(), std::back_inserter(files));
    std::copy(recordFiles.begin(), recordFiles.end(), std::back_inserter(files));
    ServerLog::Info("Memory file: ", StringUtil::join(files, ", "), ", FileId: ", fileId);
    if (!ValidateUtil::CheckCsvFileList(files)) {
        message = "Failed to parse memory file: " + fileId + " due to access or file size.";
        return false;
    }

    for (const auto& operatorFile : operatorFiles) {
        if (MemoryParse::Instance().OperatorParse(operatorFile, fileId)) {
            continue;
        }
        message = "Failed to parse operator memory file, path = " + operatorFile;
        return false;
    }

    for (const auto& recordFile : recordFiles) {
        if (MemoryParse::Instance().RecordToParse(recordFile, fileId)) {
            continue;
        }
        message = "Failed to parse operator record file, path = " + recordFile;
        return false;
    }

    ParseEndCallBack(fileId, true, "");
    Timeline::ParserStatusManager::Instance().SetFinishStatus(MEMORY_PREFIX + fileId);
    return true;
}

bool MemoryParse::InitParser(const MemoryFilePairs& filePair, const std::string& fileId, std::string &message)
{
    if (filePair.operatorFiles.empty()) {
        return false;
    }
    std::string dbPath = FileUtil::GetDbPath(*(filePair.operatorFiles.begin()), fileId);
    auto db = dynamic_cast<JsonMemoryDataBase*>(Timeline::DataBaseManager::Instance().GetMemoryDatabase(fileId));
    if (!(db->OpenDb(dbPath, false) && db->DropTable() &&
            db->CreateTable() && db->SetConfig() && db->InitStmt())) {
        message = "Failed to init memory database. Path:" + dbPath;
        return false;
    }

    if (!ParseTask(filePair, fileId, message)) {
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
    Timeline::ParserStatusManager::Instance().SetFinishStatus(MEMORY_PREFIX + fileId);
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

    // 如果输入fileId
    if (fileId.empty()) {
        MemoryParse::Instance().ranks.clear();
        auto event = std::make_unique<Protocol::ModuleResetEvent>();
        event->moduleName = Protocol::ModuleType::MEMORY;
        event->token = token;
        event->result = true;
        event->reset = true;
        session->OnEvent(std::move(event));
    } else {
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
}

} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic
