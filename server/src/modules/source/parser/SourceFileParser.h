/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SOURCEFILEPARSER_H
#define PROFILER_SERVER_SOURCEFILEPARSER_H

#include <map>
#include <unordered_map>
#include "document.h"
#include "rapidjson.h"
#include "document.h"
#include "FileParser.h"
#include "ThreadPool.h"
#include "SourceProtocolResponse.h"
#include "JsonFileProcess.h"
#include "SourceProtocol.h"
#include "SourceInstructionParser.h"
#include "SafeFile.h"

namespace Dic {
namespace Module {
namespace Source {
class SourceFileParser : public FileParser {
public:
    const static uint16_t reverseConst = 0x5a5a;
    static SourceFileParser &Instance();

    SourceFileParser();
    ~SourceFileParser() override;

    bool Parse(const std::vector<std::string> &filePaths, const std::string &fileId,
        const std::string &selectedFile) override;
    void Reset() override;
    bool CheckOperatorBinary(const std::string &selectedFilePath, std::string &errMsg = EMPTY_ERROR_MSG);
    static void PreParseTask(const std::string &fileId);
    static bool InitParser(const std::string &fileId);
    static void ParseEndCallBack(const std::string &fileId, bool result, const std::string &message);
    static void ParseTask(const std::string &fileId, std::pair<int64_t, int64_t> pos);
    static void EndParseTask(const std::string &fileId, std::shared_ptr<std::vector<std::future<void>>> futures);
    std::vector<std::string> GetCoreList();
    std::vector<std::string> GetSourceList();
    std::vector<SourceFileLine> GetApiLinesByCoreAndSource(const std::string &core, const std::string &sourceName);
    std::vector<SourceFileLineDynamicCol> GetApiLinesDynamic(const std::string &core, const std::string &sourceName);
    std::string GetInstr();
    std::vector<SourceFileInstructionDynamicCol> GetInstrDynamic(std::string &coreName);
    std::map<std::string, int> GetInstructionColumnTypeMap();
    std::map<std::string, int> GetSourceLineColumnTypeMap();
    std::string GetSourceByName(std::string &sourceName);
    bool GetDetailsBaseInfo(Protocol::DetailsBaseInfoResBody &responseBody, bool isBaseline);
    bool GetDetailsLoadInfo(Protocol::DetailsLoadInfoResBody &responseBody, bool isBaseline);
    bool GetDetailsMemoryGraph(const std::string& targetBlockId, bool isBaseline,
                               Protocol::DetailsMemoryGraphResBody &responseBody);
    bool GetDetailsMemoryTable(const std::string& targetBlockId, bool isBaseline,
                               Protocol::DetailsMemoryTableResBody &responseBody);
    void ConvertToData();
    bool GetDetailsInterCoreLoadAnalysisGraph(Protocol::DetailsInterCoreLoadGraphBody& responseBody, bool isBaseline);
    bool GetDetailsRoofline(Protocol::DetailsRooflineBody &responseBody);
    void SetFilePath(const std::string &inputFilePath);
    void SetBaselineFilePath(const std::string &inputFilePath);
    bool IsBaselineParsed(const std::string &inputFilePath);
    void SynchronizeBaselineInfo();
    void ResetBaseline();

private:
    std::string filePath;
    std::map<int, std::vector<Position>> dataBlockMap;
    std::map<std::string, std::pair<int64_t, int64_t>> traceFiles;
    SourceInstructionParser sourceInstructionParser;

    std::string baselineFilePath;
    std::map<int, std::vector<Position>> baselineDataBlockMap;

    static uint64_t CalculateTotalSize(std::vector<std::pair<int64_t, int64_t>> &filePos);

    std::unique_ptr<ThreadPool> threadPool;
    const int maxThreadNum = 4;

    std::mutex mutex;
    std::mutex trackMutex;
    std::mutex processMutex;
    std::mutex threadMutex;
    std::unordered_map<std::string, std::map<std::pair<std::string, int64_t>, int64_t>> trackIdMap;
    std::unordered_map<std::string, std::map<std::string, int64_t>> simulationPidMap;
    std::unordered_map<std::string, std::map<std::pair<std::string, std::string>, int64_t>> simulationTidMap;
    int64_t trackId = 0;
    int64_t pid = 0;
    int64_t tid = 0;

    const int dataSizeLen = 8; // 数据类型字段距离数据大小字段的偏移
    const int dataTypeLen = 1; // 填充长度字段距离数据类型字段的偏移
    const int paddingLen = 1;  // 填充长度字段距离数据类型字段的偏移
    const int reserveLen = 2;  // 实际数据距离填充长度字段的偏移
    const int filePathLen = 4096;

    bool ParseDataBlocks(std::ifstream &file, long long fileSize,
                         std::map<int, std::vector<Position>> &curDataBlockMap);
};
} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic


#endif // PROFILER_SERVER_SOURCEFILEPARSER_H
