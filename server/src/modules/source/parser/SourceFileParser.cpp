/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "SourceFileParser.h"
#include <fstream>
#include <unordered_set>
#include "ServerLog.h"
#include "rapidjson.h"
#include "InterCoreLoadGraphParser.h"
#include "ParserStatusManager.h"
#include "DataBaseManager.h"
#include "EventParser.h"
#include "TraceTime.h"
#include "TextTraceDatabase.h"
#include "CacheManager.h"
#include "BaselineManager.h"
#include "SourceProtocolResponse.h"
#include "RooflineParser.h"
#include "BinFileParseUtil.h"
#include "DetailsMemoryParser.h"
#include "GlobalProtocolEvent.h"
#include "NumberSafeUtil.h"

namespace Dic {
namespace Module {
namespace Source {
using namespace Dic;
using namespace Dic::Server;
using namespace Dic::Protocol;
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
    if (!FileUtil::CheckFilePathLength(selectedFile)) {
        ServerLog::Error("Parse bin file failed cause path length is too long.");
        return false;
    }
    std::string errMsg = "file can be written by others.";
    std::ifstream file = OpenReadFileSafely(selectedFile, std::ios::binary);
    if (!file) {
        ServerLog::Error("Open bin file failed: ", selectedFile);
        SendReadFileFailEvent(selectedFile, errMsg);
        return false;
    }

