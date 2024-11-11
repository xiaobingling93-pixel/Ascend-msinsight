/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "SourceFileParser.h"
#include <fstream>
#include <unordered_set>
#include "ServerLog.h"
#include "rapidjson.h"
#include "document.h"
#include "InterCoreLoadGraphParser.h"
#include "JsonUtil.h"
#include "ParserStatusManager.h"
#include "DataBaseManager.h"
#include "EventParser.h"
#include "TraceTime.h"
#include "FileUtil.h"
#include "TextTraceDatabase.h"
#include "CacheManager.h"
#include "BaselineManager.h"
#include "SourceProtocolResponse.h"
#include "RooflineParser.h"

namespace Dic {
namespace Module {
namespace Source {
using namespace Dic;
using namespace Dic::Server;
using namespace Dic::Module::FullDb;

static inline std::map<int64_t, std::string> unitTypeMapping = {
    {0, "Duration(μs)"},
    {1, "Instructions"},
    {2, "Data Volume(byte)"},
    {3, "PRE"}
};

SourceFileParser &SourceFileParser::Instance()
{
    static SourceFileParser instance;
    return instance;
}
SourceFileParser::SourceFileParser()
{
    threadPool = std::make_unique<ThreadPool>(SourceFileParser::maxThreadNum);
}

SourceFileParser::~SourceFileParser()
{
    threadPool->ShutDown();
}

bool SourceFileParser::Parse(const std::vector<std::string> &filePaths, const std::string &fileId,
    const std::string &selectedFile)
{
    if (!FileUtil::CheckFilePathLength(selectedFile)) {
        ServerLog::Error("Parse bin file failed cause path length is too long.");
        return false;
    }
    std::ifstream file = FileUtil::OpenReadFileSafely(selectedFile, std::ios::binary);
    if (!file) {
        ServerLog::Error("Open bin file failed: ", selectedFile);
        return false;
    }

    // 获取目标数据对象的应用，下面对数据进行改动时会影响
    auto &curDataBlockMap = Global::BaselineManager::Instance().IsBaselineId(fileId) ?
        baselineDataBlockMap : dataBlockMap;

    bool success = ParseDataBlocks(file, curDataBlockMap);
    if (!success) {
        return false;
    }
    file.close();
    Timeline::ParserStatusManager::Instance().SetParserStatus(fileId, Timeline::ParserStatus::INIT);
    threadPool->AddTask(PreParseTask, fileId);
    return true;
}

bool SourceFileParser::ParseDataBlocks(std::ifstream &file, std::map<int, std::vector<Position>> &curDataBlockMap)
{
    while (!file.eof()) {
        uint64_t dataSize;
        uint8_t dataType;
        uint8_t paddingLength;

        auto fileSize = FileUtil::GetFileSize(filePath.c_str());
        if (fileSize <= 0) {
            ServerLog::Error("Get bin file size failed : ", fileSize);
            return false;
        }
        file.read(reinterpret_cast<char *>(&dataSize), dataSizeLen);
        if (dataSize > static_cast<uint64_t>(fileSize)) {
            ServerLog::Error("The size of data block is greater than bin file size : ", dataSize);
            return false;
        }

        file.read(reinterpret_cast<char *>(&dataType), dataTypeLen);
        file.read(reinterpret_cast<char *>(&paddingLength), paddingLen);
        dataType = static_cast<int>(dataType);
        paddingLength = static_cast<int>(paddingLength);
        if (dataType == static_cast<int>(DataTypeEnum::SOURCE)) {
            if (static_cast<uint64_t>(INT64_MAX - filePathLen) < dataSize) {
                // 溢出防护
                ServerLog::Error("Source code data block in selected file is invalid which data size is :", dataSize);
                return false;
            }
            dataSize = dataSize + filePathLen;
        }

        if (!file) {
            break;
        }
        // 跳转到实际数据的开始
        file.seekg(reserveLen, std::ios::cur);

        int64_t startPos = file.tellg();
        if (startPos < 0) {
            ServerLog::Error("Parse source file failed, source file is invalid.");
            return false;
        }
        if (dataSize > static_cast<uint64_t>(INT64_MAX - startPos) || startPos + dataSize - paddingLength > INT64_MAX) {
            // 溢出防护
            ServerLog::Error("Data block in selected file is invalid which data size is :", dataSize);
            return false;
        }
        int64_t endPos = startPos + dataSize - paddingLength;
        if (startPos >= endPos) {
            ServerLog::Error("Data error: the start position is greater than the end position.");
            return false;
        }
        Position position = {startPos, endPos};
        curDataBlockMap[dataType].emplace_back(position);

        // 跳转到下一个数据块的开始位置，考虑到当前数据块的大小和填充
        file.seekg(dataSize, std::ios::cur);
    }
    return true;
}

void SourceFileParser::PreParseTask(const std::string &fileId)
{
    ServerLog::Info("Start to parse simulation timeline file. file id: ", fileId);
    if (!InitParser(fileId)) {
        ParseEndCallBack(fileId, false, "Failed to open db. Please delete dbFile and try again.");
    }
}

bool SourceFileParser::InitParser(const std::string &fileId)
{
    if (!Timeline::ParserStatusManager::Instance().SetRunningStatus(fileId)) {
        ServerLog::Info("Pre task skip this file cause set running status failed: ", fileId);
        return true;
    }
    auto db = DataBaseManager::Instance().GetTraceDatabase(fileId);
    if (db == nullptr) {
        ServerLog::Error("Failed to get database connection for: ", fileId);
        return false;
    }
    auto database = std::dynamic_pointer_cast<TextTraceDatabase, VirtualTraceDatabase>(db);
    if (database == nullptr || !(database->DropTable() && database->CreateTable())) {
        ServerLog::Error("Failed to open trace database. rankId:", fileId);
        return false;
    }
    auto &instance = SourceFileParser::Instance();
    auto curDataBlockMap = Global::BaselineManager::Instance().IsBaselineId(fileId) ?
        instance.baselineDataBlockMap :  instance.dataBlockMap;
    std::string curFilePath = Global::BaselineManager::Instance().IsBaselineId(fileId) ?
        instance.baselineFilePath : instance.filePath;
    std::vector<Position> &traceFilePos = curDataBlockMap[static_cast<int>(DataTypeEnum::TRACE)];
    std::ifstream file = FileUtil::OpenReadFileSafely(curFilePath, std::ios::in | std::ios::binary);
    if (!file) {
        ServerLog::Error("Failed to open file. filePath:", curFilePath);
        return false;
    }
    std::vector<std::pair<int64_t, int64_t>> adjustTraceFilePos;
    adjustTraceFilePos.reserve(traceFilePos.size());
    for (const auto &pos : traceFilePos) {
        adjustTraceFilePos.emplace_back(pos.startPos, pos.endPos);
    }
    file.close();

    // 计算待解析的文件大小
    uint64_t totalSize = 0;
    if (!adjustTraceFilePos.empty()) {
        totalSize = CalculateTotalSize(adjustTraceFilePos);
        if (totalSize == 0) {
            ServerLog::Error("Failed to calculate total size of data block.");
            return false;
        }
    }

    instance.fileProgressMap[fileId] = std::make_unique<FileProgress>(0, totalSize);

    std::shared_ptr<std::vector<std::future<void>>> futures = std::make_shared<std::vector<std::future<void>>>();
    for (const auto &pos : adjustTraceFilePos) {
        auto future = instance.threadPool->AddTask(ParseTask, fileId, pos);
        futures->emplace_back(std::move(future));
    }
    instance.threadPool->AddTask(EndParseTask, fileId, futures);
    return true;
}

uint64_t SourceFileParser::CalculateTotalSize(std::vector<std::pair<int64_t, int64_t>> &filePos)
{
    // 计算待解析的文件大小
    uint64_t totalSize = 0;
    for (const auto &pos : filePos) {
        int64_t size = pos.second - pos.first;
        if (size <= 0) {
            ServerLog::Error("End position of data block should be greater than start position.");
            return 0;
        }
        if (static_cast<uint64_t>(size) > UINT64_MAX - totalSize) {
            ServerLog::Error("Addition overflowed, source file is too large.");
            return 0;
        }
        totalSize += size;
    }

    return totalSize;
}

void SourceFileParser::EndParseTask(const std::string &fileId, std::shared_ptr<std::vector<std::future<void>>> futures)
{
    if (Timeline::ParserStatusManager::Instance().GetParserStatus(fileId) != Timeline::ParserStatus::RUNNING) {
        ServerLog::Info("End parse task skip this file cause timeline parser status is not running: ", fileId);
        return;
    }
    ServerLog::Info("Wait parse completed. ID:", fileId);
    for (const auto &future : *futures) {
        future.wait();
    }
    ServerLog::Info("Parse completed. ID:", fileId);
    auto db = DataBaseManager::Instance().GetTraceDatabase(fileId);
    if (db == nullptr) {
        ServerLog::Error("Failed to get database connection in end parse task. fileId:", fileId);
        return;
    }
    auto database = std::dynamic_pointer_cast<TextTraceDatabase, VirtualTraceDatabase>(db);
    if (database == nullptr) {
        ServerLog::Error("Failed to cast virtual trace database to json trace database in end parse task of source.");
        return;
    }
    database->CreateIndex();
    database->SimulationUpdateProcessSortIndex();
    CacheManager::Instance().ClearCacheByFileId(fileId);
    ServerLog::Info("Update depth completed. ID:", fileId);
    ParseEndCallBack(fileId, true, "");
}

void SourceFileParser::ParseTask(const std::string &fileId, std::pair<int64_t, int64_t> pos)
{
    if (Timeline::ParserStatusManager::Instance().GetParserStatus(fileId) != Timeline::ParserStatus::RUNNING) {
        ServerLog::Info("Parse task skip this file cause timeline parser status is not running. ID:", fileId);
        return;
    }
    ServerLog::Info("Start parse timeline from bin file:", fileId);
    auto &instance = SourceFileParser::Instance();
    std::string curFilePath = Global::BaselineManager::Instance().IsBaselineId(fileId) ? instance.baselineFilePath :
        instance.filePath;
    Timeline::EventParser eventParser(curFilePath, fileId);
    eventParser.SetSimulationStatus(true);
    // 先将文件内容切片，避免一次解析的数据量过大
    for (const auto& pair : JsonFileProcess::SplitFile(curFilePath, pos)) {
        if (!eventParser.Parse(pair.first, pair.second)) {
            if (Timeline::ParserStatusManager::Instance().SetTerminateStatus(fileId) ==
                Timeline::ParserStatus::RUNNING) {
                // 只发送一次解析失败事件
                ParseEndCallBack(fileId, false, eventParser.GetError());
                break;
            }
        }
        // 发送解析进度事件
        std::unique_ptr<FileProgress> &curFileProgress = instance.fileProgressMap[fileId];
        curFileProgress->AddToParsedSize(pair.second - pair.first);
        instance.paserProgressCallback(fileId, curFileProgress->GetParsedSize(), curFileProgress->GetTotalSize(),
                                       curFileProgress->GetProgressPercentage());
    }
}

void SourceFileParser::ParseEndCallBack(const std::string &fileId, bool result, const std::string &message)
{
    if (!(result && Timeline::ParserStatusManager::Instance().SetFinishStatus(fileId))) {
        result = false;
    }
    auto &instance = SourceFileParser::Instance();
    if (instance.paserEndCallback != nullptr) {
        instance.paserEndCallback(fileId, result, message);
    }
}

void SourceFileParser::Reset()
{
    std::unique_lock<std::mutex> lock(mutex);
    ServerLog::Info("Reset file parser and wait task completed.");
    Timeline::ParserStatusManager::Instance().SetAllTerminateStatus();
    Timeline::ParserStatusManager::Instance().SetClusterParseStatus(Timeline::ParserStatus::TERMINATE);
    threadPool->Reset();
    ServerLog::Info("Task completed.");
    auto connList = Timeline::DataBaseManager::Instance().GetAllTraceDatabase();
    for (auto &conn : connList) {
        std::string path = conn->GetDbPath();
        conn->Stop();
        if (!FileUtil::RemoveFileExDb(path)) {
            ServerLog::Error("Failed to remove file. ", path);
        }
    }
    trackIdMap.clear();
    simulationPidMap.clear();
    simulationTidMap.clear();
    trackId = 0;
    tid = 0;
    pid = 0;

    dataBlockMap.clear();
    sourceFiles.clear();
    traceFiles.clear();
    apiCores.clear();
    apiFiles.clear();
    apiInstrPos = {0, 0};

    Timeline::DataBaseManager::Instance().Clear();
    Timeline::TraceTime::Instance().Reset();
    FileParser::Reset();
    Timeline::ParserStatusManager::Instance().ClearAllParserStatus();
    ServerLog::Info("End reset file parser.");
}

bool SourceFileParser::CheckOperatorBinary(const std::string &selectedFilePath)
{
    if (!FileUtil::CheckFilePathLength(selectedFilePath)) {
        ServerLog::Error("File path length check failed.");
        return false;
    }
    std::ifstream file = FileUtil::OpenReadFileSafely(selectedFilePath, std::ios::binary);
    if (!file) {
        return false;
    }

    const int reversePadding = 10;
    uint64_t contentLength;
    uint16_t reverse;

    file.read(reinterpret_cast<char *>(&contentLength), sizeof(contentLength));
    file.seekg(reversePadding, std::ios::beg);
    file.read(reinterpret_cast<char *>(&reverse), sizeof(reverse));

    if (!file) {
        ServerLog::Error("Can't read contentLength and reverse.");
        return false;
    }

    bool isBinary = (contentLength != 0) && (reverse == SourceFileParser::reverseConst);
    file.close();
    return isBinary;
}

std::vector<std::string> SourceFileParser::GetCoreList()
{
    return this->apiCores;
}

std::vector<std::string> SourceFileParser::GetSourceList()
{
    std::vector<std::string> sourceList;
    for (const auto &entry : sourceFiles) {
        sourceList.push_back(entry.first);
    }
    return sourceList;
}

std::vector<SourceFileLine> SourceFileParser::GetApiLinesByCoreAndSource(const std::string &core,
                                                                         const std::string &sourceName)
{
    std::vector<SourceFileLine> result;

    auto it = std::find(apiCores.begin(), apiCores.end(), core);
    if (it == apiCores.end()) {
        ServerLog::Error("Can't find the specified core name : ", core);
        return result;
    }
    // never below zero
    size_t index = std::distance(apiCores.begin(), it);

    if (apiFiles.find(sourceName) == apiFiles.end()) {
        ServerLog::Warn("The specified file doesn't exist in api files, and source name is:", sourceName);
        return result;
    }
    std::vector<SourceFileLine> &vector = apiFiles[sourceName];
    for (auto line : vector) {
        if (line.cycles.size() < index + 1 || line.instructionsExecuted.size() < index + 1) {
            continue;
        }

        // filter lines without instruction executed
        if (line.instructionsExecuted[index] == 0 && line.cycles[index] == 0) {
            continue;
        }

        SourceFileLine output;
        for (const auto &pair : line.addressRange) {
            output.addressRange.emplace_back(pair.first, pair.second);
        }
        output.cycles.emplace_back(line.cycles[index]);
        output.instructionsExecuted.emplace_back(line.instructionsExecuted[index]);
        output.line = line.line;

        result.emplace_back(output);
    }
    return result;
}

std::string SourceFileParser::GetInstr()
{
    std::ifstream file = FileUtil::OpenReadFileSafely(filePath, std::ios::binary);
    if (!file) {
        ServerLog::Error("Can't open file,please check file exist or not,file name :", filePath);
        return "";
    }
    int64_t start = apiInstrPos.startPos;
    int64_t end = apiInstrPos.endPos;
    if (end < start) {
        ServerLog::Error("The dataSize parameter is error. The value of end is smaller than start.");
        file.close();
        return "";
    }
    uint64_t dataSize = end - start;
    constexpr uint64_t maxDataSize = 1024 * 1024 * 200; // limit data size to 200MB
    if (IsDataSizeExceedUpperLimit(dataSize, maxDataSize)) {
        ServerLog::Error("Data size of instruction exceeds max upper limit.");
        return "";
    }

    file.seekg(start, std::ios::beg);

    std::string content(dataSize, '\0');
    if (!file.read(&content[0], static_cast<std::streamsize>(dataSize))) {
        ServerLog::Error("Can't read file,please check file exist or not,file name :", filePath);
        file.close();
        return "";
    }
    if (!StringUtil::IsUtf8String(content)) {
        ServerLog::Error("Can't decode a text frame as utf-8,instr content is invalid.");
        file.close();
        return "";
    }
    file.close();
    return content;
}

std::string SourceFileParser::GetSourceByName(std::string sourceName)
{
    if (sourceFiles.count(sourceName) == 0) {
        ServerLog::Warn("Don't exist the specified file ", sourceName);
        return "";
    }
    std::pair<int64_t, int64_t> &pos = sourceFiles[sourceName];
    int64_t start = pos.first;
    int64_t end = pos.second;
    int64_t dataSize = end - start;
    constexpr uint64_t maxDataSize = 1024 * 1024 * 100; // limit data size to 100MB
    if (IsDataSizeExceedUpperLimit(dataSize, maxDataSize)) {
        ServerLog::Error("Data size of source file exceeds max upper limit.");
        return "";
    }

    std::ifstream file = FileUtil::OpenReadFileSafely(filePath, std::ios::binary);
    if (!file) {
        ServerLog::Error("Can't open file,please check file exist or not,file name :", filePath);
        return "";
    }
    file.seekg(start, std::ios::beg);

    std::string content(dataSize, '\0');
    if (!file.read(&content[0], static_cast<std::streamsize>(dataSize))) {
        ServerLog::Error("Can't read file,please check file exist or not,file name :", filePath);
        return "";
    }
    if (!StringUtil::IsUtf8String(content)) {
        ServerLog::Error("Can't decode a text frame as utf-8,source name :", sourceName);
        file.close();
        return "";
    }
    file.close();
    return content;
}

bool SourceFileParser::GetDetailsBaseInfo(Protocol::DetailsBaseInfoResBody &responseBody, bool isBaseline)
{
    std::string curFilePath = isBaseline ? baselineFilePath : filePath;
    std::ifstream file = FileUtil::OpenReadFileSafely(curFilePath, std::ios::binary);
    std::string baseInfo = GetSingleContentStrByDataType(file, DataTypeEnum::DETAILS_BASE_INFO, isBaseline);
    file.close();
    if (baseInfo.empty()) {
        ServerLog::Info("Details base info data does not exist.");
        return true;
    }
    try {
        std::string error;
        auto baseInfoJson = JsonUtil::TryParse<kParseNumbersAsStringsFlag>(baseInfo, error);
        if (!error.empty()) {
            ServerLog::Error("Get base info error:", error);
            return false;
        }
        responseBody = ParseJsonToBaseInfo(baseInfoJson.value());
        return true;
    } catch (const std::exception &e) {
        ServerLog::Error("Can't parse details base info,not json.Error is ", e.what());
        return false;
    }
}

Protocol::DetailsBaseInfoResBody SourceFileParser::ParseJsonToBaseInfo(const document_t &json)
{
    Protocol::DetailsBaseInfoResBody baseInfoRes;
    baseInfoRes.name = JsonUtil::GetString(json, "name");
    baseInfoRes.soc = JsonUtil::GetString(json, "soc");
    baseInfoRes.opType = JsonUtil::GetString(json, "op_type");
    baseInfoRes.blockDim = JsonUtil::GetString(json, "block_dim");
    baseInfoRes.mixBlockDim = JsonUtil::GetString(json, "mix_block_dim");
    baseInfoRes.duration = JsonUtil::GetString(json, "duration");
    baseInfoRes.deviceId = JsonUtil::GetString(json, "device_id");
    baseInfoRes.pid = JsonUtil::GetString(json, "pid");

    if (!json.HasMember("mix_block_detail") && !json.HasMember("block_detail")) {
        return baseInfoRes;
    }
    const Value& blockDetailsValue = baseInfoRes.opType == "mix" && json.HasMember("mix_block_detail") ?
        json["mix_block_detail"] : json["block_detail"];
    if (!blockDetailsValue.IsObject()) {
        return baseInfoRes;
    }
    Protocol::TableDetail<Protocol::TableRow> tableDetail;
    tableDetail.size = JsonUtil::GetVector<std::string>(blockDetailsValue, "size");
    tableDetail.headerName = JsonUtil::GetVector<std::string>(blockDetailsValue, "head_name");
    if (blockDetailsValue.HasMember("row") && blockDetailsValue["row"].IsArray()) {
        const Value &row = blockDetailsValue["row"];
        for (const auto &dataRow: row.GetArray()) {
            Protocol::TableRow memoryTableRow;
            memoryTableRow.value = JsonUtil::GetVector<std::string>(dataRow, "value");
            tableDetail.row.push_back(memoryTableRow);
        }
    }
    baseInfoRes.blockDetail = tableDetail;
    return baseInfoRes;
}

bool SourceFileParser::GetDetailsLoadInfo(Protocol::DetailsLoadInfoResBody & responseBody, bool isBaseline)
{
    std::string curFilePath = isBaseline ? baselineFilePath : filePath;
    // 从文件获取内容
    std::ifstream file = FileUtil::OpenReadFileSafely(curFilePath, std::ios::binary);
    std::string loadGraph = GetSingleContentStrByDataType(file, DataTypeEnum::DETAILS_COMPUTE_LOAD_GRAPH, isBaseline);
    std::string loadTable = GetSingleContentStrByDataType(file, DataTypeEnum::DETAILS_COMPUTE_LOAD_TABLE, isBaseline);
    file.close();
    if (loadGraph.empty() && loadTable.empty()) {
        ServerLog::Info("Details load data does not exist.");
        return false;
    }

    std::optional<Protocol::SubBlockData> blockData = ConvertStrToSubBlockData(loadGraph);
    if (blockData.has_value()) {
        responseBody.chartData = blockData.value();
    }
    std::optional<Protocol::SubBlockData> tableData = ConvertStrToSubBlockData(loadTable);
    if (tableData.has_value()) {
        responseBody.tableData = tableData.value();
    }

    // 获取blockid列表(以compare数据为准)
    std::unordered_set<std::string> blockIdSet;
    for (const auto &item: responseBody.chartData.detailDataList) {
        blockIdSet.insert(item.compare.blockId);
    }
    for (const auto &item: responseBody.tableData.detailDataList) {
        blockIdSet.insert(item.compare.blockId);
    }
    std::copy(blockIdSet.begin(), blockIdSet.end(), std::back_inserter(responseBody.blockIdList));
    return true;
}

bool SourceFileParser::GetDetailsMemoryGraph(const std::string& targetBlockId, bool isBaseline,
                                             Protocol::DetailsMemoryGraphResBody &responseBody)
{
    if (targetBlockId.empty()) {
        ServerLog::Error("Block id is empty.");
        return true;
    }
    // 读取内存表数据
    std::string curFilePath = isBaseline ? baselineFilePath : filePath;
    std::ifstream file = FileUtil::OpenReadFileSafely(curFilePath, std::ios::binary);
    std::string memoryGraph = GetSingleContentStrByDataType(file, DataTypeEnum::DETAILS_MEMORY_GRAPH, isBaseline);
    file.close();
    if (memoryGraph.empty()) {
        ServerLog::Info("Details memory graph data does not exist.");
        return true;
    }
    try {
        std::string error;
        auto graphJson = JsonUtil::TryParse<kParseNumbersAsStringsFlag>(memoryGraph, error);
        if (!error.empty()) {
            ServerLog::Error("Parse memory graph data error:", error);
            return false;
        }
        if (!graphJson.value().HasMember("core_memory_map") || !graphJson.value()["core_memory_map"].IsArray()) {
            ServerLog::Error("Memory graph data invalid.");
            return false;
        }
        Value &coreMemoryList = graphJson.value()["core_memory_map"];
        for (const auto &item: coreMemoryList.GetArray()) {
            if (targetBlockId != JsonUtil::GetString(item, "core_no")) {
                continue;
            }
            Protocol::MemoryGraph temp = ParseJsonToMemoryGraph(item);
            responseBody.coreMemory.push_back(temp);
        }
        if (responseBody.coreMemory.size() > 1) {
            ServerLog::Warn("Details memory graph data is not unique, block id:", targetBlockId);
        }
        return true;
    } catch (const std::exception &e) {
        ServerLog::Error("Can't parse details memory graph data,not json.Error is ", e.what());
        return false;
    }
}

Protocol::UtilizationRate SourceFileParser::ParseJsonToUtilizationRate(const json_t &json)
{
    Protocol::UtilizationRate utilizationRate;
    utilizationRate.cycle = JsonUtil::GetString(json, "cycle");
    utilizationRate.ratio = JsonUtil::GetString(json, "ratio");
    utilizationRate.totalCycles = JsonUtil::GetString(json, "total_cycles");
    return utilizationRate;
}

Protocol::MemoryGraph SourceFileParser::ParseJsonToMemoryGraph(const json_t &json)
{
    Protocol::MemoryGraph temp;
    temp.blockId = JsonUtil::GetString(json, "core_no");
    temp.blockType = JsonUtil::GetString(json, "op_type");
    temp.chipType = JsonUtil::GetString(json, "soc");
    temp.advice = JsonUtil::GetVector<std::string>(json, "advice");

    if (json.HasMember("memory_unit") && json["memory_unit"].IsArray()) {
        auto &memoryUnit = const_cast<Value &>(json["memory_unit"]);
        for (auto &unit: memoryUnit.GetArray()) {
            Protocol::MemoryUnit memoryUnitTemp;
            memoryUnitTemp.memoryPath = JsonUtil::GetString(unit, "memory_path");
            memoryUnitTemp.request = JsonUtil::GetInteger(unit, "request");
            memoryUnitTemp.bandwidth = JsonUtil::GetString(unit, "bandwidth");
            memoryUnitTemp.peakRatio = JsonUtil::GetString(unit, "peak_ratio");
            memoryUnitTemp.display = (JsonUtil::GetInteger(unit, "display") == 1);
            Protocol::CompareData<Protocol::MemoryUnit> compareData;
            compareData.compare = memoryUnitTemp;
            temp.memoryUnit.push_back(compareData);
        }
    }
    if (json.HasMember("L2cache") && json["L2cache"].IsObject()) {
        auto &L2cache = const_cast<Value &>(json["L2cache"]);
        Protocol::L2Cache l2CacheTemp;
        l2CacheTemp.hit = JsonUtil::GetString(L2cache, "hit");
        l2CacheTemp.miss = JsonUtil::GetString(L2cache, "miss");
        l2CacheTemp.totalRequest = JsonUtil::GetString(L2cache, "total_request");
        l2CacheTemp.hitRatio = JsonUtil::GetString(L2cache, "hit_ratio");
        temp.l2Cache.compare = l2CacheTemp;
    }

    if (json.HasMember("Vector") && json["Vector"].IsObject()) {
        temp.vector.compare = ParseJsonToUtilizationRate(json["Vector"]);
    }
    if (json.HasMember("Vector1") && json["Vector1"].IsObject()) {
        temp.vector1.compare = ParseJsonToUtilizationRate(json["Vector1"]);
    }
    if (json.HasMember("Cube") && json["Cube"].IsObject()) {
        temp.cube.compare = ParseJsonToUtilizationRate(json["Cube"]);
    }
    return temp;
}

bool SourceFileParser::GetDetailsMemoryTable(const std::string& targetBlockId, bool isBaseline,
                                             Protocol::DetailsMemoryTableResBody &responseBody)
{
    if (targetBlockId.empty()) {
        ServerLog::Info("Block id is empty.");
        return true;
    }
    std::string curFilePath = isBaseline ? baselineFilePath : filePath;
    std::ifstream file = FileUtil::OpenReadFileSafely(curFilePath, std::ios::binary);
    std::string memoryTable = GetSingleContentStrByDataType(file, DataTypeEnum::DETAILS_MEMORY_TABLE, isBaseline);
    file.close();
    if (memoryTable.empty()) {
        ServerLog::Info("Details memory table data does not exist.");
        return true;
    }
    try {
        std::string error;
        auto tableJson = JsonUtil::TryParse<kParseNumbersAsStringsFlag>(memoryTable, error);
        if (!error.empty()) {
            ServerLog::Error("Parse memory graph data error:", error);
            return false;
        }
        if (!tableJson.value().HasMember("table_per_block") || !tableJson.value()["table_per_block"].IsArray()) {
            ServerLog::Error("Memory table data invalid, can not find array member table per block.");
            return false;
        }
        Value &tableList = tableJson.value()["table_per_block"];
        for (const auto &item: tableList.GetArray()) {
            if (targetBlockId != JsonUtil::GetString(item, "block_id")) {
                continue;
            }
            responseBody.memoryTable.push_back(ParseJsonToMemoryTable(item));
        }
        if (responseBody.memoryTable.size() > 1) {
            ServerLog::Warn("Details memory graph data is not unique, block id:", targetBlockId);
        }
        return true;
    } catch (const std::exception &e) {
        ServerLog::Error("Can't parse details memory graph data,not json.Error is ", e.what());
        return false;
    }
}

Protocol::MemoryTable SourceFileParser::ParseJsonToMemoryTable(const json_t &json)
{
    Protocol::MemoryTable result;
    result.blockId = JsonUtil::GetString(json, "block_id");
    result.tableOpType = JsonUtil::GetString(json, "table_op_type");
    result.advice = JsonUtil::GetVector<std::string>(json, "advice");
    auto &tableDetailJson = const_cast<Value &>(json["table_detail"]);
    for (auto &item: tableDetailJson.GetArray()) {
        Protocol::TableDetail<Protocol::CompareData<Protocol::TableRow>> tableDetail;
        tableDetail.tableName = JsonUtil::GetString(item, "table_name");
        tableDetail.size = JsonUtil::GetVector<std::string>(item, "size");
        tableDetail.headerName = JsonUtil::GetVector<std::string>(item, "header_name");
        Value &row = item["row"];
        for (const auto &dataRow: row.GetArray()) {
            Protocol::TableRow memoryTableRow;
            memoryTableRow.name = JsonUtil::GetString(dataRow, "name");
            memoryTableRow.value = JsonUtil::GetVector<std::string>(dataRow, "value");
            Protocol::CompareData<Protocol::TableRow> compareRow;
            compareRow.compare = memoryTableRow;
            tableDetail.row.push_back(compareRow);
        }
        result.tableDetail.push_back(tableDetail);
    }
    return result;
}

void SourceFileParser::ConvertToData()
{
    std::ifstream file = FileUtil::OpenReadFileSafely(filePath, std::ios::binary);
    if (!file) {
        ServerLog::Error("Can't open file,please check file exist or not,file name :", filePath);
        return;
    }

    std::vector<Position> &sourceFilePos = dataBlockMap[static_cast<int>(DataTypeEnum::SOURCE)];
    for (auto pos : sourceFilePos) {
        int64_t start = pos.startPos;
        int64_t end = pos.endPos;

        file.seekg(start, std::ios::beg);

        std::vector<char> filePathBuffer(filePathLengthConst);
        file.read(filePathBuffer.data(), filePathBuffer.size());
        if (!file) {
            ServerLog::Error("Failed to read file path buffer.");
            break;
        }
        std::string sourceFilePath(filePathBuffer.data());
        if ((start < 0) || (filePathLengthConst > INT64_MAX - start)) {
            ServerLog::Error(std::string("Start position: ") + std::to_string(start) +
            std::string(" is illegal at covert to data in source file."));
            return;
        }
        sourceFiles[sourceFilePath] = std::make_pair(start + filePathLengthConst, end);
    }

    std::vector<Position> &apiFilePos = dataBlockMap[static_cast<int>(DataTypeEnum::API_FILE)];
    if (!apiFilePos.empty()) {
        Position &pair = apiFilePos.at(0);
        std::string jsonStr = GetContentStr(file, pair);
        ConvertApiFile(jsonStr);
    }
    auto &apiInstrPosArray = dataBlockMap[static_cast<int>(DataTypeEnum::API_INSTR)];
    if (!apiInstrPosArray.empty()) {
        apiInstrPos = apiInstrPosArray.at(0);

        Position &pair = apiInstrPosArray.at(0);
        std::string jsonStr = GetContentStr(file, pair);
        ConvertApiInstr(jsonStr);
    }
    file.close();
}

void SourceFileParser::ConvertApiInstr(const std::string &jsonStr)
{
    Document d;
    try {
        d.Parse(jsonStr.c_str());
        if (JsonUtil::IsJsonArray(d, "Cores")) {
            Value &cores = d["Cores"];
            for (auto &core : cores.GetArray()) {
                apiCores.emplace_back(core.GetString());
            }
        }
    } catch (const std::exception &e) {
        ServerLog::Error("Can't parse api instr,not json.Error is ", e.what());
    }
}

void SourceFileParser::ConvertApiFile(const std::string &jsonStr)
{
    Document d;
    try {
        d.Parse(jsonStr.c_str());
        if (JsonUtil::IsJsonArray(d, "Files")) {
            Value &fileArray = d["Files"];
            apiFiles = ConvertToFileMap(fileArray);
        }
    } catch (const std::exception &e) {
        ServerLog::Error("Can't parse api file,not json.Error is ", e.what());
    }
}

std::map<std::string, std::vector<SourceFileLine>> SourceFileParser::ConvertToFileMap(Value &fileArray)
{
    std::map<std::string, std::vector<SourceFileLine>> sourceLinesMap;

    for (auto &file : fileArray.GetArray()) {
        if (!file.IsObject()) {
            continue;
        }

        if (!file.HasMember("Source") || !file["Source"].IsString()) {
            continue;
        }
        if (!file.HasMember("Lines") || !file["Lines"].IsArray()) {
            continue;
        }

        std::string source = file["Source"].GetString();

        rapidjson::Value &lineArray = file["Lines"];
        std::vector<SourceFileLine> sourceFileLineArray = ConvertToLineArray(lineArray);

        // 将Source和对应的SourceFileLines vector添加到map中
        sourceLinesMap[source] = sourceFileLineArray;
    }
    return sourceLinesMap;
}

std::vector<SourceFileLine> SourceFileParser::ConvertToLineArray(Value &lineArray)
{
    std::vector<SourceFileLine> sourceFileLines;

    for (auto &line : lineArray.GetArray()) {
        if (!line.IsObject()) {
            continue;
        }

        SourceFileLine sourceFileLine;

        // 解析Address Range数组
        if (!line.HasMember("Address Range") || !line["Address Range"].IsArray()) {
            continue;
        }
        Value &addressRangeArray = line["Address Range"];
        for (auto &addressRange : addressRangeArray.GetArray()) {
            if (!addressRange.IsArray() || addressRange.Size() != addressRangeSize) {
                continue;
            }
            if (!addressRange[0].IsString() || !addressRange[1].IsString()) {
                continue;
            }

            const char *startAddress = addressRange[0].GetString();
            const char *endAddress = addressRange[1].GetString();

            sourceFileLine.addressRange.emplace_back(startAddress, endAddress);
        }

        // 解析Cycles数组
        if (!line.HasMember("Cycles") || !line["Cycles"].IsArray()) {
            continue;
        }
        Value &cycleArray = line["Cycles"];
        for (auto &cycle : cycleArray.GetArray()) {
            sourceFileLine.cycles.emplace_back(cycle.GetFloat());
        }

        // 解析Instructions Executed数组
        if (!line.HasMember("Instructions Executed") || !line["Instructions Executed"].IsArray()) {
            continue;
        }
        Value &instrExecutedArray = line["Instructions Executed"];
        for (auto &instrExecuted : instrExecutedArray.GetArray()) {
            sourceFileLine.instructionsExecuted.emplace_back(instrExecuted.IsInt() ? instrExecuted.GetInt() : 0);
        }

        // 解析Line
        if (!line.HasMember("Line") || !line["Line"].IsInt()) {
            continue;
        }
        int lineIndex = line["Line"].GetInt();
        sourceFileLine.line = lineIndex;

        // 将解析好的SourceFileLine对象添加到vector中
        sourceFileLines.push_back(sourceFileLine);
    }
    return sourceFileLines;
}

std::string SourceFileParser::GetSingleContentStrByDataType(std::ifstream &file, DataTypeEnum dataTypeEnum,
                                                            bool isBaseline)
{
    if (!file.is_open()) {
        return "";
    }
    // 从文件获取内容
    std::map<int, std::vector<Position>> curBlockMap = isBaseline ? baselineDataBlockMap : dataBlockMap;
    std::vector<Position> &baseInfoPos = curBlockMap[static_cast<int>(dataTypeEnum)];
    if (baseInfoPos.empty()) {
        return "";
    }
    return GetContentStr(file, baseInfoPos[0]);
}

std::optional<Protocol::SubBlockData> SourceFileParser::ConvertStrToSubBlockData(const std::string& str)
{
    if (str.empty()) {
        return std::nullopt;
    }
    Protocol::SubBlockData blockData;
    try {
        std::string error;
        auto d = JsonUtil::TryParse<kParseNumbersAsStringsFlag>(str, error);
        if (!error.empty()) {
            ServerLog::Error("Get base info error:", error);
            return std::nullopt;
        }
        blockData.advice = JsonUtil::GetVector<std::string>(d.value(), "advice");
        if (!d.value().HasMember("subblock_detail") || !d.value()["subblock_detail"].IsArray()) {
            ServerLog::Error("Error encountered while converting string to sub-block data"
                             "in the sub-block detail conversion.");
            return std::nullopt;
        }
        Value &blockDetails = d.value()["subblock_detail"];
        std::transform(blockDetails.GetArray().begin(), blockDetails.GetArray().end(),
                       std::back_inserter(blockData.detailDataList), SourceFileParser::ParseSubBlockUnitData);
    } catch (const std::exception &e) {
        ServerLog::Error("Can't convert string to sub block data.Error is ", e.what());
        return std::nullopt;
    }
    return blockData;
}

Protocol::CompareData<Protocol::SubBlockUnitData> SourceFileParser::ParseSubBlockUnitData(const json_t &item)
{
    Protocol::SubBlockUnitData unitData;
    unitData.blockId = JsonUtil::GetString(item, "block_id");
    unitData.blockType = JsonUtil::GetString(item, "block_type");
    unitData.name = JsonUtil::GetString(item, "name");
    unitData.unit = GetUnitType(JsonUtil::GetInteger(item, "unit"));
    unitData.value = JsonUtil::GetString(item, "value");
    unitData.originValue = JsonUtil::GetString(item, "origin_value");
    Protocol::CompareData<Protocol::SubBlockUnitData> compareData;
    compareData.compare = unitData;
    return compareData;
}

std::string SourceFileParser::GetContentStr(std::ifstream &file, const Position &position) const
{
    int64_t start = position.startPos;
    int64_t end = position.endPos;
    int64_t dataSize = end - start;
    constexpr uint64_t maxDataSize = 1024 * 1024 * 100; // limit data size to 100MB
    if (IsDataSizeExceedUpperLimit(dataSize, maxDataSize)) {
        ServerLog::Error("Data size of content exceeds max upper limit.");
        return "";
    }

    file.seekg(start, std::ios::beg);

    std::string jsonStr;
    jsonStr.resize(dataSize);
    file.read(&jsonStr[0], dataSize);
    if (!file) {
        ServerLog::Error("Failed to read content str.");
        return "";
    }
    return jsonStr;
}

std::string SourceFileParser::GetUnitType(int64_t unitTypeNumber)
{
    if (unitTypeMapping.find(unitTypeNumber) != unitTypeMapping.end()) {
        return unitTypeMapping[unitTypeNumber];
    } else {
        ServerLog::Error("Unknown data block type: ", unitTypeNumber);
        return "";
    }
}

bool SourceFileParser::IsDataSizeExceedUpperLimit(uint64_t realSize, uint64_t upperLimit) const
{
    return realSize > upperLimit;
}

bool SourceFileParser::GetDetailsInterCoreLoadAnalysisGraph(Protocol::DetailsInterCoreLoadGraphBody &responseBody,
                                                            bool isBaseline)
{
    std::string curFilePath = isBaseline ? baselineFilePath : filePath;
    InterCoreLoadGraphParser parser;
    std::ifstream file = FileUtil::OpenReadFileSafely(curFilePath, std::ios::binary);
    std::string json = GetSingleContentStrByDataType(file, DataTypeEnum::DETAILS_INTER_CORE_LOAD_GRAPH,
                                                     isBaseline);
    if (json.empty()) {
        ServerLog::Warn("Json for inter core load analysis in bin file is empty.");
        return false;
    }
    return parser.GetInterCoreLoadAnalysisInfo(json, responseBody);
}

bool SourceFileParser::GetDetailsRoofline(Protocol::DetailsRooflineBody &responseBody)
{
    std::ifstream file = FileUtil::OpenReadFileSafely(filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    std::string error;
    std::string baseInfo = GetSingleContentStrByDataType(file, DataTypeEnum::DETAILS_BASE_INFO);
    auto baseInfoJson = JsonUtil::TryParse(baseInfo, error);
    if (!error.empty()) {
        ServerLog::Error("Illegal base info data,can't parse jinto json");
        return false;
    }
    responseBody.soc = JsonUtil::GetString(*baseInfoJson, "soc");
    std::string jsonStr = GetSingleContentStrByDataType(file, DataTypeEnum::DETAILS_ROOFLINE);
    RooflineParser parser;
    return parser.GetDetailsRoofline(jsonStr, responseBody);
}

void SourceFileParser::SetFilePath(const std::string &inputFilePath)
{
    std::unique_lock<std::mutex> lock(mutex);
    this->filePath = FileUtil::PathPreprocess(inputFilePath);
}

void SourceFileParser::SetBaselineFilePath(const std::string &inputFilePath)
{
    std::unique_lock<std::mutex> lock(mutex);
    this->baselineFilePath = FileUtil::PathPreprocess(inputFilePath);
}

bool SourceFileParser::IsBaselineParsed(const std::string &inputFilePath)
{
    if (inputFilePath == this->filePath && !this->dataBlockMap.empty()) {
        return true;
    }
    return false;
}

void SourceFileParser::SynchronizeBaselineInfo()
{
    std::unique_lock<std::mutex> lock(mutex);
    this->baselineFilePath = this->filePath;
    this->baselineDataBlockMap = this->dataBlockMap;
}

void SourceFileParser::ResetBaseline()
{
    std::unique_lock<std::mutex> lock(mutex);
    this->baselineFilePath = "";
    this->baselineDataBlockMap.clear();
}
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic