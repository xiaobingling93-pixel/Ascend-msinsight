/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include <fstream>
#include "ServerLog.h"
#include "RegexUtil.h"
#include "FileUtil.h"
#include "JsonUtil.h"
#include "DataBaseManager.h"
#include "EventParser.h"
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

bool TraceFileParser::Parse(const std::vector<std::string> &filePathArr, const std::string &rankId,
                            const std::string &selectedFolder)
{
    start = std::chrono::system_clock::now();
    ServerLog::Info("start parse.");
    std::string dbPath = GetDbPath(selectedFolder, rankId);
    InitDatabase(dbPath, rankId);
    std::shared_ptr<std::vector<std::future<void>>> futures = std::make_unique<std::vector<std::future<void>>>();
    for (const auto &filePath: filePathArr) {
        start = std::chrono::system_clock::now();
        ServerLog::Info("start parse.", filePath);
        auto splitFileVector = TraceFileParser::SplitFile(filePath);
        if (splitFileVector.empty()) {
            ServerLog::Error("Failed to split file: ", filePath);
            continue;
        }
        for (const auto &pos: splitFileVector) {
            auto future = threadPool->AddTask([filePath, dbPath, pos, rankId]() {
                EventParser eventParser(filePath, dbPath, rankId);
                eventParser.Parse(pos.first, pos.second);
            });
            futures->emplace_back(std::move(future));
        }
        std::string parentDir = FileUtil::GetParentPath(filePath);
        if (!parentDir.empty()) {
            Memory::MemoryParse::Instance().OperatorParse(parentDir, rankId);
            Memory::MemoryParse::Instance().RecordToParse(parentDir, rankId);
            Summary::KernelParse::Instance().KernelFileParse(parentDir, rankId);
        }
    }
    auto future = threadPool->AddTask([futures, rankId]() {
        ServerLog::Info("Wait parse completed. ID:", rankId);
        for (const auto &future : *futures) {
            future.wait();
        }
        ServerLog::Info("Parse completed. ID:", rankId);
        auto database = DataBaseManager::Instance().GetTraceDatabase(rankId);
        database->CreateIndex();
        database->UpdateDepth();
        ServerLog::Info("Update depth completed. ID:", rankId);
    });
    futureMap.emplace(rankId, std::move(future));
    if (paserEndCallback != nullptr) {
        std::thread thread { [this, rankId]() { WaitParseEnd(rankId); } };
        thread.detach();
    }
    return true;
}

bool TraceFileParser::WaitParseEnd(const std::string &fileId)
{
    if (futureMap.count(fileId) == 0) {
        return false;
    }
    ServerLog::Info("Wait parse completed. ID:", fileId);
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

std::vector<std::pair<int64_t, int64_t>> TraceFileParser::SplitFile(const std::string &filePath)
{
    std::vector<std::pair<int64_t, int64_t>> result;
    std::ifstream file(filePath, std::ios::in);
    if (!file.is_open()) {
        ServerLog::Error("Failed to open file.");
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

std::string TraceFileParser::GetDbPath(const std::string &selectedFolder, const std::string &rankId)
{
    std::string dbPath = selectedFolder + "/" + rankId + ".db";
    return Dic::FileUtil::GetRealPath(dbPath);
}

int64_t TraceFileParser::GetTrackId(const std::string &fileId, const std::string &pid, int64_t tid)
{
    std::unique_lock<std::mutex> lock(trackMutex);
    std::string str = pid + std::to_string(tid);
    if (trackIdMap[fileId].count(str) > 0) {
        return trackIdMap[fileId].at(str);
    }
    if (trackId == INT64_MAX) {
        trackId = 0;
    }
    trackIdMap[fileId].emplace(str, ++trackId);
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
    DataBaseManager::Instance().Clear();
    TraceTime::Instance().Reset();
    FileParser::Reset();
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
    std::ifstream file(filePath);
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

bool TraceFileParser::InitDatabase(const std::string& dbPath, const std::string& rankId)
{
    auto database = DataBaseManager::Instance().GetTraceDatabase(rankId);
    if (!(database->OpenDb(dbPath, true) && database->CreateTable() &&
          database->SetConfig() && database->InitStmt())) {
        ServerLog::Error("Failed to open traceDatabase. path:", dbPath);
        return false;
    }
    auto summaryDatabase = Timeline::DataBaseManager::Instance().GetSummaryDatabase(rankId);
    if (!(summaryDatabase->OpenDb(dbPath, false) && summaryDatabase->CreateTable() &&
          summaryDatabase->SetConfig() && summaryDatabase->InitStmt())) {
        ServerLog::Error("Failed to open summaryDatabase. path:", dbPath);
        return false;
    }
    auto memoryDatabase = Timeline::DataBaseManager::Instance().GetMemoryDatabase(rankId);
    if (!(memoryDatabase->OpenDb(dbPath, false) && memoryDatabase->CreateTable() &&
            memoryDatabase->SetConfig() && memoryDatabase->InitStmt())) {
        ServerLog::Error("Failed to open memoryDatabase. path:", dbPath);
        return false;
    }
    return true;
}
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
