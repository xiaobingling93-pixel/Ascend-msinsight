/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2024. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_MODULE_CORE_TRACE_FILE_PARSER_H
#define DATA_INSIGHT_CORE_MODULE_CORE_TRACE_FILE_PARSER_H

#include <vector>
#include <map>
#include <unordered_map>
#include <optional>
#include <mutex>
#include "FileParser.h"
#include "ThreadPool.h"

namespace Dic {
namespace Module {
namespace Timeline {

enum class JsonFormat {
    JSON_ARRAY_FORMAT = 0,
    JSON_OBJECT_FORMAT
};

class TraceFileParser : public FileParser {
public:
    static TraceFileParser &Instance();
    bool Parse(const std::vector<std::string> &filePathArr, const std::string &rankId,
               const std::string &selectedFolder) override;
    void Reset() override;
    static void DeleteParseFile(const std::string &fileId);
    static void DeleteParseFiles(const std::vector<std::string> &fileIds);

    int64_t GetTrackId(const std::string &fileId, const std::string &pid, const std::string &tid);
private:
    TraceFileParser();
    ~TraceFileParser() override;
    const int maxThreadNum = 4;
    std::unique_ptr<ThreadPool> threadPool;
    std::map<std::string, std::future<void>> futureMap;

    static const int64_t blockSize = 1024 * 1024 * 50; // 50MB
    static const int bufferLength = 1024;
    static std::vector<std::pair<int64_t, int64_t>> SplitFile(const std::string &filePath);
    static std::vector<std::pair<int64_t, int64_t>> GetSplitPosition(std::ifstream &file);
    static bool SeekCharPosition(std::ifstream &file, char c);
    static bool SeekRegexPosition(std::ifstream &file, const std::string &regex);
    static bool InitParser(const std::vector<std::string> &filePathArr, const std::string &fileId);
    static void PreParseTask(const std::vector<std::string> &filePathArr, const std::string &fileId);
    static void ParseTask(const std::string &filePath, const std::string &fileId, std::pair<int64_t, int64_t> pos);
    static void EndParseTask(const std::string &fileId, const std::vector<std::string> &filePathArr,
                             std::shared_ptr<std::vector<std::future<void>>> futures,
                             std::chrono::time_point<std::chrono::high_resolution_clock> start);
    static void ParseEndCallBack(const std::string &fileId, bool result, const std::string &message);
    static void DeleteParseFileFromDisk(const std::string &fileId);

    std::mutex trackMutex;
    std::unordered_map<std::string, std::map<std::pair<std::string, std::string>, int64_t>> trackIdMap;
    int64_t trackId = 0;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_MODULE_CORE_TRACE_FILE_PARSER_H
