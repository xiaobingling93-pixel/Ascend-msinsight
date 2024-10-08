/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <algorithm>
#include "ParserFactory.h"
#include "ParserBin.h"
#include "ParserJson.h"
#include "ParserIpynb.h"
#include "ParserDb.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "ClusterParseThreadPoolExecutor.h"
#include "TraceTime.h"
#include "TraceFileParser.h"
#include "MemoryParse.h"
#include "FullDbParser.h"
#include "ProjectExplorerManager.h"
#include "ParserStatusManager.h"

namespace Dic {
namespace Module {
using namespace Dic;
using namespace Dic::Server;
using namespace Dic::Module::Timeline;

// 静态变量 锁初始化
std::mutex ParserFactory::mutex;
std::pair<std::string, ParserType> ParserFactory::GetImportType(const std::vector<std::string> &pathList)
{
    if (StringUtil::EndWith(pathList[0], computeBinSuffix)) {
        return std::make_pair(pathList[0], ParserType::BIN);
    }
    if (StringUtil::EndWith(pathList[0], ipynbSuffix)) {
        return std::make_pair(pathList[0], ParserType::IPYNB);
    }
    std::pair<std::string, ParserType> result;
    if (FileUtil::FindIfDbTypeByRegex(pathList[0], std::regex(traceViewReg), std::regex(DBReg))) {
        result = std::make_pair(pathList[0], ParserType::DB);
    } else {
        result = std::make_pair(pathList[0], ParserType::JSON);
    }
    return result;
}

std::shared_ptr<ParserAlloc> ParserFactory::ParserImport(ParserType allocType)
{
    std::shared_ptr<ParserAlloc> alloc;
    switch (allocType) {
        case ParserType::DB:
            alloc = std::make_shared<ParserDb>();
            break;
        case ParserType::BIN:
            alloc = std::make_shared<ParserBin>();
            break;
        case ParserType::JSON:
            alloc = std::make_shared<ParserJson>();
            break;
        case ParserType::IPYNB:
            alloc = std::make_shared<ParserIpynb>();
            break;
        default:
            alloc = std::make_shared<ParserJson>();
            break;
    }
    return alloc;
}

void ParserFactory::Reset()
{
    std::lock_guard<std::mutex> lock(mutex);
    TraceFileParser::Instance().Reset();
    Summary::KernelParse::Instance().Reset();
    Memory::MemoryParse::Instance().Reset();
    FullDb::FullDbParser::Instance().Reset();
}

void ParserAlloc::SetBaseActionOfResponse(ImportActionResponse &response, const std::string &rankId,
    const std::string &cardPath, std::vector<std::string> dataPath)
{
    Action action;
    action.cardName = rankId;
    action.rankId = rankId;
    action.result = true;
    // 路径信息，与rankId对应，用于页面上删除时，能够正确找到要删除的甬道信息（目前只有导入单卡数据需要这个信息）
    action.dataPathList = std::move(dataPath);
    // 将文件所在路径的三级目录名称作为rank的tooltip信息
    action.cardPath = "Directory: " + cardPath;
    response.body.result.emplace_back(action);
}

void ParserAlloc::ParseClusterEndProcess(std::string result)
{
    ServerLog::Info("Parse Cluster File end, send event");
    WsSession *session = WsSessionManager::Instance().GetSession();
    if (session == nullptr) {
        ServerLog::Warn("Failed to get session when parse cluster end");
        return;
    }
    auto event = std::make_unique<ParseClusterCompletedEvent>();
    event->moduleName = ModuleType::TIMELINE;
    event->result = true;
    event->body.parseResult = std::move(result);
    session->OnEvent(std::move(event));
}

void ParserAlloc::ParseEndCallBack(const std::string &fileId, bool result, const std::string &message)
{
    ServerLog::Info("Parse end, fileId:", fileId, ", result:", result);
    if (result) {
        SendParseSuccessEvent(fileId);
    } else {
        SendParseFailEvent(fileId, message);
    }
}

void ParserAlloc::ParseProgressCallBack(const std::string &fileId, uint64_t parsedSize, uint64_t totalSize,
    int progress)
{
    WsSession *session = WsSessionManager::Instance().GetSession();
    if (session == nullptr) {
        ServerLog::Warn("Failed to get session when parse progress end");
        return;
    }
    auto event = std::make_unique<ParseProgressEvent>();
    event->moduleName = ModuleType::TIMELINE;
    event->result = true;
    event->body.fileId = fileId;
    event->body.parsedSize = parsedSize;
    event->body.totalSize = totalSize;
    event->body.progress = progress;
    session->OnEvent(std::move(event));
}

void ParserAlloc::SendParseSuccessEvent(const std::string &fileId)
{
    WsSession *session = WsSessionManager::Instance().GetSession();
    if (session == nullptr) {
        ServerLog::Warn("Failed to get session when send parse success msg");
        return;
    }
    auto event = std::make_unique<ParseSuccessEvent>();
    event->moduleName = ModuleType::TIMELINE;
    event->result = true;
    event->body.unit.type = "card";
    event->body.unit.metadata.cardId = fileId;
    uint64_t min = UINT64_MAX;
    uint64_t max = 0;
    auto database = DataBaseManager::Instance().GetTraceDatabase(fileId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:", fileId);
        return;
    }
    database->QueryExtremumTimestamp(min, max);
    if (min == max && max == 0) {
        event->body.startTimeUpdated = false;
    } else {
        event->body.startTimeUpdated = true;
        TraceTime::Instance().UpdateTime(min, max);
        TraceTime::Instance().UpdateCardMinTime(fileId, min);
    }
    event->body.maxTimeStamp = TraceTime::Instance().GetDuration();
    event->body.offset = TraceTime::Instance().GetOffsetByFileId(fileId);
    SearchMetaData(fileId, event->body.unit.children);
    session->OnEvent(std::move(event));
}

void ParserAlloc::SendParseFailEvent(const std::string &fileId, const std::string &message)
{
    WsSession *session = WsSessionManager::Instance().GetSession();
    if (session == nullptr) {
        ServerLog::Warn("Failed to get session when send parse fail msg");
        return;
    }
    auto event = std::make_unique<ParseFailEvent>();
    event->moduleName = ModuleType::TIMELINE;
    event->result = true;
    event->body.rankId = fileId;
    event->body.error = message;
    session->OnEvent(std::move(event));
}

bool ParserAlloc::IsNeedReset(const ImportActionRequest &request)
{
    // 如果是切换项目，则必须重置
    if (request.params.projectAction == ProjectActionEnum::TRANSFER_PROJECT) {
        return true;
    }
    // 新增文件时，以下情况需要对当前导入内容进行重置：1.导入数据和原来数据有冲突；2.无冲突，但是当前选中项目与目标项目不一致；
    std::string curProjectName = request.projectName;
    if (request.params.isConflict || (!curProjectName.empty() && curProjectName != request.params.projectName)) {
        return true;
    }
    return false;
}

void ParserAlloc::SearchMetaData(const std::string &fileId, std::vector<std::unique_ptr<UnitTrack>> &metaData)
{
    auto database = DataBaseManager::Instance().GetTraceDatabase(fileId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:", fileId);
        return;
    }
    database->QueryUnitsMetadata(fileId, metaData);
}

std::string ParserAlloc::GetFileId(const std::string &filePath, const std::string &importPath)
{
    std::string fileId = FileUtil::GetRankIdFromFile(filePath);
    int i = 1;
    std::string result = fileId;
    std::string parentDir = FileUtil::GetParentPath(filePath);
    std::string name = FileUtil::GetFileName(filePath);
    while (DataBaseManager::Instance().HasFileId(DatabaseType::TRACE, result)) {
        auto database = DataBaseManager::Instance().GetTraceDatabase(result);
        if (database == nullptr) {
            continue;
        }
        std::string dir = FileUtil::GetParentPath(database->GetDbPath());
        if (RegexUtil::RegexMatch(name, R"(^msprof_slice_[0-9_]+\.json$)") && parentDir == dir) {
            return result;
        }
        result = fileId + "_" + std::to_string(++i);
    }
    std::string dbPath = FileUtil::GetDbPath(filePath, result);
    if (!DataBaseManager::Instance().CreatConnectionPool(result, dbPath)) {
        ServerLog::Error("Failed to create connection pool. fileId:", result);
        return "";
    }
    dataPathToDbMap[importPath].push_back(dbPath);
    return result;
}

bool ParserAlloc::CheckIsCluster(const std::string &filePath)
{
    std::vector<std::string> folders;
    std::vector<std::string> files;
    if (filePath.find(CLUSTER_ANALYSIS_OUTPUT) != std::string::npos) {
        ServerLog::Info("this folder is cluster_analysis_output, Check_Is_Cluster is true");
        return true;
    }
    if (!FileUtil::FindFolders(filePath, folders, files)) {
        ServerLog::Info("FindFolders is empty, Check_Is_Cluster is false");
        return false;
    }
    return std::any_of(folders.begin(), folders.end(),
        [](std::string &folder) { return folder == CLUSTER_ANALYSIS_OUTPUT; });
}

std::string ParserAlloc::GetDbPath(const std::string &filePath, const int index)
{
    std::string path(filePath);
    std::string suffix = DB_FILE_SUFFIX;
    if (index != 1) {
        suffix = "_" + std::to_string(index) + suffix;
    }
    auto pos = path.find_last_of('.');
    if (pos != std::string::npos) {
        path.replace(pos, path.size() - pos, suffix);
    } else {
        path.append(suffix);
    }
    return path;
}

void ParserAlloc::SendAllParseSuccess()
{
    std::string notFinishTask = "";
    while (!ParserStatusManager::Instance().IsAllFinished(notFinishTask)) {
        const int sleepTime = 2000;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
    }
    ServerLog::Info("Send all parse finished");
    WsSession *session = WsSessionManager::Instance().GetSession();
    if (session == nullptr) {
        ServerLog::Warn("Failed to get session");
        return;
    }
    auto event = std::make_unique<AllSuccessEvent>();
    event->moduleName = ModuleType::MEMORY;
    event->result = true;
    event->body.isAllPageParsed = true;
    for (const auto &item : TraceTime::Instance().ComputeCardMinTimeInfo()) {
        CardOffset cardOffset = { item.first, item.second };
        event->body.cardOffsets.emplace_back(cardOffset);
    }
    event->body.minTime = TraceTime::Instance().GetStartTime();
    session->OnEvent(std::move(event));
}

void ParserAlloc::SaveDbPath(const std::string &curProjectName,
    std::map<std::string, std::vector<std::string>> &dataPathToDbMap)
{
    Global::ProjectExplorerManager::Instance().UpdateProjectDbPath(curProjectName, dataPathToDbMap);
}
} // Module
} // Dic
