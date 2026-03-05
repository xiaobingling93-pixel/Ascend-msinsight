/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

#include "FileUtil.h"
#include "DataBaseManager.h"
#include "MemSnapshotParser.h"


#ifdef _WIN32
#include <windows.h>
#define SLEEP(ms) Sleep(ms)
#else
#define SLEEP(ms) std::this_thread::sleep_for(std::chrono::milliseconds(ms))
#endif

namespace Dic::Module {
using namespace Dic::Module::Timeline;
void MemSnapshotParserContext::Reset(std::string nPicklePath,
                                     std::string nLogPath,
                                     std::string nOutputPath)
{
    std::unique_lock<std::shared_mutex> lock(_mutex);
    picklePath = std::move(nPicklePath);
    logPath = std::move(nLogPath);
    outputDbPath = std::move(nOutputPath);
    state = ParserState::INIT;
    progress = 0;
    workDir = FileUtil::GetCurrPath();
}

bool MemSnapshotParserContext::IsFinished() const
{
    std::shared_lock<std::shared_mutex> lock(_mutex);
    return state == ParserState::FINISH_FAILURE || state == ParserState::FINISH_SUCCESS;
}

std::string MemSnapshotParserContext::GetPicklePath() const { return picklePath; }

std::string MemSnapshotParserContext::GetLogPath() const { return logPath; }

std::string MemSnapshotParserContext::GetOutputDbPath() const { return outputDbPath; }

ParserState MemSnapshotParserContext::GetState() const
{
    std::shared_lock<std::shared_mutex> lock(_mutex);
    return state;
}

void MemSnapshotParserContext::SetState(const ParserState newState)
{
    std::unique_lock<std::shared_mutex> lock(_mutex);
    state = newState;
    Server::ServerLog::Info("Snapshot pickle file parse state changed: ", static_cast<int>(state));
}

uint8_t MemSnapshotParserContext::GetProgress() const
{
    std::shared_lock<std::shared_mutex> lock(_mutex);
    return progress;
}

void MemSnapshotParserContext::SetProgress(uint8_t newProgress)
{
    std::unique_lock<std::shared_mutex> lock(_mutex);
    progress = newProgress;
    Server::ServerLog::Info("Snapshot pickle file parse progress changed: ", static_cast<int>(progress));
}

std::string MemSnapshotParserContext::GetWorkDir() const { return workDir; }

MemSnapshotParser& MemSnapshotParser::Instance()
{
    static MemSnapshotParser _instance;
    return _instance;
}

void MemSnapshotParser::Reset()
{
    Server::ServerLog::Info("[Snapshot] Parser Reset.");
    _threadPool->Reset();
    parseContext.Reset();
}

void MemSnapshotParser::AsyncParseMemSnapshotPickle(const std::string& pickleFilePath)
{
    const std::string filename = FileUtil::GetFileName(pickleFilePath);
    const auto timeT = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream timeStr;
    timeStr << std::put_time(std::localtime(&timeT), "%Y_%m_%d_%H_%M_%S");
    const std::string outputDbPath = StringUtil::StrJoin(pickleFilePath, ".db");
    const std::string logName = StringUtil::FormatString("{}_{}.log", FileUtil::StemFile(filename), timeStr.str());
    const std::string logPath = FileUtil::SplicePath(FileUtil::GetParentPath(pickleFilePath), logName);
    parseContext.Reset(pickleFilePath, logPath, outputDbPath);
    auto traceId = TraceIdManager::GenerateTraceId();
    if (CheckIfParsingNeed()) {
        Server::ServerLog::Info("[Snapshot] Parsing pickle file: {}, log file: {}, output db file: {}.",
            parseContext.GetPicklePath(), parseContext.GetLogPath(), parseContext.GetOutputDbPath());
        _threadPool->AddTask(ParseMemSnapshotTask, traceId);
        _threadPool->AddTask(ParseDaemonTask, traceId);
        return;
    }
    // 不需要重新解析的场景，可以直接发送解析成功事件，且在上述CheckIf方法中已经打开db并纳管到了databaseManager中
    auto event = Instance().BuildParseSuccessEventFromContext();
    SendEvent(std::move(event));
}

MemSnapshotParserContext& MemSnapshotParser::GetParseContext() { return parseContext; }


/***
 * @brief 检查是否需要解析或重新解析pickle文件
 * @return true 需要解析或重新解析
 * @return false 不需要解析或重新解析
 */
bool MemSnapshotParser::CheckIfParsingNeed() const
{
    const auto& context = parseContext;
    // 首先判断是否存在解析结果db文件，如果不存在则返回需要重新解析
    if (!FileUtil::CheckFileValid(context.GetOutputDbPath())) {
        Server::ServerLog::Info("[Snapshot] Parsing output db file cannot found or is not valid. Trying to re-parse.");
        return true;
    }
    // 如果已经存在db，则检查db版本是否与当前版本匹配
    auto snapshotDb = DataBaseManager::Instance().GetMemSnapshotDatabase(context.GetPicklePath());
    if (snapshotDb == nullptr) {
        Server::ServerLog::Warn("[Snapshot] Cannot get database connection by fileId: %, trying to re-parse.",
            context.GetPicklePath());
        return true;
    }
    // 可能为首次打开db
    if (!snapshotDb->IsOpen() && !snapshotDb->OpenDbReadOnly(context.GetOutputDbPath())) {
        Server::ServerLog::Warn("[Snapshot] Cannot open database file: %, trying to re-parse.", context.GetOutputDbPath());
        return true;
    }
    return snapshotDb->IsDatabaseVersionChange();
}

MemSnapshotParser::MemSnapshotParser()
{
    // MemSnapshot解析snapshot解析至少需要双线程
    _threadPool = std::make_unique<ThreadPool>(2);
}

MemSnapshotParser::~MemSnapshotParser() { _threadPool->ShutDown(); }

void MemSnapshotParser::ParseMemSnapshotTask()
{
    Server::ServerLog::Info("[Snapshot] Parse snapshot thread started.");
    std::string cmd;
#ifdef __linux__
    // linux场景直接通过python3执行
    const std::string basicScriptsDir = FileUtil::GetParentPath(Instance().parseContext.GetWorkDir());
    const std::string memSnapDumpScriptsPath = FileUtil::SplicePath(basicScriptsDir, "scripts",
        "MemSnapDump", "tools", "dump2db.py");
    const std::string executable = StringUtil::StrJoin("python3 ", memSnapDumpScriptsPath);
#else
    #ifdef _WIN32
    // windows场景需要切换到对应盘符
    const std::string switchDriverLetter = "cd \"" + Instance().parseContext.GetWorkDir().substr(0, INT_TWO) + "\"";
    const std::string dump2db = FileUtil::SplicePath(Instance().parseContext.GetWorkDir(), "dump2db.exe");
    // 从工作路径中切取盘符
    const std::string executable = StringUtil::FormatString("{} && \"{}\" ", switchDriverLetter, dump2db);
    #else
    // Mac场景直接绝对路径使用
    const std::string executable = "\"" + FileUtil::SplicePath(Instance().parseContext.GetWorkDir(), "dump2db") + "\"";
    #endif

#endif
    cmd = StringUtil::StrJoin(executable, " \"", Instance().parseContext.GetPicklePath(), "\" > \"",
                              Instance().parseContext.GetLogPath(), "\" 2>&1");
    Server::ServerLog::Info("[Snapshot] Start parsing.Cmd = ", cmd);
    Instance().parseContext.SetState(ParserState::Processing);
    try {
        int result = std::system(cmd.c_str());
        Server::ServerLog::Info("[Snapshot] Parsing finished.result = ", result);
        Instance().parseContext.SetState(result == 0 ? ParserState::FINISH_SUCCESS : ParserState::FINISH_FAILURE);
    } catch (...) {
        Server::ServerLog::Error("[Snapshot] Parsing finished.result = UNKNOWN_ERROR");
        Instance().parseContext.SetState(ParserState::FINISH_FAILURE);
    }
}

int ReadProgressInLogFile(std::ifstream& file, std::string& err)
{
    // 读取最新进展
    const std::regex progressReg(R"((\d+(?:\.\d+)?)% of entries have been processed)");
    const std::string parseFailedKeyWord = "Failed to dump the snapshot to database.";
    std::string line;
    std::string lastProgressLine;
    while (std::getline(file, line)) {
        if (line.empty()) { continue; }
        if (StringUtil::Contains(line, parseFailedKeyWord)) {
            err = line;
            return -1;
        }
        if (std::regex_search(line, progressReg)) { lastProgressLine = line; }
    }
    // 如果有新内容
    if (!lastProgressLine.empty()) {
        std::smatch match;
        if (std::regex_search(lastProgressLine, match, progressReg) && match.size() == 1) {
            return NumberUtil::StringToInt(match[1].str());
        }
    }
    return 0;
}

bool DoubleCheckSuccessInLogFile(std::ifstream& file)
{
    // 最后读取一次successfully关键字 进行二次确认
    const std::string successKeyword = "successfully";
    if (!file.is_open()) {
        Server::ServerLog::Warn("An exception occurred while re-verifying the parsing results; the output log "
                                "file % could not be opened.",
                                MemSnapshotParser::Instance().GetParseContext().GetLogPath());
        return false;
    }
    bool successKeywordFound = false;
    std::string line;
    while (std::getline(file, line)) {
        if (line.find(successKeyword) != std::string::npos) {
            successKeywordFound = true;
            break;
        }
    }
    if (!successKeywordFound) {
        Server::ServerLog::Warn(
            "An exception occurred while re-verifying the parsing results: "
            "the success keyword \"%\" was not found in the output log. Please check the log file %.",
            successKeyword, MemSnapshotParser::Instance().GetParseContext().GetLogPath());
        return false;
    }
    return true;
}

void MemSnapshotParser::ParseDaemonTask()
{
    Server::ServerLog::Info("[Snapshot] Daemon thread started.");
    const std::regex progressReg(R"((\d+(?:\.\d+)?)% of entries have been processed)");
    int checkIntervalMs = 100;
    while (!Instance().parseContext.IsFinished()) {
        std::ifstream file(Instance().parseContext.GetLogPath());
        if (!file.is_open()) {
            // 解析线程可能并未及时创建出日志文件，因此需要等待
            SLEEP(500);
            continue;
        }
        std::string error = "";
        auto newProgress = ReadProgressInLogFile(file, error);
        if (newProgress < 0 and !error.empty()) {
            Server::ServerLog::Error("Parsing failure information was detected while reading the process logs, and "
                "the daemon has exited.");
            Instance().parseContext.SetState(ParserState::FINISH_FAILURE);
            break;
        }
        Instance().parseContext.SetProgress(newProgress);
        SLEEP(checkIntervalMs); // 每1s检查一次
        checkIntervalMs = std::min(checkIntervalMs * 2, 1000);
    }
    if (Instance().parseContext.GetState() == ParserState::FINISH_SUCCESS) {
        std::ifstream file(Instance().parseContext.GetLogPath());
        if (DoubleCheckSuccessInLogFile(file) && Instance().TryOpenParsingResultDbAndSetVersion()) {
            Server::ServerLog::Info("Parse thread has successfully finished with double check.");
        }
        else {
            Server::ServerLog::Warn("The parsing thread returned a success response, but we did not find a "
                "corresponding success event confirmed in the logs.");
        }
    }
    ParseCallBack();
}

void MemSnapshotParser::ParseCallBack()
{
    const std::string filepath = Instance().parseContext.GetPicklePath();
    if (Instance().parseContext.GetState() != ParserState::FINISH_SUCCESS) {
        const std::string error = StringUtil::FormatString("Failed to parse snapshot data. For details, please check "
                                                           "{}.", Instance().parseContext.GetLogPath());
        Server::ServerLog::Error(error);
        auto event = Instance().BuildParseFailEventFromContext(error);
        SendEvent(std::move(event));
        return;
    }
    auto event = Instance().BuildParseSuccessEventFromContext();
    SendEvent(std::move(event));
}

bool MemSnapshotParser::TryOpenParsingResultDbAndSetVersion() const
{
    const std::string dbPath = parseContext.GetOutputDbPath();
    if (!FileUtil::CheckFileValid(dbPath)) {
        Server::ServerLog::Error("[Snapshot] Double Check failed to verify the validity of the result database and "
                                 "establish a connection.");
        return false;
    }
    // 从DatabaseManager中获取MemSnapshotDatabase时，应使用原文件路径作为fileId而不是解析后的Db路径
    const auto snapshotDb = DataBaseManager::Instance().GetMemSnapshotDatabase(parseContext.GetPicklePath());
    if (!snapshotDb) {
        Server::ServerLog::Error("[Snapshot] Double Check failed to get the snapshot database.");
        return false;
    }
    if (!snapshotDb->IsOpen() && !snapshotDb->OpenDbReadOnly(dbPath)) {
        Server::ServerLog::Warn("[Snapshot] Double Check failed to open database file: %.", dbPath);
        return false;
    }
    // 缺省目标版本将设置为当前profiler_server编译时间
    return snapshotDb->SetDataBaseVersion();
}

std::unique_ptr<MemScopeParseSuccessEvent> MemSnapshotParser::BuildParseSuccessEventFromContext() const
{
    auto event = std::make_unique<Protocol::MemScopeParseSuccessEvent>();
    event->moduleName = Protocol::MODULE_MEM_SCOPE; // moduleName设置为memscope以复用MemScope的事件/请求/响应路由
    event->result = true;
    Protocol::MemScopeParseSuccessEventBody body;
    body.fileId = parseContext.GetPicklePath();
    body.deviceIds["0"] = {"BLOCK"};
    body.module = Protocol::MODULE_MEM_SNAPSHOT; // body.module设置为真实数据类型以适配前端区分模块类型
    event->body = body;
    return event;
}

std::unique_ptr<ParseFailEvent> MemSnapshotParser::BuildParseFailEventFromContext(const std::string& errMsg) const
{
    auto event = std::make_unique<ParseFailEvent>();
    event->moduleName = Protocol::MODULE_TIMELINE;
    event->result = false;
    event->body.rankId = parseContext.GetPicklePath();
    event->body.error = errMsg;
    event->body.dbPath = parseContext.GetOutputDbPath();
    return event;
}
} // namespace Dic::Module
