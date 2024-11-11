/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2024. All rights reserved.
 */
#ifndef PROFILER_SERVER_JSONFILEPROCESS_H
#define PROFILER_SERVER_JSONFILEPROCESS_H
#include <functional>
#include <string>


namespace Dic {
namespace Module {
class JsonFileProcess {
public:
    static std::vector<std::pair<int64_t, int64_t>> SplitFile(const std::string &filePath,
        std::optional<std::pair<int64_t, int64_t>> position = std::nullopt);
protected:
    static const int64_t blockSize = 1024 * 1024 * 50; // 50MB
    static const int startBufferLength = 1024;
    static const int endBufferLength = 1024 * 10;
    static std::vector<std::pair<int64_t, int64_t>> GetSplitPosition(std::ifstream &file,
        std::pair<int64_t, int64_t> position = {0, 0});
private:
    static bool SeekCharPosition(std::ifstream &file, char c);
    static bool SeekRegexPosition(std::ifstream &file, const std::string &regex);
    static void ComputeSmallFilePosition(std::ifstream &file, std::vector<std::pair<int64_t, int64_t>> &result,
        const JsonFormat &json, std::pair<int64_t, int64_t> position);
};
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_JSONFILEPROCESS_H
