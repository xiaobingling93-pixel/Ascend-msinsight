/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "WsSessionManager.h"
#include "ProjectExplorerManager.h"
#include "JupyterServerManager.h"
#include "TimeUtil.h"
#include "ClusterFileParser.h"
#include "ProjectParserFactory.h"
#include "ProjectAnalyze.h"
#include "ImportActionHandler.h"


using namespace Dic;
using namespace Dic::Server;
using namespace Dic::Module::Global;
using namespace Dic::Module::Timeline;
bool ImportActionHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    auto &request = dynamic_cast<ImportActionRequest &>(*requestPtr);
    ServerLog::Info("Import action request handler start");
    WsSession &session = *WsSessionManager::Instance().GetSession();
    std::unique_ptr<ImportActionResponse> responsePtr = std::make_unique<ImportActionResponse>();
    ImportActionResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    std::string warnMsg;
    if (!request.params.CommonCheck(warnMsg)) {
        ServerLog::Warn(warnMsg);
        SetResponseResult(response, false, warnMsg);
        session.OnResponse(std::move(responsePtr));
        return false;
    }

    if (request.params.projectAction == ProjectActionEnum::ADD_FILE) {
        // ConvertToRealPath 调用 FileUtil::ConvertToRealPath 方法，其中 FileUtil::CheckDirValid 已做软链接检查
        if (!request.params.ConvertToRealPath(warnMsg)) {
            SetResponseResult(response, false, warnMsg);
            session.OnResponse(std::move(responsePtr));
            SendParseFailEvent(warnMsg);
            return false;
        }
        if (!ImportFile(request, warnMsg)) {
            SetResponseResult(response, false, warnMsg);
            session.OnResponse(std::move(responsePtr));
            return false;
        }
    } else if (request.params.projectAction == ProjectActionEnum::TRANSFER_PROJECT) {
        if (!TransferProject(request)) {
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return false;
        }
    }
    return true;
}

void ImportActionHandler::SendParseFailEvent(const std::string &message)
{
    auto event = std::make_unique<ParseFailEvent>();
    event->moduleName = MODULE_TIMELINE;
    event->result = false;
    event->body.error = message;
    SendEvent(std::move(event));
}

void ImportActionHandler::LogIfFileNotExist(const Global::ProjectExplorerInfo &projectExplorerInfo)
{
    // 拖拽的文件原始文件不会保存在本地，因此不需要对拖拽的文件路径进行校验
    if (projectExplorerInfo.importType == "drag") {
        return;
    }
    if (!FileUtil::CheckFilePathExist(projectExplorerInfo.fileName)) {
        std::string message = "paths do not exist: " + projectExplorerInfo.fileName;
        SendParseFailEvent(message);
        ServerLog::Warn(message);
    }
}

bool ImportActionHandler::TransferProject(ImportActionRequest &request)
{
    // 切换项目时，会对一级目录下所有内容进行加载
    std::vector<Global::ProjectExplorerInfo> projectExplorerInfo = Global::ProjectExplorerManager::Instance()
            .QueryProjectExplorer(request.params.projectName, std::vector<std::string>());
    if (projectExplorerInfo.empty()) {
        ServerLog::Warn("params error, project explorer info is not existed.");
        return false;
    }
    std::for_each(projectExplorerInfo.begin(), projectExplorerInfo.end(), [](const auto& project) {
        ImportActionHandler::LogIfFileNotExist(project);
    });
    if (projectExplorerInfo[0].projectType < static_cast<int>(ProjectTypeEnum::DB) ||
        projectExplorerInfo[0].projectType > static_cast<int>(ProjectTypeEnum::OTHER)) {
        ServerLog::Warn("Project type invalid!");
        return false;
    }
    auto projectTypeEnum = static_cast<ProjectTypeEnum>(projectExplorerInfo[0].projectType);
    ParserType parserType = coverProjectTypeToParserType(projectTypeEnum);
    std::shared_ptr<ProjectParserBase> factory = ParserFactory::GetProjectParser(parserType);
    ParserFactory::Reset();
    factory->Parser(projectExplorerInfo, request);
    return true;
}

bool ImportActionHandler::ImportFile(ImportActionRequest &request, std::string &warnMsg)
{
    // 如果入参的文件内容不为空，则通过文件判断文件类型获取工厂
    std::string importPath = request.params.path[0];
    std::pair<std::string, ParserType> parserType = ParserFactory::GetImportType(importPath);
    ParserType allocType = parserType.second;
    std::shared_ptr<ProjectParserBase> projectParser = ParserFactory::GetProjectParser(allocType);
    // 路径列表不为空，需要进行文件目录的新增、覆盖
    ProjectTypeEnum projectType = projectParser->GetProjectType(importPath);
    Global::ProjectExplorerInfo projectExplorerInfo;
    // 获取文件列表
    std::vector<std::string> parseFileList = projectParser->GetParseFileByImportFile(importPath, warnMsg);
    bool isNotCluster = parseFileList.size() == 1 && !ClusterFileParser::CheckIsCluster(parseFileList[0]);
    // 如果没有找到文件（warnMag不为空），并且不是集群数据，则需要发送错误提示给前端
    if (!warnMsg.empty() && isNotCluster) {
        SendParseFailEvent(warnMsg);
    }
    projectExplorerInfo.fileName = importPath;
    projectExplorerInfo.projectName = request.params.projectName;
    projectExplorerInfo.projectType = static_cast<int64_t>(projectType);
    projectExplorerInfo.importType = "import";
    projectExplorerInfo.accessTime = TimeUtil::Instance().NowStr();
    ProjectAnalyze::Instance().ProjectExportInfoBuild(allocType, parseFileList, projectExplorerInfo);
    if (!Global::ProjectExplorerManager::Instance().SaveProjectExplorer({projectExplorerInfo},
                                                                        request.params.isConflict)) {
        return false;
    }

    LogIfFileNotExist(projectExplorerInfo);
    // 以下情况需要对当前导入内容进行重置：1.导入数据和原来数据有冲突；2.无冲突，但是当前选中项目与目标项目不一致；
    if (allocType != ParserType::JSON && allocType != ParserType::DB) {
        ParserFactory::Reset();
    }
    projectParser->Parser({std::move(projectExplorerInfo)}, request);
    return true;
}
