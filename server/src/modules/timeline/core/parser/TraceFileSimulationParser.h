/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_TRACEFILESIMULATIONPARSER_H
#define PROFILER_SERVER_TRACEFILESIMULATIONPARSER_H
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
class TraceFileSimulationParser : public FileParser, protected JsonFileProcess {
public:
    static TraceFileSimulationParser &Instance();
    bool Parse(const std::vector<std::string> &filePathArr,
               const std::string &rankId,
               const std::string &selectedFolder,
               const std::string &fileId) override;
    void Reset() override;
    static void DeleteParseFiles(const std::vector<std::string> &fileIds);

    int64_t GetTrackId(const std::string &fileId, const std::string &pid, const std::string &tid);

private:
    TraceFileSimulationParser();
    ~TraceFileSimulationParser() override;
    const int maxThreadNum = 8;
    std::unique_ptr<ThreadPool> threadPool;
    static bool InitParser(const std::vector<std::string> &filePathArr,
                           const std::string &rankId,
                           const std::string &fileId);
    static void PreParseTask(const std::vector<std::string> &filePathArr,
                             const std::string &rankId,
                             const std::string &fileId);
    static void ParseTask(const std::string &filePath,
                          const std::string &rankId,
                          const std::string &fileId,
                          std::pair<int64_t, int64_t> pos);
    static void EndParseTask(const std::string &rankId,
                             const std::vector<std::string> &filePathArr,
                             std::shared_ptr<std::vector<std::future<void>>> futures,
                             std::chrono::time_point<std::chrono::high_resolution_clock> start,
                             const std::string &fileId);
    static void ParseEndCallBack(const std::string &rankId,
                                 const std::string &fileId,
                                 bool result,
                                 const std::string &message);
    static void DeleteParseFileFromDisk(const std::string &fileId);

    std::mutex trackMutex;
    std::unordered_map<std::string, std::map<std::pair<std::string, std::string>, int64_t>> trackIdMap;
    int64_t trackId = 0;
};
} // end of namespace Timeline
} // end of namespace Module
} // end of namespace Dic
#endif // PROFILER_SERVER_TRACEFILESIMULATIONPARSER_H
