/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_SCENE_CORE_TRACE_FILE_PARSER_H
#define DATA_INSIGHT_CORE_SCENE_CORE_TRACE_FILE_PARSER_H

#include <vector>
#include <map>
#include <optional>
#include <mutex>
#include "FileParser.h"
#include "ThreadPool.h"

namespace Dic {
namespace Module {
namespace Timeline {
class TraceFileParser : public FileParser {
public:
    static TraceFileParser &Instance();
    bool Parse(const std::string &filePath, const std::string &fileId) override;
    void Reset() override;

    int64_t GetTrackId(const std::string &fileId, const std::string &pid, int64_t tid);
    std::string GetFileId(const std::string &filePath);
private:
    TraceFileParser();
    ~TraceFileParser() override;
    const int MAX_THREAD_NUM = 4;
    std::unique_ptr<ThreadPool> threadPool;
    std::map<std::string, std::future<void>> futureMap;

    bool WaitParseEnd(const std::string &fileId);
    std::string GetFileIdFromFile(const std::string &filePath);
    std::string GetFileIdFromPath(const std::string &filePath);
    static const int64_t BLOCK_SIZE = 1024 * 1024 * 50; // 50MB
    static const int BUFFER_LENGTH = 1024 * 10;
    static std::vector<std::pair<uint64_t, uint64_t>> SplitFile(const std::string &filePath);
    static bool SeekCharPosition(std::ifstream &file, char c);
    static bool SeekRegexPosition(std::ifstream &file, const std::string &regex);
    static std::string GetDbPath(const std::string &filePath, const std::string &fileId);

    std::mutex trackMutex;
    std::map<std::string, std::map<std::string, int64_t>> trackIdMap;
    int64_t trackId = 0;
    std::chrono::system_clock::time_point start;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_SCENE_CORE_TRACE_FILE_PARSER_H
