/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "KernelParse.h"
#include "DataBaseManager.h"
#include "FileUtil.h"
#include "ValidateUtil.h"
#include "TraceFileParser.h"
#include "ParserStatusManager.h"
#include "ServerLog.h"
#include "WsSession.h"
#include "WsSessionManager.h"
#include "OperatorProtocolEvent.h"

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


std::vector<std::pair<std::string, std::string>> KernelParse::GetKernelFiles(const std::vector<std::string> &paths)
{
    std::vector<std::string> fileList = {};
    for (const std::string& path : paths) {
        auto files = FileUtil::FindFilesWithFilter(path, std::regex(kernelDetailReg));
        fileList.insert(fileList.end(), files.begin(), files.end());
    }
    if (fileList.empty()) {
        ServerLog::Warn("There is no kernel file.");
        return {};
    }

    std::vector<std::pair<std::string, std::string>> results = {};
    for (const auto& file : fileList) {
        std::string fileId = FileUtil::GetProfilerFileId(file);
        int i = 1;
        std::string tempId = fileId;
        while (Timeline::DataBaseManager::Instance().HasFileId(Timeline::DatabaseType::SUMMARY, tempId)) {
            tempId = fileId + "_" + std::to_string(++i);
        }
        ServerLog::Info("Kernel file: ", file, ", FileId: ", tempId);
        Timeline::DataBaseManager::Instance().GetSummaryDatabase(tempId);
        results.emplace_back(file, tempId);
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
    for (const auto& kernelFile : kernelFiles) {
        threadPool->AddTask(PreParseTask, kernelFile.first, kernelFile.second);
    }
    return true;
}

void KernelParse::PreParseTask(const std::string &filePath, const std::string &fileId)
{
    if (!InitParser(filePath, fileId)) {
        ServerLog::Warn("Failed to parse summary files for fileId:", fileId);
    }
}

bool KernelParse::InitParser(const std::string& filePath, const std::string& fileId)
{
    Timeline::ParserStatusManager::Instance().SetParserStatus(KERNEL_PREFIX + fileId, Timeline::ParserStatus::INIT);
    std::string dbPath = FileUtil::GetDbPath(filePath, fileId);
    auto database = Timeline::DataBaseManager::Instance().GetSummaryDatabase(fileId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get summary database, fileId: .", fileId, " filePath: ", filePath);
        return false;
    }
    if (!(database->OpenDb(dbPath, false) && database->DropTable() && database->CreateTable() &&
          database->SetConfig() && database->InitStmt())) {
        ServerLog::Error("Failed to init summary database. fileId: ", fileId, " filePath: ",
                         filePath, " dbPath: ", dbPath);
        return false;
    }

    if (!ParseTask(filePath, fileId)) {
        ServerLog::Error("Failed to parse kernel file. fileId: ", fileId, " filePath:", filePath);
        return false;
    }

    return true;
}

bool KernelParse::ParseTask(const std::string &filePath, const std::string &fileId)
{
    std::string statusId = KERNEL_PREFIX + fileId;
    if (!Timeline::ParserStatusManager::Instance().SetRunningStatus(statusId)) {
        ParseEndCallBack(fileId, false, "Failed to set run summary status for file " + filePath);
        ServerLog::Error("Failed to set run summary status for file ", filePath);
        return false;
    }
    if (!ValidateUtil::CheckCsvFile(filePath)) {
        ParseEndCallBack(fileId, false, "Check file Failed: " + filePath);
        return false;
    }
    auto start = std::chrono::high_resolution_clock::now();
    ServerLog::Info("Start to parse kernel detail. fileId: ", fileId, ", file path: ", filePath);
    std::ifstream file(FileUtil::PathPreprocess(filePath));
    std::string line;
    std::map<std::string, size_t> dataMap;
    std::set<std::string> devices = {};
    auto db = Timeline::DataBaseManager::Instance().GetSummaryDatabase(fileId);
    while (Timeline::ParserStatusManager::Instance().GetParserStatus(statusId) ==
           Timeline::ParserStatus::RUNNING && getline(file, line)) {
        const std::basic_string<char>& basicString(line);
        std::vector<std::string> rowVector = StringUtil::StringSplit(basicString);
        if (!rowVector.empty() and rowVector[0] == STEP_ID or rowVector[0] == MODEL_ID or rowVector[0] == DEVICE_ID) {
            for (size_t i = 0; i < rowVector.size(); ++i) {
                dataMap[rowVector[i]] = i;
            }
            continue;
        }
        Kernel kernel {};
        if (dataMap.size() < kernelTableNum or !KernelParse::mapperToKernelDetail(dataMap, rowVector, fileId, kernel)) {
            ParseEndCallBack(fileId, false, "The header is incorrect or incomplete of " + filePath);
            ServerLog::Error("The header is incorrect or incomplete. The path is: " + filePath);
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
    // 判断是否为训练场景
    if (devices.size() == 1 && devices.count(fileId) == 1) {
        ParseEndCallBack(fileId, true, "");
    } else {
        for (const std::string& device : devices) {
            auto tmpFileId = std::string().append(MSPROF_PREFIX).append(fileId).append(MSPROF_CONNECT).append(device);
            ParseEndCallBack(tmpFileId, true, "");
        }
    }

    Timeline::ParserStatusManager::Instance().SetFinishStatus(KERNEL_PREFIX + fileId);
    return true;
}

void KernelParse::ParseEndCallBack(const std::string &fileId, bool result, const std::string &msg)
{
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
    auto event = std::make_unique<Protocol::OperatorParseStatusEvent>();
    event->moduleName = Protocol::ModuleType::OPERATOR;
    event->token = token;
    event->result = true;
    event->data.rankId = fileId;
    event->data.status = result;
    event->data.error = msg;
    session->OnEvent(std::move(event));
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
    kernel.startTime = atof(row[startTimeIndex].c_str());
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
    threadPool->Reset();
    ServerLog::Info("Summary task completed.");
    auto databaseList = Timeline::DataBaseManager::Instance().GetAllSummaryDatabase();
    for (auto &db: databaseList) {
        db->ReleaseStmt();
        db->CloseDb();
    }
    Timeline::DataBaseManager::Instance().Clear(Timeline::DatabaseType::SUMMARY);
}

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic

