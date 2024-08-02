/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "SourceFileParser.h"
#include <fstream>
#include <unordered_set>
#include "ServerLog.h"
#include "rapidjson.h"
#include "document.h"
#include "JsonUtil.h"
#include "ParserStatusManager.h"
#include "DataBaseManager.h"
#include "EventParser.h"
#include "TraceTime.h"
#include "FileUtil.h"
#include "JsonTraceDatabase.h"
#include "CacheManager.h"
#include "SourceProtocolResponse.h"

namespace Dic {
namespace Module {
namespace Source {
using namespace Dic;
using namespace Dic::Server;
using namespace Dic::Module::FullDb;

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
    DataBaseManager::Instance().curIsBin = true;
    if (!FileUtil::CheckFilePathLength(selectedFile)) {
        ServerLog::Error("File path length check failed.");
        return false;
    }
    std::ifstream file(FileUtil::PathPreprocess(selectedFile), std::ios::binary);
    if (!file) {
        ServerLog::Warn("Failed to open file: ", selectedFile);
        return false;
    }

    const int dataSizeLen = 8; // 数据类型字段距离数据大小字段的偏移
    const int dataTypeLen = 1; // 填充长度字段距离数据类型字段的偏移
    const int paddingLen = 1;  // 填充长度字段距离数据类型字段的偏移
    const int reserveLen = 2;  // 实际数据距离填充长度字段的偏移
    const int filePathLen = 4096;

    while (!file.eof()) {
        uint64_t dataSize;
        uint8_t dataType;
        uint8_t paddingLength;

        file.read(reinterpret_cast<char *>(&dataSize), dataSizeLen);
        file.read(reinterpret_cast<char *>(&dataType), dataTypeLen);
        file.read(reinterpret_cast<char *>(&paddingLength), paddingLen);
        dataType = static_cast<int>(dataType);
        paddingLength = static_cast<int>(paddingLength);
        if (dataType == static_cast<int>(DataTypeEnum::SOURCE)) {
            if (INT64_MAX - filePathLen < dataSize) { // 溢出防护
                ServerLog::Error("Data unit in selected file is invalid which data size is :", dataSize);
                return false;
            }
            dataSize = dataSize + filePathLen;
        }

        if (!file) { // 检查读取是否成功
            break;
        }
        file.seekg(reserveLen, std::ios::cur); // 跳转到实际数据的开始

        int64_t startPos = file.tellg();
        if (startPos + dataSize - paddingLength >= INT64_MAX) {  // 溢出防
            ServerLog::Error("Data unit in selected file is invalid which data size is :", dataSize);
            return false;
        }
        int64_t endPos = startPos + dataSize - paddingLength;

        dataBlockMap[dataType].emplace_back(startPos, endPos);

        // 跳转到下一个数据块的开始位置，考虑到当前数据块的大小和填充
        file.seekg(dataSize, std::ios::cur);
    }
    file.close();
    Timeline::ParserStatusManager::Instance().SetParserStatus(fileId, Timeline::ParserStatus::INIT);
    ServerLog::Info("Start simulation parse. file id: ", fileId);
    threadPool->AddTask(PreParseTask, fileId);
    return true;
}

void SourceFileParser::PreParseTask(const std::string &fileId)
{
    if (!InitParser(fileId)) {
        ParseEndCallBack(fileId, false, "Failed to open db. Please delete dbFile and try again.");
    }
}

bool SourceFileParser::InitParser(const std::string &fileId)
{
    if (!Timeline::ParserStatusManager::Instance().SetRunningStatus(fileId)) {
        ServerLog::Info("Pre task skip this file.");
        return true;
    }
    auto db = DataBaseManager::Instance().GetTraceDatabase(fileId);
    if (db == nullptr) {
        ServerLog::Error("Failed to get connection.");
        return false;
    }
    auto database = std::dynamic_pointer_cast<JsonTraceDatabase, VirtualTraceDatabase>(db);
    if (database == nullptr || !(database->DropTable() && database->CreateTable())) {
        ServerLog::Error("Failed to open traceDatabase. rankId:", fileId);
        return false;
    }
    auto &instance = SourceFileParser::Instance();
    std::vector<std::pair<int64_t, int64_t>> &traceFilePos =
        instance.dataBlockMap[static_cast<int>(DataTypeEnum::TRACE)];
    std::ifstream file(instance.filePath, std::ios::in | std::ios::binary);
    if (!file) {
        ServerLog::Error("Failed to open file. filePath:", instance.filePath);
        return false;
    }
    std::vector<std::pair<int64_t, int64_t>> adjustTraceFilePos;
    for (const auto &pos : traceFilePos) {
        adjustTraceFilePos.emplace_back(AdjustPosition(file, pos.first, pos.second));
    }
    file.close();

    // 计算待解析的文件大小
    uint64_t totalSize = 0;
    for (const auto &pos : adjustTraceFilePos) {
        totalSize += (pos.second - pos.first);
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

void SourceFileParser::EndParseTask(const std::string &fileId, std::shared_ptr<std::vector<std::future<void>>> futures)
{
    if (Timeline::ParserStatusManager::Instance().GetParserStatus(fileId) != Timeline::ParserStatus::RUNNING) {
        ServerLog::Info("End parse task skip this file. ID:", fileId);
        return;
    }
    ServerLog::Info("Wait parse completed. ID:", fileId);
    for (const auto &future : *futures) {
        future.wait();
    }
    ServerLog::Info("Parse completed. ID:", fileId);
    auto db = DataBaseManager::Instance().GetTraceDatabase(fileId);
    if (db == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:", fileId);
        return;
    }
    auto database = std::dynamic_pointer_cast<JsonTraceDatabase, VirtualTraceDatabase>(db);
    if (database == nullptr) {
        ServerLog::Error("Failed to convert VirtualTraceDatabase to JsonTraceDataBase in EndParseTask of Source.");
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
        ServerLog::Info("Parse task skip this file. ID:", fileId);
        return;
    }
    ServerLog::Info("Ptart parse:", fileId);
    auto &instance = SourceFileParser::Instance();
    Timeline::EventParser eventParser(instance.filePath, fileId);
    eventParser.SetSimulationStatus(true);
    if (!eventParser.Parse(pos.first, pos.second)) {
        if (Timeline::ParserStatusManager::Instance().SetTerminateStatus(fileId) == Timeline::ParserStatus::RUNNING) {
            // 只发送一次解析失败事件
            ParseEndCallBack(fileId, false, eventParser.GetError());
        }
    }
    // 发送单卡解析进度事件
    std::unique_ptr<FileProgress> &curFileProgress = instance.fileProgressMap[fileId];
    curFileProgress->AddToParsedSize(pos.second - pos.first);
    instance.paserProgressCallback(fileId, curFileProgress->GetParsedSize(), curFileProgress->GetTotalSize(),
                                   curFileProgress->GetProgressPercentage());
}
std::pair<int64_t, int64_t> SourceFileParser::AdjustPosition(std::ifstream &file, int64_t start, int64_t end)
{
    ServerLog::Info("Start is: ", start, " End is: ", end);
    int64_t contentStart = start;
    int64_t contentEnd = end;
    file.seekg(contentStart, std::ios::beg);
    char startTemp;
    while (file.get(startTemp)) {
        if (!file) {
            ServerLog::Error("Failed to read start pos.");
            break;
        }
        if (startTemp == '[') {
            contentStart++;
            break;
        }
        contentStart++;
    }
    file.seekg(contentEnd, std::ios::beg);
    char endTemp;
    const int offset = -2;
    while (file.get(endTemp)) {
        if (!file) {
            ServerLog::Error("Failed to read end pos.");
            break;
        }
        if (endTemp == ']') {
            contentEnd--;
            break;
        }
        contentEnd--;
        file.seekg(offset, std::ifstream::cur);
    }
    return std::make_pair(contentStart, contentEnd);
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
    ServerLog::Info("Reset. wait task completed.");
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
    apiInstrPos = std::make_pair(0, 0);

    Timeline::DataBaseManager::Instance().Clear();
    Timeline::TraceTime::Instance().Reset();
    FileParser::Reset();
    Timeline::ParserStatusManager::Instance().ClearAllParserStatus();
    ServerLog::Info("End Reset trace Parser");
}

bool SourceFileParser::CheckOperatorBinary(const std::string &selectedFilePath)
{
    if (!FileUtil::CheckFilePathLength(selectedFilePath)) {
        ServerLog::Error("File path length check failed");
        return false;
    }
    std::string processedFilePath = FileUtil::PathPreprocess(selectedFilePath);
    std::ifstream file(processedFilePath, std::ios::binary);
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

    if (isBinary) {
        this->filePath = processedFilePath;
    }

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
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        ServerLog::Error("Can't open file,please check file exist or not,file name :", filePath);
        return "";
    }
    int64_t start = apiInstrPos.first;
    int64_t end = apiInstrPos.second;
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

    std::ifstream file(filePath, std::ios::binary);
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

bool SourceFileParser::GetDetailsBaseInfo(Protocol::DetailsBaseInfoResBody &responseBody)
{
    std::ifstream file(filePath, std::ios::binary);
    std::string baseInfo = GetSingleContentStrByDataType(file, DataTypeEnum::DETAILS_BASE_INFO);
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
        responseBody.name = JsonUtil::GetString(baseInfoJson.value(), "name");
        responseBody.soc = JsonUtil::GetString(baseInfoJson.value(), "soc");
        responseBody.opType = JsonUtil::GetString(baseInfoJson.value(), "op_type");
        responseBody.blockDim = JsonUtil::GetString(baseInfoJson.value(), "block_dim");
        responseBody.mixBlockDim = JsonUtil::GetString(baseInfoJson.value(), "mix_block_dim");
        responseBody.duration = JsonUtil::GetString(baseInfoJson.value(), "duration");
        responseBody.deviceId = JsonUtil::GetString(baseInfoJson.value(), "device_id");
        responseBody.pid = JsonUtil::GetString(baseInfoJson.value(), "pid");
        Value blockDetailsValue;
        if (responseBody.opType == "mix" && baseInfoJson.value().HasMember("mix_block_detail")) {
            blockDetailsValue = baseInfoJson.value()["mix_block_detail"];
        } else if (responseBody.opType != "mix" && baseInfoJson.value().HasMember("block_detail")) {
            blockDetailsValue = baseInfoJson.value()["block_detail"];
        } else {
            return true;
        }
        if (!blockDetailsValue.IsObject()) {
            return true;
        }
        Protocol::TableDetail tableDetail;
        tableDetail.size = JsonUtil::GetVector<std::string>(blockDetailsValue, "size");
        tableDetail.headerName = JsonUtil::GetVector<std::string>(blockDetailsValue, "head_name");
        if (blockDetailsValue.HasMember("row") && blockDetailsValue["row"].IsArray()) {
            Value &row = blockDetailsValue["row"];
            for (const auto &dataRow: row.GetArray()) {
                Protocol::TableRow memoryTableRow;
                memoryTableRow.value = JsonUtil::GetVector<std::string>(dataRow, "value");
                tableDetail.row.push_back(memoryTableRow);
            }
        }
        responseBody.blockDetail = tableDetail;
        return true;
    } catch (const std::exception &e) {
        ServerLog::Error("Can't parse details base info,not json.Error is ", e.what());
        return false;
    }
}

bool SourceFileParser::GetDetailsLoadInfo(Protocol::DetailsLoadInfoResBody & responseBody)
{
    // 从文件获取内容
    std::ifstream file(filePath, std::ios::binary);
    std::string loadGraph = GetSingleContentStrByDataType(file, DataTypeEnum::DETAILS_COMPUTE_LOAD_GRAPH);
    std::string loadTable = GetSingleContentStrByDataType(file, DataTypeEnum::DETAILS_COMPUTE_LOAD_TABLE);
    file.close();
    if (loadGraph.empty() && loadTable.empty()) {
        ServerLog::Info("Details load data does not exist.");
        return true;
    }

    std::optional<Protocol::SubBlockData> blockData = ConvertStrToSubBlockData(loadGraph);
    if (blockData.has_value()) {
        responseBody.chartData = blockData.value();
    }
    std::optional<Protocol::SubBlockData> tableData = ConvertStrToSubBlockData(loadTable);
    if (tableData.has_value()) {
        responseBody.tableData = tableData.value();
    }

    // 获取blockid列表
    std::unordered_set<std::string> blockIdSet;
    for (const auto &item: responseBody.chartData.detailDataList) {
        blockIdSet.insert(item.blockId);
    }
    for (const auto &item: responseBody.tableData.detailDataList) {
        blockIdSet.insert(item.blockId);
    }
    std::copy(blockIdSet.begin(), blockIdSet.end(), std::back_inserter(responseBody.blockIdList));
    return true;
}

bool SourceFileParser::GetDetailsMemoryGraph(const std::string& targetBlockId,
                                             Protocol::DetailsMemoryGraphResBody &responseBody)
{
    if (targetBlockId.empty()) {
        ServerLog::Error("Block id is empty.");
        return true;
    }
    // 读取内存表数据
    std::ifstream file(filePath, std::ios::binary);
    std::string memoryGraph = GetSingleContentStrByDataType(file, DataTypeEnum::DETAILS_MEMORY_GRAPH);
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
            temp.memoryUnit.push_back(memoryUnitTemp);
        }
    }
    if (json.HasMember("L2cache") && json["L2cache"].IsObject()) {
        auto &L2cache = const_cast<Value &>(json["L2cache"]);
        Protocol::L2Cache l2CacheTemp;
        l2CacheTemp.hit = JsonUtil::GetString(L2cache, "hit");
        l2CacheTemp.miss = JsonUtil::GetString(L2cache, "miss");
        l2CacheTemp.totalRequest = JsonUtil::GetString(L2cache, "total_request");
        l2CacheTemp.hitRatio = JsonUtil::GetString(L2cache, "hit_ratio");
        temp.l2Cache = l2CacheTemp;
    }

