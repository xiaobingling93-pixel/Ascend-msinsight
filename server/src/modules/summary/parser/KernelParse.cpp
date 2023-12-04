/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "KernelParse.h"
#include "DataBaseManager.h"
#include "FileUtil.h"
#include "TraceFileParser.h"
#include "ServerLog.h"

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

void KernelParse::StringSplit(const std::string& str, std::vector<std::string>& res)
{
    std::regex regex(R"(,(?=(?:[^"]*"[^"]*")*[^"]*$))"); // 正则表达式,用于匹配逗号（,）但是排除在引号（“）内的逗号
    std::sregex_token_iterator pos(str.begin(), str.end(), regex, -1);
    decltype(pos) end;              // 自动推导类型
    for (; pos != end; ++pos) {
        res.push_back(pos->str());
    }
}

void KernelParse::KernelFileParse(const std::string &parentDir, const std::string &fileId)
{
    auto start = std::chrono::high_resolution_clock::now();
    ServerLog::Info("start parse kernel detail.");
    std::vector<std::string> kernelFileVec = FileUtil::FindFilesByRegex(parentDir, std::regex(kernelDetailReg));
    if (kernelFileVec.empty()) {
        ServerLog::Warn("There is no kernel_details.csv for rank " + fileId);
        return;
    }

    std::string kernelFile = kernelFileVec[0];
    auto db = Timeline::DataBaseManager::Instance().GetSummaryDatabase(fileId);
    if (db->HasParseKernelFile(kernelFile)) {
        return;
    } else {
        db->AddParseKernelFile(kernelFile);
    }

    std::ifstream file(kernelFile);
    std::string line;
    std::map<std::string, std::int16_t> dataMap;

    while (getline(file, line)) {
        std::basic_string<char> ss(line);
        std::vector<std::string> row;
        std::string cell;
        StringSplit(ss, row);
        if (row[0] == "Step Id" or row[0] == "Model ID" or row[0] == "Device_id") {
            for (int i = 0; i < row.size(); i++) {
                dataMap[row[i]] = i;
            }
            continue;
        }
        if (dataMap.size() < kernelTableNum) {
            ServerLog::Error("The header of the imported file is incorrect or incomplete. The path is: " + kernelFile);
            return;
        }
        Kernel recordPtr = KernelParse::mapperToKernelDetail(dataMap, row, fileId);
        // 读取每一行数据写入kernel内
        db->InsertKernelDetail(recordPtr);
    }
    // 读取剩下的数据写入kernel内
    db->SaveKernelDetail();
    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("end parse kernel detail, cost time:",
                    std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
}

Kernel KernelParse::mapperToKernelDetail(std::map<std::string, int16_t> dataMap,
    std::vector<std::string> row, const std::string &fileId)
{
    std::int16_t deviceIndex = 0;
    std::int16_t stepIndex = 0;
    std::int16_t nameIndex;
    std::int16_t typeIndex;
    std::int16_t acceleratorIndex;
    std::int16_t startTimeIndex;
    std::int16_t durationIndex;
    std::int16_t waitTimeIndex;
    if (dataMap.find("Step Id") != dataMap.end()) {
        stepIndex = dataMap["Step Id"];
        nameIndex = dataMap["Name"];
        typeIndex = dataMap["Type"];
        acceleratorIndex = dataMap["Accelerator Core"];
        startTimeIndex = dataMap["Start Time(us)"];
        durationIndex = dataMap["Duration(us)"];
        waitTimeIndex = dataMap["Wait Time(us)"];
    } else {
        deviceIndex = dataMap["Device_id"];
        nameIndex = dataMap["Op Name"];
        typeIndex = dataMap["OP Type"];
        acceleratorIndex = dataMap["Task Type"];
        startTimeIndex = dataMap["Task Start Time(us)"];
        durationIndex = dataMap["Task Duration(us)"];
        waitTimeIndex = dataMap["Task Wait Time(us)"];
    }

    Kernel oper {};
    oper.rankId = fileId;
    oper.name = row[nameIndex];
    oper.stepId = dataMap.count("Step Id") != 0 ? row[stepIndex] : "";
    oper.type = row[typeIndex];
    oper.acceleratorCore = row[acceleratorIndex];
    oper.startTime = atof(row[startTimeIndex].c_str());
    oper.duration = atof(row[durationIndex].c_str());
    oper.waitTime = atof(row[waitTimeIndex].c_str());
    oper.blockDim = atof(row[dataMap["Block Dim"]].c_str());
    oper.inputDataTypes = row[dataMap["Input Data Types"]];
    oper.inputShapes = row[dataMap["Input Shapes"]];
    oper.inputFormats = row[dataMap["Input Formats"]];
    oper.outputDataTypes = row[dataMap["Output Data Types"]];
    oper.outputShapes = row[dataMap["Output Shapes"]];
    oper.outputFormats = row[dataMap["Output Formats"]];
    return oper;
}

KernelParse::~KernelParse()
{
    threadPool->ShutDown();
}

bool KernelParse::Parse(const std::vector<std::string> &filePaths, const std::string &fileId,
                        const std::string &selectedFolder)
{
    start = std::chrono::system_clock::now();
    ServerLog::Info("start parse file for summary.");
    auto database = Timeline::DataBaseManager::Instance().GetSummaryDatabase(fileId);
    std::string dbPath = FileUtil::GetDbPath(selectedFolder, fileId);
    if (!(database->OpenDb(dbPath, false) && database->CreateTable() &&
          database->SetConfig() && database->InitStmt())) {
        ServerLog::Error("Failed to open database. path:", dbPath);
        return false;
    }
    std::shared_ptr<std::vector<std::future<void>>> futures = std::make_unique<std::vector<std::future<void>>>();

    for (const auto &filePath: filePaths) {
        auto future = threadPool->AddTask([futures, fileId, &filePath, this]() {
            ServerLog::Info("Wait parse completed. ID:", fileId);
            for (const auto &future: *futures) {
                future.wait();
            }
            ServerLog::Info("Parse completed. ID:", fileId);
            KernelFileParse(filePath, fileId);
            ServerLog::Info("Update depth completed. ID:", fileId);
        });
        futureMap.emplace(fileId, std::move(future));
    }
    if (paserEndCallback != nullptr) {
        std::thread thread{[this, fileId]() {
            WaitParseEnd(fileId);
        }};
        thread.detach();
    }
    return true;
}

bool KernelParse::WaitParseEnd(const std::string &fileId)
{
    if (futureMap.count(fileId) == 0) {
        return false;
    }
    ServerLog::Info("Wait summary parse completed. ID:", fileId);
    auto &future = futureMap.at(fileId);
    future.wait();
    futureMap.erase(fileId);
    auto dur = std::chrono::duration<double, std::milli>(std::chrono::system_clock::now() - start);
    ServerLog::Info("end parse. ID:", fileId, ". time:", dur.count());
    if (paserEndCallback != nullptr) {
        paserEndCallback(fileId, true);
    }
    return true;
}

void KernelParse::Reset()
{
    ServerLog::Info("Reset. wait task completed.");
    threadPool->Reset();
    ServerLog::Info("Task completed.");
    auto databaseList = Timeline::DataBaseManager::Instance().GetAllSummaryDatabase();
    for (auto &database: databaseList) {
        std::string path = database->GetDbPath();
        database->ReleaseStmt();
        database->CloseDb();
        database->ClearParseKernelFile();
        if (!FileUtil::RemoveFile(path)) {
            ServerLog::Error("Failed to remove file. ", path);
        }
    }
    Timeline::DataBaseManager::Instance().Clear();
}

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic

