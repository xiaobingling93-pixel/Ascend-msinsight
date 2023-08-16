/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include <fstream>
#include "ServerLog.h"
#include "RegexUtil.h"
#include "FileUtil.h"
#include "EventParser.h"
#include "TraceFileParser.h"

namespace Dic {
namespace Scene {
namespace Core {
using namespace Dic::Server;
ThreadPool TraceFileParser::threadPool = ThreadPool(TraceFileParser::MAX_THREAD_NUM);
bool TraceFileParser::Parse(const std::string &filePath, const std::string &fileId)
{
    auto start = std::chrono::system_clock::now();
    ServerLog::Info("start parse.");
    auto splitFile = TraceFileParser::SplitFile(filePath);
    std::string dbPath = GetDbPath(filePath, fileId);
    TraceDatabase database;
    database.OpenDb(dbPath, true);
    database.CreateTable();
    for (const auto &pos : splitFile) {
        EventParser eventParser(filePath, dbPath);
        eventParser.Parse(pos.first, pos.second);
    }
    database.CreateIndex();
    database.UpdateDepth();
    auto dur = std::chrono::system_clock::now() - start;
    ServerLog::Info("end parse. time:", dur.count());
    return true;
}

bool TraceFileParser::WaitParseEnd()
{
    return false;
}

std::vector<std::pair<uint64_t, uint64_t>> TraceFileParser::SplitFile(const std::string &filePath)
{
    std::vector<std::pair<uint64_t, uint64_t>> result;
    std::ifstream file(filePath, std::ios::in);
    if (!file.is_open()) {
        ServerLog::Error("Failed to open file.");
        return result;
    }
    file.seekg(0, std::ifstream::end);
    uint64_t fileSize = file.tellg();
    file.clear();
    file.seekg(0, std::ios::beg);
    bool endFlag = false;
    while (!endFlag) {
        if (!SeekCharPosition(file, '{')) {
            ServerLog::Info("Failed to find start position.");
            break;
        }
        uint64_t start = file.tellg();
        std::string endRegex;
        if (start + BLOCK_SIZE >= fileSize) {
            file.seekg(0 - BUFFER_LENGTH, std::ifstream::end);
            endRegex = R"(\}\s*\])";
            endFlag = true;
        } else {
            file.seekg(BLOCK_SIZE, std::ifstream::cur);
            endRegex = R"(\}\s*,\s\{)";
        }
        if (!SeekRegexPosition(file, endRegex)) {
            ServerLog::Info("Failed to find end position.");
            break;
        }
        uint64_t end = file.tellg();
        result.emplace_back(start, end);
    }
    file.close();
    return result;
}

bool TraceFileParser::SeekCharPosition(std::ifstream &file, char c)
{
    auto cur = file.tellg();
    std::unique_ptr<char[]> buffer = std::make_unique<char[]>(BUFFER_LENGTH);
    if (!file.read(buffer.get(), BUFFER_LENGTH)) {
        ServerLog::Error("Failed to read file.");
        return false;
    }
    file.seekg(cur);
    std::string str(buffer.get(), BUFFER_LENGTH);
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
    std::unique_ptr<char[]> buffer = std::make_unique<char[]>(BUFFER_LENGTH);
    if (!file.read(buffer.get(), BUFFER_LENGTH)) {
        ServerLog::Error("Failed to read file.");
        return false;
    }
    file.seekg(cur);
    std::string str(buffer.get(), BUFFER_LENGTH);
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
    std::string fileName = Dic::FileUtil::GetFileName(filePath);
    auto pos = fileName.find_last_of('.');
    std::string dbPath = fileName.substr(0, pos) + "_" + fileId + ".db";
    return Dic::FileUtil::GetRealPath(dbPath);
}

} // end of namespace Core
} // end of namespace Scene
} // end of namespace Dic
