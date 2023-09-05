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
    bool Parse(const std::string &filePath, const std::string &fileId) override;
    void Reset() override;
    bool OperatorParse(const std::string &filePath, const std::string &fileId);
    bool RecordToParse(const std::string &filePath, const std::string &fileId);

private:
    const int maxThreadNum = 4;

    std::unique_ptr<MemoryDataBase> database;
    std::unique_ptr<ThreadPool> threadPool;
    std::map<std::string, std::future<void>> futureMap;
    std::chrono::system_clock::time_point start;

    bool WaitParseEnd(const std::string &fileId);

    Record mapperToRecordDetail(std::map<std::string, std::int16_t> dataMap, std::vector<std::string>);
    Operator mapperToOperatorDetail(std::map<std::string, std::int16_t> dataMap, std::vector<std::string>);

    std::string GetDbPath(const std::string &filePath, const std::string &fileId);
};
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_MEMORYPARSE_H