    if (json.HasMember("Vector") && json["Vector"].IsObject()) {
        temp.vector = ParseJsonToUtilizationRate(json["Vector"]);
    }
    if (json.HasMember("Vector1") && json["Vector1"].IsObject()) {
        temp.vector1 = ParseJsonToUtilizationRate(json["Vector1"]);
    }
    if (json.HasMember("Cube") && json["Cube"].IsObject()) {
        temp.cube = ParseJsonToUtilizationRate(json["Cube"]);
    }
    return temp;
}

bool SourceFileParser::GetDetailsMemoryTable(const std::string& targetBlockId,
                                             Protocol::DetailsMemoryTableResBody &responseBody)
{
    if (targetBlockId.empty()) {
        ServerLog::Info("Block id is empty.");
        return true;
    }
    std::ifstream file(filePath, std::ios::binary);
    std::string memoryTable = GetSingleContentStrByDataType(file, DataTypeEnum::DETAILS_MEMORY_TABLE);
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
            ServerLog::Error("Memory table data invalid.");
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
        Protocol::TableDetail tableDetail;
        tableDetail.tableName = JsonUtil::GetString(item, "table_name");
        tableDetail.size = JsonUtil::GetVector<std::string>(item, "size");
        tableDetail.headerName = JsonUtil::GetVector<std::string>(item, "header_name");
        Value &row = item["row"];
        for (const auto &dataRow: row.GetArray()) {
            Protocol::TableRow memoryTableRow;
            memoryTableRow.name = JsonUtil::GetString(dataRow, "name");
            memoryTableRow.value = JsonUtil::GetVector<std::string>(dataRow, "value");
            tableDetail.row.push_back(memoryTableRow);
        }
        result.tableDetail.push_back(tableDetail);
    }
    return result;
}

void SourceFileParser::ConvertToData()
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        ServerLog::Error("Can't open file,please check file exist or not,file name :", filePath);
        return;
    }

    std::vector<std::pair<int64_t, int64_t>> &sourceFilePos = dataBlockMap[static_cast<int>(DataTypeEnum::SOURCE)];
    for (auto pos : sourceFilePos) {
        int64_t start = pos.first;
        int64_t end = pos.second;

        file.seekg(start, std::ios::beg);

        std::vector<char> filePathBuffer(filePathLengthConst);
        file.read(filePathBuffer.data(), filePathBuffer.size());
        if (!file) {
            ServerLog::Error("Failed to read file path buffer.");
            break;
        }
        std::string sourceFilePath(filePathBuffer.data());
        sourceFiles[sourceFilePath] = std::make_pair(start + filePathLengthConst, end);
    }

    std::vector<std::pair<int64_t, int64_t>> &apiFilePos = dataBlockMap[static_cast<int>(DataTypeEnum::API_FILE)];
    if (!apiFilePos.empty()) {
        std::pair<int64_t, int64_t> &pair = apiFilePos.at(0);
        std::string jsonStr = GetContentStr(file, pair);
        ConvertApiFile(jsonStr);
    }
    auto &apiInstrPosArray = dataBlockMap[static_cast<int>(DataTypeEnum::API_INSTR)];
    if (!apiInstrPosArray.empty()) {
        apiInstrPos = apiInstrPosArray.at(0);

        std::pair<int64_t, int64_t> &pair = apiInstrPosArray.at(0);
        std::string jsonStr = GetContentStr(file, pair);
        ConvertApiInstr(jsonStr);
    }
    file.close();
}

