/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include <fstream>
#include "ServerLog.h"
#include "RegexUtil.h"
#include "FileUtil.h"
#include "JsonUtil.h"
#include "StringUtil.h"
#include "DataBaseManager.h"
#include "EventParser.h"
#include "ParserStatusManager.h"
#include "TraceTime.h"
#include "MemoryParse.h"
#include "KernelParse.h"
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

bool TraceFileParser::Parse(const std::vector<std::string> &filePathArr, const std::string &fileId,
                            const std::string &selectedFolder)
{
    ServerLog::Info("start parse.");
    ParserStatusManager::Instance().SetParserStatus(fileId, ParserStatus::INIT);
    threadPool->AddTask(PreParseTask, filePathArr, fileId);
    return true;
}

void TraceFileParser::PreParseTask(const std::vector<std::string> &filePathArr, const std::string &fileId)
{
    if (!ParserStatusManager::Instance().SetRunningStatus(fileId)) {
        ServerLog::Info("Pre task skip this file.");
        return;
    }
    std::string dbPath = GetDbPath(filePathArr[0], fileId);
    if (!InitDatabase(dbPath, fileId)) {
        ServerLog::Error("Failed to Initial database.");
        return;
    }
    auto &instance = TraceFileParser::Instance();
    std::shared_ptr<std::vector<std::future<void>>> futures = std::make_unique<std::vector<std::future<void>>>();
    for (const auto &filePath: filePathArr) {
        auto splitFile = TraceFileParser::SplitFile(filePath);
        if (splitFile.empty()) {
            ServerLog::Error("Failed to split file.");
            ParseEndCallBack(fileId, false);
            continue;
        }

        for (const auto &pos : splitFile) {
            auto future = instance.threadPool->AddTask(ParseTask, filePath, fileId, dbPath, pos);
            futures->emplace_back(std::move(future));
        }

        std::string parentDir = FileUtil::GetParentPath(filePath);
        if (!parentDir.empty()) {
            Memory::MemoryParse::Instance().OperatorParse(parentDir, fileId);
            Memory::MemoryParse::Instance().RecordToParse(parentDir, fileId);
            Summary::KernelParse::Instance().KernelFileParse(parentDir, fileId);
        }
    }
    instance.threadPool->AddTask(EndParseTask, fileId, futures);
}

void TraceFileParser::ParseTask(const std::string &filePath, const std::string &fileId,
                                const std::string &dbPath, std::pair<int64_t, int64_t> pos)
{
    if (ParserStatusManager::Instance().GetParserStatus(fileId) != ParserStatus::RUNNING) {
        ServerLog::Info("Parse task skip this file. ID:", fileId);
        return;
    }
    EventParser eventParser(filePath, dbPath, fileId);
    eventParser.Parse(pos.first, pos.second);
}

void TraceFileParser::EndParseTask(const std::string &fileId, std::shared_ptr<std::vector<std::future<void>>> futures)
{
    if (ParserStatusManager::Instance().GetParserStatus(fileId) != ParserStatus::RUNNING) {
        DeleteParseFileFromDisk(fileId);
        ServerLog::Info("End parse task skip this file. ID:", fileId);
        return;
    }
    ServerLog::Info("Wait parse completed. ID:", fileId);
    for (const auto &future : *futures) {
        future.wait();
    }
    ServerLog::Info("Parse completed. ID:", fileId);
    auto database = DataBaseManager::Instance().GetTraceDatabase(fileId);
    database->CreateIndex();
    database->UpdateDepth();
    database->updateOverlapDuration();
    ServerLog::Info("Update depth completed. ID:", fileId);
    ParseEndCallBack(fileId, true);
}

void TraceFileParser::ParseEndCallBack(const std::string &fileId, bool result)
{
    if (!(result && ParserStatusManager::Instance().SetFinishStatus(fileId))) {
        DeleteParseFileFromDisk(fileId);
        result = false;
    }
    auto &instance = TraceFileParser::Instance();
    if (instance.paserEndCallback != nullptr) {
        instance.paserEndCallback(fileId, result);
    }
}

std::vector<std::pair<int64_t, int64_t>> TraceFileParser::SplitFile(const std::string &filePath)
{
#ifdef _WIN32
    std::string path(filePath);
    if (StringUtil::IsUtf8String(filePath)) {
        path = StringUtil::Utf8ToGbk(filePath.c_str());
    }
    std::ifstream file(path, std::ios::in);
#else
    std::ifstream file(filePath, std::ios::in);
#endif
    std::vector<std::pair<int64_t, int64_t>> result;
    if (!file.is_open()) {
        ServerLog::Error("Failed to open file. ", filePath);
        return result;
    }
    file.seekg(0, std::ifstream::end);
    int64_t fileSize = file.tellg();
    file.clear();
    file.seekg(0, std::ios::beg);
    bool endFlag = false;
    while (!endFlag) {
        if (!SeekCharPosition(file, '{')) {
            ServerLog::Info("Failed to find start position.");
            break;
        }
        int64_t start = file.tellg();
        std::string endRegex;
        if (start + blockSize >= fileSize) {
            file.seekg(0 - bufferLength, std::ifstream::end);
            endRegex = R"(\}\s*\])";
            endFlag = true;
        } else {
            file.seekg(blockSize, std::ifstream::cur);
            endRegex = R"(\}\s*,\s\{)";
        }
        if (!SeekRegexPosition(file, endRegex)) {
            ServerLog::Info("Failed to find end position.");
            break;
        }
        int64_t end = file.tellg();
        result.emplace_back(start, end);
    }
    file.close();
    return result;
}

bool TraceFileParser::SeekCharPosition(std::ifstream &file, char c)
{
    auto cur = file.tellg();
    std::unique_ptr<char[]> buffer = std::make_unique<char[]>(bufferLength);
    if (!file.read(buffer.get(), bufferLength)) {
        ServerLog::Error("Failed to read file.");
        return false;
    }
    file.seekg(cur);
    std::string str(buffer.get(), bufferLength);
    uint64_t offset = str.find(c);
    if (offset == std::string::npos) {
        ServerLog::Error("Failed to find separator.");
        return false;
    }
    file.seekg(offset, std::ifstream::cur);
    return true;
}

bool TraceFileParser::SeekRegexPosition(std::ifstream &file, const std::string &regex)
{
    auto cur = file.tellg();
    std::unique_ptr<char[]> buffer = std::make_unique<char[]>(bufferLength);
    if (!file.read(buffer.get(), bufferLength)) {
        ServerLog::Error("Failed to read file.");
        return false;
    }
    file.seekg(cur);
    std::string str(buffer.get(), bufferLength);
    auto result = RegexUtil::RegexSearch(str, regex);
    if (!result.has_value()) {
        ServerLog::Error("Failed to find match regex.");
        return false;
    }
    file.seekg(result.value().position(), std::ifstream::cur);
    return true;
}

std::string TraceFileParser::GetDbPath(const std::string &filePath, const std::string &fileId)
{
    std::string path = FileUtil::GetRealPath(filePath);
    std::string suffix = ".db";
    auto pos = fileId.find_last_of('_');
    if (pos != std::string::npos) {
        suffix = fileId.substr(pos) + suffix;
    }
    pos = path.find_last_of('.');
    if (pos != std::string::npos) {
        path.replace(pos, path.size() - pos, suffix);
    } else {
        path.append(suffix);
    }
    return path;
}

int64_t TraceFileParser::GetTrackId(const std::string &fileId, const std::string &pid, int64_t tid)
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

void TraceFileParser::Reset()
{
    ServerLog::Info("Reset. wait task completed.");
    threadPool->Reset();
    ServerLog::Info("Task completed.");
    auto databaseList = DataBaseManager::Instance().GetAllTraceDatabase();
    for (auto &database : databaseList) {
        std::string path = database->GetDbPath();
        database->ReleaseStmt();
        database->CloseDb();
        if (!FileUtil::RemoveFile(path)) {
            ServerLog::Error("Failed to remove file. ", path);
        }
    }
    trackIdMap.clear();
    trackId = 0;
    DataBaseManager::Instance().Clear();
    TraceTime::Instance().Reset();
    FileParser::Reset();
    ParserStatusManager::Instance().ClearAllParserStatus();
}

std::string TraceFileParser::GetFileId(const std::string &filePath)
{
    std::string fileId = GetFileIdFromFile(filePath);
    if (fileId.empty()) {
        fileId = GetFileIdFromPath(filePath);
    }
    return fileId;
}

