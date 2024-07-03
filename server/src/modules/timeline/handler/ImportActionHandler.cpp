/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ImportActionHandler.h"
#include "ServerLog.h"
#include "ExecUtil.h"
#include "WsSessionManager.h"
#include "OperatorProtocolEvent.h"
#include "ProjectExplorerManager.h"
#include "SourceFileParser.h"
#include "JupyterServerManager.h"
#include "FileUtil.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic;
using namespace Dic::Server;

void ImportActionHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    ImportActionRequest &request = dynamic_cast<ImportActionRequest &>(*requestPtr.get());
    std::string token = request.token;
    ServerLog::Info("Import action request handler start");
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<ImportActionResponse> responsePtr = std::make_unique<ImportActionResponse>();
    ImportActionResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    std::string warnMsg;
    if (!request.params.CommonCheck(warnMsg)) {
        ServerLog::Warn(warnMsg);
        SetResponseResult(response, false, warnMsg);
        session.OnResponse(std::move(responsePtr));
        return;
    }

    std::shared_ptr<ParserAlloc> factory;
    if (!request.params.path.empty()) {
        // 如果入参的文件内容不为空，则为导入新的文件
        if (!request.params.ConvertToRealPath(warnMsg)) {
            SetResponseResult(response, false, warnMsg);
            session.OnResponse(std::move(responsePtr));
            return;
        }
        if (!ImportFile(request)) {
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return;
        }
    } else {
        // 如果入参的文件内容为空，则为项目切换
        if (!TransferProject(request)) {
            SetResponseResult(response, false);
            session.OnResponse(std::move(responsePtr));
            return;
        }
    }
}

void ImportActionHandler::SendParseFailEvent(const std::string &token, const std::string &message)
{
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session when send parseFailEvent.");
        return;
    }
    WsSession *session = WsSessionManager::Instance().GetSession(token);
    if (session == nullptr) {
        return;
    }
    auto event = std::make_unique<ParseFailEvent>();
    event->moduleName = ModuleType::TIMELINE;
    event->token = token;
    event->result = false;
    event->body.error = message;
    session->OnEvent(std::move(event));
}

void ImportActionHandler::LogIfFileNotExist(const std::vector<Global::ProjectExplorerInfo> &projectExplorerInfo,
                                            std::string token)
{
    // 拖拽的文件原始文件不会保存在本地，因此不需要对拖拽的文件路径进行校验
    if (!projectExplorerInfo.empty() && projectExplorerInfo[0].importType == "drag") {
        return;
    }
    for (auto file: projectExplorerInfo) {
        if (!FileUtil::CheckFilePathExist(file.fileName)) {
            string message = "paths do not exist: " + file.fileName;
            SendParseFailEvent(token, message);
            ServerLog::Warn(message);
        }
    }
}

bool ImportActionHandler::TransferProject(ImportActionRequest &request)
{
    std::vector<Global::ProjectExplorerInfo> projectExplorerInfo = Global::ProjectExplorerManager::Instance()
            .QueryProjectExplorer(request.params.projectName, std::vector<std::string>());
    if (projectExplorerInfo.empty()) {
        ServerLog::Warn("params error, project explorer info is not existed.");
        return false;
    }
    LogIfFileNotExist(projectExplorerInfo, request.token);

    auto projectTypeEnum = static_cast<ProjectTypeEnum>(projectExplorerInfo[0].projectType);
    ParserType parserType = coverProjectTypeToParserType(projectTypeEnum);
    std::shared_ptr<ParserAlloc> factory = ParserFactory::ParserImport(parserType);
    factory->Reset();
    factory->Parser(projectExplorerInfo, request);
    return true;
}

bool ImportActionHandler::ImportFile(ImportActionRequest &request)
{
    // 如果入参的文件内容不为空，则通过文件判断文件类型获取工厂
    std::pair<std::string, ParserType> parserType = ParserFactory::GetImportType(request.params.path);
    ParserType allocType = parserType.second;
    std::shared_ptr<ParserAlloc> factory = ParserFactory::ParserImport(allocType);
    // 路径列表不为空，需要进行文件目录的新增、覆盖
    ProjectTypeEnum projectType = factory->GetProjectType(request.params.path);
    if (!Global::ProjectExplorerManager::Instance().SaveProjectExplorer(request.params.projectName,
                                                                        request.params.path[0], projectType,
                                                                        "import", std::vector<std::string>())) {
        ServerLog::Warn("Save file path failed.");
        return false;
    }

    std::vector<Global::ProjectExplorerInfo> projectExplorerInfo = Global::ProjectExplorerManager::Instance()
            .QueryProjectExplorer(request.params.projectName, std::vector<std::string>());
    if (projectExplorerInfo.empty()) {
        return false;
    }
    LogIfFileNotExist(projectExplorerInfo, request.token);
    std::vector<std::string> filePathUnderProject;
    for (const auto &item: projectExplorerInfo) {
        filePathUnderProject.push_back(item.fileName);
    }
    factory->Reset();
    factory->Parser(projectExplorerInfo, request);
    return true;
}

} // Timeline
} // Module
} // Dic