    // 获取目标数据对象的应用，下面对数据进行改动时会影响
    auto &curDataBlockMap = Global::BaselineManager::Instance().IsBaselineId(fileId) ?
        baselineDataBlockMap : dataBlockMap;
    auto fileSize = FileUtil::GetFileSize(selectedFile.c_str());
    if (fileSize <= 0) {
        ServerLog::Error("Check size of bin file failed when parse data blocks, the size is ", fileSize);
        return false;
    }
    bool success = ParseDataBlocks(file, fileSize, curDataBlockMap);
    if (!success) {
        return false;
    }
    file.close();
    ConvertToData();
    Timeline::ParserStatusManager::Instance().SetParserStatus(fileId, Timeline::ParserStatus::INIT);
    threadPool->AddTask(PreParseTask, fileId);
    return true;
}

bool SourceFileParser::ParseDataBlocks(std::ifstream &file, long long fileSize,
                                       std::map<int, std::vector<Position>> &curDataBlockMap)
{
    while (!file.eof()) {
        uint64_t dataSize;
        uint8_t dataType;
        uint8_t paddingLength;

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
            dataSize = NumberSafe::Add(dataSize, filePathLen);
        }

        if (!file) {
            break;
        }

        if (dataType == static_cast<int>(DataTypeEnum::API_INSTR)) {
            file.read(reinterpret_cast<char *>(&instrVersion), instrVersionLen);
        } else {
            file.seekg(instrVersionLen, std::ios::cur);
        }
        file.seekg(reserveLen, std::ios::cur);

        int64_t startPos = file.tellg();
        if (startPos < 0 || dataSize > static_cast<uint64_t>(INT64_MAX - startPos) ||
            static_cast<uint64_t>(startPos) + dataSize < paddingLength) {
            ServerLog::Error("Parse source file failed, source file is invalid.");
            return false;
        }
        int64_t endPos = NumberSafe::Sub(NumberSafe::Add(startPos, static_cast<int64_t>(dataSize)), paddingLength);
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
    std::ifstream file = OpenReadFileSafely(curFilePath, std::ios::in | std::ios::binary);
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
        totalSize += static_cast<uint64_t>(size);
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
    auto db = DataBaseManager::Instance().GetTraceDatabase(fileId);
    if (db == nullptr) {
        ServerLog::Warn("Failed to get connection when parse bin json,ID: ", fileId);
        return;
    }
    std::shared_ptr<TextTraceDatabase> databasePtr =
            std::dynamic_pointer_cast<TextTraceDatabase, VirtualTraceDatabase>(db);
    if (databasePtr == nullptr) {
        ServerLog::Warn("Failed to get text connection when parse bin json,ID: ", fileId);
        return;
    }
    Timeline::EventParser eventParser(curFilePath, fileId, databasePtr);
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
        curFileProgress->AddToParsedSize(NumberSafe::Sub(pair.second, pair.first));
        instance.parseProgressCallback(fileId, curFileProgress->GetParsedSize(), curFileProgress->GetTotalSize(),
                                       curFileProgress->GetProgressPercentage());
    }
}

void SourceFileParser::ParseEndCallBack(const std::string &fileId, bool result, const std::string &message)
{
    if (!(result && Timeline::ParserStatusManager::Instance().SetFinishStatus(fileId))) {
        result = false;
    }
    auto &instance = SourceFileParser::Instance();
    if (instance.parseEndCallback != nullptr) {
        instance.parseEndCallback(fileId, fileId, result, message);
    }
}

void SourceFileParser::Reset()
{
    std::unique_lock<std::mutex> lock(mutex);
    ServerLog::Info("Reset file parser and wait task completed.");
    Timeline::ParserStatusManager::Instance().SetAllTerminateStatus();
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
    traceFiles.clear();
    sourceInstructionParser.Reset();

    Timeline::DataBaseManager::Instance().Clear();
    Timeline::TraceTime::Instance().Reset();
    FileParser::Reset();
    Timeline::ParserStatusManager::Instance().ClearAllParserStatus();
    ServerLog::Info("End reset file parser.");
}

bool SourceFileParser::CheckOperatorBinary(const std::string &selectedFilePath, std::string &errMsg)
{
    if (!FileUtil::CheckFilePathLength(selectedFilePath)) {
        ServerLog::Error("File path length check failed.");
        return false;
    }
    std::ifstream file = OpenReadFileSafely(selectedFilePath, std::ios::binary);
    if (!file) {
        file.close();
        ServerLog::Error("Failed to open file, file is invalid!");
        return false;
    }

    const int reversePadding = 10;
    uint64_t contentLength;
    uint16_t reverse;

    file.read(reinterpret_cast<char *>(&contentLength), sizeof(contentLength));
    file.seekg(reversePadding, std::ios::beg);
    file.read(reinterpret_cast<char *>(&reverse), sizeof(reverse));

    if (!file) {
        file.close();
        ServerLog::Error("Can't read contentLength and reverse.");
        return false;
    }

    bool isBinary = (contentLength != 0) && (reverse == SourceFileParser::reverseConst);
    file.close();
    return isBinary;
}

std::vector<std::string> SourceFileParser::GetCoreList()
{
    std::unique_lock<std::mutex> lock(mutex);
    return sourceInstructionParser.GetCoreList();
}

std::vector<std::string> SourceFileParser::GetSourceList()
{
    std::unique_lock<std::mutex> lock(mutex);
    return sourceInstructionParser.GetSourceList();
}

std::vector<SourceFileLine> SourceFileParser::GetApiLinesByCoreAndSource(const std::string &core,
                                                                         const std::string &sourceName)
{
    std::unique_lock<std::mutex> lock(mutex);
    return sourceInstructionParser.GetApiLinesByCoreAndSource(core, sourceName);
}

std::string SourceFileParser::GetInstr()
{
    std::unique_lock<std::mutex> lock(mutex);
    return sourceInstructionParser.GetInstr(filePath);
}

std::vector<SourceApiInstruction> SourceFileParser::GetInstructions(std::string &coreName)
{
    std::unique_lock<std::mutex> lock(mutex);
    return sourceInstructionParser.GetInstructions(coreName);
}

std::vector<SourceFileInstructionDynamicCol> SourceFileParser::GetInstrDynamic(std::string &coreName)
{
    std::unique_lock<std::mutex> lock(mutex);
    return sourceInstructionParser.GetInstrDynamic(coreName);
}

std::map<std::string, int> SourceFileParser::GetInstructionColumnTypeMap()
{
    std::unique_lock<std::mutex> lock(mutex);
    return sourceInstructionParser.GetInstructionColumnTypeMap();
}

std::map<std::string, int> SourceFileParser::GetSourceLineColumnTypeMap()
{
    std::unique_lock<std::mutex> lock(mutex);
    return sourceInstructionParser.GetSourceLineColumnTypeMap();
}

std::string SourceFileParser::GetSourceByName(std::string &sourceName)
{
    std::unique_lock<std::mutex> lock(mutex);
    return sourceInstructionParser.GetSourceByName(sourceName, filePath);
}

bool SourceFileParser::GetDetailsBaseInfo(Protocol::DetailsBaseInfoResBody &responseBody, bool isBaseline)
{
    DetailsMemoryParser parser;
    std::string curFilePath = isBaseline ? baselineFilePath : filePath;
    std::map<int, std::vector<Position>> curBlockMap = isBaseline ? baselineDataBlockMap : dataBlockMap;
    return parser.GetDetailsBaseInfo(responseBody, curFilePath, curBlockMap);
}


bool SourceFileParser::GetDetailsLoadInfo(Protocol::DetailsLoadInfoResBody &responseBody, bool isBaseline)
{
    DetailsMemoryParser parser;
    std::string curFilePath = isBaseline ? baselineFilePath : filePath;
    std::map<int, std::vector<Position>>& curBlockMap = isBaseline ? baselineDataBlockMap : dataBlockMap;
    return parser.GetDetailsLoadInfo(responseBody, curFilePath, curBlockMap);
}

bool SourceFileParser::GetDetailsMemoryGraph(const std::string& targetBlockId, bool isBaseline,
                                             Protocol::DetailsMemoryGraphResBody &responseBody)
{
    DetailsMemoryParser parser;
    std::string curFilePath = isBaseline ? baselineFilePath : filePath;
    std::map<int, std::vector<Position>> curBlockMap = isBaseline ? baselineDataBlockMap : dataBlockMap;
    return parser.GetDetailsMemoryGraph(targetBlockId, responseBody, curFilePath, curBlockMap);
}

bool SourceFileParser::GetDetailsMemoryTable(const std::string& targetBlockId, bool isBaseline,
                                             Protocol::DetailsMemoryTableResBody &responseBody)
{
    DetailsMemoryParser parser;
    std::string curFilePath = isBaseline ? baselineFilePath : filePath;
    std::map<int, std::vector<Position>> curBlockMap = isBaseline ? baselineDataBlockMap : dataBlockMap;
    return parser.GetDetailsMemoryTable(targetBlockId, responseBody, curFilePath, curBlockMap);
}

void SourceFileParser::ConvertToData()
{
    std::unique_lock<std::mutex> lock(mutex);
    std::vector<Position> &sourceFilePos = dataBlockMap[static_cast<int>(DataTypeEnum::SOURCE)];
    std::vector<Position> &apiFilePos = dataBlockMap[static_cast<int>(DataTypeEnum::API_FILE)];
    std::vector<Position> &apiInstrPosArray = dataBlockMap[static_cast<int>(DataTypeEnum::API_INSTR)];
    sourceInstructionParser.ConvertToData(filePath, sourceFilePos, apiFilePos, apiInstrPosArray);
}

bool SourceFileParser::GetDetailsInterCoreLoadAnalysisGraph(Protocol::DetailsInterCoreLoadGraphBody &responseBody,
                                                            bool isBaseline)
{
    std::string curFilePath = isBaseline ? baselineFilePath : filePath;
    InterCoreLoadGraphParser parser;
    std::ifstream file = OpenReadFileSafely(curFilePath, std::ios::binary);
    std::map<int, std::vector<Position>> curBlockMap = isBaseline ? baselineDataBlockMap : dataBlockMap;
    std::string json =
        BinFileParseUtil::GetSingleContentStrByDataType(file, DataTypeEnum::DETAILS_INTER_CORE_LOAD_GRAPH, curBlockMap);
    if (json.empty()) {
        ServerLog::Warn("Json for inter core load analysis in bin file is empty.");
        return false;
    }
    return parser.GetInterCoreLoadAnalysisInfo(json, responseBody);
}

bool SourceFileParser::GetDetailsRoofline(Protocol::DetailsRooflineBody &responseBody)
{
    std::ifstream file = OpenReadFileSafely(filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    std::string error;
    std::string baseInfo =
        BinFileParseUtil::GetSingleContentStrByDataType(file, DataTypeEnum::DETAILS_BASE_INFO, dataBlockMap);
    auto baseInfoJson = JsonUtil::TryParse(baseInfo, error);
    if (!error.empty()) {
        ServerLog::Error("Illegal base info data,can't parse into json");
        return false;
    }
    responseBody.soc = JsonUtil::GetString(*baseInfoJson, "soc");
    std::string jsonStr =
        BinFileParseUtil::GetSingleContentStrByDataType(file, DataTypeEnum::DETAILS_ROOFLINE, dataBlockMap);
    RooflineParser parser;
    return parser.GetDetailsRoofline(jsonStr, responseBody);
}

void SourceFileParser::SetFilePath(const std::string &inputFilePath)
{
    std::unique_lock<std::mutex> lock(mutex);
    this->filePath = FileUtil::PathPreprocess(inputFilePath);
}

std::string SourceFileParser::GetFilePath()
{
    return this->filePath;
}

std::vector<Position> SourceFileParser::GetPositionByType(DataTypeEnum type)
{
    std::unique_lock<std::mutex> lock(mutex);
    auto it = dataBlockMap.find(static_cast<int>(type));
    if (it != dataBlockMap.end()) {
        return it->second;
    } else {
        return {};
    }
}

bool SourceFileParser::HasCachelineRecords()
{
    std::unique_lock<std::mutex> lock(mutex);
    auto it = dataBlockMap.find(static_cast<int>(Module::Source::DataTypeEnum::DISPLAY_CACHE));
    if (it != dataBlockMap.end()) {
        return true;
    }
    return false;
}

int8_t SourceFileParser::GetInstrVersion()
{
    return  instrVersion;
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

std::vector<SourceFileLineDynamicCol> SourceFileParser::GetApiLinesDynamic(
    const std::string &core, const std::string &sourceName)
{
    std::unique_lock<std::mutex> lock(mutex);
    return sourceInstructionParser.GetApiLinesDynamic(core, sourceName);
}
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic