/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef PROFILER_SERVER_MEMORYPARSE_H
#define PROFILER_SERVER_MEMORYPARSE_H

#include <string>
#include <map>
#include <functional>
#include "GlobalDefs.h"
#include "JsonMemoryDataBase.h"
#include "ThreadPool.h"
#include "MemoryDef.h"
#include "FileParser.h"
#include "ConstantDefs.h"

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
    bool Parse(const std::vector<std::string>& pathList, const std::string& token);
    void Reset() override;
    bool OperatorParse(const std::string &filePath, const std::string &fileId);
    bool RecordToParse(const std::string &filePath, const std::string &fileId);

    const std::string memoryOperatorReg = R"((operator_memory|operator_memory_[0-9]{1,14})\.csv$)";
    const std::string memoryRecordReg = R"((memory_record|memory_record_[0-9]{1,14})\.csv$)";

private:
    const int maxThreadNum = 4;
    const int operatorTableNum = 5;
    const int recordTableNum = 5;
    std::map<std::string, Protocol::MemorySuccess> ranks;
    bool isCluster;

    std::unique_ptr<JsonMemoryDataBase> database;
    std::unique_ptr<ThreadPool> threadPool;
    std::map<std::string, std::future<void>> futureMap;

    Record mapperToRecordDetail(std::map<std::string, size_t> dataMap, std::vector<std::string>);
    Operator mapperToOperatorDetail(std::map<std::string, size_t> dataMap, std::vector<std::string>);

    void GetMapValid(const std::vector<std::string> &vec, std::map<std::string, size_t> dataMap);

    std::vector<std::pair<std::string, MemoryFilePair>> GetMemoryFiles(
            const std::vector<std::string>& paths);
    static void SetParseCallBack(const std::string& token);
    static void ParseEndCallBack(const std::string& fileId, bool result, const std::string &message);
    static void ParseCallBack(const std::string &token, const std::string& fileId, bool result, const std::string &msg);

    static void PreParseTask(const MemoryFilePair& filePair, const std::string& fileId);
    static bool ParseTask(const MemoryFilePair& filePair, const std::string& fileId, std::string &message);
    static bool InitParser(const MemoryFilePair& filePair, const std::string& fileId, std::string &message);
};
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_MEMORYPARSE_H
