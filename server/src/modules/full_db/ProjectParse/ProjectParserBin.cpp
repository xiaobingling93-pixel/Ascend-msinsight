/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <algorithm>
#include "TimelineRequestHandler.h"
#include "ModuleRequestHandler.h"
#include "CommonDefs.h"
#include "DataBaseManager.h"
#include "GlobalProtocolEvent.h"
#include "SourceFileParser.h"
#include "TraceTime.h"
#include "BaselineManager.h"
#include "ProjectAnalyze.h"
#include "DataBaseManager.h"
#include "TrackInfoManager.h"
#include "ProjectParserBin.h"

namespace Dic::Module {
using namespace Dic::Server;
using namespace Dic::Module::Global;

void ProjectParserBin::Parser(const std::vector<ProjectExplorerInfo> &projectInfos, ImportActionRequest &request)
{
    std::unique_ptr<ImportActionResponse> responsePtr = std::make_unique<ImportActionResponse>();
    ImportActionResponse &response = *responsePtr;
    ModuleRequestHandler::SetBaseResponse(request, response);
    if (std::empty(projectInfos)) {
        SendParseFailEvent("", "", "Project explorer info is not existed.");
        // 这里需要返回一个true应答,否则前端会陷入不停loading中
        SendResponse(std::move(responsePtr), true);
        return;
    }
    Timeline::DataBaseManager::Instance().SetDataType(Timeline::DataType::TEXT);

    ModuleRequestHandler::SetResponseResult(response, true);
    response.command = Protocol::REQ_RES_IMPORT_ACTION;
    response.moduleName = MODULE_TIMELINE;
    response.body.reset = true;
    response.body.subParseFileInfo = projectInfos[0].subParseFileInfo;
    // 导入bin文件时，只会有二层文件树，且二级目录数量为1，因此这里直接对rankId进行赋值
    for (auto &item: projectInfos[0].projectFileTree) {
        for (auto &subItem: item->subParseFile) {
            subItem->rankId = subItem->parseFilePath;
        }
    }
    if (!Global::ProjectExplorerManager::Instance().UpdateParseFileInfo(projectInfos[0].projectName,
                                                                        projectInfos[0].subParseFileInfo)) {
        ServerLog::Error("Failed to update project in parsing");
    }
    response.body.projectFileTree = projectInfos[0].projectFileTree;

    std::string selectedFolder = projectInfos[0].fileName;

    // compute
    std::string errorMessage;
    if (FileUtil::CheckFilePathLength(selectedFolder) &&
        Source::SourceFileParser::Instance().CheckOperatorBinary(selectedFolder, errorMessage)) {
        ServerLog::Info("Import file is binary.Start parse source binary file.");
        Source::SourceFileParser::Instance().SetFilePath(selectedFolder);
        HandleCompute(response, selectedFolder);
        Timeline::TraceTime::Instance().SetIsSimulation(true);
        SaveDbPath(projectInfos[0].projectName, dataPathToDbMap);
        SendResponse(std::move(responsePtr), true);
    } else {
        if (!errorMessage.empty()) {
            Dic::Protocol::SendReadFileFailEvent(selectedFolder, errorMessage);
        } else {
            SendParseFailEvent("", "", "Import file is invalid, path: " + selectedFolder);
        }

        // 这里需要返回一个true应答,否则前端会陷入不停loading中
        SendResponse(std::move(responsePtr), true);
    }
}

void ProjectParserBin::HandleCompute(ImportActionResponse &response, const std::string &selectedFolder)
{
    ServerLog::Info("Start parser source binary.");
    Source::SourceFileParser &sourceFileParser = Source::SourceFileParser::Instance();
    sourceFileParser.Reset();
    SetParseCallBack(sourceFileParser);
    std::vector<std::string> empty;
    auto files = GetSimulationTraceFiles(selectedFolder, response.body);
    std::string fileId;
    std::string rankId;
    if (!files.empty()) {
        rankId = files.front().first;
        fileId = files.front().second;
        dataPathToDbMap[selectedFolder].push_back(fileId);
        FullDb::DataBaseManager::Instance().CreatTraceConnectionPool(rankId, fileId);
    } else {
        fileId = selectedFolder;
        ServerLog::Error("Simulation trace files is empty.");
    }
    response.body.isSimulation = true;
    std::map<std::string, std::vector<std::string>> rankListMap = FileUtil::SplitToRankList(files);
    RankInfo rankInfo("", "", rankId, rankId, rankId);
    Timeline::TrackInfoManager::Instance().SetRankListByFileId(fileId, rankInfo);
    sourceFileParser.Parse(empty, rankId, selectedFolder, fileId);
    for (const auto &rankEntry: rankListMap) {
        if (rankEntry.second.empty()) {
            continue;
        }
        std::string cardPath = FileUtil::GetRankIdFromPath(rankEntry.second[0]);
        SetBaseActionOfResponse(response, rankEntry.second[0], fileId, cardPath, std::vector<std::string>{});
    }
    ModuleRequestHandler::SetResponseResult(response, true);
    response.body.isBinary = true;
    response.body.coreList = sourceFileParser.GetCoreList();
    response.body.sourceList = sourceFileParser.GetSourceList();
    response.body.hasCachelineRecords = sourceFileParser.HasCachelineRecords();
    response.body.version = sourceFileParser.GetInstrVersion();
}

std::vector<std::pair<std::string, std::string>> ProjectParserBin::GetSimulationTraceFiles(
    const std::string &selectFilePath,
    ImportActionResBody &body)
{
    body.isCluster = false;
    std::vector<std::pair<std::string, std::string>> files;
    std::string fileId = FileUtil::GetSingleFileIdWithDb(selectFilePath);
    if (fileId.empty()) {
        ServerLog::Error("File id is empty");
        return files;
    }
    files.emplace_back(selectFilePath, fileId);
    return files;
}

void ProjectParserBin::SetParseCallBack(FileParser &fileParser)
{
    std::function<void(const std::string, const std::string, bool, const std::string)> func =
            std::bind(ParseEndCallBack, std::placeholders::_1, std::placeholders::_2,
                      std::placeholders::_3,
                      std::placeholders::_4);
    fileParser.SetParseEndCallBack(func);

    // 复用解析完成回调函数设置逻辑
    std::function<void(const std::string, uint64_t parsedSize, uint64_t totalSize, int progress)> progressFunc =
            std::bind(ParseProgressCallBack, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                      std::placeholders::_4);
    fileParser.SetParseProgressCallBack(progressFunc);
}

ProjectTypeEnum ProjectParserBin::GetProjectType(const std::string &dataPath)
{
    return ProjectTypeEnum::BIN;
}

void ProjectParserBin::ParserBaseline(const Global::ProjectExplorerInfo &projectInfo,
    Global::BaselineInfo &baselineInfo)
{
    if (projectInfo.fileInfoMap.empty()) {
        return;
    }
    std::string filePath = projectInfo.subParseFileInfo[0]->parseFilePath;
    std::string fileId = FileUtil::GetSingleFileIdWithDb(filePath);
    std::string rankId = filePath;
    baselineInfo.rankId = rankId;
    baselineInfo.cardName = "Baseline_" + rankId;
    baselineInfo.fileId = fileId;
    Timeline::TrackInfoManager::Instance().SetRankListByFileId(fileId, {"", "", rankId, rankId, rankId});
    Global::BaselineManager::Instance().SetBaselineInfo(baselineInfo);
    Source::SourceFileParser &sourceFileParser = Source::SourceFileParser::Instance();
    // 如果文件已经被解析则直接返回
    if (sourceFileParser.IsBaselineParsed(rankId)) {
        baselineInfo.errorMessage = "Can't set oneself as baseline.";
        sourceFileParser.SynchronizeBaselineInfo();
        return;
    }
    // 创建数据库连接池
    if (!Timeline::DataBaseManager::Instance().CreatTraceConnectionPool(rankId, fileId)) {
        ServerLog::Warn("Fail to create connection pool, fileId:", fileId, ", path:", rankId, '.');
    }
    // 只需要对比timeline和details页面内容，因此不需要对source相关的内容做处理
    std::string message;
    if (sourceFileParser.CheckOperatorBinary(filePath, message)) {
        sourceFileParser.SetBaselineFilePath(filePath);
        sourceFileParser.Parse(std::vector<std::string>(), rankId, filePath, fileId);
    } else {
        ServerLog::Error("Failed to parse baseline bin file cause ", message);
    }
}

void ProjectParserBin::BuildProjectExploreInfo(Dic::Module::Global::ProjectExplorerInfo &projectInfo,
    const std::vector<std::string> &parsedFiles)
{
    ProjectParserBase::BuildProjectExploreInfo(projectInfo, parsedFiles);
    std::for_each(parsedFiles.begin(), parsedFiles.end(), [&projectInfo](const std::string &parseFile) {
        ProjectParserBin::BuildProjectInfoFromParseFile(projectInfo, parseFile);
    });
}

void ProjectParserBin::BuildProjectInfoFromParseFile(ProjectExplorerInfo &projectInfo, const std::string &parsedFile)
{
    // bin类型没有上层结构
    auto parseFileInfo = std::make_shared<ParseFileInfo>();
    parseFileInfo->parseFilePath = parsedFile;
    parseFileInfo->type = ParseFileType::COMPUTE;
    parseFileInfo->curDirName = FileUtil::GetFileName(parsedFile);
    parseFileInfo->subId = parsedFile;
    parseFileInfo->fileId = FileUtil::GetSingleFileIdWithDb(parsedFile);
    projectInfo.AddSubParseFileInfo(parseFileInfo);
}

static ProjectAnalyzeRegister<ProjectParserBin> pRegBIN(ParserType::BIN);
}