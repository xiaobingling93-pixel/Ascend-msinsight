/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_SCENE_CORE_TRACE_FILE_PARSER_H
#define DATA_INSIGHT_CORE_SCENE_CORE_TRACE_FILE_PARSER_H

#include <vector>
#include <optional>
#include "FileParser.h"
#include "ThreadPool.h"

namespace Dic {
namespace Scene {
namespace Core {
class TraceFileParser : public FileParser {
public:
    TraceFileParser() = default;
    ~TraceFileParser() override = default;
    bool Parse(const std::string &filePath, const std::string &fileId) override;
    bool WaitParseEnd() override;

private:
    static const int MAX_THREAD_NUM = 4;
    static ThreadPool threadPool;

    static const int64_t BLOCK_SIZE = 1024 * 1024 * 50; // 50MB
    static const int BUFFER_LENGTH = 1024 * 10;
    static std::vector<std::pair<uint64_t, uint64_t>> SplitFile(const std::string &filePath);
    static bool SeekCharPosition(std::ifstream &file, char c);
    static bool SeekRegexPosition(std::ifstream &file, const std::string &regex);
    static std::string GetDbPath(const std::string &filePath, const std::string &fileId);
};
} // end of namespace Core
} // end of namespace Scene
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_SCENE_CORE_TRACE_FILE_PARSER_H
