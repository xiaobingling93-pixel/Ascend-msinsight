/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "JupyterServerManager.h"
#include "JupyterFileParser.h"
#include "WsSession.h"
#include "WsSessionManager.h"
#include "ConstantDefs.h"
#include "JupyterProtocolEvent.h"
#include "ParamsParser.h"
#include "ParserIpynb.h"

namespace Dic {
namespace Module {
using namespace Jupyter;
ParserIpynb::ParserIpynb() = default;

ParserIpynb::~ParserIpynb() = default;

void ParserIpynb::Parser(const std::vector<Global::ProjectExplorerInfo> &projectInfos, ImportActionRequest &request)
{
    // 初始化jupyter落盘日志信息
    const Server::ParamsOption &option = Server::ParamsParser::Instance().GetOption();
    Dic::Module::Jupyter::JupyterServerManager::Instance().InitJupyterLogPath(option.logPath);
    std::string path = projectInfos[0].fileName;
    if (!FileUtil::CheckDirValid(path)) {
        SendParseFailEvent("", "Invalid ipynb file.");
        return;
    }
    // 检查jupyter服务是否存在
    std::string jupyterVersionResult;
    if (CmdUtil::ExecuteCmdWithResult("jupyter-lab --version", jupyterVersionResult)
            && !jupyterVersionResult.empty()) {
        IpynbImportResponse(request, path);
        JupyterFileParser::Instance().GetThreadPool()->AddTask(JupyterProcess, path);
    } else {
        SendParseFailEvent("", "Jupyter env is not ready.");
    }
}

void ParserIpynb::IpynbImportResponse(ImportActionRequest &request, const std::string &fileName)
{
    Server::WsSession &session = *Server::WsSessionManager::Instance().GetSession();
    std::unique_ptr<ImportActionResponse> responsePtr = std::make_unique<ImportActionResponse>();
    ImportActionResponse &response = *responsePtr.get();
    ModuleRequestHandler::SetBaseResponse(request, response);
    response.command = Protocol::REQ_RES_IMPORT_ACTION;
    response.moduleName = MODULE_TIMELINE;
    response.body.isIpynb = true;
    response.body.reset = true;
    response.body.subdirectoryList.push_back(fileName);
    ModuleRequestHandler::SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
}

void ParserIpynb::JupyterProcess(const std::string &file)
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

void ParserIpynb::SendJupyterInfo(std::string url)
{
    Server::ServerLog::Info("Parse jupyter file end, send event");
    Server::WsSession *session = Server::WsSessionManager::Instance().GetSession();
    if (session == nullptr) {
        Server::ServerLog::Warn("Failed to get session");
        return;
    }
    auto event = std::make_unique<ParseJupyterCompletedEvent>();
    event->moduleName = MODULE_JUPYTER;
    event->result = true;
    if (url.empty()) {
        event->body.parseResult = PARSE_RESULT_FAIL;
    } else {
        event->body.parseResult = PARSE_RESULT_OK;
    }
    event->body.url = std::move(url);
    session->OnEvent(std::move(event));
}

ProjectTypeEnum ParserIpynb::GetProjectType(const std::vector<std::string> &dataPath)
{
    return ProjectTypeEnum::IPYNB;
}
}
}