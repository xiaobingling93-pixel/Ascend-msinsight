/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <fstream>
#include "ServerLog.h"
#include "rapidjson.h"
#include "document.h"
#include "JsonUtil.h"
#include "ParserStatusManager.h"
#include "DataBaseManager.h"
#include "EventParser.h"
#include "TraceTime.h"
#include "FileUtil.h"
#include "SourceFileParser.h"

namespace Dic {
namespace Module {
namespace Source {
using namespace Dic;
using namespace Dic::Server;

SourceFileParser &SourceFileParser::Instance()
{
    static SourceFileParser instance;
    return instance;
}
SourceFileParser::SourceFileParser()
{
    threadPool = std::make_unique<ThreadPool>(SourceFileParser::maxThreadNum);
}

SourceFileParser::~SourceFileParser()
{
    threadPool->ShutDown();
}

bool SourceFileParser::Parse(const std::vector<std::string> &filePaths, const std::string &fileId,
    const std::string &selectedFile)
{
    std::ifstream file(FileUtil::PathPreprocess(selectedFile), std::ios::binary);
    if (!file) {
        ServerLog::Warn("Failed to open file: ", selectedFile);
        return false;
    }

    const int dataSizeLen = 8; // 数据类型字段距离数据大小字段的偏移
    const int dataTypeLen = 1; // 填充长度字段距离数据类型字段的偏移
    const int paddingLen = 1;  // 填充长度字段距离数据类型字段的偏移
    const int reserveLen = 2;  // 实际数据距离填充长度字段的偏移
    const int filePathLen = 4096;

    while (!file.eof()) {
        uint64_t dataSize;
        uint8_t dataType;
        uint8_t paddingLength;

        file.read(reinterpret_cast<char *>(&dataSize), dataSizeLen);
        file.read(reinterpret_cast<char *>(&dataType), dataTypeLen);
        file.read(reinterpret_cast<char *>(&paddingLength), paddingLen);
        dataType = static_cast<int>(dataType);
        paddingLength = static_cast<int>(paddingLength);
        if (dataType == static_cast<int>(DataTypeEnum::SOURCE)) {
            dataSize = dataSize + filePathLen;
        }

        if (!file) { // 检查读取是否成功
            break;
        }
        file.seekg(reserveLen, std::ios::cur); // 跳转到实际数据的开始

        int64_t startPos = file.tellg();
        int64_t endPos = startPos + dataSize - paddingLength;

        dataBlockMap[dataType].emplace_back(startPos, endPos);

        // 跳转到下一个数据块的开始位置，考虑到当前数据块的大小和填充
        file.seekg(dataSize, std::ios::cur);
    }
    file.close();
    Timeline::ParserStatusManager::Instance().SetParserStatus(fileId, Timeline::ParserStatus::INIT);
    ServerLog::Info("Start simulation parse. file id: ", fileId);
    threadPool->AddTask(PreParseTask, fileId);
    return true;
}

void SourceFileParser::PreParseTask(const std::string &fileId)
{
    if (!InitParser(fileId)) {
        ParseEndCallBack(fileId, false, "Failed to init trace file parser.");
    }
}

bool SourceFileParser::InitParser(const std::string &fileId)
{
    if (!Timeline::ParserStatusManager::Instance().SetRunningStatus(fileId)) {
        ServerLog::Info("Pre task skip this file.");
        return true;
    }
    auto database = Timeline::DataBaseManager::Instance().GetTraceDatabase(fileId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection.");
        return false;
    }

    if (!(database->DropTable() && database->CreateTable())) {
        ServerLog::Error("Failed to open traceDatabase. rankId:", fileId);
        return false;
    }
    auto &instance = SourceFileParser::Instance();
    std::vector<std::pair<int64_t, int64_t>> &traceFilePos =
        instance.dataBlockMap[static_cast<int>(DataTypeEnum::TRACE)];
    std::ifstream file(instance.filePath, std::ios::in | std::ios::binary);
    std::vector<std::pair<int64_t, int64_t>> adjustTraceFilePos;
    for (const auto &pos : traceFilePos) {
        adjustTraceFilePos.emplace_back(AdjustPosition(file, pos.first, pos.second));
    }
    std::shared_ptr<std::vector<std::future<void>>> futures = std::make_shared<std::vector<std::future<void>>>();
    for (const auto &pos : adjustTraceFilePos) {
        auto future = instance.threadPool->AddTask(ParseTask, fileId, pos);
        futures->emplace_back(std::move(future));
    }
    instance.threadPool->AddTask(EndParseTask, fileId, futures);
    return true;
}

void SourceFileParser::EndParseTask(const std::string &fileId, std::shared_ptr<std::vector<std::future<void>>> futures)
{
    if (Timeline::ParserStatusManager::Instance().GetParserStatus(fileId) != Timeline::ParserStatus::RUNNING) {
        ServerLog::Info("End parse task skip this file. ID:", fileId);
        return;
    }
    ServerLog::Info("Wait parse completed. ID:", fileId);
    for (const auto &future : *futures) {
        future.wait();
    }
    ServerLog::Info("Parse completed. ID:", fileId);
    auto database = Timeline::DataBaseManager::Instance().GetTraceDatabase(fileId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:", fileId);
        return;
    }
    database->CreateIndex();
    database->UpdateSimulationDepth();
    ServerLog::Info("Update depth completed. ID:", fileId);
    ParseEndCallBack(fileId, true, "");
}

void SourceFileParser::ParseTask(const std::string &fileId, std::pair<int64_t, int64_t> pos)
{
    if (Timeline::ParserStatusManager::Instance().GetParserStatus(fileId) != Timeline::ParserStatus::RUNNING) {
        ServerLog::Info("Parse task skip this file. ID:", fileId);
        return;
    }
    ServerLog::Info("Ptart parse:", fileId);
    auto &instance = SourceFileParser::Instance();
    Timeline::EventParser eventParser(instance.filePath, fileId);
    eventParser.SetSimulationStatus(true);
    if (!eventParser.Parse(pos.first, pos.second)) {
        if (Timeline::ParserStatusManager::Instance().SetTerminateStatus(fileId) == Timeline::ParserStatus::RUNNING) {
            // 只发送一次解析失败事件
            ParseEndCallBack(fileId, false, eventParser.GetError());
        }
    }
}
std::pair<int64_t, int64_t> SourceFileParser::AdjustPosition(std::ifstream &file, int64_t start, int64_t end)
{
    int64_t contentStart = start;
    int64_t contentEnd = end;
    file.seekg(contentStart, std::ios::beg);
    char startTemp;
    while (file.get(startTemp)) {
        if (startTemp == '[') {
            contentStart++;
            break;
        }
        contentStart++;
    }
    file.seekg(contentEnd, std::ios::beg);
    char endTemp;
    const int offset = -2;
    while (file.get(endTemp)) {
        if (endTemp == ']') {
            contentEnd--;
            break;
        }
        contentEnd--;
        file.seekg(offset, std::ifstream::cur);
    }
    return std::make_pair(contentStart, contentEnd);
}

void SourceFileParser::ParseEndCallBack(const std::string &fileId, bool result, const std::string &message)
{
    if (!(result && Timeline::ParserStatusManager::Instance().SetFinishStatus(fileId))) {
        result = false;
    }
    auto &instance = SourceFileParser::Instance();
    if (instance.paserEndCallback != nullptr) {
        instance.paserEndCallback(fileId, result, message);
    }
}

void SourceFileParser::Reset()
{
    ServerLog::Info("Reset. wait task completed.");
    Timeline::ParserStatusManager::Instance().SetAllTerminateStatus();
    Timeline::ParserStatusManager::Instance().SetClusterParseStatus(Timeline::ParserStatus::TERMINATE);
    threadPool->Reset();
    ServerLog::Info("Task completed.");
    auto connList = Timeline::DataBaseManager::Instance().GetAllTraceDatabase();
    for (auto &conn : connList) {
        std::string path = conn->GetDbPath();
        conn->Stop();
        if (!FileUtil::RemoveFile(path)) {
            ServerLog::Error("Failed to remove file. ", path);
        }
    }
    trackIdMap.clear();
    simulationPidMap.clear();
    simulationTidMap.clear();
    trackId = 0;
    tid = 0;
    pid = 0;
    dataBlockMap[static_cast<int>(DataTypeEnum::TRACE)].clear();
    Timeline::DataBaseManager::Instance().Clear();
    Timeline::TraceTime::Instance().Reset();
    FileParser::Reset();
    Timeline::ParserStatusManager::Instance().ClearAllParserStatus();
    ServerLog::Info("End Reset trace Parser");
}

bool SourceFileParser::CheckOperatorBinary(const std::string &selectedFilePath)
{
    std::ifstream file(FileUtil::PathPreprocess(selectedFilePath), std::ios::binary);
    if (!file) {
        return false;
    }

    const int reversePadding = 10;
    uint64_t contentLength;
    uint16_t reverse;

    file.read(reinterpret_cast<char *>(&contentLength), sizeof(contentLength));
    file.seekg(reversePadding, std::ios::beg);
    file.read(reinterpret_cast<char *>(&reverse), sizeof(reverse));

    bool isBinary = (contentLength != 0) && (reverse == SourceFileParser::reverseConst);
    file.close();

    if (isBinary) {
        this->filePath = FileUtil::PathPreprocess(selectedFilePath);
    }

    return isBinary;
}

std::vector<std::string> SourceFileParser::GetCoreList()
{
    return this->apiCores;
}

std::vector<std::string> SourceFileParser::GetSourceList()
{
    std::vector<std::string> sourceList;
    for (const auto &entry : apiFiles) {
        sourceList.push_back(entry.first);
    }
    return sourceList;
}

std::vector<SourceFileLine> SourceFileParser::GetApiLinesByCoreAndSource(std::string core, std::string sourceName)
{
    std::vector<SourceFileLine> result;

    auto it = std::find(apiCores.begin(), apiCores.end(), core);
    if (it == apiCores.end()) {
        ServerLog::Error("Can't find the specified core name : ", core);
        return result;
    }
    int index = std::distance(apiCores.begin(), it);

    if (apiFiles.count(sourceName) == 0) {
        ServerLog::Warn("The specified file doesn't exist in api files, and source name is:", sourceName);
        return result;
    }
    std::vector<SourceFileLine> &vector = apiFiles[sourceName];
    for (auto line : vector) {
        if (line.cycles.size() < index + 1 || line.instructionsExecuted.size() < index + 1) {
            continue;
        }

        SourceFileLine output;
        for (const auto &pair : line.addressRange) {
            output.addressRange.emplace_back(std::make_pair(pair.first, pair.second));
        }
        output.cycles.emplace_back(line.cycles[index]);
        output.instructionsExecuted.emplace_back(line.instructionsExecuted[index]);
        output.line = line.line;

        result.emplace_back(output);
    }
    return result;
}

std::string SourceFileParser::GetInstr()
{
    std::ifstream file(filePath);
    if (!file) {
        ServerLog::Error("Can't open file,please check file exist or not,file name :", filePath);
        return "";
    }
    int64_t start = apiInstrPos.first;
    int64_t end = apiInstrPos.second;
    int64_t dataSize = end - start;

    file.seekg(start, std::ios::beg);

    std::string content(dataSize, '\0');
    if (!file.read(&content[0], static_cast<std::streamsize>(dataSize))) {
        ServerLog::Error("Can't read file,please check file exist or not,file name :", filePath);
        return "";
    }
    return content;
}

std::string SourceFileParser::GetSourceByName(std::string sourceName)
{
    if (sourceFiles.count(sourceName) == 0) {
        ServerLog::Warn("Don't exist the specified file ", sourceName);
    }
    std::pair<int64_t, int64_t> &pos = sourceFiles[sourceName];
    int64_t start = pos.first;
    int64_t end = pos.second;
    int64_t dataSize = end - start;

    std::ifstream file(filePath);
    if (!file) {
        ServerLog::Error("Can't open file,please check file exist or not,file name :", filePath);
        return "";
    }
    file.seekg(start, std::ios::beg);

    std::string content(dataSize, '\0');
    if (!file.read(&content[0], static_cast<std::streamsize>(dataSize))) {
        ServerLog::Error("Can't read file,please check file exist or not,file name :", filePath);
        return "";
    }
    return content;
}

void SourceFileParser::ConvertToData()
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        ServerLog::Error("Can't open file,please check file exist or not,file name :", filePath);
        return;
    }

    std::vector<std::pair<int64_t, int64_t>> &sourceFilePos = dataBlockMap[static_cast<int>(DataTypeEnum::SOURCE)];
    for (auto pos : sourceFilePos) {
        int64_t start = pos.first;
        int64_t end = pos.second;

        file.seekg(start, std::ios::beg);

        std::vector<char> filePathBuffer(filePathLengthConst);
        file.read(filePathBuffer.data(), filePathBuffer.size());
        std::string filePath(filePathBuffer.data());
        sourceFiles[filePath] = std::make_pair(start + filePathLengthConst, end);
    }

    std::vector<std::pair<int64_t, int64_t>> &apiFilePos = dataBlockMap[static_cast<int>(DataTypeEnum::API_FILE)];
    if (!apiFilePos.empty()) {
        std::pair<int64_t, int64_t> &pair = apiFilePos.at(0);
        int64_t start = pair.first;
        int64_t end = pair.second;
        int64_t dataSize = end - start;

        file.seekg(start, std::ios::beg);

        std::string jsonStr;
        jsonStr.resize(dataSize);
        file.read(&jsonStr[0], dataSize);

        rapidjson::Document d;
        d.Parse(jsonStr.c_str());
        if (JsonUtil::IsJsonArray(d, "Cores")) {
            rapidjson::Value &cores = d["Cores"];
            for (auto &core : cores.GetArray()) {
                apiCores.emplace_back(core.GetString());
            }
        }
        if (JsonUtil::IsJsonArray(d, "Files")) {
            rapidjson::Value &fileArray = d["Files"];
            apiFiles = ConvertToFileMap(fileArray);
        }
    }
    auto &apiInstrPosArray = dataBlockMap[static_cast<int>(DataTypeEnum::API_INSTR)];
    if (!apiInstrPosArray.empty()) {
        apiInstrPos = apiInstrPosArray.at(0);
    }
    file.close();
}
int64_t SourceFileParser::GetSimulationPid(const std::string &fileId, const std::string &processName)
{
    std::unique_lock<std::mutex> lock(processMutex);
    if (simulationPidMap[fileId].count(processName) > 0) {
        return simulationPidMap[fileId].at(processName);
    }
    if (pid == INT64_MAX) {
        pid = 0;
    }
    simulationPidMap[fileId].emplace(processName, ++pid);
    return pid;
}

int64_t SourceFileParser::GetSimulationTid(const std::string &fileId, const std::string &processName,
    const std::string &threadName)
{
    std::unique_lock<std::mutex> lock(threadMutex);
    auto item = std::make_pair(processName, threadName);
    if (simulationTidMap[fileId].count(item) > 0) {
        return simulationTidMap[fileId].at(item);
    }
    if (tid == INT64_MAX) {
        tid = 0;
    }
    simulationTidMap[fileId].emplace(item, ++tid);
    return tid;
}

std::map<std::string, std::vector<SourceFileLine>> SourceFileParser::ConvertToFileMap(Value &fileArray)
{
    std::map<std::string, std::vector<SourceFileLine>> sourceLinesMap;

    for (auto &file : fileArray.GetArray()) {
        if (!file.IsObject()) {
            continue;
        }

        if (!file.HasMember("Source") || !file["Source"].IsString()) {
            continue;
        }
        if (!file.HasMember("Lines") || !file["Lines"].IsArray()) {
            continue;
        }

        std::string source = file["Source"].GetString();

        rapidjson::Value &lineArray = file["Lines"];
        std::vector<SourceFileLine> sourceFileLineArray = ConvertToLineArray(lineArray);

        // 将Source和对应的SourceFileLines vector添加到map中
        sourceLinesMap[source] = sourceFileLineArray;
    }
    return sourceLinesMap;
}

std::vector<SourceFileLine> SourceFileParser::ConvertToLineArray(Value &lineArray)
{
    std::vector<SourceFileLine> sourceFileLines;

    for (auto &line : lineArray.GetArray()) {
        if (!line.IsObject()) {
            continue;
        }

        SourceFileLine sourceFileLine;

        // 解析Address Range数组
        if (!line.HasMember("Address Range") || !line["Address Range"].IsArray()) {
            continue;
        }
        Value &addressRangeArray = line["Address Range"];
        for (auto &addressRange : addressRangeArray.GetArray()) {
            if (!addressRange.IsArray() || addressRange.Size() != addressRangeSize) {
                continue;
            }
            if (!addressRange[0].IsString() || !addressRange[1].IsString()) {
                continue;
            }

            const char *startAddress = addressRange[0].GetString();
            const char *endAddress = addressRange[1].GetString();

            sourceFileLine.addressRange.emplace_back(startAddress, endAddress);
        }

        // 解析Cycles数组
        if (!line.HasMember("Cycles") || !line["Cycles"].IsArray()) {
            continue;
        }
        Value &cycleArray = line["Cycles"];
        for (auto &cycle : cycleArray.GetArray()) {
            sourceFileLine.cycles.emplace_back(cycle.GetFloat());
        }

        // 解析Instructions Executed数组
        if (!line.HasMember("Instructions Executed") || !line["Instructions Executed"].IsArray()) {
            continue;
        }
        Value &instrExecutedArray = line["Instructions Executed"];
        for (auto &instrExecuted : instrExecutedArray.GetArray()) {
            sourceFileLine.instructionsExecuted.emplace_back(instrExecuted.GetInt());
        }

        // 解析Line
        if (!line.HasMember("Line") || !line["Line"].IsInt()) {
            continue;
        }
        int lineIndex = line["Line"].GetInt();
        sourceFileLine.line = lineIndex;

        // 将解析好的SourceFileLine对象添加到vector中
        sourceFileLines.push_back(sourceFileLine);
    }
    return sourceFileLines;
}
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic