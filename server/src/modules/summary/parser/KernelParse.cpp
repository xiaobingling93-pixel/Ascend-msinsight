/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "KernelParse.h"
#include "DataBaseManager.h"
#include "FileUtil.h"
#include "TraceFileParser.h"
#include "ParserStatusManager.h"
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

std::vector<std::string> KernelParse::StringSplit(const std::string& str)
{
    std::vector<std::string> result;
    std::string subStr = "";
    int count = 0;
    for (char ch : str) {
        // 根据字符串内 ” 的数量来判断是否是一个完整的字符串，count % 2 = 0 为偶数个，满足要求
        if (ch == ',' and count % 2 == 0) {
            if (count != 0) {
                subStr = '\"' + subStr + '\"';
            }
            result.push_back(subStr);
            subStr = "";
            count = 0;
        } else if (ch == '\"') {
            count++;
        } else {
            subStr += ch;
        }
    }
    result.push_back(subStr);
    return result;
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

    while (Timeline::ParserStatusManager::Instance().GetParserStatus(fileId) ==
    Timeline::ParserStatus::RUNNING && getline(file, line)) {
        const std::basic_string<char>& basicString(line);
        std::vector<std::string> rowVector = StringSplit(basicString);
        if (rowVector[0] == "Step Id" or rowVector[0] == "Model ID" or rowVector[0] == "Device_id") {
            for (int i = 0; i < rowVector.size(); i++) {
                dataMap[rowVector[i]] = i;
            }
            continue;
        }
        Kernel kernel {};
        if (dataMap.size() < kernelTableNum or !KernelParse::mapperToKernelDetail(dataMap, rowVector, fileId, kernel)) {
            ServerLog::Error("The header of the imported file is incorrect or incomplete. The path is: " + kernelFile);
            return;
        }
        // 读取每一行数据写入kernel内
        db->InsertKernelDetail(kernel);
    }
    // 读取剩下的数据写入kernel内
    db->SaveKernelDetail();
    auto end = std::chrono::high_resolution_clock::now();
    ServerLog::Info("end parse kernel detail, cost time:",
                    std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
}

bool KernelParse::mapperToKernelDetail(std::map<std::string, int16_t> dataMap,
                                       std::vector<std::string> row, const std::string &fileId, Kernel &kernel)
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
    } else if (dataMap.find("Device_id") != dataMap.end()) {
        deviceIndex = dataMap["Device_id"];
        nameIndex = dataMap["Op Name"];
        typeIndex = dataMap["OP Type"];
        acceleratorIndex = dataMap["Task Type"];
        startTimeIndex = dataMap["Task Start Time(us)"];
        durationIndex = dataMap["Task Duration(us)"];
        waitTimeIndex = dataMap["Task Wait Time(us)"];
    } else {
        ServerLog::Error("The file header does not contain 'Step Id' or 'Device Id'.");
        return false;
    }

    kernel.rankId = fileId;
    kernel.name = row[nameIndex];
    kernel.stepId = dataMap.count("Step Id") != 0 ? row[stepIndex] : "";
    kernel.type = row[typeIndex];
    kernel.acceleratorCore = row[acceleratorIndex];
    kernel.startTime = atof(row[startTimeIndex].c_str());
    kernel.duration = atof(row[durationIndex].c_str());
    kernel.waitTime = atof(row[waitTimeIndex].c_str());
    kernel.blockDim = atof(row[dataMap["Block Dim"]].c_str());
    kernel.inputDataTypes = row[dataMap["Input Data Types"]];
    kernel.inputShapes = row[dataMap["Input Shapes"]];
    kernel.inputFormats = row[dataMap["Input Formats"]];
    kernel.outputDataTypes = row[dataMap["Output Data Types"]];
    kernel.outputShapes = row[dataMap["Output Shapes"]];
    kernel.outputFormats = row[dataMap["Output Formats"]];
    return true;
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
    auto db = Timeline::DataBaseManager::Instance().GetSummaryDatabase(fileId);
    std::string dbPath = FileUtil::GetDbPath(selectedFolder, fileId);
    if (!(db->OpenDb(dbPath, false) && db->CreateTable() &&
          db->SetConfig() && db->InitStmt())) {
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
    for (auto &db: databaseList) {
        db->ReleaseStmt();
        db->CloseDb();
        db->ClearParseKernelFile();
    }
    Timeline::DataBaseManager::Instance().Clear();
}

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic

