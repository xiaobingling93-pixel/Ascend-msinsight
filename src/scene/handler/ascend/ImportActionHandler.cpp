/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ImportActionHandler.h"
#include "ServerLog.h"
#include "ExecUtil.h"
#include "FileUtil.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceFileParser.h"

namespace Dic {
namespace Scene {
using namespace Dic;
using namespace Dic::Server;
using namespace Dic::Scene::Core;
void ImportActionHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr) {
    ImportActionRequest &request = dynamic_cast<ImportActionRequest &>(*requestPtr.get());
    std::string token = request.token;
    ServerLog::Info("Import action, token = ", StringUtil::AnonymousString(token));
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(token),
                        ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<ImportActionResponse> responsePtr = std::make_unique<ImportActionResponse>();
    ImportActionResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    std::string path = request.params.path;
    if (path.empty() || path == "browser") {
        path = ExecUtil::SelectFolder();
    }
    auto traceFiles = FindTraceFile(path);
    if (traceFiles.empty()) {
        ServerLog::Error("Failed to find trace file, path = ", path);
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    SetParseCallBack(token);
    for (const auto &file : traceFiles) {
        std::string fileId = GetFileId(file);
        if (DataBaseManager::Instance().HasFileId(fileId)) {
            continue;
        }
        Action action{fileId, fileId, true};
        action.result = TraceFileParser::Instance().Parse(file, fileId);
        response.body.result.emplace_back(action);
    }
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

void ImportActionHandler::ParseEndCallBack(const std::string token, const std::string fileId, bool result)
{
    ServerLog::Info("Parse end, token = ", StringUtil::AnonymousString(token), " fileId:", fileId, ", result:", result);
    auto event = std::make_unique<ParseSuccessEvent>();
    event->body.uint.type = "card";
    event->body.uint.metadata.cardId = fileId;
    std::vector<std::unique_ptr<UintTrack>> metaData;
    SearchMetaData(fileId, metaData);
    WsSessionManager::Instance().OnParseSuccessEvent(token, *event);
}

std::vector<std::string> ImportActionHandler::FindTraceFile(const std::string &path)
{
    std::vector<std::string> traceFiles;
    std::function<void(const std::string&, int)> find = [&find, this, &traceFiles](const std::string &path, int depth) {
        if (depth > 5) {
            return;
        }
        auto folders = FileUtil::FindFolders(path);
        for (const auto &folder : folders) {
            std::string tmpPath = FileUtil::SplicePath(path, folder);
            if (FileUtil::IsFolder(tmpPath)) {
                find(tmpPath, depth + 1);
            } else if (folder == traceFileName) {
                traceFiles.push_back(tmpPath);
            }
        }
    };
    find(path, 0);
    return traceFiles;
}

void ImportActionHandler::SetParseCallBack(const std::string &token)
{
    std::function<void(const std::string, bool)> func =
        std::bind(ParseEndCallBack, token, std::placeholders::_1, std::placeholders::_2);
    TraceFileParser::Instance().SetParseEndCallBack(func);
}

std::string ImportActionHandler::GetFileId(const std::string &path)
{
    static int i = 0;
    return std::to_string(i++);
}

void ImportActionHandler::SearchMetaData(const std::string &fileId, std::vector<std::unique_ptr<UintTrack>> &metaData)
{

}
} // Dic
} // Scene