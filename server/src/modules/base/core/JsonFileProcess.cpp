/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2024. All rights reserved.
 */
#include "pch.h"
#include "FileParser.h"
#include "JsonFileProcess.h"

namespace Dic {
namespace Module {
std::vector<std::pair<int64_t, int64_t>> JsonFileProcess::SplitFile(const std::string &filePath)
{
#ifdef _WIN32
    std::string path(filePath);
    if (Dic::StringUtil::IsUtf8String(filePath)) {
        path = Dic::StringUtil::Utf8ToGbk(filePath.c_str());
    }
    std::ifstream file(path, std::ios::in | std::ios::binary);
#else
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
#endif
    if (!file.is_open()) {
        Dic::Server::ServerLog::Error("Failed to open json file. ", filePath);
        return {};
    }
    std::vector<std::pair<int64_t, int64_t>> result = GetSplitPosition(file);
    file.close();
    return result;
}

std::vector<std::pair<int64_t, int64_t>> JsonFileProcess::GetSplitPosition(std::ifstream &file)
{
    std::vector<std::pair<int64_t, int64_t>> result = {};
    file.seekg(0, std::ifstream::end); // 将当前位置移动到文件尾，以获取文件大小
    int64_t fileSize = file.tellg();
    file.clear();
    file.seekg(0, std::ios::beg); // 将当前位置移动到文件开头，正式开始处理
    // 首先判断Trace文件的格式是JSON Object Format还是JSON Array
    // Format，二者有差异，Object格式实际的数据应该从traceEvents之后开始
    Dic::Module::JsonFormat json = SeekRegexPosition(file, R"(\"traceEvents")") ?
        Dic::Module::JsonFormat::JSON_OBJECT_FORMAT :
        Dic::Module::JsonFormat::JSON_ARRAY_FORMAT;
    if (fileSize <= blockSize) {
        // 如果是Object类型，则有效数据从 ”traceEvents“: [  的"["之后开始
        ComputeSmallFilePosition(file, result, json);
        return result;
    }
    // 前面获取JsonFormat时，如果是Json Object Format，则已将当前位置移动到traceEvents之后，两种文件处理的逻辑一致
    bool endFlag = false;
    while (!endFlag) {
        if (!SeekCharPosition(file, '{')) {
            Dic::Server::ServerLog::Info("Failed to find json format start position.");
            break;
        }
        int64_t start = file.tellg();
        std::string endRegex;
        if (start + blockSize >= fileSize) {
            file.seekg(0 - endBufferLength, std::ifstream::end);
            endRegex = R"(\}\s*\]|\}\s*,\s*\])";
            endFlag = true;
        } else {
            file.seekg(blockSize, std::ifstream::cur);
            endRegex = R"(\}\s*,\s*\{)";
        }
        if (!SeekRegexPosition(file, endRegex)) {
            Dic::Server::ServerLog::Info("Failed to find json format end position.");
            break;
        }
        int64_t end = file.tellg();
        result.emplace_back(start, end);
    }
    return result;
}

void JsonFileProcess::ComputeSmallFilePosition(std::ifstream &file, std::vector<std::pair<int64_t, int64_t>> &result,
    const JsonFormat &json)
{
    if (json == JsonFormat::JSON_OBJECT_FORMAT) {
        if (!SeekRegexPosition(file, R"(\[\s*\{)")) {
            Server::ServerLog::Warn("Failed to find start position of json object format.");
            return;
        }
        int64_t start = file.tellg();
        file.seekg(0 - endBufferLength, std::ifstream::end);
        if (SeekRegexPosition(file, R"(\}\s*\]\s*\})")) {
            int64_t end = file.tellg();
            result.emplace_back(start + 1, end); // 此处的1表示跳过R"(\[\s*\{)"中的"["
        }
    } else {
        result.emplace_back(0, 0);
    }
}

bool JsonFileProcess::SeekCharPosition(std::ifstream &file, char c)
{
    auto cur = file.tellg();
    std::unique_ptr<char[]> buffer = std::make_unique<char[]>(startBufferLength);
    file.read(buffer.get(), startBufferLength);
    int64_t readCount = file.gcount();
    if (readCount <= 0) {
        Dic::Server::ServerLog::Error("Seek char. Failed to read file.");
        return false;
    }
    file.seekg(cur);
    std::string str(buffer.get(), readCount);
    size_t offset = str.find(c);
    if (offset == std::string::npos) {
        Dic::Server::ServerLog::Error("Failed to find separator.");
        return false;
    }
    file.seekg(static_cast<int64_t>(offset), std::ifstream::cur);
    return true;
}

bool JsonFileProcess::SeekRegexPosition(std::ifstream &file, const std::string &regex)
{
    auto cur = file.tellg();
    std::unique_ptr<char[]> buffer = std::make_unique<char[]>(endBufferLength);
    file.read(buffer.get(), endBufferLength);
    int64_t readCount = file.gcount();
    if (readCount <= 0) {
        Dic::Server::ServerLog::Error("Seek regex. Failed to read file.");
        return false;
    }
    file.seekg(cur);
    std::string str(buffer.get(), readCount);
    auto result = Dic::RegexUtil::RegexSearch(str, regex);
    if (!result.has_value()) {
        Dic::Server::ServerLog::Warn("Can't find match regex:", regex);
        return false;
    }
    file.seekg(result.value().position(), std::ifstream::cur);
    return true;
}
} // end of namespace Module
} // end of namespace Dic
