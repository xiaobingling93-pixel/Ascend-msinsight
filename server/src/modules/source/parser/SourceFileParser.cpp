/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <fstream>
#include "ServerLog.h"
#include "rapidjson.h"
#include "document.h"
#include "JsonUtil.h"
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

bool SourceFileParser::Parse(const std::vector<std::string> &filePaths,
                             const std::string &fileId,
                             const std::string &selectedFile)
{
    std::ifstream file(FileUtil::PathPreprocess(selectedFile), std::ios::binary);
    if (!file) {
        ServerLog::Warn("Failed to open file: ", selectedFile);
        return false;
    }

    const int dataSizeLen = 8; // 数据类型字段距离数据大小字段的偏移
    const int dataTypeLen = 1;  // 填充长度字段距离数据类型字段的偏移
    const int paddingLen = 1;  // 填充长度字段距离数据类型字段的偏移
    const int reserveLen = 2;     // 实际数据距离填充长度字段的偏移
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

    return true;
}

void SourceFileParser::Reset()
{
    FileParser::Reset();
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
    for (const auto &entry: apiFiles) {
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
    for (auto line: vector) {
        if (line.cycles.size() < index + 1 || line.instructionsExecuted.size() < index + 1) {
            continue;
        }

        SourceFileLine output;
        for (const auto &pair: line.addressRange) {
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
    for (auto pos :sourceFilePos) {
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
            for (auto &core: cores.GetArray()) {
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

std::map<std::string, std::vector<SourceFileLine>> SourceFileParser::ConvertToFileMap(Value &fileArray)
{
    std::map<std::string, std::vector<SourceFileLine>> sourceLinesMap;

    for (auto &file: fileArray.GetArray()) {
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

    for (auto &line: lineArray.GetArray()) {
        if (!line.IsObject()) {
            continue;
        }

        SourceFileLine sourceFileLine;

        // 解析Address Range数组
        if (!line.HasMember("Address Range") || !line["Address Range"].IsArray()) {
            continue;
        }
        Value &addressRangeArray = line["Address Range"];
        for (auto &addressRange: addressRangeArray.GetArray()) {
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
        for (auto &cycle: cycleArray.GetArray()) {
            sourceFileLine.cycles.emplace_back(cycle.GetFloat());
        }

        // 解析Instructions Executed数组
        if (!line.HasMember("Instructions Executed") || !line["Instructions Executed"].IsArray()) {
            continue;
        }
        Value &instrExecutedArray = line["Instructions Executed"];
        for (auto &instrExecuted: instrExecutedArray.GetArray()) {
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

SourceFileParser::SourceFileParser()
{
}

SourceFileParser::~SourceFileParser()
{
}

} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic