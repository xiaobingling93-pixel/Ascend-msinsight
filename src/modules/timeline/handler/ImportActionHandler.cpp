/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ImportActionHandler.h"
#include "ServerLog.h"
#include "ExecUtil.h"
#include "FileUtil.h"
#include "RegexUtil.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "TraceFileParser.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic;
using namespace Dic::Server;
void ImportActionHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
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
    if (request.params.path.empty()) {
        ServerLog::Warn("Import path is empty.");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    auto traceFiles = FindAllTraceFile(request.params.path);
    if (traceFiles.empty()) {
        ServerLog::Error("Failed to find trace file.");
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    SetParseCallBack(token);
    std::vector<std::pair<std::string, std::string>> files;
    for (const auto &file : traceFiles) {
        std::string fileId = TraceFileParser::Instance().GetFileId(file);
        if (DataBaseManager::Instance().HasFileId(fileId)) {
            continue;
        }
        response.body.result.emplace_back(Action{fileId, fileId, true});
        files.emplace_back(std::make_pair(file, fileId));
    }
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    // 先回复消息，再解析，小文件解析可能比回复消息还快
    for (const auto &file : files) {
        TraceFileParser::Instance().Parse(file.first, file.second);
    }
}

void ImportActionHandler::ParseEndCallBack(const std::string token, const std::string fileId, bool result)
{
    ServerLog::Info("Parse end, token = ", StringUtil::AnonymousString(token), " fileId:", fileId, ", result:", result);
    WsSession *session = WsSessionManager::Instance().GetSession(token);
    if (session == nullptr) {
        ServerLog::Warn("Failed to get session, token = ", StringUtil::AnonymousString(token));
        return;
    }
    auto event = std::make_unique<ParseSuccessEvent>();
    event->moduleName = ModuleType::TIMELINE;
    event->token = token;
    event->result = result;
    event->body.unit.type = "card";
    event->body.unit.metadata.cardId = fileId;
    uint64_t min = UINT64_MAX;
    uint64_t max = 0;
    DataBaseManager::Instance().GetTraceDatabase(fileId)->QueryExtremumTimestamp(min, max);
    if (min == max && max == 0) {
        event->body.startTimeUpdated = false;
    } else {
        event->body.startTimeUpdated = true;
        TraceTime::Instance().UpdateTime(min, max);
    }
    event->body.maxTimeStamp = TraceTime::Instance().GetDuration();
    SearchMetaData(fileId, event->body.unit.children);
    session->OnEvent(std::move(event));
}

std::vector<std::string> ImportActionHandler::FindAllTraceFile(const std::vector<std::string> &pathList)
{
    std::vector<std::string> traceFiles;
    for (const auto &path : pathList) {
        if (path == "browser") {
            return FindTraceFile(ExecUtil::SelectFolder());
        }
        auto files = FindTraceFile(path);
        if (files.empty()) {
            ServerLog::Warn("Can't find trace file in path:", path);
            continue;
        }
        traceFiles.insert(traceFiles.end(), files.begin(), files.end());
    }
    return traceFiles;
}

std::vector<std::string> ImportActionHandler::FindTraceFile(const std::string &path)
{
    std::vector<std::string> traceFiles;
    if (!FileUtil::IsFolder(path)) {
        traceFiles.emplace_back(path);
        return traceFiles;
    }
    std::function<void(const std::string&, int)> find = [&find, this, &traceFiles](const std::string &path, int depth) {
        if (depth > 5) {
            return;
        }
        auto folders = FileUtil::FindFolders(path);
        if (std::find(folders.begin(), folders.end(), "ASCEND_PROFILER_OUTPUT") != folders.end()) {
            FindAscendFolder(path, traceFiles);
            return;
        }
        for (const auto &folder : folders) {
            std::string tmpPath = FileUtil::SplicePath(path, folder);
            if (FileUtil::IsFolder(tmpPath)) {
                find(tmpPath, depth + 1);
            } else if (IsJsonValid(folder)) {
                traceFiles.push_back(tmpPath);
            }
        }
    };
    find(path, 0);
    return traceFiles;
}

bool ImportActionHandler::IsJsonValid(const std::string &fileName)
{
    static std::string reg = R"((trace_view|msprof.*)\.json$)";
    auto result = RegexUtil::RegexMatch(fileName, reg);
    return result.has_value();
}

void ImportActionHandler::FindAscendFolder(const std::string &path, std::vector<std::string> &traceFiles)
{
    std::string traceFilePath = FileUtil::SplicePath(path, "ASCEND_PROFILER_OUTPUT");
    traceFilePath = FileUtil::SplicePath(traceFilePath, "trace_view.json");
    ServerLog::Info("FindAscendFolder. ", traceFilePath);
    if (FileUtil::CheckDirectoryExist(traceFilePath)) {
        traceFiles.emplace_back(traceFilePath);
        ServerLog::Info("FindAscendFolder2. ");
        return;
    }
    std::function<void(const std::string&, int)> find = [&find, this, &traceFiles](const std::string &path, int depth) {
        if (depth > 5) {
            return;
        }
        auto folders = FileUtil::FindFolders(path);
        for (const auto &folder : folders) {
            std::string tmpPath = FileUtil::SplicePath(path, folder);
            if (FileUtil::IsFolder(tmpPath)) {
                find(tmpPath, depth + 1);
            } else if (IsJsonValid(folder)) {
                traceFiles.push_back(tmpPath);
            }
        }
    };
    auto folders = FileUtil::FindFolders(path);
    static std::string reg = R"(PROF_.*)";
    for (const auto &folder : folders) {
        if (!RegexUtil::RegexMatch(folder, reg).has_value()) {
            continue;
        }
        std::string tmpPath = FileUtil::SplicePath(path, folder);
        if (FileUtil::IsFolder(tmpPath)) {
            find(tmpPath, 0);
        }
        break;
    }
}

void ImportActionHandler::SetParseCallBack(const std::string &token)
{
    std::function<void(const std::string, bool)> func =
        std::bind(ParseEndCallBack, token, std::placeholders::_1, std::placeholders::_2);
    TraceFileParser::Instance().SetParseEndCallBack(func);
}

void ImportActionHandler::SearchMetaData(const std::string &fileId, std::vector<std::unique_ptr<UnitTrack>> &metaData)
{
    DataBaseManager::Instance().GetTraceDatabase(fileId)->QueryUnitsMetadata(fileId, metaData);
}

} // Timeline
} // Module
} // Dic