std::string TraceFileParser::GetFileIdFromFile(const std::string &filePath)
{
#ifdef _WIN32
    std::string path(filePath);
    if (StringUtil::IsUtf8String(filePath)) {
        path = StringUtil::Utf8ToGbk(filePath.c_str());
    }
    std::ifstream file(path, std::ios::in);
#else
    std::ifstream file(filePath, std::ios::in);
#endif
    if (!file.is_open()) {
        ServerLog::Error("Failed to open file.");
        return "";
    }
    std::unique_ptr<char[]> buffer = std::make_unique<char[]>(bufferLength);
    if (!file.read(buffer.get(), bufferLength)) {
        ServerLog::Error("Failed to read file.");
        return "";
    }
    std::string str(buffer.get(), bufferLength);
    std::string rankId =
            str.substr(str.find_first_of('{'), str.find_first_of('}') - str.find_first_of('{') + 1);
    std::string error;
    auto json = JsonUtil::TryParse(rankId, error);
    if (!json.has_value()) {
        ServerLog::Warn("Failed to parse json.", error, " string:", rankId);
        return "";
    }
    if (json.value().contains("rank_id")) {
        return nlohmann::to_string(json.value()["rank_id"]);
    }
    return "";
}

std::string TraceFileParser::GetFileIdFromPath(const std::string &filePath)
{
    const int fileIdPosition = 3; // 上上层目录
    std::string path = FileUtil::GetRealPath(filePath);
    auto pos = path.find_first_of('\\');
    while (pos != std::string::npos) {
        path.replace(pos, 1, "/");
        pos = path.find_first_of('\\');
    }
    auto list = StringUtil::Split(path, "/");
    if (list.size() < fileIdPosition) {
        return "";
    }
    return list.at(list.size() - fileIdPosition);
}

void TraceFileParser::DeleteParseFileFromDisk(const std::string &fileId)
{
    ServerLog::Info("Delete file. id:", fileId);
    ParserStatusManager::Instance().ClearParserStatus(fileId);
    auto database = DataBaseManager::Instance().GetTraceDatabase(fileId);
    std::string path = database->GetDbPath();
    database->ReleaseStmt();
    database->CloseDb();
    if (!path.empty()) {
        FileUtil::RemoveFile(path);
    }
    DataBaseManager::Instance().ReleaseTraceDatabase(fileId);
}

void TraceFileParser::DeleteParseFile(const std::string &fileId)
{
    auto oldStatus = ParserStatusManager::Instance().SetTerminateStatus(fileId);
    ServerLog::Info("Delete file. id:", fileId, ", status:", static_cast<int>(oldStatus));
    if (oldStatus == ParserStatus::FINISH) {
        DeleteParseFileFromDisk(fileId);
    }
}

bool TraceFileParser::InitDatabase(const std::string& dbPath, const std::string& rankId)
{
    auto database = DataBaseManager::Instance().GetTraceDatabase(rankId);
    if (!(database->OpenDb(dbPath, true) && database->CreateTable() &&
          database->SetConfig() && database->InitStmt())) {
        ParseEndCallBack(rankId, false);
        ServerLog::Error("Failed to open traceDatabase. path:", dbPath);
        return false;
    }
    auto summaryDatabase = Timeline::DataBaseManager::Instance().GetSummaryDatabase(rankId);
    if (!(summaryDatabase->OpenDb(dbPath, false) && summaryDatabase->CreateTable() &&
          summaryDatabase->SetConfig() && summaryDatabase->InitStmt())) {
        ParseEndCallBack(rankId, false);
        ServerLog::Error("Failed to open summaryDatabase. path:", dbPath);
        return false;
    }
    auto memoryDatabase = Timeline::DataBaseManager::Instance().GetMemoryDatabase(rankId);
    if (!(memoryDatabase->OpenDb(dbPath, false) && memoryDatabase->CreateTable() &&
            memoryDatabase->SetConfig() && memoryDatabase->InitStmt())) {
        ParseEndCallBack(rankId, false);
        ServerLog::Error("Failed to open memoryDatabase. path:", dbPath);
        return false;
    }
    return true;
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
