/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "JupyterServerManager.h"
#include "JupyterFileParser.h"
#include "ConstantDefs.h"
#include "JupyterProtocolEvent.h"
#include "ParamsParser.h"
#include "CommonDefs.h"
#include "ProjectAnalyze.h"
#include "ProjectParserIpynb.h"

using namespace Dic::Module;
using namespace Dic::Module::Global;
using namespace Jupyter;
void ProjectParserIpynb::Parser(const std::vector<ProjectExplorerInfo> &projectInfos, ImportActionRequest &request)
{
    // 初始化jupyter落盘日志信息
    const Server::ParamsOption &option = Server::ParamsParser::Instance().GetOption();
    Dic::Module::Jupyter::JupyterServerManager::Instance().InitJupyterLogPath(option.logPath);
    std::string path = projectInfos[0].fileName;
    if (!FileUtil::CheckDirValid(path)) {
        SendParseFailEvent("", "Invalid ipynb file.");
        IpynbImportResponse(request, projectInfos[0], false);
        return;
    }
    // 检查jupyter服务是否存在
    std::string jupyterVersionResult;
    if (CmdUtil::ExecuteCmdWithResult("jupyter-lab --version", jupyterVersionResult)
            && !jupyterVersionResult.empty()) {
        IpynbImportResponse(request, projectInfos[0], true);
        JupyterFileParser::Instance().GetThreadPool()->AddTask(JupyterProcess, path);
    } else {
        SendParseFailEvent("", "Jupyter env is not ready.");
        IpynbImportResponse(request, projectInfos[0], false);
    }
}

void ProjectParserIpynb::IpynbImportResponse(ImportActionRequest &request, const ProjectExplorerInfo &projectInfo,
                                             bool isDisplay)
{
    std::unique_ptr<ImportActionResponse> responsePtr = std::make_unique<ImportActionResponse>();
    ImportActionResponse &response = *responsePtr.get();
    ModuleRequestHandler::SetBaseResponse(request, response);
    response.command = Protocol::REQ_RES_IMPORT_ACTION;
    response.moduleName = MODULE_TIMELINE;
    response.body.isIpynb = isDisplay;
    response.body.reset = true;
    response.body.subParseFileInfo = projectInfo.subParseFileInfo;
    response.body.projectFileTree = projectInfo.projectFileTree;
    SendResponse(std::move(responsePtr), true);
}

void ProjectParserIpynb::JupyterProcess(const std::string &file)
{
    std::string url;
    // 从根目录重置服务(如果当前启动目录无法打开文件，则在当前文件根目录下重启Jupyter服务)
    JupyterServerManager &server = JupyterServerManager::Instance();
    if (server.ResetUnderRootPath(file)) {
        // 重启成功，则获取url内容
        url = server.GetJupyterUrl(file);
    }
    SendJupyterInfo(url);
}

void ProjectParserIpynb::SendJupyterInfo(std::string url)
{
    Server::ServerLog::Info("Parse jupyter file end, send event");
    auto event = std::make_unique<ParseJupyterCompletedEvent>();
    event->moduleName = MODULE_JUPYTER;
    event->result = true;
    if (url.empty()) {
        event->body.parseResult = PARSE_RESULT_FAIL;
    } else {
        event->body.parseResult = PARSE_RESULT_OK;
    }
    event->body.url = std::move(url);
    SendEvent(std::move(event));
}

ProjectTypeEnum ProjectParserIpynb::GetProjectType(const std::vector<std::string> &dataPath)
{
    return ProjectTypeEnum::IPYNB;
}

void ProjectParserIpynb::BuildProjectExploreInfo(ProjectExplorerInfo &projectInfo,
                                                 const std::vector<std::string> &parsedFiles)
{
    ProjectParserBase::BuildProjectExploreInfo(projectInfo, parsedFiles);
    for (const auto &file: parsedFiles) {
        auto parsedFileInfo = std::make_shared<ParseFileInfo>();
        parsedFileInfo->subId = file;
        parsedFileInfo->type = ParseFileType::IPYNB;
        parsedFileInfo->curDirName = FileUtil::GetFileName(file);
        projectInfo.AddSubParseFileInfo(projectInfo.fileName, ParseFileType::PROJECT, parsedFileInfo);
    }
}

ProjectAnalyzeRegister<ProjectParserIpynb> pRegIPYNB(ParserType::IPYNB);
