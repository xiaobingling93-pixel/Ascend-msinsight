/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
#include "pch.h"
#include "DataBaseManager.h"
#include "TraceFileParser.h"
#include "ParserStatusManager.h"
#include "CommonDefs.h"
#include "WsSessionManager.h"
#include "OperatorProtocolEvent.h"
#include "RegexUtil.h"
#include "TraceTime.h"
#include "BaselineManager.h"
#include "KernelParse.h"

namespace Dic::Module::Summary {
using namespace Dic::Server;


std::map<std::vector<std::string>, std::function<void(const std::map<std::string, size_t> &dataMap,
    const std::vector<std::string> &rows, const std::string &fileId, Kernel &kernel)>> KernelParse::parseFuncMap;

KernelParse &KernelParse::Instance()
{
    static KernelParse instance;
    return instance;
}

KernelParse::KernelParse()
{
    threadPool = std::make_unique<ThreadPool>(KernelParse::maxThreadNum);
    InitKernelParseMap();
}

KernelParse::~KernelParse()
{
    if (threadPool != nullptr) {
        threadPool->ShutDown();
    }
}

void KernelParse::InitKernelParseMap()
{
    parseFuncMap.emplace(std::vector<std::string>{
        FIELD_NAME, FIELD_TYPE, FIELD_ACCELERATOR_CORE, FIELD_DURATION, FIELD_WAIT_TIME, FIELD_BLOCK_DIM },
        std::bind(&KernelParse::ParsePyTorchOpBaseInfoData, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::placeholders::_4));
    parseFuncMap.emplace(std::vector<std::string>{
        FIELD_OP_NAME, FIELD_OP_TYPE, FIELD_TASK_TYPE, FIELD_TASK_DURATION, FIELD_TASK_WAIT_TIME, FIELD_BLOCK_DIM },
        std::bind(&KernelParse::ParseMsProfOpBaseInfoData, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::placeholders::_4));
    parseFuncMap.emplace(std::vector<std::string>{ STEP_ID },
        std::bind(&KernelParse::ParsePyTorchStepInfoData, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::placeholders::_4));
    parseFuncMap.emplace(std::vector<std::string>{ FIELD_OP_STATE },
        std::bind(&KernelParse::ParseOpStateInfoData, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::placeholders::_4));
    parseFuncMap.emplace(std::vector<std::string>{ FIELD_INPUT_DATA_TYPES, FIELD_INPUT_SHAPES, FIELD_INPUT_FORMATS,
        FIELD_OUTPUT_DATA_TYPES, FIELD_OUTPUT_SHAPES, FIELD_OUTPUT_FORMATS },
        std::bind(&KernelParse::ParseShapeInfoData, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
        std::placeholders::_4));
    parseFuncMap.emplace(std::vector<std::string>{ FIELD_START_TIME },
        std::bind(&KernelParse::ParseStartTimeInfoData, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::placeholders::_4));
    parseFuncMap.emplace(std::vector<std::string>{ FIELD_TASK_START_TIME },
        std::bind(&KernelParse::ParseTaskStartTimeInfoData, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::placeholders::_4));
    parseFuncMap.emplace(std::vector<std::string>{ FIELD_AICORE_TIME },
        std::bind(&KernelParse::ParseAICoreMetricsInfoData, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::placeholders::_4));
}

std::map<std::string, std::vector<std::string>> KernelParse::GetKernelFiles(const std::vector<std::string> &paths)
{
    std::vector<std::string> fileList = {};
    for (const std::string& path : paths) {
        auto files = FileUtil::FindFilesWithFilter(path, std::regex(KERNEL_DETAIL_REG));
        fileList.insert(fileList.end(), files.begin(), files.end());
    }
    if (fileList.empty()) {
        ServerLog::Warn("There is no kernel file.");
        return {};
    }
    std::map<std::string, std::vector<std::string>> results = {};
    std::map<std::string, std::string> hasResetFileIdMap = {};
    for (const auto& file : fileList) {
        std::string fileId = FileUtil::GetProfilerFileId(file);
        int i = 1;
        std::string tempId = fileId;
        std::string parentDir = FileUtil::GetParentPath(file);
        std::string name = FileUtil::GetFileName(file);
        while (Timeline::DataBaseManager::Instance().HasFileId(Timeline::DatabaseType::SUMMARY, tempId)) {
            std::string dbPath = Timeline::DataBaseManager::Instance().GetSummaryDatabase(tempId)->GetDbPath();
            std::string dbParentPath = FileUtil::GetParentPath(dbPath);
            if (RegexUtil::RegexSearch(name, SLICE_STR).has_value() && parentDir == dbParentPath) {
                break;
            }
            tempId = fileId + "_" + std::to_string(++i);
        }

        ServerLog::Info("Kernel file: ", file, ", FileId: ", tempId);
        std::string dbPath = FileUtil::GetDbPath(file, tempId);
        Timeline::DataBaseManager::Instance().GetSummaryDatabase(tempId)->SetDbPath(dbPath);
        results[tempId].push_back(file);
    }
    return results;
}

bool KernelParse::Parse(const std::vector<std::string> &pathList)
{
    if (threadPool == nullptr) {
        ServerLog::Error("Failed to get thread pool in kernel parse.");
        return false;
    }
    auto kernelFiles = GetKernelFiles(pathList);
    if (kernelFiles.empty()) {
        ServerLog::Warn("Kernel file is empty.");
        return false;
    }
    SetParseCallBack();
    for (const auto& kernelFile : kernelFiles) {
        Timeline::ParserStatusManager::Instance().SetParserStatus(KERNEL_PREFIX + kernelFile.first,
                                                                  Timeline::ParserStatus::INIT);
        threadPool->AddTask(PreParseTask, kernelFile.second, kernelFile.first);
    }
    return true;
}

void KernelParse::PreParseTask(const std::vector<std::string>& filePathList, const std::string &fileId)
{
    std::string message;
    if (!InitParser(filePathList, fileId, message)) {
        ServerLog::Error("Failed to parse summary files for fileId:", fileId, "reason: ", message);
        ParseEndCallBack(fileId, false, message);
    }
}

void KernelParse::PostParseTask(const std::set<std::string>& devices, const std::string& fileId)
{
    Timeline::ParserStatusManager::Instance().SetFinishStatus(KERNEL_PREFIX + fileId);
    if (devices.size() == 1 && devices.count(fileId) == 1) {
        ParseEndCallBack(fileId, true, "");
    } else {
        for (const std::string& device : devices) {
            auto tmpFileId = std::string().append(MSPROF_PREFIX).append(fileId)
                    .append(MSPROF_CONNECT).append(device);
            ParseEndCallBack(tmpFileId, true, "");
        }
    }
}

bool KernelParse::InitParser(const std::vector<std::string>& filePathList, const std::string& fileId,
                             std::string &message)
{
    if (filePathList.empty()) {
        return false;
    }
    if (!Timeline::ParserStatusManager::Instance().SetRunningStatus(KERNEL_PREFIX + fileId)) {
        ServerLog::Info("Pre task skip this kernel file.");
        return false;
    }
    std::string dbPath = FileUtil::GetDbPath(filePathList[0], fileId);
    auto database = std::dynamic_pointer_cast<TextSummaryDataBase, VirtualSummaryDataBase>(
        Timeline::DataBaseManager::Instance().GetSummaryDatabase(fileId));
    if (database == nullptr) {
        message = "Failed to get summary database, fileId: ." + fileId + " filePath: " + filePathList[0];
        return false;
    }
    if (!database->OpenDb(dbPath, false)) {
        message = "Failed to init summary database. fileId: " + fileId + " filePath: " +
                filePathList[0] + " dbPath: " + dbPath;
        return false;
    }
    if (!database->IsDatabaseVersionChange() && database->HasFinishedParseLastTime()) {
        uint64_t minTimestamp = database->QueryMinStartTime();
        Timeline::TraceTime::Instance().UpdateTime(minTimestamp, 0);
        std::set<std::string> devices = database->QueryRankIds();
        PostParseTask(devices, fileId);
        return true;
    }
    if (!database->DropTable() or !database->CreateTable() or !database->SetConfig() or
        !database->UpdateParseStatus(NOT_FINISH_STATUS)) {
        message = "Failed to init summary database. fileId: " + fileId + " filePath: " +
                  filePathList[0] + " dbPath: " + dbPath;
        return false;
    }
    if (!ParseTask(filePathList, fileId, message) or !database->UpdateParseStatus(FINISH_STATUS)) {
        return false;
    }
    return true;
}

bool KernelParse::ParseTask(const std::vector<std::string>& filePathList, const std::string &fileId,
                            std::string &message)
{
    std::string statusId = KERNEL_PREFIX + fileId;
    if (!ValidateUtil::CheckCsvFileList(filePathList)) {
        message = "Check file Failed: " + fileId;
        return false;
    }
    std::set<std::string> devices = {};
    for (const auto &filePath: filePathList) {
        if (!ParseKernelCsv(filePath, fileId, statusId, message, devices)) {
            return false;
        }
    }
    // 判断是否为训练场景
    PostParseTask(devices, fileId);
    return true;
}

bool KernelParse::GetUtilizationColumns(const std::vector<std::string> &rowVector,
    std::vector<std::string> &columns)
{
    if (rowVector.size() < 8 || rowVector.size() > 60) { // base column 8 and no more than 60 columns
        return false;
    }
    size_t index = 0;
    auto it = std::find(rowVector.begin(), rowVector.end(), FIELD_AICORE_TIME);
    if (it != rowVector.end()) {
        index =  std::distance(rowVector.begin(), it);
    }
    if (index == 0) {
        return true;
    }
    for (auto i = index; i < rowVector.size(); ++i) {
        std::string origin = rowVector.at(i);
        std::map<std::string, std::string> strPairs = {
            {"(us)", "_us_"},
            {"(%)", "_PCT_"},
            {"(GB/s)", "_GB_s_"},
            {"\r", ""}
        };
        for (const auto& item : strPairs) {
            size_t pos = origin.find(item.first);
            if (pos != std::string::npos) {
                origin.replace(pos, item.first.length(), item.second);
            }
        }
        // 表头只能是字母、数字、下划线、短线、空格
        if (!RegexUtil::RegexMatch(origin, R"(^[a-zA-Z0-9\s\-_]+$)")) {
            columns.clear();
            return true;
        }
        columns.push_back(origin);
    }
    return true;
}

bool KernelParse::CheckHeaderFieldAndFilterParseFunc(std::vector<std::string> rowVector,
    std::vector<std::function<void(const std::map<std::string, size_t> &dataMap,
    const std::vector<std::string> &rows, const std::string &fileId, Kernel &kernel)>> &parseFuncList)
{
    bool flag = false;
    std::sort(rowVector.begin(), rowVector.end());
    // 检查基础的字段
    for (auto item : VALID_HEADERS) {
        if (std::includes(rowVector.begin(), rowVector.end(), item.begin(), item.end())) {
            flag = true;
            break;
        }
    }
    if (!flag) {
        ServerLog::Error("Failed to check base head field.");
        return false;
    }
    // 根据字段匹配解析函数，可扩展
    for (auto &it : parseFuncMap) {
        std::vector key = std::vector<std::string>(it.first);
        std::sort(key.begin(), key.end());
        if (std::includes(rowVector.begin(), rowVector.end(), key.begin(), key.end())) {
            parseFuncList.push_back(it.second);
        }
    }
    if (parseFuncList.empty()) {
        ServerLog::Error("There is no valid parsing function.");
        return false;
    }

    return true;
}

inline void KernelParse::ParsePyTorchOpBaseInfoData(const std::map<std::string, size_t> &dataMap,
    const std::vector<std::string> &row, const std::string &fileId, Kernel &kernel)
{
    kernel.rankId = fileId;
    kernel.name = row[dataMap.at(FIELD_NAME)];
    kernel.type = row[dataMap.at(FIELD_TYPE)];
    kernel.acceleratorCore = row[dataMap.at(FIELD_ACCELERATOR_CORE)];
    kernel.duration = atof(row[dataMap.at(FIELD_DURATION)].c_str());
    kernel.waitTime = atof(row[dataMap.at(FIELD_WAIT_TIME)].c_str());
    kernel.blockDim = atof(row[dataMap.at(FIELD_BLOCK_DIM)].c_str());
}

inline void KernelParse::ParsePyTorchStepInfoData(const std::map<std::string, size_t> &dataMap,
    const std::vector<std::string> &row, const std::string &fileId, Kernel &kernel)
{
    kernel.stepId = row[dataMap.at(STEP_ID)];
}

inline void KernelParse::ParseMsProfOpBaseInfoData(const std::map<std::string, size_t> &dataMap,
    const std::vector<std::string> &row, const std::string &fileId, Kernel &kernel)
{
    kernel.rankId = row[dataMap.at(DEVICE_ID)];
    kernel.name = row[dataMap.at(FIELD_OP_NAME)];
    kernel.type = row[dataMap.at(FIELD_OP_TYPE)];
    kernel.acceleratorCore = row[dataMap.at(FIELD_TASK_TYPE)];
    kernel.duration = atof(row[dataMap.at(FIELD_TASK_DURATION)].c_str());
    kernel.waitTime = atof(row[dataMap.at(FIELD_TASK_WAIT_TIME)].c_str());
    kernel.blockDim = atof(row[dataMap.at(FIELD_BLOCK_DIM)].c_str());
}

inline void KernelParse::ParseShapeInfoData(const std::map<std::string, size_t> &dataMap,
    const std::vector<std::string> &row, const std::string &fileId, Kernel &kernel)
{
    kernel.inputDataTypes = row[dataMap.at(FIELD_INPUT_DATA_TYPES)];
    kernel.inputShapes = row[dataMap.at(FIELD_INPUT_SHAPES)];
    kernel.inputFormats = row[dataMap.at(FIELD_INPUT_FORMATS)];
    kernel.outputDataTypes = row[dataMap.at(FIELD_OUTPUT_DATA_TYPES)];
    kernel.outputShapes = row[dataMap.at(FIELD_OUTPUT_SHAPES)];
    kernel.outputFormats = row[dataMap.at(FIELD_OUTPUT_FORMATS)];
}

inline void KernelParse::ParseOpStateInfoData(const std::map<std::string, size_t> &dataMap,
    const std::vector<std::string> &row, const std::string &fileId, Kernel &kernel)
{
    kernel.state = row[dataMap.at(FIELD_OP_STATE)];
}

inline void KernelParse::ParseStartTimeInfoData(const std::map<std::string, size_t> &dataMap,
    const std::vector<std::string> &row, const std::string &fileId, Kernel &kernel)
{
    const std::string& timestamp = row[dataMap.at(FIELD_START_TIME)];
    kernel.startTime =  NumberUtil::TimestampUsToNs(NumberUtil::StringToLongDouble(timestamp));
}

inline void KernelParse::ParseTaskStartTimeInfoData(const std::map<std::string, size_t> &dataMap,
    const std::vector<std::string> &row, const std::string &fileId, Kernel &kernel)
{
    const std::string& timestamp = row[dataMap.at(FIELD_TASK_START_TIME)];
    kernel.startTime =  NumberUtil::TimestampUsToNs(NumberUtil::StringToLongDouble(timestamp));
}

inline void KernelParse::ParseAICoreMetricsInfoData(const std::map<std::string, size_t> &dataMap,
    const std::vector<std::string> &row, const std::string &fileId, Kernel &kernel)
{
    if (dataMap.find(FIELD_AICORE_TIME) == dataMap.end()) {
        return;
    }
    for (size_t i = dataMap.at(FIELD_AICORE_TIME); i < row.size(); ++i) {
        kernel.utilizationInfo.push_back(row.at(i));
    }
}
 
bool KernelParse::ProcessHeaderGetParseFunc(std::shared_ptr<TextSummaryDataBase> db,
    std::vector<std::string> &rowVector, std::vector<std::string> &columns,
    std::map<std::string, size_t> &dataMap,
    std::vector<std::function<void(const std::map<std::string, size_t> &dataMap,
    const std::vector<std::string> &rows, const std::string &fileId, Kernel &kernel)>> &parseProcessList)
{
    // 获取每一列，更新db表
    if (!GetUtilizationColumns(rowVector, columns) or !db->ExtendColumns(TABLE_KERNEL, columns) or
        !db->InitStmt(columns)) {
        return false;
    }
    // 检查表头并拿到每一类数据对应的解析函数
    if (!CheckHeaderFieldAndFilterParseFunc(rowVector, parseProcessList)) {
        return false;
    }
    // 拿到每一类数据在哪一列，存储在dataMap中
    for (size_t i = 0; i < rowVector.size(); ++i) {
        dataMap[rowVector[i]] = i;
    }
    return true;
}

bool KernelParse::ParseKernelCsv(const std::string& filePath, const std::string &fileId, const std::string& statusId,
                                 std::string &message, std::set<std::string>& devices)
{
    auto start = std::chrono::high_resolution_clock::now();
    ServerLog::Info("Start to parse kernel detail. fileId: ", fileId, ", file path: ", fileId);
    std::ifstream file = OpenReadFileSafely(filePath);
    std::string line;
    std::map<std::string, size_t> dataMap;
    auto db = std::dynamic_pointer_cast<TextSummaryDataBase, VirtualSummaryDataBase>(
        Timeline::DataBaseManager::Instance().GetSummaryDatabase(fileId));
    if (db == nullptr) {
        ServerLog::Error("Failed to get connection.");
        return false;
    }
    bool isHeader = true;
    // 用来存储数据处理的系列函数
    std::vector<std::function<void(const std::map<std::string, size_t> &dataMap,
        const std::vector<std::string> &rows, const std::string &fileId, Kernel &kernel)>> parseProcessList;

    std::string realFileId = Global::BaselineManager::Instance().IsBaselineId(fileId) ?
        FileUtil::GetProfilerFileId(filePath) : fileId;
    std::vector<std::string> columns;
    while (Dic::Module::Timeline::ParserStatusManager::Instance().GetParserStatus(statusId) ==
           Dic::Module::Timeline::ParserStatus::RUNNING && getline(file, line)) {
        // 获取每一行数据存储在rowVector里面
        const std::basic_string<char>& basicString(line);
        std::vector<std::string> rowVector = StringUtil::StringSplit(basicString);
        // 如果是表头且非空
        if (isHeader and !rowVector.empty()) {
            // 获取每一列，更新db表
            ProcessHeaderGetParseFunc(db, rowVector, columns, dataMap, parseProcessList);
            isHeader = false;
            continue;
        }
        // 解析非表头数据存储在db里
        Kernel kernel {};
        for (const auto& parseFunc : parseProcessList) {
            parseFunc(dataMap, rowVector, realFileId, kernel);
        }
        // 如果这列数据和列名个数对不上，说明可能有数据缺了，不存储这一行
        if (rowVector.size() != dataMap.size() || kernel.utilizationInfo.size() != columns.size()) {
            ServerLog::Warn("Invalid data is discarded in the line %.", line);
            continue;
        }
        // 记录有多少device
        devices.emplace(dataMap.find(DEVICE_ID) != dataMap.end() ? rowVector[dataMap[DEVICE_ID]] : fileId);
        // 读取每一行数据写入kernel内
        db->InsertKernelDetail(kernel, columns);
    }
    // 读取剩下的数据写入kernel内
    db->SaveKernelDetail(columns);
    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("Finish to parse kernel detail, ", filePath, " cost time:",
                    std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    Timeline::TraceTime::Instance().UpdateTime(db->QueryMinStartTime(), 0);
    file.close();
    return true;
}

void KernelParse::ParseEndCallBack(const std::string &fileId, bool result, const std::string &msg)
{
    ServerLog::Info("Parse kernel file end: ", fileId);
    // 错误处理逻辑后续增加
    if (!result) {
        return;
    }
    auto &instance = KernelParse::Instance();
    if (instance.paserEndCallback != nullptr) {
        instance.paserEndCallback(fileId, result, msg);
    }
}

void KernelParse::ParseCallBack(const std::string &fileId, bool result, const std::string &msg)
{
    auto *session = WsSessionManager::Instance().GetSession();
    if (session == nullptr) {
        ServerLog::Error("Failed to get session for summary callback.");
        return;
    }
    if (fileId.empty()) {
        auto event = std::make_unique<Protocol::ModuleResetEvent>();
        event->moduleName = MODULE_OPERATOR;
        event->result = true;
        event->reset = true;
        session->OnEvent(std::move(event));
    } else {
        auto event = std::make_unique<Protocol::OperatorParseStatusEvent>();
        event->moduleName = MODULE_OPERATOR;
        event->result = true;
        event->data.rankId = fileId;
        event->data.status = result;
        event->data.error = msg;
        session->OnEvent(std::move(event));
    }
}

void KernelParse::SetParseCallBack()
{
    std::function<void(const std::string, bool, const std::string)> func =
            std::bind(ParseCallBack, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    KernelParse::Instance().SetParseEndCallBack(func);
}

bool KernelParse::Parse(const std::vector<std::string> &filePaths, const std::string &fileId,
                        const std::string &selectedFolder)
{
    if (threadPool == nullptr) {
        ServerLog::Error("Failed to get thread pool in kernel parse.");
        return false;
    }
    // 获取kernel文件
    std::vector<std::string> kernelFile = FileUtil::FindFilesWithFilter(selectedFolder, std::regex(KERNEL_DETAIL_REG));
    if (kernelFile.empty()) {
        return false;
    }
    // 初始化DB
    std::string dbPath = FileUtil::GetDbPath(kernelFile[0], fileId);
    Timeline::DataBaseManager::Instance().GetSummaryDatabase(fileId)->SetDbPath(dbPath);
    // 初始化解析状态
    Timeline::ParserStatusManager::Instance().SetParserStatus(KERNEL_PREFIX + fileId,
                                                              Timeline::ParserStatus::INIT);
    threadPool->AddTask(PreParseTask, kernelFile, fileId);
    return true;
}

void KernelParse::Reset()
{
    ServerLog::Info("Summary reset. wait task completed.");
    ParseEndCallBack("", true, "");
    if (threadPool != nullptr) {
        threadPool->Reset();
    }
    ServerLog::Info("Summary task completed.");
    auto databaseList = Timeline::DataBaseManager::Instance().GetAllSummaryDatabase();
    for (auto &db: databaseList) {
        auto database = dynamic_cast<TextSummaryDataBase*>(db);
        if (database != nullptr) {
            database->ReleaseStmt();
            database->CloseDb();
        }
    }
    Timeline::DataBaseManager::Instance().Clear(Timeline::DatabaseType::SUMMARY);
}

} // end of namespace Dic::Module::Summary

