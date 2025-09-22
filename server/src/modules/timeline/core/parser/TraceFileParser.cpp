/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2024. All rights reserved.
 */

#include "pch.h"
#include "DataBaseManager.h"
#include "EventParser.h"
#include "ParserStatusManager.h"
#include "TraceTime.h"
#include "ClusterParseThreadPoolExecutor.h"
#include "EventNotifyThreadPoolExecutor.h"
#include "CacheManager.h"
#include "TrackInfoManager.h"
#include "BaselineManager.h"
#include "TraceFileParser.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;
TraceFileParser &TraceFileParser::Instance()
{
    static TraceFileParser instance;
    return instance;
}

TraceFileParser::TraceFileParser()
{
    threadPool = std::make_unique<ThreadPool>(TraceFileParser::maxThreadNum);
}

TraceFileParser::~TraceFileParser()
{
    threadPool->ShutDown();
}

bool TraceFileParser::Parse(const std::vector<std::string> &filePathArr,
                            const std::string &rankId,
                            const std::string &selectedFolder,
                            const std::string &fileId)
{
    ServerLog::Info("start parse. file id:", fileId);
    ParserStatusManager::Instance().SetParserStatus(rankId, ParserStatus::INIT);
    threadPool->AddTask(PreParseTask, TraceIdManager::GetTraceId(), filePathArr, rankId, fileId);
    return true;
}

void TraceFileParser::PreParseTask(const std::vector<std::string> &filePathArr,
                                   const std::string &rankId,
                                   const std::string &fileId)
{
    if (!InitParser(filePathArr, rankId, fileId)) {
        auto msg = "Failed to open db. Please delete dbFile and try again or see logs in " +
                   ServerLog::GetCurrentLogPath();
#if defined(__linux__) || defined(__APPLE__)
        msg += FILE_DESCRIPTOR_RUN_OUT_MESSAGE;
#endif
        ParseEndCallBack(rankId, "", false, msg);
    }
}

bool TraceFileParser::CheckInitParser(const std::string &fileId)
{
    if (!ParserStatusManager::Instance().SetRunningStatus(fileId)) {
        ServerLog::Info("Pre task skip this file.");
        return false;
    }
    return true;
}

bool TraceFileParser::InitParser(const std::vector<std::string> &filePathArr,
                                 const std::string &rankId,
                                 const std::string &fileId)
{
    if (!CheckInitParser(rankId)) {
        return false;
    }
    auto db = DataBaseManager::Instance().GetTraceDatabaseByFileId(fileId);
    if (db == nullptr) {
        ServerLog::Error("Failed to get connection.");
        return false;
    }
    auto database = std::dynamic_pointer_cast<TextTraceDatabase, VirtualTraceDatabase>(db);
    if (database == nullptr) {
        ServerLog::Error("Failed to open trace database. rankId:", rankId);
        return false;
    }
    std::string statusInfo = ComputeStatusInfoFromPathArr(filePathArr);
    if ((database->HasFinishedParseLastTime(statusInfo) &&
        !Global::BaselineManager::Instance().IsBaselineRankId(rankId)) ||
        StringUtil::EndWith(filePathArr[0], "profiler.db")) {
        uint64_t min = UINT64_MAX;
        uint64_t max = 0;
        if (!database->QueryExtremumTimestamp(min, max)) {
            return false;
        }
        auto threadMap = database->QueryAllThreadMap();
        database->ExecSql("ALTER TABLE process ADD COLUMN parentPid TEXT DEFAULT '0';");
        TrackInfoManager::Instance().UpdateTrackIdMap(rankId, threadMap);
        Timeline::TraceTime::Instance().UpdateTime(min, 0);
        Timeline::TraceTime::Instance().UpdateCardTimeDuration(rankId, min, max);
        ParseEndCallBack(rankId, fileId, true, "");
        ParserStatusManager::Instance().SetFinishStatus(rankId);
        return true;
    }
    if (!database->DropTable() || !database->CreateTable() || !database->UpdateParseStatus(NOT_FINISH_STATUS)) {
        ServerLog::Error("Failed to init trace database. rankId:", rankId);
        return false;
    }
    InitFileProcess(filePathArr, rankId);
    return true;
}

std::string TraceFileParser::ComputeStatusInfoFromPathArr(const std::vector<std::string> &filePathArr)
{
    std::vector<std::string> tempPathArr(filePathArr);
    std::sort(tempPathArr.begin(), tempPathArr.end());
    std::string statusInfo = StringUtil::join(tempPathArr, ",");
    return statusInfo;
}

void TraceFileParser::InitFileProcess(const std::vector<std::string> &filePathArr, const std::string &fileId)
{
    auto &instance = Instance();
    auto start = std::chrono::high_resolution_clock::now();
    std::shared_ptr<std::vector<std::future<void>>> futures = std::make_shared<std::vector<std::future<void>>>();
    for (const auto &filePath : filePathArr) {
        ServerLog::Info("Start parse. file id:", fileId, ". path:", filePath);
        auto splitFile = SplitFile(filePath);
        instance.fileProgressMap[fileId] = std::make_unique<FileProgress>(0, FileUtil::GetFileSize(filePath.c_str()));
        if (splitFile.empty()) {
            ServerLog::Error("Failed to split file. filePath: %", filePath);
            ParseEndCallBack(fileId, "", false, "Failed to split file: " + filePath);
            continue;
        }

        for (const auto &pos : splitFile) {
            auto future = instance.threadPool->AddTask(ParseTask, TraceIdManager::GetTraceId(), filePath, fileId, pos);
            futures->emplace_back(std::move(future));
        }
    }
    instance.threadPool->AddTask(EndParseTask, TraceIdManager::GetTraceId(), fileId, filePathArr, futures, start);
}

void TraceFileParser::ParseTask(const std::string &filePath, const std::string &fileId, std::pair<int64_t, int64_t> pos)
{
    if (ParserStatusManager::Instance().GetParserStatus(fileId) != ParserStatus::RUNNING) {
        ServerLog::Info("Parse task skip this file. ID:", fileId);
        return;
    }
    auto db = DataBaseManager::Instance().GetTraceDatabaseByRankId(fileId);
    if (db == nullptr) {
        ServerLog::Warn("Failed to get connection when parse timeline json,ID: ", fileId);
        return;
    }
    std::shared_ptr<TextTraceDatabase> databasePtr =
            std::dynamic_pointer_cast<TextTraceDatabase, VirtualTraceDatabase>(db);
    if (databasePtr == nullptr) {
        ServerLog::Warn("Failed to get text connection when parse timeline json,ID: ", fileId);
        return;
    }
    EventParser eventParser(filePath, fileId, databasePtr);
    if (!eventParser.Parse(pos.first, pos.second)) {
        if (ParserStatusManager::Instance().SetTerminateStatus(fileId) == ParserStatus::RUNNING) {
            // 只发送一次解析失败事件
            ParseEndCallBack(fileId, "", false, eventParser.GetError());
        }
    }
    // 发送单卡解析进度事件
    auto &instance = TraceFileParser::Instance();
    std::unique_ptr<FileProgress> &curFileProgress = instance.fileProgressMap[fileId];
    curFileProgress->AddToParsedSize(pos.second - pos.first);
    instance.parseProgressCallback(fileId, curFileProgress->GetParsedSize(), curFileProgress->GetTotalSize(),
                                   curFileProgress->GetProgressPercentage());
}

