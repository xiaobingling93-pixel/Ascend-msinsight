// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.

#include "ServerLog.h"
#include "LogUtil.h"

namespace Dic::Server {
static std::unique_ptr<LogUtil> printInstance = nullptr;
static std::unique_ptr<LogUtil> recordInstance = nullptr;
std::string Dic::Server::ServerLog::currentLogPath = "";
#ifdef _WIN32
const std::string_view PATH_SLASH = "\\";
#else
const std::string_view PATH_SLASH = "/";
#endif

void ServerLog::Initialize(const std::string &logPath, const int &logSize, const std::string &logLevelStr,
                           std::string wsPortStr)
{
    ServerLog &serverLog = Instance();
    serverLog.wsPort = wsPortStr;
    if (printInstance == nullptr) {
        printInstance = std::make_unique<LogUtil>(LogOutType::TERMINAL, "", wsPortStr);
    }
    if (recordInstance == nullptr) {
        std::vector<std::string> paths = {logPath, "profiler_server.log"};
        std::string logFileName = StringUtil::join(paths, PATH_SLASH);
        LogLevel logLevel = LogUtil::GetLogLevel(logLevelStr);
        recordInstance = std::make_unique<LogUtil>(LogOutType::FILE, logFileName, wsPortStr, logSize);
        recordInstance->SetLogLevel(logLevel);
    }
}
void ServerLog::Initialize(const LogLevel &level)
{
    if (recordInstance == nullptr) {
        std::vector<std::string> paths = {".", "profiler_server.log"};
        std::string logPath = StringUtil::join(paths, PATH_SLASH);
        const int logSize = 32 * 1024 * 1024;
        auto instance = std::make_unique<LogUtil>(LogOutType::FILE, logPath, wsPort);
        instance->SetLogLevel(level).SetMaxSize(logSize);
        recordInstance = std::move(instance);
    }
    if (printInstance == nullptr) {
        printInstance = std::make_unique<LogUtil>(LogOutType::TERMINAL, "", wsPort);
    }
}
const std::string ServerLog::GetLogHead(const LogLevel &level)
{
    std::string head;
    head.append(LogPrefix::Instance().TimePrefix()).append("|")
        .append(LogPrefix::Instance().LevelPrefix(level)).append(" ");
    return head;
}
void ServerLog::RecordImpl(const LogLevel &level, std::vector<std::string> &logStrList)
{
    recordInstance->LogT(level, GetLogHead(level), logStrList);
    ServerLog::currentLogPath = recordInstance->GetLogFilePath();
}

void ServerLog::PrintImpl(const LogLevel &level, std::vector<std::string> &logStrList)
{
    printInstance->LogT(level, GetLogHead(level), logStrList);
}
ServerLog &ServerLog::Instance()
{
    static ServerLog instance;
    return instance;
}
}