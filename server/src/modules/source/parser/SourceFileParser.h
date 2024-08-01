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

namespace Dic {
namespace Module {
namespace Source {
struct SourceFileLine {
    int line;
    std::vector<float> cycles;
    std::vector<int> instructionsExecuted;
    std::vector<std::pair<std::string, std::string>> addressRange;
};

enum class DataTypeEnum : int {
    SOURCE = 1,
    TRACE = 2,
    API_FILE = 3,
    API_INSTR = 4,
    DETAILS_BASE_INFO = 5,
    DETAILS_COMPUTE_LOAD_GRAPH = 6,
    DETAILS_COMPUTE_LOAD_TABLE = 7,
    DETAILS_MEMORY_GRAPH = 8,
    DETAILS_MEMORY_TABLE = 9,
};

class SourceFileParser : public FileParser {
public:
    const static uint16_t reverseConst = 0x5a5a;
    const static uint16_t filePathLengthConst = 4096;
    const static uint16_t addressRangeSize = 2;

    static SourceFileParser &Instance();

    SourceFileParser();
    ~SourceFileParser() override;

    bool Parse(const std::vector<std::string> &filePaths, const std::string &fileId,
        const std::string &selectedFile) override;
    void Reset() override;
    bool CheckOperatorBinary(const std::string &selectedFilePath);
    static void PreParseTask(const std::string &fileId);
    static bool InitParser(const std::string &fileId);
    static void ParseEndCallBack(const std::string &fileId, bool result, const std::string &message);
    static void ParseTask(const std::string &fileId, std::pair<int64_t, int64_t> pos);
    static void EndParseTask(const std::string &fileId, std::shared_ptr<std::vector<std::future<void>>> futures);
    static std::pair<int64_t, int64_t> AdjustPosition(std::ifstream &file, int64_t start, int64_t end);
    std::vector<std::string> GetCoreList();
    std::vector<std::string> GetSourceList();
    std::vector<SourceFileLine> GetApiLinesByCoreAndSource(const std::string &core, const std::string &sourceName);
    std::string GetInstr();
    std::string GetSourceByName(std::string sourceName);
    bool GetDetailsBaseInfo(Protocol::DetailsBaseInfoResBody &responseBody);
    bool GetDetailsLoadInfo(Protocol::DetailsLoadInfoResBody &responseBody);
    bool GetDetailsMemoryGraph(const std::string& targetBlockId, Protocol::DetailsMemoryGraphResBody &responseBody);
    bool GetDetailsMemoryTable(const std::string& targetBlockId, Protocol::DetailsMemoryTableResBody &responseBody);
    void ConvertToData();
    int64_t GetSimulationPid(const std::string &fileId, const std::string &processName);
    int64_t GetSimulationTid(const std::string &fileId, const std::string &processName, const std::string &threadName);

private:
    std::string filePath;
    std::map<int, std::vector<std::pair<int64_t, int64_t>>> dataBlockMap;
    std::map<std::string, std::pair<int64_t, int64_t>> sourceFiles;
    std::map<std::string, std::pair<int64_t, int64_t>> traceFiles;

    std::vector<std::string> apiCores;
    std::map<std::string, std::vector<SourceFileLine>> apiFiles;
    std::pair<int64_t, int64_t> apiInstrPos;

    void ConvertApiInstr(const std::string &jsonStr);
    void ConvertApiFile(const std::string &jsonStr);
    std::map<std::string, std::vector<SourceFileLine>> ConvertToFileMap(rapidjson::Value &fileArray);
    std::vector<SourceFileLine> ConvertToLineArray(rapidjson::Value &lineArray);
    std::string GetSingleContentStrByDataType(std::ifstream &file, DataTypeEnum dataTypeEnum);
    std::optional<Protocol::SubBlockData> ConvertStrToSubBlockData(const std::string& str);
    std::string GetContentStr(std::ifstream &file, const std::pair<int64_t, int64_t> &pair) const;
    std::string GetUnitType(int64_t unitTypeNumber);
    bool IsDataSizeExceedUpperLimit(uint64_t realSize, uint64_t upperLimit) const;
    static Protocol::MemoryGraph ParseJsonToMemoryGraph(const json_t &json);
    static Protocol::MemoryTable ParseJsonToMemoryTable(const json_t &json);
    static Protocol::UtilizationRate ParseJsonToUtilizationRate(const json_t &json);

    std::unique_ptr<ThreadPool> threadPool;
    const int maxThreadNum = 4;

    std::mutex trackMutex;
    std::mutex processMutex;
    std::mutex threadMutex;
    std::unordered_map<std::string, std::map<std::pair<std::string, int64_t>, int64_t>> trackIdMap;
    std::unordered_map<std::string, std::map<std::string, int64_t>> simulationPidMap;
    std::unordered_map<std::string, std::map<std::pair<std::string, std::string>, int64_t>> simulationTidMap;
    int64_t trackId = 0;
    int64_t pid = 0;
    int64_t tid = 0;
    std::map<int64_t, std::string> unitTypeMapping = {
        {0, "Duration(μs)"},
        {1, "Instructions"},
        {2, "Data Volume(byte)"},
        {3, "PRE"}
    };
};
} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic


#endif // PROFILER_SERVER_SOURCEFILEPARSER_H