void TraceFileParser::EndParseTask(const std::string &rankId, const std::vector<std::string> &filePathArr,
                                   std::shared_ptr<std::vector<std::future<void>>> futures,
                                   std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
    if (ParserStatusManager::Instance().GetParserStatus(rankId) != ParserStatus::RUNNING) {
        ParserStatusManager::Instance().SetFinishStatus(rankId);
        ServerLog::Info("End parse task skip this file. ID:", rankId);
        return;
    }
    ServerLog::Info("Wait parse completed. ID:", rankId);
    for (const auto &future : *futures) {
        future.wait();
    }
    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("Parse completed. ID:", rankId,
                    " Cost time(ms): ", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    auto db = DataBaseManager::Instance().GetTraceDatabaseByRankId(rankId);
    if (db == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:", rankId);
        ParserStatusManager::Instance().SetFinishStatus(rankId);
        return;
    }
    auto database = std::dynamic_pointer_cast<TextTraceDatabase, VirtualTraceDatabase>(db);
    if (database == nullptr) {
        ServerLog::Error("Failed to convert virtual trace database to json trace database in end parse task.");
        return;
    }
    database->CreateIndex();
    database->DeleteEmptyThread();
    database->DeleteEmptyFlow();
    std::string statusInfo = ComputeStatusInfoFromPathArr(filePathArr);
    database->UpdateParseStatus(statusInfo);
    std::vector<std::string> taskStatusList = {CONNECTION_UNIT, WAIT_TIME_UNIT, OVERLAP_ANALYSIS_UNIT};
    for (const auto &item: taskStatusList) {
        database->UpdateValueIntoStatusInfoTable(item, FINISH_STATUS);
    }
    ServerLog::Info("Update depth completed. ID:", rankId);
    ParseEndCallBack(rankId, database->GetDbPath(), true, "");
    ParserStatusManager::Instance().SetFinishStatus(rankId);
}

void TraceFileParser::ParseEndCallBack(const std::string &rankId,
                                       const std::string &fileId,
                                       bool result,
                                       const std::string &message)
{
    auto oldStatus = ParserStatusManager::Instance().GetParserStatus(rankId);
    auto &instance = TraceFileParser::Instance();
    if (instance.parseEndCallback != nullptr && oldStatus != ParserStatus::TERMINATE) {
        instance.parseEndCallback(rankId, fileId, result, message);
    }
}

int64_t TraceFileParser::GetTrackId(const std::string &fileId, const std::string &pid, const std::string &tid)
{
    std::unique_lock<std::mutex> lock(trackMutex);
    auto item = std::make_pair(pid, tid);
    if (trackIdMap[fileId].count(item) > 0) {
        return trackIdMap[fileId].at(item);
    }
    if (trackId == UINT64_MAX) {
        trackId = 0;
    }
    trackIdMap[fileId].emplace(item, ++trackId);
    return trackId;
}

void TraceFileParser::Reset()
{
    ServerLog::Info("Reset. wait task completed.");
    ParserStatusManager::Instance().SetAllTerminateStatus();
    threadPool->Reset();
    ClusterParseThreadPoolExecutor::Instance().GetThreadPool()->Reset();
    EventNotifyThreadPoolExecutor::Instance().GetThreadPool()->Reset();
    ServerLog::Info("Task completed.");
    auto connList = DataBaseManager::Instance().GetAllTraceDatabase();
    for (auto &conn : connList) {
        std::string path = conn->GetDbPath();
        conn->Stop();
    }
    DataBaseManager::Instance().ClearClusterDb();
    trackIdMap.clear();
    TrackInfoManager::Instance().Reset();
    trackId = 0;
    DataBaseManager::Instance().Clear();
    TraceTime::Instance().Reset();
    FileParser::Reset();
    ParserStatusManager::Instance().ClearAllParserStatus();
    CacheManager::Instance().ClearAll();
    ServerLog::Info("End Reset trace Parser");
}

void TraceFileParser::DeleteParseFileFromDisk(const std::string &fileId)
{
    ServerLog::Info("Delete file. id:", fileId);
    ParserStatusManager::Instance().ClearParserStatus(fileId);
    std::string path = DataBaseManager::Instance().GetDbPathByRankId(fileId);
    DataBaseManager::Instance().ReleaseDatabaseByRankId(fileId);
    if (!path.empty()) {
        FileUtil::RemoveFileExDb(path);
    }
}


void TraceFileParser::DeleteParseFiles(const std::vector<std::string> &fileIds)
{
    for (const auto &fileId : fileIds) {
        auto status = ParserStatusManager::Instance().SetTerminateStatus(fileId);
        auto kernelStatus = ParserStatusManager::Instance().SetTerminateStatus(KERNEL_PREFIX + fileId);
        auto memoryStatus = ParserStatusManager::Instance().SetTerminateStatus(MEMORY_PREFIX + fileId);
        ServerLog::Info("Before delete file. id:", fileId, ", status:", static_cast<int>(status),
            ", kernelStatus:", static_cast<int>(kernelStatus), ", memoryStatus:", static_cast<int>(memoryStatus));
    }
    ParserStatusManager::Instance().WaitAllFinished(fileIds);
    for (const auto &fileId : fileIds) {
        auto oldStatus = ParserStatusManager::Instance().GetParserStatus(fileId);
        ServerLog::Info("Delete file. id:", fileId, ", status:", static_cast<int>(oldStatus));
        if (oldStatus == ParserStatus::FINISH) {
            DeleteParseFileFromDisk(fileId);
        }
        CacheManager::Instance().ClearCacheByRankId(fileId);
    }
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
