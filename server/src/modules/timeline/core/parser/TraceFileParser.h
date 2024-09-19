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
#include "JsonFileProcess.h"

namespace Dic {
namespace Module {
namespace Timeline {
class TraceFileParser : public FileParser, protected JsonFileProcess {
public:
    static TraceFileParser &Instance();
    bool Parse(const std::vector<std::string> &filePathArr, const std::string &rankId,
        const std::string &selectedFolder) override;
    void Reset() override;
    static void DeleteParseFiles(const std::vector<std::string> &fileIds);
    static void ParseEndCallBack(const std::string &fileId, bool result, const std::string &message);

    int64_t GetTrackId(const std::string &fileId, const std::string &pid, const std::string &tid);

private:
    TraceFileParser();
    ~TraceFileParser() override;
    const int maxThreadNum = 4;
    std::unique_ptr<ThreadPool> threadPool;
    static bool CheckInitParser(const std::string &fileId);
    static bool InitParser(const std::vector<std::string> &filePathArr, const std::string &fileId);
    static void PreParseTask(const std::vector<std::string> &filePathArr, const std::string &fileId);
    static void ParseTask(const std::string &filePath, const std::string &fileId, std::pair<int64_t, int64_t> pos);
    static void EndParseTask(const std::string &fileId, const std::vector<std::string> &filePathArr,
        std::shared_ptr<std::vector<std::future<void>>> futures,
        std::chrono::time_point<std::chrono::high_resolution_clock> start);
    static void DeleteParseFileFromDisk(const std::string &fileId);

    std::mutex trackMutex;
    std::unordered_map<std::string, std::map<std::pair<std::string, std::string>, uint64_t>> trackIdMap;
    uint64_t trackId = 0;

    static void InitFileProcess(const std::vector<std::string> &filePathArr, const std::string &fileId);

    static std::string ComputeStatusInfoFromPathArr(const std::vector<std::string> &filePathArr);
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_MODULE_CORE_TRACE_FILE_PARSER_H
