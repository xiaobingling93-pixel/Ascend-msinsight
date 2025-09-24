/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "ProjectParserDbNPUMonitor.h"
#include "CommonDefs.h"
#include "ServerLog.h"
#include "DataBaseManager.h"
#include "ProjectAnalyze.h"
#include "FullDbParser.h"
#include "BaselineManager.h"

namespace Dic {
namespace Module {
using namespace Server;
using namespace FullDb;

ProjectTypeEnum ProjectParserDbNPUMonitor::GetProjectType(const std::string &dataPath)
{
    return ProjectTypeEnum::DB_NPUMONITOR;
}

std::vector<std::string> ProjectParserDbNPUMonitor::GetParseFileByImportFile(const std::string &importFile,
                                                                             std::string &error)
{
    std::vector<std::string> npuMonitorFiles = FileUtil::FindNPUMonitorFiles(importFile);
    if (npuMonitorFiles.empty()) {
        error = "No parsable db files found, Possible reasons:; 1.File not exist; "
                "2.The nesting depth of the imported sub-file exceeds 5; 3.The sub-file path length exceeds " +
                std::to_string(FileUtil::GetFilePathLengthLimit());
        ServerLog::Info(error);
        return {importFile};
    }
    // npumonitor的表结构和PTA的db格式相同
    DataBaseManager::Instance().SetFileType(FileType::PYTORCH);
    return npuMonitorFiles;
}

void ProjectParserDbNPUMonitor::BuildProjectExploreInfo(Dic::Module::Global::ProjectExplorerInfo &info,
                                                        const std::vector<std::string> &parsedFiles)
{
    ProjectParserBase::BuildProjectExploreInfo(info, parsedFiles);
    for (const auto &parsedFile : parsedFiles) {
        auto parseFileInfoRank = std::make_shared<ParseFileInfo>();
        parseFileInfoRank->parseFilePath = parsedFile;
        parseFileInfoRank->type = ParseFileType::RANK;
        parseFileInfoRank->subId = parsedFile;
        parseFileInfoRank->curDirName = FileUtil::GetFileName(parsedFile);
        parseFileInfoRank->fileId = parsedFile;
        info.AddSubParseFileInfo(info.fileName, ParseFileType::PROJECT, parseFileInfoRank);
    }
}

void ProjectParserDbNPUMonitor::ParserBaseline(const Global::ProjectExplorerInfo &projectInfo,
                                               Global::BaselineInfo &baselineInfo)
{
    // 该函数主要复用ProjectParserDb::ParserBaseline()的逻辑，除去了找文件的步骤，因为npumonitor数据的parsedFilePath就是db文件的路径，不需要额外查找
    if (projectInfo.fileInfoMap.empty()) {
        return;
    }

    DataBaseManager::Instance().SetBaselineFileType(FileType::PYTORCH);
    Timeline::DataBaseManager::Instance().SetDataType(Timeline::DataType::DB);

    auto hostInfoMap = GetReportFiles({ projectInfo });
    if (std::empty(hostInfoMap)) {
        Global::BaselineManager::Instance().SetBaselineInfo(baselineInfo);
        baselineInfo.errorMessage = "NPU monitor db get host info failed!";
        return;
    }
    FilterHostMap(hostInfoMap, baselineInfo.parsedFilePath);
    if (std::empty(hostInfoMap.begin()->second) || std::empty(hostInfoMap.begin()->second.begin()->second)) {
        Global::BaselineManager::Instance().SetBaselineInfo(baselineInfo);
        baselineInfo.errorMessage = "NPU monitor db get rank info failed!";
        return;
    }
    std::string rankId = hostInfoMap.begin()->first + hostInfoMap.begin()->second.begin()->second[0];

    baselineInfo.rankId = rankId;
    baselineInfo.cardName = "Baseline_" + rankId;
    baselineInfo.host = hostInfoMap.begin()->first;
    baselineInfo.fileId = baselineInfo.parsedFilePath;
    Global::BaselineManager::Instance().SetBaselineInfo(baselineInfo);
    if (!Timeline::DataBaseManager::Instance().CreateTraceConnectionPool(baselineInfo.rankId, baselineInfo.parsedFilePath)) {
        ServerLog::Error("Failed to create baseline connection pool for NPU monitor. ");
    }
    if (Timeline::DataBaseManager::Instance().IsContainDatabasePath(baselineInfo.parsedFilePath)) {
        ServerLog::Warn("Baseline has been parsed.");
        return;
    }
    FullDb::FullDbParser::Instance().Parse(std::vector<std::string>{ baselineInfo.rankId },
        baselineInfo.parsedFilePath);
}

ProjectAnalyzeRegister<ProjectParserDbNPUMonitor> pRegDBNPUMonitor(ParserType::DB_NPUMONITOR);
}
}
