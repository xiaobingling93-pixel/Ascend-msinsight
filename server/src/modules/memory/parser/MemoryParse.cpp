/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "MemoryParse.h"
#include "ServerLog.h"

#include "FileUtil.h"
#include "TraceFileParser.h"
#include "ParserStatusManager.h"
#include "DataBaseManager.h"
#include "FileDef.h"

namespace Dic {
namespace Module {
namespace Memory {
using namespace Dic::Server;
MemoryParse &MemoryParse::Instance()
{
    static MemoryParse instance;
    return instance;
}

bool MemoryParse::Parse(const std::vector<std::string> &filePaths, const std::string &fileId,
                        const std::string &selectedFolder)
{
    start = std::chrono::system_clock::now();
    ServerLog::Info("start parse.");

    auto database = Timeline::DataBaseManager::Instance().GetMemoryDatabase(fileId);
    std::string dbPath = FileUtil::GetDbPath(selectedFolder, fileId);
    if (!(database->OpenDb(dbPath, false) && database->CreateTable() &&
    database->SetConfig() && database->InitStmt())) {
        ServerLog::Error("Failed to open database. path:", dbPath);
        return false;
    }
    std::shared_ptr<std::vector<std::future<void>>> futures = std::make_unique<std::vector<std::future<void>>>();
    for (std::string filePath: filePaths) {
        auto future = threadPool->AddTask([futures, fileId, &filePath, this]() {
            ServerLog::Info("Wait parse completed. ID:", fileId);
            for (const auto &future: *futures) {
                future.wait();
            }
            ServerLog::Info("Parse completed. ID:", fileId);
            std::string parentDir = FileUtil::GetParentPath(filePath);
            OperatorParse(parentDir, fileId);
            RecordToParse(parentDir, fileId);
            ServerLog::Info("Update depth completed. ID:", fileId);
        });
        futureMap.emplace(fileId, std::move(future));
    }
    if (paserEndCallback != nullptr) {
        std::thread thread{[this, fileId]() {
            WaitParseEnd(fileId);
        }};
        thread.detach();
    }
    return true;
}

bool MemoryParse::WaitParseEnd(const std::string &fileId)
{
    if (futureMap.count(fileId) == 0) {
        return false;
    }
    ServerLog::Info("Wait memory parse completed. ID:", fileId);
    auto &future = futureMap.at(fileId);
    future.wait();
    futureMap.erase(fileId);
    auto dur = std::chrono::duration<double, std::milli>(std::chrono::system_clock::now() - start);
    ServerLog::Info("end parse. ID:", fileId, ". time:", dur.count());
    if (paserEndCallback != nullptr) {
        paserEndCallback(fileId, true);
    }
    return true;
}

void MemoryParse::OperatorParse(const std::string &parentDir, const std::string &fileId)
{
    auto start = std::chrono::high_resolution_clock::now();
    ServerLog::Info("start parse Operator.");
    std::vector<std::string> memoryFileVec = FileUtil::FindFilesByRegex(parentDir, std::regex(memoryOperatorReg));
    if (memoryFileVec.empty()) {
        ServerLog::Warn("There is no operator_memory.csv for rank " + fileId);
        return;
    }
    std::string operatorFile = memoryFileVec[0];
    auto memoryDatabase = Timeline::DataBaseManager::Instance().GetMemoryDatabase(fileId);
    std::ifstream file(operatorFile);
    std::string line;
    std::map<std::string, std::int16_t> dataMap;
    while (Timeline::ParserStatusManager::Instance().GetParserStatus(fileId) ==
    Timeline::ParserStatus::RUNNING && getline(file, line)) {
        std::stringstream ss(line);
        std::vector<std::string> row;
        std::string cell;

        while (getline(ss, cell, ',')) {
            row.push_back(cell);
        }
        if (row[0] == Dic::NAME || row[0] == Dic::DEVICE_ID) {
            for (int i = 0; i < row.size(); i++) {
                dataMap[row[i]] = i;
            }
            GetMapValid((row[0] == Dic::NAME ? OPERATOR_CSV : OPERATOR_CSV_MSPROF), dataMap);
            memoryDatabase->SetInferenceType(row[0] == Dic::DEVICE_ID);
            continue;
        }
        if (dataMap.size() < operatorTableNum) {
            ServerLog::Error("The header of the file is incorrect or incomplete. The path is: " + operatorFile);
            return;
        }
        Operator opePtr = MemoryParse::mapperToOperatorDetail(dataMap, row);
        // 读取每一行数据并插入到operator内
        memoryDatabase->insertOperatorDetail(opePtr);
    }
    // 读取剩下的数据并插入到operator内
    memoryDatabase->SaveOperatorDetail();
    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("end parse Operator, cost time:",
                    std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
}

void MemoryParse::GetMapValid(const std::vector<std::string>& vec, std::map<std::string, std::int16_t> dataMap)
{
    for (const std::string& col : vec) {
        if (dataMap.find(col) == dataMap.end()) {
            ServerLog::Error("The file lacks a parameter column : ", col);
        }
    }
}

MemoryParse::MemoryParse()
{
    threadPool = std::make_unique<ThreadPool>(MemoryParse::maxThreadNum);
}

Operator MemoryParse::mapperToOperatorDetail(std::map<std::string, std::int16_t> dataMap, std::vector<std::string> row)
{
    Operator anOperator {};
    std::int16_t nameIndex = dataMap[NAME];
    std::int16_t allocationTimeIndex = dataMap[ALLOCATION_TIME];
    std::int16_t sizeIndex = dataMap[SIZE];
    std::int16_t durationIndex = dataMap[DURATION];
    anOperator.name = row[nameIndex];
    anOperator.size = atof(row[sizeIndex].c_str());
    anOperator.allocationTime = atof(row[allocationTimeIndex].c_str());
    anOperator.duration = atof(row[durationIndex].c_str());

    if (dataMap.count(RELEASE_TIME)) {
        std::int16_t releaseTimeIndex = dataMap[RELEASE_TIME];
        anOperator.releaseTime = atof(row[releaseTimeIndex].c_str());
    } else {
        anOperator.releaseTime = anOperator.allocationTime + anOperator.duration;
    }

    return anOperator;
}

Record MemoryParse::mapperToRecordDetail(std::map<std::string, std::int16_t> dataMap, std::vector<std::string> row)
{
    Record record {};
    std::int16_t nameIndex = dataMap[COMPONENT];
    std::int16_t timeStampIndex = dataMap[TIMESTAMP];
    record.component = row[nameIndex];
    record.timesTamp = atof(row[timeStampIndex].c_str());
    // msprof场景
    if (dataMap.count(Dic::DEVICE_ID)) {
        std::int16_t totalAllocatedIndex = dataMap[TOTAL_ALLOCATED_KB];
        std::int16_t totalReservedIndex = dataMap[TOTAL_RESERVED_KB];
        std::int16_t deviceTypeIndex = dataMap[DEVICE];
        record.totalAllocated = atof(row[totalAllocatedIndex].c_str()) / KB_TO_MB;
        record.totalReserved = atof(row[totalReservedIndex].c_str()) / KB_TO_MB;
        record.deviceType = row[deviceTypeIndex];
    } else {
        std::int16_t totalAllocatedIndex = dataMap[TOTAL_ALLOCATED_MB];
        std::int16_t totalReservedIndex = dataMap[TOTAL_RESERVED_MB];
        std::int16_t deviceTypeIndex = dataMap[DEVICETYPE];
        record.totalAllocated = atof(row[totalAllocatedIndex].c_str());
        record.totalReserved = atof(row[totalReservedIndex].c_str());
        record.deviceType = row[deviceTypeIndex];
    }
    return record;
}

void MemoryParse::RecordToParse(const std::string &parentDir, const std::string &fileId)
{
    auto start = std::chrono::high_resolution_clock::now();
    ServerLog::Info("start parse Record.");
    std::vector<std::string> memoryFileVec = FileUtil::FindFilesByRegex(parentDir, std::regex(memoryRecordReg));
    if (memoryFileVec.empty()) {
        ServerLog::Warn("There is no memory_record.csv for rank " + fileId);
        return;
    }
    std::string recordFile = memoryFileVec[0];
    auto database = Timeline::DataBaseManager::Instance().GetMemoryDatabase(fileId);
    std::ifstream file(recordFile);
    std::string line;
    std::map<std::string, std::int16_t> dataMap;
    while (Timeline::ParserStatusManager::Instance().GetParserStatus(fileId) ==
    Timeline::ParserStatus::RUNNING && getline(file, line)) {
        std::stringstream ss(line);
        std::vector<std::string> row;
        std::string cell;
        while (getline(ss, cell, ',')) {
            row.push_back(cell);
        }
        if (row[0] == Dic::COMPONENT || row[0] == Dic::DEVICE_ID) {
            for (int i = 0; i < row.size(); i++) {
                dataMap[row[i]] = i;
            }
            GetMapValid((row[0] == Dic::COMPONENT ? RECORD_CSV : RECORD_CSV_MSPROF), dataMap);
            continue;
        }
        if (dataMap.size() < recordTableNum) {
            ServerLog::Error("The header of the imported file is incorrect or incomplete. The path is: " + recordFile);
            return;
        }
        Record recordPtr = MemoryParse::mapperToRecordDetail(dataMap, row);
        // 读取每一行数据并插入到record内
        database->insertRecordDetail(recordPtr);
    }
    // 读取剩下的数据并插入到record内
    database->SaveRecordDetail();
    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("end parse Record, cost time:",
                    std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
}

MemoryParse::~MemoryParse()
{
    threadPool->ShutDown();
}

void MemoryParse::Reset()
{
    ServerLog::Info("Reset. wait task completed.");
    threadPool->Reset();
    ServerLog::Info("Task completed.");
    auto databaseList = Timeline::DataBaseManager::Instance().GetAllMemoryDatabase();
    for (auto &db: databaseList) {
        std::string path = db->GetDbPath();
        db->ReleaseStmt();
        db->CloseDb();
        if (!FileUtil::RemoveFile(path)) {
            ServerLog::Error("Failed to remove file. ", path);
        }
    }
    Timeline::DataBaseManager::Instance().Clear();
}

} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic
