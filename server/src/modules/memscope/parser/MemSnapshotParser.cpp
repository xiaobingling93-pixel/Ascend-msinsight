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
#include "MemScopeProtocolEvent.h"
#include "TimelineProtocolEvent.h"
#include "MemSnapshotParser.h"


#ifdef _WIN32
#include <windows.h>
#define SLEEP(ms) Sleep(ms)
#else
#define SLEEP(ms) std::this_thread::sleep_for(std::chrono::milliseconds(ms))
#endif

namespace Dic::Module {
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
    _threadPool->AddTask(ParseMemSnapshotTask, traceId);
    _threadPool->AddTask(ParseDaemonTask, traceId);
}

MemSnapshotParserContext& MemSnapshotParser::GetParseContext() { return parseContext; }

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
        if (DoubleCheckSuccessInLogFile(file)) {
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
        auto event = std::make_unique<ParseFailEvent>();
        event->moduleName = Protocol::MODULE_TIMELINE;
        event->result = false;
        event->body.rankId = filepath;
        event->body.error = error;
        event->body.dbPath = filepath;
        SendEvent(std::move(event));
        return;
    }
    auto event = std::make_unique<Protocol::MemScopeParseSuccessEvent>();
    event->moduleName = Protocol::MODULE_MEM_SCOPE;
    event->result = true;
    Protocol::MemScopeParseSuccessEventBody body;
    body.fileId = Instance().parseContext.GetOutputDbPath();
    event->body = body;
    SendEvent(std::move(event));
}
}
