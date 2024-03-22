/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

#include "KernelParse.h"
#include "DataBaseManager.h"
#include "FileUtil.h"
#include "ValidateUtil.h"
#include "TraceFileParser.h"
#include "ParserStatusManager.h"
#include "ServerLog.h"
#include "CommonDefs.h"
#include "NumberUtil.h"
#include "WsSession.h"
#include "WsSessionManager.h"
#include "OperatorProtocolEvent.h"
#include "TraceTime.h"

namespace Dic {
namespace Module {
namespace Summary {
using namespace Dic::Server;

KernelParse &KernelParse::Instance()
{
    static KernelParse instance;
    return instance;
}

KernelParse::KernelParse()
{
    threadPool = std::make_unique<ThreadPool>(KernelParse::maxThreadNum);
}

KernelParse::~KernelParse()
{
    threadPool->ShutDown();
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
        if (!hasResetFileIdMap[fileId].empty()) {
            tempId = hasResetFileIdMap[fileId];
        } else {
            while (Timeline::DataBaseManager::Instance().HasFileId(Timeline::DatabaseType::SUMMARY, tempId)) {
                tempId = fileId + "_" + std::to_string(++i);
            }
            if (std::strcmp(fileId.c_str(), tempId.c_str()) != 0) {
                hasResetFileIdMap[fileId] = tempId;
            }
        }
        ServerLog::Info("Kernel file: ", file, ", FileId: ", tempId);
        if (results[tempId].empty()) {
            Timeline::DataBaseManager::Instance().GetSummaryDatabase(tempId);
        }
        results[fileId].push_back(file);
    }
    return results;
}

bool KernelParse::Parse(const std::vector<std::string>& pathList, const std::string& token)
{
    auto kernelFiles = GetKernelFiles(pathList);
    if (kernelFiles.empty()) {
        ServerLog::Warn("Kernel file is empty.");
        return false;
    }
    SetParseCallBack(token);
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

bool KernelParse::InitParser(const std::vector<std::string>& filePathList, const std::string& fileId,
                             std::string &message)
{
    if (filePathList.empty()) {
        return false;
    }
    std::string dbPath = FileUtil::GetDbPath(filePathList[0], fileId);
    auto database =
        dynamic_cast<JsonSummaryDataBase *>(Timeline::DataBaseManager::Instance().GetSummaryDatabase(fileId));
    if (database == nullptr) {
        message = "Failed to get summary database, fileId: ." + fileId + " filePath: " + filePathList[0];
        return false;
    }
    if (!(database->OpenDb(dbPath, false) && database->DropTable() && database->CreateTable() &&
          database->SetConfig() && database->InitStmt())) {
        message = "Failed to init summary database. fileId: " + fileId + " filePath: " +
                filePathList[0] + " dbPath: " + dbPath;
        return false;
    }
    if (!ParseTask(filePathList, fileId, message)) {
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
    if (devices.size() == 1 && devices.count(fileId) == 1) {
        ParseEndCallBack(fileId, true, "");
    } else {
        for (const std::string& device : devices) {
            auto tmpFileId = std::string().append(MSPROF_PREFIX).append(fileId)
                    .append(MSPROF_CONNECT).append(device);
            ParseEndCallBack(tmpFileId, true, "");
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
    while (Timeline::ParserStatusManager::Instance().GetParserStatus(statusId) ==
           Timeline::ParserStatus::RUNNING && getline(file, line)) {
        const std::basic_string<char>& basicString(line);
        std::vector<std::string> rowVector = StringUtil::StringSplit(basicString);
        if (!rowVector.empty() and rowVector[0] == STEP_ID or rowVector[0] == MODEL_ID
                                   or rowVector[0] == DEVICE_ID) {
            for (size_t i = 0; i < rowVector.size(); ++i) {
                dataMap[rowVector[i]] = i;
            }
            continue;
        }
        Kernel kernel {};
        if (dataMap.size() < kernelTableNum or !KernelParse::mapperToKernelDetail(dataMap, rowVector,
                                                                                  fileId, kernel)) {
            message = "The header is incorrect or incomplete of " + filePath;
            return false;
        }
        // 记录有多少device
        devices.emplace(dataMap.count(DEVICE_ID) > 0 ? rowVector[dataMap[DEVICE_ID]] : fileId);
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
    return true;
}

bool KernelParse::IsFileValid(const std::vector<std::string>& filePathList, const std::string &fileId,
                              const std::string& statusId, std::string &message)
{
    // 检查并设置文件状态为RUNNING
    if (!Timeline::ParserStatusManager::Instance().SetRunningStatus(statusId)) {
        message = "Failed to set run summary status for file id " + fileId;
        // 如果文件解析信息不存在或状态不为INIT则返回false
        return false;
    }
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
    Timeline::ParserStatusManager::Instance().SetFinishStatus(KERNEL_PREFIX + fileId);
    // 错误处理逻辑后续增加
    if (!result) {
        return;
    }
    auto &instance = KernelParse::Instance();
    if (instance.paserEndCallback != nullptr) {
        instance.paserEndCallback(fileId, result, msg);
    }
}

void KernelParse::ParseCallBack(const std::string &token, const std::string &fileId, bool result,
    const std::string &msg)
{
    WsSession *session = WsSessionManager::Instance().GetSession(token);
    if (session == nullptr) {
        ServerLog::Error("Failed to get session token for summary callback.");
        return;
    }
    if (fileId.empty()) {
        auto event = std::make_unique<Protocol::ModuleResetEvent>();
        event->moduleName = Protocol::ModuleType::OPERATOR;
        event->token = token;
        event->result = true;
        event->reset = true;
        session->OnEvent(std::move(event));
    } else {
        auto event = std::make_unique<Protocol::OperatorParseStatusEvent>();
        event->moduleName = Protocol::ModuleType::OPERATOR;
        event->token = token;
        event->result = true;
        event->data.rankId = fileId;
        event->data.status = result;
        event->data.error = msg;
        session->OnEvent(std::move(event));
    }
}

void KernelParse::SetParseCallBack(const std::string& token)
{
    std::function<void(const std::string, bool, const std::string)> func =
            std::bind(ParseCallBack, token, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    KernelParse::Instance().SetParseEndCallBack(func);
}


bool KernelParse::mapperToKernelDetail(std::map<std::string, size_t> dataMap,
                                       std::vector<std::string> row, const std::string &fileId, Kernel &kernel)
{
    size_t nameIndex;
    size_t typeIndex;
    size_t acceleratorIndex;
    size_t startTimeIndex;
    size_t durationIndex;
    size_t waitTimeIndex;
    if (dataMap.count(FIELD_START_TIME) != 0) {
        startTimeIndex = dataMap[FIELD_START_TIME];
    } else if (dataMap.count(FIELD_TASK_START_TIME) != 0) {
        startTimeIndex = dataMap[FIELD_TASK_START_TIME];
    } else {
        ServerLog::Error("The file header does not contain 'Start Time(us)' or 'Task Start Time(us)'.");
        return false;
    }

    if (dataMap.find(FIELD_ACCELERATOR_CORE) != dataMap.end()) {
        nameIndex = dataMap[FIELD_NAME];
        typeIndex = dataMap[FIELD_TYPE];
        acceleratorIndex = dataMap[FIELD_ACCELERATOR_CORE];
        durationIndex = dataMap[FIELD_DURATION];
        waitTimeIndex = dataMap[FIELD_WAIT_TIME];
    } else if (dataMap.find(FIELD_TASK_TYPE) != dataMap.end()) {
        nameIndex = dataMap[FIELD_OP_NAME];
        typeIndex = dataMap[FIELD_OP_TYPE];
        acceleratorIndex = dataMap[FIELD_TASK_TYPE];
        durationIndex = dataMap[FIELD_TASK_DURATION];
        waitTimeIndex = dataMap[FIELD_TASK_WAIT_TIME];
    } else {
        ServerLog::Error("The file header does not contain 'Step Id' or 'Device Id'.");
        return false;
    }

    kernel.rankId = dataMap.count(DEVICE_ID) != 0 ? row[dataMap[DEVICE_ID]] : fileId;
    kernel.name = row[nameIndex];
    kernel.stepId = dataMap.count(STEP_ID) != 0 ? row[dataMap[STEP_ID]] : "";
    kernel.type = row[typeIndex];
    kernel.acceleratorCore = row[acceleratorIndex];
    kernel.startTime = NumberUtil::TimestampUsToNs(NumberUtil::StringToLongDouble(row[startTimeIndex]));
    kernel.duration = atof(row[durationIndex].c_str());
    kernel.waitTime = atof(row[waitTimeIndex].c_str());
    kernel.blockDim = atof(row[dataMap[FIELD_BLOCK_DIM]].c_str());
    kernel.inputDataTypes = row[dataMap[FIELD_INPUT_DATA_TYPES]];
    kernel.inputShapes = row[dataMap[FIELD_INPUT_SHAPES]];
    kernel.inputFormats = row[dataMap[FIELD_INPUT_FORMATS]];
    kernel.outputDataTypes = row[dataMap[FIELD_OUTPUT_DATA_TYPES]];
    kernel.outputShapes = row[dataMap[FIELD_OUTPUT_SHAPES]];
    kernel.outputFormats = row[dataMap[FIELD_OUTPUT_FORMATS]];
    return true;
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
        database->ReleaseStmt();
        database->CloseDb();
    }
    Timeline::DataBaseManager::Instance().Clear(Timeline::DatabaseType::SUMMARY);
}

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic

