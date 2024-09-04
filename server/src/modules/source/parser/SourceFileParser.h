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
    DETAILS_INTER_CORE_LOAD_GRAPH = 12,
    DETAILS_ROOFLINE = 13
};

struct Position {
    int64_t startPos;
    int64_t endPos;
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
    bool GetDetailsBaseInfo(Protocol::DetailsBaseInfoResBody &responseBody, bool isBaseline);
    bool GetDetailsLoadInfo(Protocol::DetailsLoadInfoResBody &responseBody, bool isBaseline);
    bool GetDetailsMemoryGraph(const std::string& targetBlockId, bool isBaseline,
                               Protocol::DetailsMemoryGraphResBody &responseBody);
    bool GetDetailsMemoryTable(const std::string& targetBlockId, bool isBaseline,
                               Protocol::DetailsMemoryTableResBody &responseBody);
    void ConvertToData();
    int64_t GetSimulationPid(const std::string &fileId, const std::string &processName);
    int64_t GetSimulationTid(const std::string &fileId, const std::string &processName, const std::string &threadName);
    bool GetDetailsInterCoreLoadAnalysisGraph(Protocol::DetailsInterCoreLoadGraphBody& responseBody);
    bool GetDetailsRoofline(Protocol::DetailsRooflineBody &responseBody);
    void SetFilePath(const std::string &inputFilePath);
    void SetBaselineFilePath(const std::string &inputFilePath);

private:
    std::string filePath;
    std::map<int, std::vector<Position>> dataBlockMap;
    std::map<std::string, std::pair<int64_t, int64_t>> sourceFiles;
    std::map<std::string, std::pair<int64_t, int64_t>> traceFiles;

    std::string baselineFilePath;
    std::map<int, std::vector<Position>> baselineDataBlockMap;

    std::vector<std::string> apiCores;
    std::map<std::string, std::vector<SourceFileLine>> apiFiles;
    Position apiInstrPos;

    void ConvertApiInstr(const std::string &jsonStr);
    void ConvertApiFile(const std::string &jsonStr);
    std::map<std::string, std::vector<SourceFileLine>> ConvertToFileMap(rapidjson::Value &fileArray);
    std::vector<SourceFileLine> ConvertToLineArray(rapidjson::Value &lineArray);
    std::string GetSingleContentStrByDataType(std::ifstream &file, DataTypeEnum dataTypeEnum,
                                              bool isBaseline = false);
    std::optional<Protocol::SubBlockData> ConvertStrToSubBlockData(const std::string& str);
    std::string GetContentStr(std::ifstream &file, const Position &position) const;
    static std::string GetUnitType(int64_t unitTypeNumber);
    bool IsDataSizeExceedUpperLimit(uint64_t realSize, uint64_t upperLimit) const;
    static Protocol::MemoryGraph ParseJsonToMemoryGraph(const json_t &json);
    static Protocol::MemoryTable ParseJsonToMemoryTable(const json_t &json);
    static Protocol::UtilizationRate ParseJsonToUtilizationRate(const json_t &json);
    static Protocol::DetailsBaseInfoResBody ParseJsonToBaseInfo(const document_t &json);

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

    static Protocol::CompareData<Protocol::SubBlockUnitData> ParseSubBlockUnitData(const json_t &item);
    const int dataSizeLen = 8; // 数据类型字段距离数据大小字段的偏移
    const int dataTypeLen = 1; // 填充长度字段距离数据类型字段的偏移
    const int paddingLen = 1;  // 填充长度字段距离数据类型字段的偏移
    const int reserveLen = 2;  // 实际数据距离填充长度字段的偏移
    const int filePathLen = 4096;
};
} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic


#endif // PROFILER_SERVER_SOURCEFILEPARSER_H
