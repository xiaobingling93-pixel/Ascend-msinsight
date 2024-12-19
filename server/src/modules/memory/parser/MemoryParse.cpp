/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
#include "pch.h"
#include "TraceFileParser.h"
#include "ParserStatusManager.h"
#include "ProtocolDefs.h"
#include "DataBaseManager.h"
#include "FileDef.h"
#include "WsSender.h"
#include "TraceTime.h"
#include "MemoryParse.h"

namespace Dic {
namespace Module {
namespace Memory {
using namespace Dic::Server;
MemoryParse &MemoryParse::Instance()
{
    static MemoryParse instance;
    return instance;
}

MemoryParse::MemoryParse()
{
    threadPool = std::make_unique<ThreadPool>(MemoryParse::maxThreadNum);
    ranks = {};
}

MemoryParse::~MemoryParse()
{
    threadPool->ShutDown();
}

bool MemoryParse::Parse(const std::vector<std::string> &filePaths, const std::string &fileId,
                        const std::string &selectedFolder)
{
    MemoryFilePairs memoryFilePairs = GetMemoryFile(selectedFolder);
    if (memoryFilePairs.recordFiles.empty()) {
        return false;
    }
    std::string dbPath = FileUtil::GetDbPath(*memoryFilePairs.recordFiles.begin(), fileId);
    Timeline::DataBaseManager::Instance().GetMemoryDatabase(fileId)->SetDbPath(dbPath);
    Timeline::ParserStatusManager::Instance().SetParserStatus(MEMORY_PREFIX + fileId,
                                                              Timeline::ParserStatus::INIT);
    threadPool->AddTask(PreParseTask, memoryFilePairs, fileId);
    return true;
}

bool MemoryParse::OperatorParse(const std::string &filePath, const std::string &fileId)
{
    auto start = std::chrono::high_resolution_clock::now();
    ServerLog::Info("Start parsing Operator Memory: ", filePath, ", FileId: ", fileId);
    auto memoryDatabase = std::dynamic_pointer_cast<TextMemoryDataBase, VirtualMemoryDataBase>(
        Timeline::DataBaseManager::Instance().GetMemoryDatabase(fileId));
    std::ifstream file = OpenReadFileSafely(filePath);
    std::string line;
    std::map<std::string, size_t> dataMap;
    bool isHeader = true;
    while (getline(file, line)) {
        if (Timeline::ParserStatusManager::Instance().GetParserStatus(MEMORY_PREFIX + fileId) !=
            Timeline::ParserStatus::RUNNING) {
            ServerLog::Error("Parsing process of operator_memory.csv is interrupted.");
            return false;
        }
        std::vector<std::string> row = StringUtil::StringSplit(line);
        if (isHeader) {
            if (row.empty()) {
                ServerLog::Error("The first line of operator_memory.csv is not header.");
                return false;
            }
            for (size_t i = 0; i < row.size(); i++) {
                dataMap[row[i]] = i;
            }
            bool columnExist = GetMapValid((row[0] == Dic::NAME ? OPERATOR_CSV : OPERATOR_CSV_MSPROF), dataMap);
            if (!columnExist) {
                return false;
            }
            isHeader = false;
        } else {
            // 如果某一行数据个数与表头不一致，则跳过
            if (dataMap.size() != row.size()) {
                continue;
            }
            Operator opePtr = MemoryParse::mapperToOperatorDetail(dataMap, row);
            // 读取每一行数据并插入到operator内
            memoryDatabase->InsertOperatorDetail(opePtr);
        }
    }
    // 读取剩下的数据并插入到operator内
    memoryDatabase->SaveOperatorDetail();
    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("End parsing Operator Memory: ", filePath, ", cost time: ",
                    std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    uint64_t minStartTime = memoryDatabase->QueryMinOperatorAllocationTime();
    Timeline::TraceTime::Instance().UpdateTime(minStartTime, 0);
    return true;
}

bool MemoryParse::GetMapValid(const std::vector<std::string>& vec, const std::map<std::string, size_t> &dataMap)
{
    for (const std::string& col : vec) {
        if (dataMap.find(col) == dataMap.end()) {
            ServerLog::Error("The file lacks a parameter column : ", col);
            return false;
        }
    }
    return true;
}

Operator MemoryParse::mapperToOperatorDetail(std::map<std::string, size_t> dataMap, std::vector<std::string> row)
{
    Operator anOperator {};
    size_t nameIndex = dataMap[NAME];
    size_t allocationTimeIndex = dataMap[ALLOCATION_TIME];
    size_t sizeIndex = dataMap[SIZE];
    size_t durationIndex = dataMap[DURATION];
    anOperator.name = row[nameIndex];
    anOperator.size = atof(row[sizeIndex].c_str());
    anOperator.allocationTime = NumberUtil::TimestampUsToNs(
        NumberUtil::StringToLongDouble(row[allocationTimeIndex]));
    anOperator.duration = atof(row[durationIndex].c_str());

    if (dataMap.count(RELEASE_TIME)) {
        size_t releaseTimeIndex = dataMap[RELEASE_TIME];
        anOperator.releaseTime = NumberUtil::TimestampUsToNs(
            NumberUtil::StringToLongDouble(row[releaseTimeIndex]));
    } else {
        anOperator.releaseTime = NumberUtil::TimestampUsToNs(
            NumberUtil::StringToLongDouble(row[allocationTimeIndex]) + anOperator.duration);
    }

    if (dataMap.find(ALLOCATION_ACTIVE_MB) != dataMap.end()) {
        anOperator.activeDuration = atof(row[dataMap[ACTIVE_DURATION]].c_str());
        anOperator.activeReleaseTime = NumberUtil::TimestampUsToNs(
            NumberUtil::StringToLongDouble(row[dataMap[ACTIVE_RELEASE_TIME]]));
        anOperator.allocationActive = atof(row[dataMap[ALLOCATION_ACTIVE_MB]].c_str());
        anOperator.releaseActive = atof(row[dataMap[RELEASE_ACTIVE_MB]].c_str());
        anOperator.streamId = row[dataMap[STREAM_PTR]];
    }
    if (dataMap.find(ALLOCATION_ALLOCATED_KB) != dataMap.end()) {
        anOperator.allocationAllocated = atof(row[dataMap[ALLOCATION_ALLOCATED_KB]].c_str()) / KB_TO_MB;
        anOperator.allocationReserved = atof(row[dataMap[ALLOCATION_RESERVED_KB]].c_str()) / KB_TO_MB;
        anOperator.releaseAllocated = atof(row[dataMap[RELEASE_ALLOCATED_KB]].c_str()) / KB_TO_MB;
        anOperator.releaseReserved = atof(row[dataMap[RELEASE_RESERVED_KB]].c_str()) / KB_TO_MB;
    } else {
        anOperator.allocationAllocated = atof(row[dataMap[ALLOCATION_ALLOCATED_MB]].c_str());
        anOperator.allocationReserved = atof(row[dataMap[ALLOCATION_RESERVED_MB]].c_str());
        anOperator.releaseAllocated = atof(row[dataMap[RELEASE_ALLOCATED_MB]].c_str());
        anOperator.releaseReserved = atof(row[dataMap[RELEASE_RESERVED_MB]].c_str());
    }

    return anOperator;
}

Record MemoryParse::mapperToRecordDetail(std::map<std::string, size_t> dataMap, std::vector<std::string> row)
{
    Record record {};
    size_t nameIndex = dataMap[COMPONENT];
    size_t timeStampIndex = dataMap[TIMESTAMP];
    record.component = row[nameIndex];
    record.timesTamp = NumberUtil::TimestampUsToNs(NumberUtil::StringToLongDouble(row[timeStampIndex]));
    // msprof场景
    if (dataMap.count(Dic::DEVICE_ID)) {
        size_t totalAllocatedIndex = dataMap[TOTAL_ALLOCATED_KB];
        size_t totalReservedIndex = dataMap[TOTAL_RESERVED_KB];
        size_t deviceTypeIndex = dataMap[DEVICE];
        record.totalAllocated = atof(row[totalAllocatedIndex].c_str()) / KB_TO_MB;
        record.totalReserved = atof(row[totalReservedIndex].c_str()) / KB_TO_MB;
        record.totalActivated = dataMap.count(TOTAL_ACTIVE_KB) == 0 ?
                0 : atof(row[dataMap[TOTAL_ACTIVE_KB]].c_str()) / KB_TO_MB;
        record.deviceType = row[deviceTypeIndex];
    } else {
        size_t totalAllocatedIndex = dataMap[TOTAL_ALLOCATED_MB];
        size_t totalReservedIndex = dataMap[TOTAL_RESERVED_MB];
        size_t deviceTypeIndex = dataMap[DEVICETYPE];
        record.totalAllocated = atof(row[totalAllocatedIndex].c_str());
        record.totalReserved = atof(row[totalReservedIndex].c_str());
        record.totalActivated = dataMap.count(TOTAL_ACTIVE_MB) == 0 ?
                0 : atof(row[dataMap[TOTAL_ACTIVE_MB]].c_str());
        record.deviceType = row[deviceTypeIndex];
    }
    if (dataMap.find(STREAM_PTR) != dataMap.end()) {
        record.streamId = row[dataMap[STREAM_PTR]];
    }
    return record;
}


StaticOp MemoryParse::mapperToStaticOpDetail(std::map<std::string, size_t> dataMap, std::vector<std::string> row)
{
    StaticOp staticOp {};
    size_t deviceId = dataMap[DEVICE_ID];
    size_t opName = dataMap[OP_NAME];
    size_t modelName = dataMap[MODEL_NAME];
    size_t graphId = dataMap[GRAPH_ID];
    staticOp.deviceId = row[deviceId];
    staticOp.opName = row[opName];
    staticOp.modelName = row[modelName];
    staticOp.graphId = row[graphId];
    size_t nodeIndexStart = dataMap[NODE_INDEX_START];
    size_t nodeIndexEnd = dataMap[NODE_INDEX_END];
    size_t size = dataMap[SIZE_KB];
    staticOp.nodeIndexStart = atof(row[nodeIndexStart].c_str());
    staticOp.nodeIndexEnd = atof(row[nodeIndexEnd].c_str());
    staticOp.size = atof(row[size].c_str());

    return staticOp;
}

Component MemoryParse::mapperToComponentDetail(std::map<std::string, size_t> dataMap, std::vector<std::string> row)
{
    Component component {};
    size_t componentIndex = dataMap[COMPONENT];
    size_t timestampIndex = dataMap[TIMESTAMP];
    size_t deviceIndex = dataMap[DEVICE];
    component.component = row[componentIndex];
    component.timestamp = NumberUtil::TimestampUsToNs(NumberUtil::StringToLongDouble(row[timestampIndex]));
    component.device = row[deviceIndex];
    if (dataMap.find(TOTAL_RESERVED_MB) != dataMap.end()) {
        size_t totalReservedIndex = dataMap[TOTAL_RESERVED_MB];
        component.totalReserved = NumberUtil::StringToDouble(row[totalReservedIndex]);
    } else {
        size_t totalReservedIndex = dataMap[TOTAL_RESERVED_KB];
        component.totalReserved = NumberUtil::StringToDouble(row[totalReservedIndex]) / mbToKb;
    }

    return component;
}

bool MemoryParse::RecordToParse(const std::string &filePath, const std::string &fileId)
{
    auto start = std::chrono::high_resolution_clock::now();
    ServerLog::Info("Start parsing Memory Record: ", filePath, ", FileId: ", fileId);
    auto database = std::dynamic_pointer_cast<TextMemoryDataBase, VirtualMemoryDataBase>(
        Timeline::DataBaseManager::Instance().GetMemoryDatabase(fileId));
    std::ifstream file = OpenReadFileSafely(filePath);
    std::string line;
    std::map<std::string, size_t> dataMap;
    bool isHeader = true;
    while (getline(file, line)) {
        if (Timeline::ParserStatusManager::Instance().GetParserStatus(MEMORY_PREFIX + fileId) !=
            Timeline::ParserStatus::RUNNING) {
            ServerLog::Error("Parsing process of memory_record.csv is interrupted.");
            return false;
        }
        std::vector<std::string> row = StringUtil::StringSplit(line);
        if (isHeader) {
            if (row.empty()) {
                ServerLog::Error("The first line of memory_record.csv is not header.");
                return false;
            }
            for (size_t i = 0; i < row.size(); i++) {
                dataMap[row[i]] = i;
            }
            bool columnExist = GetMapValid((row[0] == Dic::COMPONENT ? RECORD_CSV : RECORD_CSV_MSPROF), dataMap);
            if (!columnExist) {
                return false;
            }
            isHeader = false;
        } else {
            // 如果某一行数据个数与表头不一致，则跳过
            if (row.size() != dataMap.size()) {
                continue;
            }
            Record recordPtr = MemoryParse::mapperToRecordDetail(dataMap, row);
            // 读取每一行数据并插入到record内
            database->InsertRecordDetail(recordPtr);
        }
    }
    // 读取剩下的数据并插入到record内
    database->SaveRecordDetail();
    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("End parsing Memory Record: ", filePath, ", cost time:",
                    std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    uint64_t minTimestamp = database->QueryMinRecordTimestamp();
    Timeline::TraceTime::Instance().UpdateTime(minTimestamp, 0);
    return true;
}

bool MemoryParse::StaticOpParse(const std::string &filePath, const std::string &fileId)
{
    auto start = std::chrono::high_resolution_clock::now();
    ServerLog::Info("Start parsing Memory Static Operator: ", filePath, ", FileId: ", fileId);
    auto database = std::dynamic_pointer_cast<TextMemoryDataBase, VirtualMemoryDataBase>(
        Timeline::DataBaseManager::Instance().GetMemoryDatabase(fileId));
    std::ifstream file = OpenReadFileSafely(filePath);
    std::string line;
    std::map<std::string, size_t> dataMap;
    bool isHeader = true;
    while (getline(file, line)) {
        if (Timeline::ParserStatusManager::Instance().GetParserStatus(MEMORY_PREFIX + fileId) !=
            Timeline::ParserStatus::RUNNING) {
            ServerLog::Error("Parsing process of static_op_mem.csv is interrupted.");
            return false;
        }
        std::vector<std::string> row = StringUtil::StringSplit(line);
        if (isHeader) {
            if (row.empty()) {
                ServerLog::Error("The first line of static_op_mem.csv is not header.");
                return false;
            }
            for (size_t i = 0; i < row.size(); i++) {
                dataMap[row[i]] = i;
            }
            bool columnExist = GetMapValid(STATIC_OP_MEM_CSV, dataMap);
            if (!columnExist) {
                return false;
            }
            isHeader = false;
        } else {
            // 如果某一行数据个数与表头不一致，则跳过
            if (row.size() != dataMap.size()) {
                continue;
            }
            StaticOp staticOpPtr = MemoryParse::mapperToStaticOpDetail(dataMap, row);
            // 读取每一行数据并插入到static_op内
            database->InsertStaticOpDetail(staticOpPtr);
        }
    }
    // 读取剩下的数据并插入到static_op内
    database->SaveStaticOpDetail();
    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("End parsing Static Op Mem: ", filePath, ", cost time:",
                    std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    uint64_t minTimestamp = database->QueryMinRecordTimestamp();
    Timeline::TraceTime::Instance().UpdateTime(minTimestamp, 0);
    return true;
}

bool MemoryParse::ComponentParse(const std::string &filePath, const std::string &fileId)
{
    auto start = std::chrono::high_resolution_clock::now();
    ServerLog::Info("Start parsing Npu Module Mem: ", filePath, ", FileId: ", fileId);
    auto memoryDatabase = std::dynamic_pointer_cast<TextMemoryDataBase, VirtualMemoryDataBase>(
        Timeline::DataBaseManager::Instance().GetMemoryDatabase(fileId));
    if (!memoryDatabase) {
        ServerLog::Error("Could not get the pointer to memory database.");
        return false;
    }
    std::ifstream file = OpenReadFileSafely(filePath);
    std::string line;
    std::map<std::string, size_t> dataMap;
    bool isHeader = true;
    while (getline(file, line)) {
        if (Timeline::ParserStatusManager::Instance().GetParserStatus(MEMORY_PREFIX + fileId) !=
            Timeline::ParserStatus::RUNNING) {
            ServerLog::Error("Parsing process of npu_module_mem.csv is interrupted.");
            return false;
        }
        std::vector<std::string> row = StringUtil::StringSplit(line);
        if (isHeader) {
            if (row.empty()) {
                ServerLog::Error("The first line of npu_module_mem.csv is not header.");
                return false;
            }
            for (size_t i = 0; i < row.size(); i++) {
                dataMap[row[i]] = i;
            }
            bool columnExist = GetMapValid(NPU_MODULE_MEM_CSV_PYTORCH, dataMap) ||
                GetMapValid(NPU_MODULE_MEM_CSV_MINDSPORE, dataMap);
            if (!columnExist) {
                return false;
            }
            isHeader = false;
        } else {
            // 如果某一行数据个数与表头不一致，则跳过
            if (dataMap.size() != row.size()) {
                continue;
            }
            Component componentPtr = MemoryParse::mapperToComponentDetail(dataMap, row);
            // 读取每一行数据并插入到module内
            memoryDatabase->InsertComponentDetail(componentPtr);
        }
    }
    // 读取剩下的数据并插入到module内
    memoryDatabase->SaveComponentDetail();
    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("End parsing Npu Module Mem: ", filePath, ", cost time: ",
                    std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    uint64_t minStartTime = memoryDatabase->QueryMinComponentTimestamp();
    Timeline::TraceTime::Instance().UpdateTime(minStartTime, 0);
    return true;
}

void MemoryParse::Reset()
{
    ServerLog::Info("Memory reset. Wait task completed.");
    ParseEndCallBack("", true, "");
    threadPool->Reset();
    ranks.clear();
    ServerLog::Info("Memory task completed.");
    auto databaseList = Timeline::DataBaseManager::Instance().GetAllMemoryDatabase();
    for (auto &item: databaseList) {
        auto db = dynamic_cast<TextMemoryDataBase*>(item);
        if (db != nullptr) {
            db->ReleaseStmt();
            db->CloseDb();
        }
    }

    Timeline::DataBaseManager::Instance().Clear(Timeline::DatabaseType::MEMORY);
}

std::vector<std::string> MemoryParse::GetPeerDirOperatorFile(const std::string& operatorFile, const std::string &reg)
{
    std::vector<std::string> recordFiles;
    std::vector<std::string> folders;
    std::vector<std::string> files;
    if (!FileUtil::FindFolders(FileUtil::GetParentPath(operatorFile), folders, files)) {
        ServerLog::Warn("There is no file under ", FileUtil::GetParentPath(operatorFile));
        return recordFiles;
    }

    for (const auto& one : files) {
        if (RegexUtil::RegexMatch(one, reg)) {
            recordFiles.push_back(FileUtil::SplicePath(FileUtil::GetParentPath(operatorFile), one));
            if (!RegexUtil::RegexSearch(one, SLICE_STR).has_value()) {
                break;
            }
        }
    }

    return recordFiles;
}

std::vector<std::string> MemoryParse::GetMemoryRecordFileLists(const std::vector<std::string>& paths)
{
    std::vector<std::string> fileList = {};
    for (const std::string& path : paths) {
        std::vector<std::string> files = {};
        if (FileUtil::IsFolder(path)) {
            files = FileUtil::FindFilesWithFilter(path, std::regex(memoryRecordReg)); // 这个文件pytorch和MindSpore都有
        } else {
            std::string fileName = FileUtil::GetFileName(path);
            // 如果导入时选择Memory文件，此时在此文件父目录下进行匹配，以实现匹配memory_record文件或operator_memory文件
            if (RegexUtil::RegexMatch(fileName, memoryRecordReg) ||
                    RegexUtil::RegexMatch(fileName, memoryOperatorReg)) {
                files = FileUtil::FindFilesWithFilter(FileUtil::GetParentPath(path), std::regex(memoryRecordReg));
            }
        }
        if (!files.empty()) {
            fileList.insert(fileList.end(), files.begin(), files.end());
        }
    }
    return fileList;
}

MemoryFilePairs MemoryParse::GetMemoryFile(const std::string &path)
{
    MemoryFilePairs result;
    std::vector<std::string> fileList = GetMemoryRecordFileLists(std::vector<std::string>{path});
    if (fileList.empty()) {
        return result;
    }
    std::vector<std::string> operatorFiles = GetPeerDirOperatorFile(fileList[0], memoryOperatorReg);
    std::vector<std::string> staticOpFiles = GetPeerDirOperatorFile(fileList[0], staticOpMemReg);
    std::vector<std::string> componentFiles = GetPeerDirOperatorFile(fileList[0], npuModuleMemReg);
    result.recordFiles.insert(fileList[0]);
    result.operatorFiles.insert(operatorFiles.begin(), operatorFiles.end());
    result.staticOpFiles.insert(staticOpFiles.begin(), staticOpFiles.end());
    result.componentFiles.insert(componentFiles.begin(), componentFiles.end());
    return result;
}

std::map<std::string, MemoryFilePairs> MemoryParse::GetMemoryFiles(const std::vector<std::string>& paths)
{
    std::vector<std::string> fileList = GetMemoryRecordFileLists(paths);
    if (fileList.empty()) {
        ServerLog::Warn("There is no memory record file.");
        return {};
    }
    std::map<std::string, MemoryFilePairs> results = {};
    for (const auto& recordFile : fileList) {
        std::vector<std::string> operatorFiles = GetPeerDirOperatorFile(recordFile, memoryOperatorReg);
        std::vector<std::string> staticOpFiles = GetPeerDirOperatorFile(recordFile, staticOpMemReg);
        std::vector<std::string> componentFiles = GetPeerDirOperatorFile(recordFile, npuModuleMemReg);
        if (operatorFiles.empty() && staticOpFiles.empty()) {
            ServerLog::Warn("There is no memory record file or static op mem file paired with ", recordFile);
            continue;
        }
        std::string fileId = FileUtil::GetProfilerFileId(recordFile);
        int i = 1;
        std::string tempId = fileId;
        while (Timeline::DataBaseManager::Instance().HasFileId(Timeline::DatabaseType::MEMORY, tempId)) {
            std::string dbPath = Timeline::DataBaseManager::Instance().GetMemoryDatabase(tempId)->GetDbPath();
            if (RegexUtil::RegexSearch(FileUtil::GetFileName(recordFile), SLICE_STR).has_value() &&
                FileUtil::GetParentPath(recordFile) == FileUtil::GetParentPath(dbPath)) {
                break;
            }
            tempId = fileId + "_" + std::to_string(++i);
        }
        std::string dbPath = FileUtil::GetDbPath(recordFile, tempId);
        Timeline::DataBaseManager::Instance().GetMemoryDatabase(tempId)->SetDbPath(dbPath);
        results[tempId].recordFiles.insert(recordFile);
        results[tempId].operatorFiles.insert(operatorFiles.begin(), operatorFiles.end());
        results[tempId].staticOpFiles.insert(staticOpFiles.begin(), staticOpFiles.end());
        results[tempId].componentFiles.insert(componentFiles.begin(), componentFiles.end());
        if (ranks.count(tempId) == 0) {
            Protocol::MemorySuccess one = {tempId, false, true};
            ranks.emplace(tempId, one);
        }
    }

    for (auto &result : results) {
        std::vector<std::string> files;
        std::copy(result.second.operatorFiles.begin(), result.second.operatorFiles.end(), std::back_inserter(files));
        std::copy(result.second.recordFiles.begin(), result.second.recordFiles.end(), std::back_inserter(files));
        std::copy(result.second.staticOpFiles.begin(), result.second.staticOpFiles.end(), std::back_inserter(files));
        std::copy(result.second.componentFiles.begin(), result.second.componentFiles.end(), std::back_inserter(files));
        ServerLog::Info("Memory file: ", StringUtil::join(files, ", "), ", FileId: ", result.first);
    }
    isCluster = (results.size() > 1);
    return results;
}

bool MemoryParse::Parse(const std::vector<std::string> &pathList)
{
    auto memoryFiles = GetMemoryFiles(pathList);
    if (memoryFiles.empty()) {
        ServerLog::Warn("Memory file is empty.");
        return false;
    }
    SetParseCallBack();
    if (memoryFiles.size() > 1) {
        ParseEndCallBack("", true, "");
    }
    for (const auto& memoryFile : memoryFiles) {
        Timeline::ParserStatusManager::Instance().SetParserStatus(MEMORY_PREFIX + memoryFile.first,
                                                                  Timeline::ParserStatus::INIT);
        threadPool->AddTask(PreParseTask, memoryFile.second, memoryFile.first);
    }
    return true;
}

void MemoryParse::PreParseTask(const MemoryFilePairs& filePair, const std::string& fileId)
{
    std::string message;
    if (!InitParser(filePair, fileId, message)) {
        ServerLog::Error("Failed to parse memory files for fileId:", fileId, ", reason: ", message);
        ParseEndCallBack(fileId, false, message);
    }
}

bool MemoryParse::ParseTask(const MemoryFilePairs& filePair, const std::string& fileId, std::string &message)
{
    std::set<std::string> operatorFiles = filePair.operatorFiles;
    std::set<std::string> recordFiles = filePair.recordFiles;
    std::set<std::string> staticOpFiles = filePair.staticOpFiles;
    std::set<std::string> componentFiles = filePair.componentFiles;
    std::vector<std::string> files;
    std::copy(operatorFiles.begin(), operatorFiles.end(), std::back_inserter(files));
    std::copy(recordFiles.begin(), recordFiles.end(), std::back_inserter(files));
    std::copy(staticOpFiles.begin(), staticOpFiles.end(), std::back_inserter(files));
    std::copy(componentFiles.begin(), componentFiles.end(), std::back_inserter(files));
    if (!ValidateUtil::CheckCsvFileList(files)) {
        message = "Failed to parse memory file: " + fileId + " due to access or file size.";
        return false;
    }

    for (const auto& operatorFile : operatorFiles) {
        if (MemoryParse::Instance().OperatorParse(operatorFile, fileId)) {
            continue;
        }
        message = "Failed to parse operator memory file, path = " + operatorFile;
        return false;
    }

    for (const auto& recordFile : recordFiles) {
        if (MemoryParse::Instance().RecordToParse(recordFile, fileId)) {
            continue;
        }
        message = "Failed to parse operator record file, path = " + recordFile;
        return false;
    }

    for (const auto& staticOpFile : staticOpFiles) {
        if (MemoryParse::Instance().StaticOpParse(staticOpFile, fileId)) {
            continue;
        }
        message = "Failed to parse staticOp record file, path = " + staticOpFile;
        return false;
    }

    for (const auto& componentFile : componentFiles) {
        if (MemoryParse::Instance().ComponentParse(componentFile, fileId)) {
            continue;
        }
        message = "Failed to parse npu module mem file, path = " + componentFile;
        return false;
    }

    ParseEndCallBack(fileId, true, "");
    Timeline::ParserStatusManager::Instance().SetFinishStatus(MEMORY_PREFIX + fileId);
    return true;
}

bool MemoryParse::InitParser(const MemoryFilePairs& filePair, const std::string& fileId, std::string &message)
{
    if (filePair.recordFiles.empty()) {
        return false;
    }
    if (!Timeline::ParserStatusManager::Instance().SetRunningStatus(MEMORY_PREFIX + fileId)) {
        message = "Failed to set run memory status for file ";
        // 如果文件解析信息不存在或状态不为INIT则返回false
        return false;
    }
    std::string dbPath = FileUtil::GetDbPath(*(filePair.recordFiles.begin()), fileId);
    auto db = std::dynamic_pointer_cast<TextMemoryDataBase, VirtualMemoryDataBase>(
        Timeline::DataBaseManager::Instance().GetMemoryDatabase(fileId));
    if (!db->OpenDb(dbPath, false)) {
        message = "Failed to open db file. Please delete the file manually: " + dbPath;
        return false;
    }

    if (!db->IsDatabaseVersionChange() && db->HasFinishedParseLastTime()) {
        Timeline::ParserStatusManager::Instance().SetFinishStatus(MEMORY_PREFIX + fileId);
        uint64_t minTimestamp = std::min(db->QueryMinRecordTimestamp(), db->QueryMinOperatorAllocationTime());
        Timeline::TraceTime::Instance().UpdateTime(minTimestamp, 0);
        ParseEndCallBack(fileId, true, "");
        return true;
    }

    if (!db->DropTable() or !db->CreateTable() or !db->SetConfig() or !db->InitStmt() or
            !db->UpdateParseStatus(NOT_FINISH_STATUS)) {
        message = "Failed to init memory database. Path:" + dbPath;
        return false;
    }

    if (!ParseTask(filePair, fileId, message) or !db->UpdateParseStatus(FINISH_STATUS)) {
        return false;
    }

    return true;
}

void MemoryParse::SetParseCallBack()
{
    std::function<void(const std::string, bool, const std::string)> func =
            std::bind(ParseCallBack, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    MemoryParse::Instance().SetParseEndCallBack(func);
}

void MemoryParse::ParseEndCallBack(const std::string &fileId, bool result, const std::string &message)
{
    Timeline::ParserStatusManager::Instance().SetFinishStatus(MEMORY_PREFIX + fileId);
    // 错误处理逻辑后续增加
    if (!result) {
        return;
    }
    if (MemoryParse::Instance().ranks.count(fileId) == 0) {
        return;
    } else {
        MemoryParse::Instance().ranks[fileId].parseSuccess = true;
    }

    auto &instance = MemoryParse::Instance();
    if (instance.paserEndCallback != nullptr) {
        instance.paserEndCallback(fileId, result, message);
    }
}

void MemoryParse::ParseCallBack(const std::string &fileId, bool result, const std::string &msg)
{
    // 如果输入fileId
    if (fileId.empty()) {
        MemoryParse::Instance().ranks.clear();
        auto event = std::make_unique<Protocol::ModuleResetEvent>();
        event->moduleName = Protocol::MODULE_MEMORY;
        event->result = true;
        event->reset = true;
        SendEvent(std::move(event));
    } else {
        auto event = std::make_unique<Protocol::ParseMemoryCompletedEvent>();
        event->moduleName = Protocol::MODULE_TIMELINE;
        event->result = true;
        event->isCluster = MemoryParse::Instance().isCluster;
        std::vector<Protocol::MemorySuccess> memoryResult;
        for (const auto& pair : MemoryParse::Instance().ranks) {
            memoryResult.push_back(pair.second);
        }
        event->memoryResult = memoryResult;
        SendEvent(std::move(event));
    }
}

} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic
