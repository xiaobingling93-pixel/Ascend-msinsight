/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_MEMORYPARSE_H
#define PROFILER_SERVER_MEMORYPARSE_H

#include <string>
#include <map>
#include <functional>
#include "GlobalDefs.h"
#include "MemoryDataBase.h"
#include "ThreadPool.h"
#include "MemoryDef.h"
#include "FileParser.h"

namespace Dic {
namespace Module {
namespace Memory {
class MemoryParse : public FileParser {
public:
    static MemoryParse &Instance();
    MemoryParse();

    ~MemoryParse() override;
    bool Parse(const std::vector<std::string> &filePaths, const std::string &fileId,
               const std::string &selectedFolder) override;
    void Reset() override;
    bool OperatorParse(const std::string &filePath, const std::string &fileId);
    bool RecordToParse(const std::string &filePath, const std::string &fileId);

private:
    const int maxThreadNum = 1;

    std::unique_ptr<MemoryDataBase> database;
    std::unique_ptr<ThreadPool> threadPool;
    std::map<std::string, std::future<void>> futureMap;
    std::chrono::system_clock::time_point start;
    const std::string memoryOperatorFile = "operator_memory.csv";
    const std::string memoryRecordFile = "memory_record.csv";

    bool WaitParseEnd(const std::string &fileId);

    Record mapperToRecordDetail(std::map<std::string, std::int16_t> dataMap, std::vector<std::string>);
    Operator mapperToOperatorDetail(std::map<std::string, std::int16_t> dataMap, std::vector<std::string>);

    std::string GetDbPath(const std::string &filePath, const std::string &fileId);
};
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_MEMORYPARSE_H