int64_t SourceFileParser::GetSimulationPid(const std::string &fileId, const std::string &processName)
{
    std::unique_lock<std::mutex> lock(processMutex);
    if (simulationPidMap[fileId].count(processName) > 0) {
        return simulationPidMap[fileId].at(processName);
    }
    if (pid == INT64_MAX) {
        pid = 0;
    }
    simulationPidMap[fileId].emplace(processName, ++pid);
    return pid;
}

int64_t SourceFileParser::GetSimulationTid(const std::string &fileId, const std::string &processName,
    const std::string &threadName)
{
    std::unique_lock<std::mutex> lock(threadMutex);
    auto item = std::make_pair(processName, threadName);
    if (simulationTidMap[fileId].count(item) > 0) {
        return simulationTidMap[fileId].at(item);
    }
    if (tid == INT64_MAX) {
        tid = 0;
    }
    simulationTidMap[fileId].emplace(item, ++tid);
    return tid;
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
            sourceFileLine.instructionsExecuted.emplace_back(instrExecuted.GetInt());
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

std::string SourceFileParser::GetSingleContentStrByDataType(std::ifstream &file, DataTypeEnum dataTypeEnum)
{
    if (!file.is_open()) {
        return "";
    }
    // 从文件获取内容
    std::vector<std::pair<int64_t, int64_t>> &baseInfoPos =
            dataBlockMap[static_cast<int>(dataTypeEnum)];
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
        Value &blockDetails = d.value()["subblock_detail"];
        for (auto &item : blockDetails.GetArray()) {
            Protocol::SubBlockUnitData unitData;
            unitData.blockId = JsonUtil::GetString(item, "block_id");
            unitData.blockType = JsonUtil::GetString(item, "block_type");
            unitData.name = JsonUtil::GetString(item, "name");
            unitData.unit = GetUnitType(JsonUtil::GetInteger(item, "unit"));
            unitData.value = JsonUtil::GetString(item, "value");
            unitData.originValue = JsonUtil::GetString(item, "origin_value");
            blockData.detailDataList.emplace_back(unitData);
        }
    } catch (const std::exception &e) {
        ServerLog::Error("Can't convert string to sub block data.Error is ", e.what());
        return std::nullopt;
    }
    return blockData;
}

std::string SourceFileParser::GetContentStr(std::ifstream &file, const std::pair<int64_t, int64_t> &pair) const
{
    int64_t start = pair.first;
    int64_t end = pair.second;
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
        ServerLog::Error("Unknown unit type.");
        return "";
    }
}

bool SourceFileParser::IsDataSizeExceedUpperLimit(uint64_t realSize, uint64_t upperLimit) const
{
    return realSize > upperLimit;
}
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic