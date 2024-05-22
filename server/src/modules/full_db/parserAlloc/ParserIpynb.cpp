/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "ParserIpynb.h"

#include "JupyterServerManager.h"
#include "JupyterFileParser.h"
#include "WsSession.h"
#include "WsSessionManager.h"
#include "ServerLog.h"
#include "CmdUtil.h"
#include "ConstantDefs.h"
#include "JupyterProtocolEvent.h"


namespace Dic {
namespace Module {
using namespace Jupyter;
ParserIpynb::ParserIpynb() = default;

ParserIpynb::~ParserIpynb() = default;

void ParserIpynb::Parser(const std::string &path, ImportActionRequest &request)
{
    // 检查jupyter服务是否存在
    std::string jupyterVersionResult;
    if (CmdUtil::ExecuteCmdWithResult("jupyter-lab --version", jupyterVersionResult)
            && !jupyterVersionResult.empty()) {
        IpynbImportResponse(request);
        JupyterFileParser::Instance().GetThreadPool()->AddTask(JupyterProcess, request.token, path);
    } else {
        SendParseFailEvent(request.token, "", "Jupyter env is not ready.");
    }
}

void ParserIpynb::IpynbImportResponse(ImportActionRequest &request)
{
    std::string token = request.token;
    Server::WsSession &session = *Server::WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<ImportActionResponse> responsePtr = std::make_unique<ImportActionResponse>();
    ImportActionResponse &response = *responsePtr.get();
    ModuleRequestHandler::SetBaseResponse(request, response);
    response.command = Protocol::REQ_RES_IMPORT_ACTION;
    response.moduleName = Protocol::ModuleType::TIMELINE;
    response.body.isIpynb = true;
    response.body.reset = true;
    ModuleRequestHandler::SetResponseResult(response, true);
    session.OnResponse(std::move(responsePtr));
}

void ParserIpynb::JupyterProcess(const std::string &token, const std::string &file)
{
    std::string url;
    // 从根目录重置服务(如果当前启动目录无法打开文件，则在当前文件根目录下重启Jupyter服务)
    JupyterServerManager &server = JupyterServerManager::Instance();
    if (server.ResetUnderRootPath(file)) {
        // 重启成功，则获取url内容
        url = server.GetJupyterUrl(file);
    }
    SendJupyterInfo(token, url);
}

void ParserIpynb::SendJupyterInfo(const std::string &token, std::string url)
{
    Server::ServerLog::Info("Parse jupyter file end, send event");
    Server::WsSession *session = Server::WsSessionManager::Instance().GetSession(token);
    if (session == nullptr) {
        Server::ServerLog::Warn("Failed to get session token ");
        return;
    }
    auto event = std::make_unique<ParseJupyterCompletedEvent>();
    event->moduleName = ModuleType::JUPYTER;
    event->token = token;
    event->result = true;
    if (url.empty()) {
        event->body.parseResult = PARSE_RESULT_FAIL;
    } else {
        event->body.parseResult = PARSE_RESULT_OK;
    }
    event->body.url = std::move(url);
    session->OnEvent(std::move(event));
}
}
}