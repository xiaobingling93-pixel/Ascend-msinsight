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
    bool Parse(const std::vector<std::string>& pathList);
    void Reset() override;
    bool OperatorParse(const std::string &filePath, const std::string &fileId);
    bool RecordToParse(const std::string &filePath, const std::string &fileId);
    bool StaticOpParse(const std::string &filePath, const std::string &fileId);

    const std::string memoryOperatorReg = R"(operator_memory(_slice_[0-9]{1,4})?(_[0-9]{1,14})?.csv$)";
    const std::string memoryRecordReg = R"(memory_record(_slice_[0-9]{1,4})?(_[0-9]{1,14})?.csv$)";
    const std::string staticOpMemReg = R"(static_op_mem(_[0-9]{1,14})?.csv$)";

private:
    const uint32_t maxThreadNum = 4;
    const uint32_t operatorTableNum = 5;
    const uint32_t recordTableNum = 5;
    const uint32_t staticOpTableNum = 7;
    std::map<std::string, Protocol::MemorySuccess> ranks;
    bool isCluster = false;

    std::unique_ptr<JsonMemoryDataBase> database;
    std::unique_ptr<ThreadPool> threadPool;
    std::map<std::string, std::future<void>> futureMap;

    Record mapperToRecordDetail(std::map<std::string, size_t> dataMap, std::vector<std::string>);
    Operator mapperToOperatorDetail(std::map<std::string, size_t> dataMap, std::vector<std::string>);
    StaticOp mapperToStaticOpDetail(std::map<std::string, size_t> dataMap, std::vector<std::string>);

    void GetMapValid(const std::vector<std::string> &vec, std::map<std::string, size_t> dataMap);
    std::vector<std::string> GetPeerDirOperatorFile(const std::string& operatorFile, const std::string &reg);
    std::map<std::string, MemoryFilePairs> GetMemoryFiles(const std::vector<std::string>& paths);
    std::vector<std::string> GetMemoryRecordFileLists(const std::vector<std::string>& paths);
    static void SetParseCallBack();
    static void ParseEndCallBack(const std::string& fileId, bool result, const std::string &message);
    static void ParseCallBack(const std::string &fileId, bool result, const std::string &msg);

    static void PreParseTask(const MemoryFilePairs& filePair, const std::string& fileId);
    static bool ParseTask(const MemoryFilePairs& filePair, const std::string& fileId, std::string &message);
    static bool InitParser(const MemoryFilePairs& filePair, const std::string& fileId, std::string &message);
};
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic

#endif // PROFILER_SERVER_MEMORYPARSE_H
