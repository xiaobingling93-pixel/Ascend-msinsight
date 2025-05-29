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
#include "ProjectParserBin.h"

using namespace Dic::Module;
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
    if (!files.empty()) {
        fileId = files.front().second;
        dataPathToDbMap[selectedFolder].push_back(fileId);
        FullDb::DataBaseManager::Instance().CreatConnectionPool(fileId, fileId);
    } else {
        fileId = selectedFolder;
        ServerLog::Error("Simulation trace files is empty.");
    }
    response.body.isSimulation = true;
    std::map<std::string, std::vector<std::string>> rankListMap = FileUtil::SplitToRankList(files);
    sourceFileParser.Parse(empty, fileId, selectedFolder);
    for (const auto &rankEntry : rankListMap) {
        if (rankEntry.second.empty()) {
            continue;
        }
        std::string cardPath = FileUtil::GetRankIdFromPath(rankEntry.second[0]);
        SetBaseActionOfResponse(response, rankEntry.first, fileId, cardPath, std::vector<std::string>{});
    }
    ModuleRequestHandler::SetResponseResult(response, true);
    response.body.isBinary = true;
    response.body.coreList = sourceFileParser.GetCoreList();
    response.body.sourceList = sourceFileParser.GetSourceList();
    response.body.hasCachelineRecords =  sourceFileParser.HasCachelineRecords();
    response.body.version = sourceFileParser.GetInstrVersion();
}

std::vector<std::pair<std::string, std::string>> ProjectParserBin::GetSimulationTraceFiles(
    const std::string &selectFilePath,
    ImportActionResBody &body)
{
    body.isCluster = false;
    std::vector<std::pair<std::string, std::string>> files;
    std::string fileId = GetFileIdWithDb(selectFilePath);
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
    std::string fileId = GetFileIdWithDb(filePath);
    std::string dbPath = FileUtil::GetDbPath(filePath, fileId);
    baselineInfo.rankId = fileId;
    baselineInfo.cardName = "baseline" + fileId;
    Global::BaselineManager::Instance().SetBaselineInfo(baselineInfo);
    if (!Timeline::DataBaseManager::Instance().CreatConnectionPool(fileId, dbPath)) {
        ServerLog::Error("Fail to create connection pool, fileId:", fileId, ", path:", dbPath, '.');
        return;
    }
    Source::SourceFileParser &sourceFileParser = Source::SourceFileParser::Instance();
    // 如果文件已经被解析则直接返回
    if (sourceFileParser.IsBaselineParsed(filePath)) {
        sourceFileParser.SynchronizeBaselineInfo();
        return;
    }
    // 只需要对比timeline和details页面内容，因此不需要对source相关的内容做处理
    std::string message;
    if (sourceFileParser.CheckOperatorBinary(filePath, message)) {
        sourceFileParser.SetBaselineFilePath(filePath);
        sourceFileParser.Parse(std::vector<std::string>(), fileId, filePath);
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
    parseFileInfo->fileId = GetFileIdWithDb(parsedFile);
    projectInfo.AddSubParseFileInfo(parseFileInfo);
}
std::string ProjectParserBin::GetFileIdWithDb(const std::string &filePath)
{
    // 避免一个目录下有多个bin文件，这里通过文件名做区分
    std::string fileNameWithoutEx = FileUtil::StemFile(filePath);
    std::string dbFileName = StringUtil::StrJoin(fileNameWithoutEx, "_mindstudio_insight_data.db");
    std::string dir = FileUtil::GetParentPath(filePath);
    return FileUtil::SplicePath(dir, dbFileName);
}

ProjectAnalyzeRegister<ProjectParserBin> pRegBIN(ParserType::BIN);