/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2024. All rights reserved.
 */

#include "pch.h"
#include "DataBaseManager.h"
#include "EventParser.h"
#include "ParserStatusManager.h"
#include "TraceTime.h"
#include "CacheManager.h"
#include "TraceFileSimulationParser.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;

TraceFileSimulationParser &TraceFileSimulationParser::Instance()
{
    static TraceFileSimulationParser instance;
    return instance;
}

TraceFileSimulationParser::TraceFileSimulationParser()
{
    threadPool = std::make_unique<ThreadPool>(TraceFileSimulationParser::maxThreadNum);
}

TraceFileSimulationParser::~TraceFileSimulationParser()
{
    threadPool->ShutDown();
}

bool TraceFileSimulationParser::Parse(const std::vector<std::string> &filePathArr, const std::string &fileId,
    const std::string &selectedFolder)
{
    ServerLog::Info("start parse. file id:", fileId);
    ParserStatusManager::Instance().SetParserStatus(fileId, ParserStatus::INIT);
    threadPool->AddTask(PreParseTask, filePathArr, fileId);
    return true;
}

void TraceFileSimulationParser::PreParseTask(const std::vector<std::string> &filePathArr, const std::string &fileId)
{
    if (!InitParser(filePathArr, fileId)) {
        auto msg = "Failed to open db. Please delete dbFile and try again or see logs.";
        ParseEndCallBack(fileId, false, msg);
    }
}

bool TraceFileSimulationParser::InitParser(const std::vector<std::string> &filePathArr, const std::string &fileId)
{
    if (!ParserStatusManager::Instance().SetRunningStatus(fileId)) {
        ServerLog::Info("Pre task skip this file.");
        return false;
    }
    auto db = DataBaseManager::Instance().GetTraceDatabase(fileId);
    if (db == nullptr) {
        ServerLog::Error("Failed to get connection. fileId: ", fileId);
        return false;
    }
    std::shared_ptr<TextTraceDatabase> database =
        std::dynamic_pointer_cast<TextTraceDatabase, VirtualTraceDatabase>(db);
    if (database == nullptr) {
        ServerLog::Error("Failed to convert virtual trace database to json trace database in event parser.");
        return false;
    }
    if (!(database->DropTable() && database->CreateTable())) {
        ServerLog::Error("Failed to open trace database. rankId:", fileId);
        return false;
    }
    auto &instance = TraceFileSimulationParser::Instance();
    auto start = std::chrono::high_resolution_clock::now();
    std::shared_ptr<std::vector<std::future<void>>> futures = std::make_shared<std::vector<std::future<void>>>();
    for (const auto &filePath : filePathArr) {
        ServerLog::Info("Start parse. file id:", fileId, ". path:", filePath);
        auto splitFile = TraceFileSimulationParser::SplitFile(filePath);
        instance.fileProgressMap[fileId] = std::make_unique<FileProgress>(0, FileUtil::GetFileSize(filePath.c_str()));
        if (splitFile.empty()) {
            ServerLog::Error("Failed to split file.");
            ParseEndCallBack(fileId, false, "Failed to split file: " + filePath);
            continue;
        }

        for (const auto &pos : splitFile) {
            auto future = instance.threadPool->AddTask(ParseTask, filePath, fileId, pos);
            futures->emplace_back(std::move(future));
        }
    }
    instance.threadPool->AddTask(EndParseTask, fileId, filePathArr, futures, start);
    return true;
}

void TraceFileSimulationParser::ParseTask(const std::string &filePath, const std::string &fileId,
    std::pair<int64_t, int64_t> pos)
{
    if (ParserStatusManager::Instance().GetParserStatus(fileId) != ParserStatus::RUNNING) {
        ServerLog::Info("Parse task skip this file. ID:", fileId);
        return;
    }
    auto db = DataBaseManager::Instance().GetTraceDatabase(fileId);
    if (db == nullptr) {
        ServerLog::Warn("Failed to get connection when parse simulation json,ID: ", fileId);
        return;
    }
    std::shared_ptr<TextTraceDatabase> databasePtr =
            std::dynamic_pointer_cast<TextTraceDatabase, VirtualTraceDatabase>(db);
    if (databasePtr == nullptr) {
        ServerLog::Warn("Failed to get text connection when parse simulation json,ID: ", fileId);
        return;
    }
    EventParser eventParser(filePath, fileId, databasePtr);
    eventParser.SetSimulationStatus(true);
    if (!eventParser.Parse(pos.first, pos.second)) {
        if (ParserStatusManager::Instance().SetTerminateStatus(fileId) == ParserStatus::RUNNING) {
            // 只发送一次解析失败事件
            ParseEndCallBack(fileId, false, eventParser.GetError());
        }
    }
    // 发送单卡解析进度事件
    auto &instance = TraceFileSimulationParser::Instance();
    std::unique_ptr<FileProgress> &curFileProgress = instance.fileProgressMap[fileId];
    curFileProgress->AddToParsedSize(pos.second - pos.first);
    instance.parseProgressCallback(fileId, curFileProgress->GetParsedSize(), curFileProgress->GetTotalSize(),
                                   curFileProgress->GetProgressPercentage());
}

void TraceFileSimulationParser::EndParseTask(const std::string &fileId, const std::vector<std::string> &filePathArr,
    std::shared_ptr<std::vector<std::future<void>>> futures,
    std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
    if (ParserStatusManager::Instance().GetParserStatus(fileId) != ParserStatus::RUNNING) {
        ParserStatusManager::Instance().SetFinishStatus(fileId);
        ServerLog::Info("End parse task skip this file. ID:", fileId);
        return;
    }
    ServerLog::Info("Wait parse completed. ID:", fileId);
    for (const auto &future : *futures) {
        future.wait();
    }
    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("Parse completed. ID:", fileId,
        " Cost time(ms): ", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    auto database = std::dynamic_pointer_cast<TextTraceDatabase, VirtualTraceDatabase>(
        DataBaseManager::Instance().GetTraceDatabase(fileId));
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:", fileId);
        ParserStatusManager::Instance().SetFinishStatus(fileId);
        return;
    }
    database->CreateIndex();
    database->SimulationUpdateProcessSortIndex();
    CacheManager::Instance().ClearCacheByFileId(fileId);
    ServerLog::Info("Update depth completed. ID:", fileId);
    ParseEndCallBack(fileId, true, "");
}

void TraceFileSimulationParser::ParseEndCallBack(const std::string &fileId, bool result, const std::string &message)
{
    auto oldStatus = ParserStatusManager::Instance().GetParserStatus(fileId);
    ParserStatusManager::Instance().SetFinishStatus(fileId);
    auto &instance = TraceFileSimulationParser::Instance();
    if (instance.parseEndCallback != nullptr && oldStatus != ParserStatus::TERMINATE) {
        ServerLog::Info("TraceFileSimulationParser send Message");
        instance.parseEndCallback(fileId, fileId, result, message);
    }
}

int64_t TraceFileSimulationParser::GetTrackId(const std::string &fileId, const std::string &pid, const std::string &tid)
{
    std::unique_lock<std::mutex> lock(trackMutex);
    auto item = std::make_pair(pid, tid);
    if (trackIdMap[fileId].count(item) > 0) {
        return trackIdMap[fileId].at(item);
    }
    if (trackId == INT64_MAX) {
        trackId = 0;
    }
    trackIdMap[fileId].emplace(item, ++trackId);
    return trackId;
}

void TraceFileSimulationParser::Reset()
{
    ServerLog::Info("Reset. wait task completed.");
    ParserStatusManager::Instance().SetAllTerminateStatus();
    threadPool->Reset();
    ServerLog::Info("Task completed.");
    auto connList = DataBaseManager::Instance().GetAllTraceDatabase();
    for (auto &conn : connList) {
        std::string path = conn->GetDbPath();
        conn->Stop();
        if (!FileUtil::RemoveFileExDb(path)) {
            ServerLog::Error("Failed to remove file. ", path);
        }
    }
    trackIdMap.clear();
    trackId = 0;
    DataBaseManager::Instance().Clear();
    TraceTime::Instance().Reset();
    FileParser::Reset();
    ParserStatusManager::Instance().ClearAllParserStatus();
    CacheManager::Instance().ClearAll();
    ServerLog::Info("End Reset trace Parser");
}

void TraceFileSimulationParser::DeleteParseFileFromDisk(const std::string &fileId)
{
    ServerLog::Info("Delete file. id:", fileId);
    ParserStatusManager::Instance().ClearParserStatus(fileId);
    std::string path = DataBaseManager::Instance().GetDbPath(fileId);
    DataBaseManager::Instance().ReleaseDatabase(fileId);
    if (!path.empty()) {
        FileUtil::RemoveFileExDb(path);
    }
}


void TraceFileSimulationParser::DeleteParseFiles(const std::vector<std::string> &fileIds)
{
    for (const auto &fileId : fileIds) {
        auto status = ParserStatusManager::Instance().SetTerminateStatus(fileId);
        ServerLog::Info("Before delete file. id:", fileId, ", status:", static_cast<int>(status));
    }
    ParserStatusManager::Instance().WaitAllFinished(fileIds);
    for (const auto &fileId : fileIds) {
        auto oldStatus = ParserStatusManager::Instance().GetParserStatus(fileId);
        ServerLog::Info("Delete file. id:", fileId, ", status:", static_cast<int>(oldStatus));
        if (oldStatus == ParserStatus::FINISH) {
            DeleteParseFileFromDisk(fileId);
        }
        CacheManager::Instance().ClearCacheByFileId(fileId);
    }
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
