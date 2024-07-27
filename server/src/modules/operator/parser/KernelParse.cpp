/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */
#include "pch.h"
#include "DataBaseManager.h"
#include "TraceFileParser.h"
#include "ParserStatusManager.h"
#include "CommonDefs.h"
#include "WsSession.h"
#include "WsSessionManager.h"
#include "OperatorProtocolEvent.h"
#include "TraceTime.h"
#include "KernelParse.h"

namespace Dic {
namespace Module {
namespace Summary {
using namespace Dic::Server;


std::map<std::string, std::function<void(const std::map<std::string, size_t> &dataMap,
        const std::vector<std::string> &rows, const std::string &fileId, Kernel &kernel)>> KernelParse::kernelParseMap;

KernelParse &KernelParse::Instance()
{
    static KernelParse instance;
    return instance;
}

KernelParse::KernelParse()
{
    threadPool = std::make_unique<ThreadPool>(KernelParse::maxThreadNum);
    InitkernelParseMap();
}

KernelParse::~KernelParse()
{
    threadPool->ShutDown();
}

void KernelParse::InitkernelParseMap()
{
    kernelParseMap.emplace(ASCEND_PYTORCH_PROF + L1, std::bind(&KernelParse::ParseAscendPyTorchprofKernel,
                                                               std::placeholders::_1, std::placeholders::_2,
                                                               std::placeholders::_3, std::placeholders::_4));
    kernelParseMap.emplace(MSPROF + L1, std::bind(&KernelParse::ParseMsprofKernel,
                                                  std::placeholders::_1, std::placeholders::_2,
                                                  std::placeholders::_3, std::placeholders::_4));
    kernelParseMap.emplace(ASCEND_PYTORCH_PROF + L0, std::bind(&KernelParse::ParseAscendPyTorchprofKernelL0,
                                                               std::placeholders::_1, std::placeholders::_2,
                                                               std::placeholders::_3, std::placeholders::_4));
    kernelParseMap.emplace(MSPROF + L0, std::bind(&KernelParse::ParseMsprofKernelL0,
                                                  std::placeholders::_1, std::placeholders::_2,
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
        results[fileId].push_back(file);
    }
    return results;
}

bool KernelParse::Parse(const std::vector<std::string> &pathList)
{
    auto kernelFiles = GetKernelFiles(pathList);
    if (kernelFiles.empty()) {
        ServerLog::Warn("Kernel file is empty.");
        return false;
    }
    SetParseCallBack();
    if (kernelFiles.size() > 1) {
        // 给前端发消息，清空原有数据
        ParseEndCallBack("", true, "");
    }
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
    auto database =
        dynamic_cast<JsonSummaryDataBase *>(Timeline::DataBaseManager::Instance().GetSummaryDatabase(fileId));
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
    if (!database->DropTable() or !database->CreateTable() or !database->SetConfig() or !database->InitStmt() or
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
    if (!IsFileValid(filePathList, fileId, statusId, message)) {
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

bool KernelParse::CheckHeaderField(const std::map<std::string, size_t>& dataMap)
{
    std::vector<std::string> header4Train = {
        FIELD_OP_NAME, FIELD_OP_TYPE, FIELD_TASK_TYPE, FIELD_TASK_START_TIME,
        FIELD_TASK_DURATION, FIELD_TASK_WAIT_TIME, FIELD_BLOCK_DIM
    };
    std::vector<std::string> header4Msprof = {
        FIELD_NAME, FIELD_TYPE, FIELD_ACCELERATOR_CORE, FIELD_START_TIME,
        FIELD_DURATION, FIELD_WAIT_TIME, FIELD_BLOCK_DIM
    };
    for (int i = 0; i < header4Train.size(); ++i) {
        if (dataMap.find(header4Train[i]) == dataMap.end() && dataMap.find(header4Msprof[i]) == dataMap.end()) {
            ServerLog::Error("The file doesn't contain \"", header4Train[i], "\" or \"" + header4Msprof[i] + "\".");
            return false;
        }
    }
    return true;
}

bool KernelParse::ParseKernelCsv(const std::string& filePath, const std::string &fileId, const std::string& statusId,
                                 std::string &message, std::set<std::string>& devices)
{
    auto start = std::chrono::high_resolution_clock::now();
    ServerLog::Info("Start to parse kernel detail. fileId: ", fileId, ", file path: ", fileId);
    std::ifstream file(FileUtil::PathPreprocess(filePath));
    std::string line;
    std::map<std::string, size_t> dataMap;
    auto db = dynamic_cast<JsonSummaryDataBase*>(Timeline::DataBaseManager::Instance().GetSummaryDatabase(fileId));
    bool isHeader = true;
    std::string type;
    while (Timeline::ParserStatusManager::Instance().GetParserStatus(statusId) ==
           Timeline::ParserStatus::RUNNING && getline(file, line)) {
        const std::basic_string<char>& basicString(line);
        std::vector<std::string> rowVector = StringUtil::StringSplit(basicString);
        if (!rowVector.empty() and isHeader) {
            for (size_t i = 0; i < rowVector.size(); ++i) {
                dataMap[rowVector[i]] = i;
            }
            if (!CheckHeaderField(dataMap)) {
                message = "The header is incorrect or incomplete of " + filePath;
                return false;
            }
            type = dataMap.find(FIELD_NAME) != dataMap.end() ? ASCEND_PYTORCH_PROF : MSPROF;
            type += dataMap.find(FIELD_INPUT_SHAPES) != dataMap.end() ? L1 : L0;
            if (kernelParseMap.find(type) == kernelParseMap.end()) {
                ServerLog::Error("Kernel detail type is empty. ");
                return false;
            }
            isHeader = false;
            continue;
        }
        Kernel kernel {};
        kernelParseMap[type](dataMap, rowVector, fileId, kernel);
        // 记录有多少device
        devices.emplace(dataMap.find(DEVICE_ID) != dataMap.end() ? rowVector[dataMap[DEVICE_ID]] : fileId);
        // 读取每一行数据写入kernel内
        db->InsertKernelDetail(kernel);
    }
    // 读取剩下的数据写入kernel内
    db->SaveKernelDetail();
    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("Finish to parse kernel detail, ", filePath, " cost time:",
                    std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    uint64_t minStartTime = db->QueryMinStartTime();
    Timeline::TraceTime::Instance().UpdateTime(minStartTime, 0);
    file.close();
    return true;
}

bool KernelParse::IsFileValid(const std::vector<std::string>& filePathList, const std::string &fileId,
                              const std::string& statusId, std::string &message)
{
    // csv文件有效性校验
    if (!ValidateUtil::CheckCsvFileList(filePathList)) {
        message = "Check file Failed: " + fileId;
        // 如果文件流不正常、文件不可读、文件大小超过2G则返回false
        return false;
    }
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
    WsSession *session = WsSessionManager::Instance().GetSession();
    if (session == nullptr) {
        ServerLog::Error("Failed to get session token for summary callback.");
        return;
    }
    if (fileId.empty()) {
        auto event = std::make_unique<Protocol::ModuleResetEvent>();
        event->moduleName = Protocol::ModuleType::OPERATOR;
        event->result = true;
        event->reset = true;
        session->OnEvent(std::move(event));
    } else {
        auto event = std::make_unique<Protocol::OperatorParseStatusEvent>();
        event->moduleName = Protocol::ModuleType::OPERATOR;
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

inline void KernelParse::ParseAscendPyTorchprofKernel(const std::map<std::string, size_t> &dataMap,
                                                      const std::vector<std::string> &rows, const std::string &fileId,
                                                      Kernel &kernel)
{
    ParseAscendPyTorchprofKernelL0(dataMap, rows, fileId, kernel);
    ParsePublicNotL0(dataMap, rows, kernel); // 非Level0时才有input/output相关数据
}

inline void KernelParse::ParseMsprofKernel(const std::map<std::string, size_t> &dataMap,
                                           const std::vector<std::string> &rows, const std::string &fileId,
                                           Kernel &kernel)
{
    ParseMsprofKernelL0(dataMap, rows, fileId, kernel);
    ParsePublicNotL0(dataMap, rows, kernel); // 非Level0时才有input/output相关数据
}

inline void KernelParse::ParseAscendPyTorchprofKernelL0(const std::map<std::string, size_t> &dataMap,
                                                        const std::vector<std::string> &rows, const std::string &fileId,
                                                        Kernel &kernel)
{
    size_t startTimeIndex = dataMap.find(FIELD_START_TIME) != dataMap.end() ?
            dataMap.at(FIELD_START_TIME) : dataMap.at(FIELD_TASK_START_TIME);
    kernel.rankId = dataMap.find(DEVICE_ID) != dataMap.end() ? rows[dataMap.at(DEVICE_ID)] : fileId;
    kernel.name = rows[dataMap.at(FIELD_NAME)];
    kernel.stepId = dataMap.find(STEP_ID) != dataMap.end() ? rows[dataMap.at(STEP_ID)] : "";
    kernel.type = rows[dataMap.at(FIELD_TYPE)];
    kernel.acceleratorCore = rows[dataMap.at(FIELD_ACCELERATOR_CORE)];
    kernel.startTime = NumberUtil::TimestampUsToNs(NumberUtil::StringToLongDouble(rows[startTimeIndex]));
    kernel.duration = atof(rows[dataMap.at(FIELD_DURATION)].c_str());
    kernel.waitTime = atof(rows[dataMap.at(FIELD_WAIT_TIME)].c_str());
    kernel.blockDim = atof(rows[dataMap.at(FIELD_BLOCK_DIM)].c_str());
}

inline void KernelParse::ParseMsprofKernelL0(const std::map<std::string, size_t> &dataMap,
                                             const std::vector<std::string> &rows, const std::string &fileId,
                                             Kernel &kernel)
{
    kernel.rankId = dataMap.find(DEVICE_ID) != dataMap.end() ? rows[dataMap.at(DEVICE_ID)] : fileId;
    kernel.name = rows[dataMap.at(FIELD_OP_NAME)];
    kernel.stepId = dataMap.find(STEP_ID) != dataMap.end() ? rows[dataMap.at(STEP_ID)] : "";
    kernel.type = rows[dataMap.at(FIELD_OP_TYPE)];
    kernel.acceleratorCore = rows[dataMap.at(FIELD_TASK_TYPE)];
    kernel.startTime = NumberUtil::TimestampUsToNs(NumberUtil::StringToLongDouble(
        rows[dataMap.at(FIELD_TASK_START_TIME)]));
    kernel.duration = atof(rows[dataMap.at(FIELD_TASK_DURATION)].c_str());
    kernel.waitTime = atof(rows[dataMap.at(FIELD_TASK_WAIT_TIME)].c_str());
    kernel.blockDim = atof(rows[dataMap.at(FIELD_BLOCK_DIM)].c_str());
}

inline void KernelParse::ParsePublicNotL0(const std::map<std::string, size_t> &dataMap,
                                          const std::vector<std::string> &rows, Kernel &kernel)
{
    kernel.inputDataTypes = rows[dataMap.at(FIELD_INPUT_DATA_TYPES)];
    kernel.inputShapes = rows[dataMap.at(FIELD_INPUT_SHAPES)];
    kernel.inputFormats = rows[dataMap.at(FIELD_INPUT_FORMATS)];
    kernel.outputDataTypes = rows[dataMap.at(FIELD_OUTPUT_DATA_TYPES)];
    kernel.outputShapes = rows[dataMap.at(FIELD_OUTPUT_SHAPES)];
    kernel.outputFormats = rows[dataMap.at(FIELD_OUTPUT_FORMATS)];
}

bool KernelParse::Parse(const std::vector<std::string> &filePaths, const std::string &fileId,
                        const std::string &selectedFolder)
{
    // 待废弃
    return false;
}

void KernelParse::Reset()
{
    ServerLog::Info("Summary reset. wait task completed.");
    ParseEndCallBack("", true, "");
    threadPool->Reset();
    ServerLog::Info("Summary task completed.");
    auto databaseList = Timeline::DataBaseManager::Instance().GetAllSummaryDatabase();
    for (auto &db: databaseList) {
        auto database = dynamic_cast<JsonSummaryDataBase*>(db);
        if (database != nullptr) {
            database->ReleaseStmt();
            database->CloseDb();
        }
    }
    Timeline::DataBaseManager::Instance().Clear(Timeline::DatabaseType::SUMMARY);
}

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic

