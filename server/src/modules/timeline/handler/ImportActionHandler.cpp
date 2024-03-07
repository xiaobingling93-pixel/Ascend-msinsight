/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ImportActionHandler.h"
#include "ServerLog.h"
#include "ExecUtil.h"
#include "FileDef.h"
#include "FileUtil.h"
#include "RegexUtil.h"
#include "WsSessionManager.h"
#include "ParserStatusManager.h"
#include "OperatorProtocolEvent.h"
#include "CommonDefs.h"

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
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(token),
                        ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<ImportActionResponse> responsePtr = std::make_unique<ImportActionResponse>();
    ImportActionResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    if (request.params.path.empty()) {
        ServerLog::Warn("Import path is empty.");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }

    std::pair<std::string, ParserType> pathType = GetImportType(request.params.path);
    if (pathType.first.empty()) {
        ServerLog::Warn("Import files is empty.");
        SetResponseResult(response, true);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    ParserType allocType = pathType.second;
    std::shared_ptr<ParserAlloc> factory = ParserFactory::ParserImport(allocType);
    factory->Parser(pathType.first, request);
}


std::pair<std::string, ParserType> ImportActionHandler::GetImportType(const std::vector<std::string> &pathList)
{
    std::pair<std::string, ParserType> result;
    auto dbFiles = FileUtil::FindFilesByRegex(pathList[0], std::regex(DBReg));
    auto traceFiles = FileUtil::FindFilesByRegex(pathList[0], std::regex(traceViewReg));
    auto clusterPath = FileUtil::FindFilesAndFoldersByRegex(pathList[0], std::regex(clusterReg), true);
    if (!dbFiles.empty()) {
        result = std::make_pair(pathList[0], ParserType::DB);
    } else if (!traceFiles.empty() or !clusterPath.empty()) {
        result = std::make_pair(pathList[0], ParserType::JSON);
    } else if (!FileUtil::IsFolder(pathList[0])) {
        result = std::make_pair(pathList[0], ParserType::BIN);
    }
    return result;
}

} // Timeline
} // Module
} // Dic